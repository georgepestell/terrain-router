#include "tsr/API/GDALHandler.hpp"

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
#include <stdexcept>
#include <string>

#include "tsr/MapUtils.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/logging.hpp"

#include <gdal/ogr_spatialref.h>
#include <tuple>
#include <vector>

namespace tsr::API {

GDALDatasetH parseGDALDatasetFromString(std::string dataString) {

  TSR_LOG_TRACE("parsing response string");

  const char *filepath = "/vsimem/dataset.tiff";

  // Create virtual file in memory
  // (more efficient than unnecessary storageIO operations)

  auto res = VSIFileFromMemBuffer(
      filepath,
      reinterpret_cast<GByte *>(const_cast<char *>(dataString.c_str())),
      dataString.size(), FALSE);

  if (res == nullptr) {
    TSR_LOG_ERROR("could not create virtual memory file");
    throw std::runtime_error("could not create virtual file");
  }

  GDALAllRegister();

  TSR_LOG_TRACE("Opening memory file");
  GDALDatasetH dataset(GDALOpen(filepath, GA_ReadOnly));

  if (!dataset) {
    TSR_LOG_ERROR("Could not parse gdal dataset from string");
    throw std::runtime_error("Could not parse gdal dataset from string");
  }

  TSR_LOG_TRACE("Returning dataset");
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

GDALDatasetH warpWGS84DatasetToUTM(GDALDatasetH handle) {

  GDALAllRegister();

  GDALDataset *dataset = static_cast<GDALDataset *>(handle);

  if (dataset == nullptr) {
    TSR_LOG_ERROR("cannot warp null GDAL dataset");
    throw std::runtime_error("cannot warp null GDAL dataset");
  }

  GDALAllRegister();

  TSR_LOG_TRACE("Setting up warp options");
  GDALWarpAppOptions *warpOptions = GDALWarpAppOptionsNew(NULL, NULL);
  GDALWarpAppOptionsSetWarpOption(
      warpOptions, "-t_srs",
      "+proj=utm +zone=30 +datum=WGS84 +units=m +no_defs");

  GDALDatasetH datasets[1] = {handle};

  int err = 0;
  TSR_LOG_TRACE("warping");
  GDALDatasetH warpedDataset =
      GDALWarp("/tmp/out.tiff", NULL, 1, datasets, warpOptions, &err);

  TSR_LOG_TRACE("warped");
  if (err != 0) {
    TSR_LOG_ERROR("Error: {}", err);
  }

  GDALWarpAppOptionsFree(warpOptions);

  return warpedDataset;
}

} // namespace tsr::API
