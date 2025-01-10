#include "gtest/gtest.h"

#include "tsr/API/APICaller.hpp"
#include "tsr/Logging.hpp"

using namespace tsr;

using namespace tsr::API;

TEST(TestAPI, testAPICallerInitializeNoThrow) {
  // Initialize the caller object
  ASSERT_NO_THROW(APICaller caller);
}

TEST(TestAPI, testAPICallerFetchNoThrow) {
  // Initialize API
  APICaller caller;
  // Make HTTPS request
  ASSERT_NO_THROW(caller.fetchDataFromAPI("https://example.com"));
}