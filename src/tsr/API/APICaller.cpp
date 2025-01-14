#include "tsr/API/APICaller.hpp"
#include "tsr/API/APICallerConfig.hpp"
#include "tsr/Logging.hpp"

#include <cstddef>
#include <curl/curl.h>
#include <curl/easy.h>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <vector>

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
size_t APICallbackHelper(void *buffer, size_t size, size_t nmemb,
                         void *userdata) {

  std::vector<char> *memory = static_cast<std::vector<char> *>(userdata);
  memory->insert(memory->end(), static_cast<char *>(buffer),
                 static_cast<char *>(buffer) + size * nmemb);
  return size * nmemb;
}

APICaller::~APICaller() = default;
APICaller::APICaller() = default;

template <typename dataType> void fetch_DEM_from_opentopography() {}

bool APICaller::FetchDataFromAPI(const std::string &url,
                                 const std::string &filepath) {
  CURLcode response_code;

  CURL *curl = curl_easy_init();

  // Intialize CURL
  if (!curl) {
    TSR_LOG_ERROR("Cannot make API request: curl is not initialized");
    throw std::runtime_error(
        "Cannot make API request: curl is not initialized");
  }

  std::vector<char> buffer;

  // Set curl options
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, APICallbackHelper);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

  TSR_LOG_TRACE("Calling API ({})", url);
  response_code = curl_easy_perform(curl);

  // Check response code
  if (response_code != CURLE_OK) {
    TSR_LOG_ERROR("CURL Error: {}", curl_easy_strerror(response_code));
    return 1;
  }

  std::ofstream file(filepath.c_str(), std::ios::binary);
  if (file.is_open()) {
    file.write(buffer.data(), buffer.size());
    file.close();
  } else {
    TSR_LOG_ERROR("failed to open file for curl write");
  }

  curl_easy_cleanup(curl);

  return 0;
}

}; // namespace tsr::API