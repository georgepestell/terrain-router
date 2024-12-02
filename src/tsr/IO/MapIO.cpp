#include "tsr/IO/MapIO.hpp"
#include "tsr/logging.hpp"
#include <gdal.h>

namespace tsr::IO {

std::unique_ptr<GDALDatasetH>
load_gdal_dataset_from_file(std::string filepath) {
  GDALAllRegister();
  auto src_ds =
      std::make_unique<GDALDatasetH>(GDALOpen(filepath.c_str(), GA_ReadOnly));

  if (*src_ds == NULL) {
    TSR_LOG_ERROR("Failed to open GDAL dataset ({})", filepath);
    throw std::runtime_error("Failed to open GDAL dataset: " + filepath);
  }

  return src_ds;
}

} // namespace tsr::IO