/**
 * Set of functions handling the chunking of DEM data and processed mesh storage
 */
#include "tsr/API/Chunking.hpp"

#include "tsr/API/APICaller.hpp"
#include "tsr/API/GDALHandler.hpp"

#include "tsr/logging.hpp"

#include <cmath>
#include <fmt/core.h>
#include <gdal.h>
#include <memory>
#include <string>

namespace tsr::API {

#define CHUNK_DEM_SIZE_ARCSECONDS 1;
#define CHUNK_MESH_SIZE_ARCSECONDS 0.1;
#define DEM_URL                                                                \
  "https://portal.opentopography.org/API/"                                     \
  "globaldem?demtype=COP30&south={}&west={}&north="                            \
  "{}&east={}&outputFormat=GeoTiff&API_Key={}"

std::string parseDEMURL(std::string key, double min_lat, double min_lng,
                        double max_lat, double max_lng) {
  return fmt::format(DEM_URL, min_lat, min_lng, max_lat, max_lng, key);
}

double getChunkMin(double value, double tile_resolution) {
  return std::floor(value / tile_resolution) * tile_resolution;
}

GDALDatasetH getChunk(double lat, double lng, std::string opentopography_key) {

  double tile_resolution_degrees = 0.1; /// 0.1 degrees = ~ 1.1km tiles

  double min_lat = getChunkMin(lat, tile_resolution_degrees);
  double min_lng = getChunkMin(lng, tile_resolution_degrees);

  double max_lat = min_lat + tile_resolution_degrees;
  double max_lng = min_lng + tile_resolution_degrees;

  TSR_LOG_TRACE("coords: {},{} {},{}", min_lat, min_lng, max_lat, max_lng);

  APICaller apiCaller;

  std::string response = apiCaller.fetchDataFromAPI(
      parseDEMURL(opentopography_key, min_lat, min_lng, max_lat, max_lng));

  return parseGDALDatasetFromString(response);
}

} // namespace tsr::API