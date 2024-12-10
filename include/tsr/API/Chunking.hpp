#pragma once

#include <gdal/gdal_priv.h>
#include <memory>
#include <string>

namespace tsr::API {

GDALDatasetH getChunk(double lat, double lng, std::string opentopography_key);

}