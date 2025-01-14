#include "tsr/IO/MapIO.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point3.hpp"
#include <cpl_conv.h>
#include <cpl_error.h>
#include <cpl_port.h>
#include <cstddef>
#include <gdal.h>
#include <gdal_priv.h>
#include <simdjson.h>

#include <stdexcept>
#include <string>
#include <vector>

#define NODATA_VALUE -9999.0

namespace tsr::IO {

void LoadGdalDatasetFromFile(std::string filepath, GDALDatasetH &dataset) {
  GDALAllRegister();

  dataset = GDALOpen(filepath.c_str(), GA_ReadOnly);

  if (dataset == NULL) {
    TSR_LOG_ERROR("Failed to open GDAL dataset ({})", filepath);
    throw std::runtime_error("Failed to open GDAL dataset: " + filepath);
  }
}

void LoadVectorGdalDatasetFromFile(std::string filepath,
                                        GDALDatasetH &dataset) {

  TSR_LOG_TRACE("opening vector GDAL dataset");
  TSR_LOG_TRACE("file: {}", filepath);

  GDALAllRegister();

  dataset =
      GDALOpenEx(filepath.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);

  if (dataset == NULL) {
    TSR_LOG_ERROR("Failed to open GDAL dataset ({})", filepath);
    throw std::runtime_error("Failed to open GDAL dataset: " + filepath);
  }
}

void WriteGdalDatasetToFile(const std::string filename,
                            const GDALDatasetH &hDataset) {

  GDALDataset *dataset = static_cast<GDALDataset *>(hDataset);

  if (!dataset) {
    TSR_LOG_ERROR("cannot write empty dataset to file");
    throw std::runtime_error("cannot write empty dataset to file");
  }

  TSR_LOG_TRACE("detecting raster / vector status");

  GDALDriver *driver;
  GDALAllRegister();
  if (dataset->GetRasterCount() > 0) {
    // Raster dataset
    TSR_LOG_TRACE("detected raster dataset");
    driver = (GDALDriver *)GDALGetDriverByName("GTiff");
  } else {
    // Vector dataset
    TSR_LOG_TRACE("detected vector dataset");
    driver = (GDALDriver *)GDALGetDriverByName("GeoJSON");
  }

  if (!driver) {
    TSR_LOG_ERROR("GTIFF GDAL driver not found");
    throw std::runtime_error("GTIFF GDAL driver not found");
  }

  // Set creation options for better performance and smaller file size
  char *options[] = {"COMPRESS=LZW", // Enable compression
                     "TILED=YES",    // Use tiling for better performance
                     "BIGTIFF=YES",  // Allow large files
                     nullptr};

  TSR_LOG_TRACE("Copying dataset file");

  auto copy = driver->DefaultCreateCopy(filename.c_str(), dataset, FALSE,
                                        options, NULL, NULL);

  TSR_LOG_TRACE("Copied GDAL dataset file");

  if (!copy) {
    TSR_LOG_ERROR("GDAL copy failed");
    throw std::runtime_error("GDAL copy failed");
  }

  GDALReleaseDataset(copy);
}

std::vector<std::vector<Point3>>
ExtractGdalDatasetContours(GDALDataset *dataset, int layerID) {

  if (!dataset) {
    TSR_LOG_ERROR("dataset invalid");
    throw std::runtime_error("dataset invalid");
  }

  // Setup the contours vector
  std::vector<std::vector<Point3>> contours;

  TSR_LOG_ERROR("Not implemented");

  return contours;
}

std::vector<Point3> ExtractGdalDatasetPoints(GDALDatasetH hDataset,
                                                 int layerID) {

  GDALDataset *dataset = static_cast<GDALDataset *>(hDataset);
  if (!dataset) {
    TSR_LOG_ERROR("Dataset invalid");
    throw std::runtime_error("dataset invalid");
  }

  if (layerID > dataset->GetRasterCount()) {
    TSR_LOG_ERROR("Layer ID invalid");
    throw std::runtime_error("layer ID invalid");
  }

  GDALRasterBand *band = dataset->GetRasterBand(layerID);

  if (!band) {
    TSR_LOG_ERROR("Could not load raster band");
    throw std::runtime_error("could not load raster band");
  }

  int bandXSize = band->GetXSize();
  int bandYSize = band->GetYSize();

  double geotransform[6];
  GDALGetGeoTransform(dataset, geotransform);

  std::vector<Point3> points;
  std::vector<float> line(bandXSize);
  for (int row = 0; row < bandYSize; row++) {
    if (band->RasterIO(GF_Read, 0, row, bandXSize, 1, line.data(), bandXSize, 1,
                       GDT_Float32, 0, 0) != CE_None) {
      TSR_LOG_ERROR("failed to read line {}", row);
      continue;
    }

    // Loop through each column
    for (int col = 0; col < bandXSize; col++) {

      double z = static_cast<double>(line[col]);

      if (z == NODATA_VALUE) {
        // TSR_LOG_TRACE("NO DATA VALUE FOUND");
        continue;
      }

      double x =
          geotransform[0] + col * geotransform[1] + row * geotransform[2];
      double y =
          geotransform[3] + col * geotransform[4] + row * geotransform[5];

      points.push_back({x, y, z});
    }
  }

  return points;
}

} // namespace tsr::IO