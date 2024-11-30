#include "tsr/API/APICaller.hpp"
#include "tsr/API/APICallerConfig.hpp"
#include "tsr/logging.hpp"

#include <curl/easy.h>
#include <stdexcept>
#include <string>

namespace tsr::API {

/**
 * @brief Helper callback function for CURL which appends the request contents
 * to a given output string
 *
 * @param contents API response contents
 * @param size Size of each memory blocks storing the response contents
 * @param nmemb Number of memory blocks storing the response contents
 * @param output String to append contents to
 * @return size_t Size of the response
 */
size_t APICallbackHelper(void *contents, size_t size, size_t nmemb,
                         std::string *output) {
  size_t totalSize = size * nmemb;
  output->append((char *)contents, totalSize);
  return totalSize;
}

APICaller::~APICaller() = default;

template <typename dataType> void fetch_DEM_from_opentopography() {}

APICaller::APICaller() { this->config = std::make_unique<APICallerConfig>(); }

APICallerConfig::APICallerConfig() {

  this->curl = curl_easy_init();

  if (!this->curl) {
    TSR_LOG_ERROR("Failed to initialize CURL");
    throw std::runtime_error("Failed to initialize CURL");
  }

  curl_easy_setopt(this->curl, CURLOPT_WRITEFUNCTION, APICallbackHelper);
}

std::string APICaller::fetchDataFromAPI(const std::string &url) {
  CURLcode response_code;
  std::string response_string;

  // Intialize CURL
  if (!this->config->curl) {
    TSR_LOG_ERROR("Cannot make API request: curl is not initialized");
    throw std::runtime_error(
        "Cannot make API request: curl is not initialized");
  }

  // Set curl options
  curl_easy_setopt(this->config->curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(this->config->curl, CURLOPT_WRITEDATA, &response_string);

  TSR_LOG_TRACE("Calling API ({})", url);
  response_code = curl_easy_perform(this->config->curl);

  // Check response code
  if (response_code != CURLE_OK) {
    TSR_LOG_ERROR("CURL Error: {}", curl_easy_strerror(response_code));
  }

  return response_string;
}

}; // namespace tsr::API