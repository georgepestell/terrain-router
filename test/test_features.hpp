#include <gtest/gtest.h>

#include "tsr/Feature.hpp"
#include "tsr/FeatureManager.hpp"

using namespace tsr;

TEST(FeatureTest, testFeatureInitializeNoThrow) {
  ASSERT_NO_THROW(Feature<std::any> feature(""));
}

TEST(FeatureTest, testFeatureIsEqualsFalse) {
  Feature<std::any> feature1("feature1");
  Feature<std::any> feature2("feature2");
  ASSERT_TRUE(feature1.isEqual(feature2));
}

TEST(FeatureTest, testFeatureIsEqualsSameObject) {
  Feature<std::any> feature1("feature1");
  ASSERT_TRUE(feature1.isEqual(feature1));
}
TEST(FeatureTest, testFeatureIsEqualsSameName) {
  Feature<std::any> feature1("feature1");
  Feature<std::any> feature2("feature1");
  ASSERT_TRUE(feature1.isEqual(feature2));
}

TEST(FeatureManagerTest, testInitializeNoThrow) {
  ASSERT_NO_THROW(FeatureManager feature_manager);
}
