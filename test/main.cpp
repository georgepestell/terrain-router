#include "tsr/logging.hpp"

#include "gtest/gtest.h"

#include <sys/resource.h>

using namespace tsr;

class Environment : public ::testing::Environment {
public:
  void SetUp() override {
    log_set_global_loglevel(LogLevel::TRACE);
    log_set_global_logstream(LogStream::STDERR);
  }
};

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  // gtest takes ownership of the TestEnvironment ptr - we don't delete it.
  ::testing::AddGlobalTestEnvironment(new Environment);

  struct rlimit limit;
  getrlimit(RLIMIT_STACK, &limit);
  limit.rlim_cur = 32 * 1024 * 1024;
  setrlimit(RLIMIT_STACK, &limit);

  return RUN_ALL_TESTS();
}

// #include "test_api.hpp"
#include "test_feature.hpp"
// #include "test_featureManager.hpp"
// #include "test_router.hpp"
// #include "test_triangulation.hpp"

// #include "test_GDALHandler.hpp"
// #include "test_MeshUtils.hpp"