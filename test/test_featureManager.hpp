#include <gtest/gtest.h>

#include "tsr/FeatureManager.hpp"
#include "tsr/Features/SimpleBooleanFeature.hpp"
#include "tsr/Features/SimpleBooleanToDoubleFeature.hpp"

#include <memory>

using namespace tsr;

TEST(testFeatureManager, testSimpleDAG) {

  auto boolFeature = std::make_shared<SimpleBooleanFeature>("Feature");

  auto unusedDependency =
      std::make_shared<SimpleBooleanFeature>("Unused Feature");

  auto boolToDoubleFeature = std::make_shared<SimpleBooleanToDoubleFeature>(
      "SimpleBooleanToDoubleFeature");

  boolToDoubleFeature->add_dependency(boolFeature);
  boolToDoubleFeature->add_dependency(unusedDependency);

  auto sharedChildDependency =
      std::make_shared<SimpleBooleanFeature>("Shared Child Dep");

  boolFeature->add_dependency(sharedChildDependency);
  unusedDependency->add_dependency(sharedChildDependency);

  FeatureManager fm;

  fm.outputFeature = boolToDoubleFeature;

  ASSERT_FALSE(fm.has_dependency_cycle());
}

TEST(testFeatureManager, testMoreComplexDAG) {

  auto boolFeature = std::make_shared<SimpleBooleanFeature>("Feature");

  auto unusedDependency =
      std::make_shared<SimpleBooleanFeature>("Unused Feature");

  auto boolToDoubleFeature = std::make_shared<SimpleBooleanToDoubleFeature>(
      "SimpleBooleanToDoubleFeature");

  boolToDoubleFeature->add_dependency(boolFeature);
  boolToDoubleFeature->add_dependency(unusedDependency);

  auto childDependency1 = std::make_shared<SimpleBooleanFeature>("Child Dep 1");
  auto childDependency2 = std::make_shared<SimpleBooleanFeature>("Child Dep 2");

  boolFeature->add_dependency(childDependency1);
  unusedDependency->add_dependency(childDependency2);

  childDependency1->add_dependency(unusedDependency);

  FeatureManager fm;

  fm.outputFeature = boolToDoubleFeature;

  ASSERT_FALSE(fm.has_dependency_cycle());
}

TEST(testFeatureManager, testSimpleCycle) {

  auto boolFeature = std::make_shared<SimpleBooleanFeature>("Feature");

  auto unusedDependency =
      std::make_shared<SimpleBooleanFeature>("Unused Feature");

  auto boolToDoubleFeature = std::make_shared<SimpleBooleanToDoubleFeature>(
      "SimpleBooleanToDoubleFeature");

  boolToDoubleFeature->add_dependency(boolFeature);
  boolToDoubleFeature->add_dependency(unusedDependency);

  auto childDependency1 = std::make_shared<SimpleBooleanFeature>("Child Dep 1");
  auto childDependency2 = std::make_shared<SimpleBooleanFeature>("Child Dep 2");

  boolFeature->add_dependency(childDependency1);
  childDependency1->add_dependency(childDependency2);

  childDependency2->add_dependency(boolFeature);

  FeatureManager fm;

  fm.outputFeature = boolToDoubleFeature;

  ASSERT_TRUE(fm.has_dependency_cycle());
}