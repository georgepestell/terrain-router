#pragma once

#include <gdal/gdal.h>
#include <string>

namespace tsr {
struct DataFile {
  GDALDatasetH dataset;
  std::string filename;
  DataFile(GDALDatasetH dataset, std::string filename)
      : dataset(dataset), filename(filename) {};
};
} // namespace tsr