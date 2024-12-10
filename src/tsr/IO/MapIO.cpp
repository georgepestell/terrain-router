#include "tsr/IO/MapIO.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/logging.hpp"
#include <gdal.h>
#include <simdjson.h>

namespace tsr::IO {

GDALDatasetH load_gdal_dataset_from_file(std::string filepath) {
  GDALAllRegister();
  GDALDatasetH src_ds = GDALOpen(filepath.c_str(), GA_ReadOnly);

  if (src_ds == NULL) {
    TSR_LOG_ERROR("Failed to open GDAL dataset ({})", filepath);
    throw std::runtime_error("Failed to open GDAL dataset: " + filepath);
  }

  return src_ds;
}

std::vector<std::vector<Point_3>>
extract_contours_from_vector_dataset(GDALDatasetH dataset_handle,
                                     std::string layerID) {

  // Get the dataset
  GDALDataset *dataset = static_cast<GDALDataset *>(dataset_handle);

  // Setup the contours vector
  std::vector<std::vector<Point_3>> contours;

  return contours;
}

} // namespace tsr::IO