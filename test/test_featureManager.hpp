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

  boolToDoubleFeature->AddDependency(boolFeature);
  boolToDoubleFeature->AddDependency(unusedDependency);

  auto sharedChildDependency =
      std::make_shared<SimpleBooleanFeature>("Shared Child Dep");

  boolFeature->AddDependency(sharedChildDependency);
  unusedDependency->AddDependency(sharedChildDependency);

  FeatureManager fm;

  fm.outputFeature = boolToDoubleFeature;

  ASSERT_FALSE(fm.HasDependencyCycle());
}

TEST(testFeatureManager, testMoreComplexDAG) {

  auto boolFeature = std::make_shared<SimpleBooleanFeature>("Feature");

  auto unusedDependency =
      std::make_shared<SimpleBooleanFeature>("Unused Feature");

  auto boolToDoubleFeature = std::make_shared<SimpleBooleanToDoubleFeature>(
      "SimpleBooleanToDoubleFeature");

  boolToDoubleFeature->AddDependency(boolFeature);
  boolToDoubleFeature->AddDependency(unusedDependency);

  auto childDependency1 = std::make_shared<SimpleBooleanFeature>("Child Dep 1");
  auto childDependency2 = std::make_shared<SimpleBooleanFeature>("Child Dep 2");

  boolFeature->AddDependency(childDependency1);
  unusedDependency->AddDependency(childDependency2);

  childDependency1->AddDependency(unusedDependency);

  FeatureManager fm;

  fm.outputFeature = boolToDoubleFeature;

  ASSERT_FALSE(fm.HasDependencyCycle());
}

TEST(testFeatureManager, testSimpleCycle) {

  auto boolFeature = std::make_shared<SimpleBooleanFeature>("Feature");

  auto unusedDependency =
      std::make_shared<SimpleBooleanFeature>("Unused Feature");

  auto boolToDoubleFeature = std::make_shared<SimpleBooleanToDoubleFeature>(
      "SimpleBooleanToDoubleFeature");

  boolToDoubleFeature->AddDependency(boolFeature);
  boolToDoubleFeature->AddDependency(unusedDependency);

  auto childDependency1 = std::make_shared<SimpleBooleanFeature>("Child Dep 1");
  auto childDependency2 = std::make_shared<SimpleBooleanFeature>("Child Dep 2");

  boolFeature->AddDependency(childDependency1);
  childDependency1->AddDependency(childDependency2);

  childDependency2->AddDependency(boolFeature);

  FeatureManager fm;

  fm.outputFeature = boolToDoubleFeature;

  ASSERT_TRUE(fm.HasDependencyCycle());
}