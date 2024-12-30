#pragma once

/**
    APICaller allows API requests to be made to various mapping data sources.
 */

#include <curl/curl.h>
#include <string>

namespace tsr::API {

/**
 * @brief Calls the opentopography API, fetching a raster DEM for a given area
 *
 * @tparam dataType The datatype of the height data. See -ot parameter of
 * gdalwarp.
 */
template <typename dataType> void fetch_DEM_from_opentopography();

class APICallerConfig;

class APICaller {
public:
  bool fetchDataFromAPI(const std::string &url, const std::string &filepath);

  APICaller();
  ~APICaller();
};

}; // namespace tsr::API
