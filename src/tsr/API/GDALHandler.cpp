#include "tsr/PointProcessor.hpp"
#include <cpl_vsi.h>
#include <gdal/gdal_priv.h>

#include "tsr/logging.hpp"

namespace tsr::API {

std::unique_ptr<GDALDataset, decltype(&GDALClose)>
parseGDALDatasetFromGeoTiffString(std::string dataString) {

  const char *filepath = "/vsimem/dataset.tiff";

  // Create virtual file in memory
  // (more efficient than unnecessary storageIO operations)
  VSIFileFromMemBuffer(
      filepath,
      reinterpret_cast<GByte *>(const_cast<char *>(dataString.c_str())),
      dataString.size(), FALSE);

  std::unique_ptr<GDALDataset, decltype(&GDALClose)> dataset(
      static_cast<GDALDataset *>(GDALOpen(filepath, GA_ReadOnly)), GDALClose);

  if (!dataset) {
    TSR_LOG_ERROR("Could not parse gdal dataset from string");
    throw std::runtime_error("Could not parse gdal dataset from string");
  }

  return dataset;
}

} // namespace tsr::API
