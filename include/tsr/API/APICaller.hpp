#pragma once

/**
    APICaller allows API requests to be made to various mapping data sources.
 */

#include <memory>
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
  std::string fetchDataFromAPI(const std::string &url);
  APICaller();
  ~APICaller();

private:
  std::unique_ptr<APICallerConfig> config;
};

}; // namespace tsr::API
