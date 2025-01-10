#include "tsr/API/GDALHandler.hpp"

#include "tsr/IO/MapIO.hpp"
#include "tsr/PointProcessor.hpp"
#include <cpl_error.h>
#include <cpl_port.h>
#include <cpl_progress.h>
#include <cpl_vsi.h>
#include <cstddef>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/gdal_utils.h>
#include <gdal_alg.h>
#include <gdalwarper.h>
#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <stdexcept>
#include <string>

#include "tsr/ChunkInfo.hpp"

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "tsr/DataFile.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/IO/ImageIO.hpp"

#include "tsr/Logging.hpp"
#include "tsr/PointProcessor.hpp"

#include <gdal/ogr_spatialref.h>
#include <vector>

namespace tsr::API {

GDALDatasetH parseGDALDatasetFromString(std::string data,
                                        std::string filepath) {

  CPLSetConfigOption("GDAL_CACHEMAX", "512"); // Limit cache size to 512 MB

  // Create temporary file
  std::ofstream file(filepath);
  if (!file) {
    TSR_LOG_ERROR("failed to open temporary file");
    throw std::runtime_error("failed to open temporary file");
  }

  TSR_LOG_TRACE("writing response to file");
  IO::write_data_to_file(filepath, data);

  file.close();

  TSR_LOG_TRACE("opening file");
  TSR_LOG_TRACE("file: {}", filepath);
  GDALDatasetH dataset;

  IO::load_gdal_dataset_from_file(filepath, dataset);

  if (!dataset) {
    TSR_LOG_ERROR("Could not parse gdal dataset from string");
    throw std::runtime_error("Could not parse gdal dataset from string");
  }

  // Return the filepath to the dataset
  return dataset;
}

std::vector<double> getDatasetCenter(GDALDataset *dataset) {
  if (dataset == nullptr) {
    TSR_LOG_ERROR("dataset null");
    throw std::runtime_error("dataset null");
  }

  double geoTransform[6];
  if (dataset->GetGeoTransform(geoTransform) != CE_None) {
    TSR_LOG_ERROR("could not get the GeoTransform of dataset");
    throw std::runtime_error("could not get the GeoTransform of dataset");
  }

  int xSize = dataset->GetRasterXSize();
  int ySize = dataset->GetRasterYSize();

  double centerX = xSize / 2.0;
  double centerY = ySize / 2.0;

  double centerLongitude =
      geoTransform[0] + centerX * geoTransform[1] + centerY * geoTransform[2];
  double centerLatitude =
      geoTransform[3] + centerX * geoTransform[4] + centerY * geoTransform[5];

  return {centerLatitude, centerLongitude};
}

GDALDatasetH warpVectorDatasetToUTM(GDALDatasetH hDataset,
                                    std::string filepath) {

  GDALDataset *dataset = static_cast<GDALDataset *>(hDataset);

  if (dataset == nullptr) {
    TSR_LOG_ERROR("cannot warp null GDAL dataset");
    throw std::runtime_error("cannot warp null GDAL dataset");
  }

  TSR_LOG_TRACE("preparing transform options");

  OGRSpatialReference sourceSRS;
  sourceSRS.SetWellKnownGeogCS("WGS84");

  OGRSpatialReference targetSRS;
  targetSRS.SetWellKnownGeogCS("WGS84");
  targetSRS.SetUTM(30, TRUE); // Zone 30, Northern Hemisphere

  OGRCoordinateTransformation *transform =
      OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS);

  if (!transform) {
    TSR_LOG_ERROR("failed to create vector transformation");

    throw std::runtime_error("failed to create vector transformation");
  }

  const char *translateArgs[6] = {
      "-t_srs", "+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs",
      "-f",     "GeoJSON",
      "lines",  nullptr};

  GDALVectorTranslateOptions *warpOptions =
      GDALVectorTranslateOptionsNew((char **)translateArgs, nullptr);

  TSR_LOG_TRACE("warping");
  GDALDatasetH warpedDataset;
  try {

    warpedDataset = GDALVectorTranslate(filepath.c_str(), nullptr, 1, &hDataset,
                                        warpOptions, nullptr);
  } catch (std::exception e) {
    TSR_LOG_TRACE("{}", e.what());
    TSR_LOG_ERROR("failed to warp dataset");
    GDALVectorTranslateOptionsFree(warpOptions);
    throw e;
  }

  GDALVectorTranslateOptionsFree(warpOptions);

  if (!warpedDataset) {
    TSR_LOG_TRACE("Failed to warp dataset");
    throw std::runtime_error("failed to warp dataset");
  }

  TSR_LOG_TRACE("warp success");

  return warpedDataset;
}

GDALDatasetH warpRasterDatasetToUTM(GDALDatasetH hDataset,
                                    std::string filepath) {

  GDALDataset *dataset = static_cast<GDALDataset *>(hDataset);

  if (!dataset) {
    TSR_LOG_ERROR("cannot warp null GDAL dataset");
    throw std::runtime_error("cannot warp null GDAL dataset");
  }

  const int bandCount = GDALGetRasterCount(dataset);
  if (bandCount == 0) {
    TSR_LOG_ERROR("Cannot warp dataset with 0 bands");
    throw std::runtime_error("cannot warp dataset with 0 bands");
  }

  GDALAllRegister();

  GDALWarpOptions *warpOptions = GDALCreateWarpOptions();

  const char *sourceWKT = GDALGetProjectionRef(dataset);

  // Define the UTM target SRS
  char *warpedWKT = nullptr;
  OGRSpatialReferenceH dstSRS = OSRNewSpatialReference(nullptr);
  OSRSetWellKnownGeogCS(dstSRS, "WGS84");
  OSRSetUTM(dstSRS, 30, TRUE); // Zone 30, Northern Hemisphere
  OSRExportToWkt(dstSRS, &warpedWKT);

  void *transformArg = GDALCreateGenImgProjTransformer(dataset, sourceWKT, NULL,
                                                       warpedWKT, FALSE, 0, 1);

  // Get approximate output georeferenced bounds and resolution for file.
  double warpedADFGeoTransform[6];
  int nPixels = 0, nLines = 0;
  CPLErr eErr;
  eErr = GDALSuggestedWarpOutput(dataset, GDALGenImgProjTransform, transformArg,
                                 warpedADFGeoTransform, &nPixels, &nLines);

  if (eErr != CE_None) {
    TSR_LOG_ERROR("Failed to suggest warp output");
    throw std::runtime_error("failed to suggest warp output");
  }

  GDALDriver *driver = (GDALDriver *)dataset->GetDriver();

  GDALDataType eDT = GDALGetRasterDataType(GDALGetRasterBand(dataset, 1));

  // Create temporary warped file
  GDALDatasetH warpedDataset = driver->Create(filepath.c_str(), nPixels, nLines,
                                              bandCount, eDT, nullptr);

  GDALSetProjection(warpedDataset, warpedWKT);
  GDALSetGeoTransform(warpedDataset, warpedADFGeoTransform);

  // Copy colour table
  GDALColorTableH colourTable;
  colourTable = GDALGetRasterColorTable(GDALGetRasterBand(dataset, 1));
  if (colourTable != NULL) {
    GDALSetRasterColorTable(GDALGetRasterBand(warpedDataset, 1), colourTable);
  }

  double NODATA_VALUE = -9999.00;

  warpOptions->hSrcDS = dataset;
  warpOptions->hDstDS = warpedDataset;

  warpOptions->nBandCount = bandCount;
  warpOptions->panSrcBands = (int *)CPLMalloc(sizeof(int) * bandCount);
  warpOptions->panDstBands = (int *)CPLMalloc(sizeof(int) * bandCount);
  warpOptions->pfnProgress = GDALTermProgress;
  warpOptions->padfDstNoDataReal =
      (double *)CPLMalloc(sizeof(double) * bandCount);
  warpOptions->papszWarpOptions =
      CSLSetNameValue(warpOptions->papszWarpOptions, "INIT_DEST", "NO_DATA");
  warpOptions->papszWarpOptions =
      CSLSetNameValue(warpOptions->papszWarpOptions, "DST_NODATA",
                      std::to_string(NODATA_VALUE).c_str());

  for (int i = 0; i < bandCount; ++i) {
    warpOptions->panSrcBands[i] = i + 1;
    warpOptions->panDstBands[i] = i + 1;
    warpOptions->padfDstNoDataReal[i] = NODATA_VALUE; // Assign no-data value
  }

  warpOptions->pTransformerArg = GDALCreateGenImgProjTransformer(
      dataset, sourceWKT, warpedDataset, warpedWKT, FALSE, 0.0, 1);
  warpOptions->pfnTransformer = GDALGenImgProjTransform;

  // Init warper
  GDALWarpOperation op;
  op.Initialize(warpOptions);
  op.ChunkAndWarpImage(0, 0, nPixels, nLines);

  GDALDestroyGenImgProjTransformer(warpOptions->pTransformerArg);
  GDALDestroyWarpOptions(warpOptions);
  OSRDestroySpatialReference(dstSRS);

  return warpedDataset;
}

GDALDatasetH rasterizeDataset(const GDALDatasetH &source_dataset,
                              std::string filepath, const ChunkInfo &chunk,
                              double pixel_resolution) {

  TSR_LOG_TRACE("preparing rasterization");

  GDALDataset *dataset = static_cast<GDALDataset *>(source_dataset);

  if (!dataset) {
    TSR_LOG_ERROR("dataset null for rasterization");
    throw std::runtime_error("dataset null for rasterization");
  }

  OGRLayer *layer = dataset->GetLayer(0);
  if (!layer) {
    TSR_LOG_ERROR("dataset has no layers for rasterization");
    throw std::runtime_error("dataset has no layers for rasterization");
  }

  char **options = NULL;

  std::string pixelResolutionString = std::to_string(pixel_resolution);
  std::string minLat = std::to_string(chunk.minLat);
  std::string maxLat = std::to_string(chunk.maxLat);
  std::string minLng = std::to_string(chunk.minLng);
  std::string maxLng = std::to_string(chunk.maxLng);

  options = CSLAddString(options, "-init");
  options = CSLAddString(options, "0");
  options = CSLAddString(options, "-burn");
  options = CSLAddString(options, "255");
  options = CSLAddString(options, "-ot");
  options = CSLAddString(options, "Int16");
  options = CSLAddString(options, "-tr");
  options = CSLAddString(options, pixelResolutionString.c_str());
  options = CSLAddString(options, pixelResolutionString.c_str());
  options = CSLAddString(options, "-a_nodata");
  options = CSLAddString(options, "0");
  options = CSLAddString(options, "-of");
  options = CSLAddString(options, "GTiff");
  options = CSLAddString(options, "-co");
  options = CSLAddString(options, "PROFILE=GeoTIFF");
  options = CSLAddString(options, "-l");
  options = CSLAddString(options, "multipolygons");
  // options = CSLAddString(options, "-l");
  // options = CSLAddString(options, "multipolygon");
  options = CSLAddString(options, "-te");
  options = CSLAddString(options, minLng.c_str());
  options = CSLAddString(options, minLat.c_str());
  options = CSLAddString(options, maxLng.c_str());
  options = CSLAddString(options, maxLat.c_str());

  GDALRasterizeOptions *rasterizeOptions =
      GDALRasterizeOptionsNew(options, NULL);

  TSR_LOG_TRACE("Rasterizing");

  int usageError;
  GDALDatasetH outputDataset = GDALRasterize(
      filepath.c_str(), NULL, source_dataset, rasterizeOptions, &usageError);

  CSLDestroy(options);
  GDALRasterizeOptionsFree(rasterizeOptions);

  if (!outputDataset) {
    TSR_LOG_ERROR("Rasterization failed");
    throw std::runtime_error("rasterization failed");
  }

  if (usageError != CE_None) {
    TSR_LOG_ERROR("Rasterization failed");
    throw std::runtime_error("rasterization failed");
  }

  TSR_LOG_TRACE("rasterization complete");
  return outputDataset;
}

std::vector<std::vector<Point2>>
extract_feature_contours(const std::string &filepath,
                         double adf_geotransform[6],
                         double simplification_factor) {

  // TODO: Open the file as a MAT img
  auto image = IO::load_image_from_file(filepath);

  auto contours = IO::extract_feature_contours(
      image, simplification_factor, adf_geotransform[0], adf_geotransform[3],
      adf_geotransform[1], adf_geotransform[5]);

  TSR_LOG_TRACE("contour count: {}", contours.size());

  return contours;
}

} // namespace tsr::API