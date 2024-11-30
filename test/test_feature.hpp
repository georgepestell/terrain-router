#include "tsr/Delaunay_3.hpp"
#include "tsr/Features/SimpleBooleanFeature.hpp"
#include "tsr/Features/SimpleBooleanToDoubleFeature.hpp"
#include "tsr/Point_3.hpp"
#include <gtest/gtest.h>

#include <memory>

using namespace tsr;

TEST(testFeature, testFeatureIsEqualsFalse) {
  SimpleBooleanFeature feature1("feature1");
  SimpleBooleanFeature feature2("feature2");
  ASSERT_NE(feature1, feature2);
}

TEST(testFeature, testFeatureIsEqualsSameObject) {
  SimpleBooleanFeature feature1("feature1");
  ASSERT_EQ(feature1, feature1);
}
TEST(testFeature, testFeatureIsEqualsSameName) {
  SimpleBooleanFeature feature1("feature1");
  SimpleBooleanFeature feature2("feature1");
  ASSERT_EQ(feature1, feature2);
}

TEST(testFeature, testSimpleBooleanFeature) {

  auto boolFeature =
      std::make_shared<SimpleBooleanFeature>("SimpleBoolFeature");

  Face_handle tmpFace;
  Point_3 tmpPoint;
  ASSERT_FALSE(boolFeature->calculate(tmpFace, tmpPoint, tmpPoint));
}

TEST(testFeature, testSimpleBoolToIntegerFeature) {

  auto boolFeature = std::make_shared<SimpleBooleanFeature>("Feature");

  auto boolToIntFeature =
      std::make_shared<SimpleBooleanToDoubleFeature>("BOOL_TO_DOUBLE");

  boolToIntFeature->add_dependency(boolFeature);

  Point_3 tmpPoint;
  Face_handle tmpFace;
  ASSERT_EQ(boolToIntFeature->calculate(tmpFace, tmpPoint, tmpPoint), -10);
}
