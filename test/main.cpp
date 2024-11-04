#include "tsr/logging.hpp"

#include "gtest/gtest.h"

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
  return RUN_ALL_TESTS();
}

#include "test_api.hpp"
#include "test_dtm.hpp"
