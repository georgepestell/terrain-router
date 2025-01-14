#include <curl/curl.h>

namespace tsr::API {

class APICallerConfig {
public:
  APICallerConfig();
  CURL *curl;
};

} // namespace tsr::API