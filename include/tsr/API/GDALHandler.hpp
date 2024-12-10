#pragma once

#include <gdal/gdal.h>

namespace tsr::API {

GDALDatasetH parseGDALDatasetFromString(std::string);

GDALDatasetH warpWGS84DatasetToUTM(GDALDatasetH handle);

} // namespace tsr::API