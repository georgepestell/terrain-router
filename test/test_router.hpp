#include <gtest/gtest.h>

#include "tsr/FeatureManager.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/logging.hpp"

#include "tsr/Delaunay_3.hpp"
#include "tsr/Router.hpp"

#include "tsr/Features/DistanceFeature.hpp"
#include "tsr/Features/GradientFeature.hpp"
#include "tsr/Features/GradientSpeedFeature.hpp"
#include "tsr/Features/MultiplierFeature.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

using namespace tsr;

TEST(TestRouter, routerInitiailizeTest) {

  /*
   *
   * (s) -- (1) -- (2)
   *   \  /    \ /   \
   *   (x) -- (x) -- (e)
   *
   */

  std::vector<Point_3> points;

  points.push_back(Point_3(0, 5, 0));
  points.push_back(Point_3(5, 0, 0));
  points.push_back(Point_3(10, 5, 0));
  points.push_back(Point_3(15, 0, 0));
  points.push_back(Point_3(20, 5, 0));
  points.push_back(Point_3(25, 0, 0));

  TSR_LOG_TRACE("Initializing TIN");
  Delaunay_3 tin;
  tin.insert(points.begin(), points.end());

  FeatureManager fm;
  fm.setOutputFeature(std::make_shared<GradientFeature>("GRADIENT"));

  ASSERT_NO_THROW(Router router(tin, fm));
}

TEST(TestRouter, routerInitializeFailsWithInvalidFeatureManagerTest) {
  Delaunay_3 tin;
  FeatureManager fm;

  ASSERT_THROW(Router router(tin, fm), std::runtime_error);
}

TEST(TestRouter, routerDistanceCheck) {

  /*
   *
   * (s) -- (1) -- (2)
   *   \  /    \ /   \
   *   (x) -- (x) -- (e)
   *
   */

  std::vector<Point_3> points;

  points.push_back(Point_3(0, 5, 0));
  points.push_back(Point_3(5, 0, 0));
  points.push_back(Point_3(10, 5, 0));
  points.push_back(Point_3(15, 0, 0));
  points.push_back(Point_3(20, 5, 0));
  points.push_back(Point_3(25, 0, 0));

  Delaunay_3 tin;
  tin.insert(points.begin(), points.end());

  TSR_LOG_TRACE("Initializing TIN");

  FeatureManager featureManager;
  auto distanceFeature = std::make_shared<DistanceFeature>("DISTANCE_FEATURE");

  featureManager.setOutputFeature(distanceFeature);

  Router router(tin, featureManager);

  // Get the vertex handle of a point
  Point_3 start_point(0, 5, 0);
  Point_3 end_point(25, 0, 0);

  TSR_LOG_TRACE("Calculating route");
  auto route = router.calculateRoute(start_point, end_point);

  for (auto &point : route) {
    TSR_LOG_INFO("Point: ({}, {}, {})", point.x(), point.y(), point.z());
  }

  ASSERT_EQ(route.size(), 4);
  ASSERT_EQ(route.at(0), points.at(0));
  ASSERT_EQ(route.at(1), points.at(2));
  ASSERT_EQ(route.at(2), points.at(4));
  ASSERT_EQ(route.at(3), points.at(5));
}

TEST(TestRouter, routerGradientCheck) {

  /*
   *
   * (s) -- (x) -- (x)
   *   \  /    \ /   \
   *   (1) -- (2) -- (e)
   *
   */

  std::vector<Point_3> points;

  points.push_back(Point_3(0, 5, 0));
  points.push_back(Point_3(5, 0, 0));
  points.push_back(Point_3(10, 5, 200));
  points.push_back(Point_3(15, 0, 0));
  points.push_back(Point_3(20, 5, 0));
  points.push_back(Point_3(25, 0, 0));

  TSR_LOG_TRACE("Initializing TIN");

  Delaunay_3 tin;
  tin.insert(points.begin(), points.end());

  TSR_LOG_TRACE("Initializing feature manager");

  FeatureManager feature_manager;

  auto gradientFeature = std::make_shared<GradientFeature>("GRADIENT");
  auto distanceFeature = std::make_shared<DistanceFeature>("DISTANCE");
  auto gradientSpeedFeature =
      std::make_shared<GradientSpeedFeature>("GRADIENT_SPEED");

  gradientSpeedFeature->add_dependency(gradientFeature);

  auto speedFeature = std::make_shared<MultiplierFeature>("SPEED");

  speedFeature->add_dependency(distanceFeature, MultiplierFeature::DOUBLE);
  speedFeature->add_dependency(gradientSpeedFeature, MultiplierFeature::DOUBLE);

  feature_manager.setOutputFeature(speedFeature);

  TSR_LOG_TRACE("Initializing router");

  Router router(tin, feature_manager);

  // Get the vertex handle of a point
  Point_3 start_point(0, 5, 0);
  Point_3 end_point(25, 0, 0);

  TSR_LOG_TRACE("Calculating route");
  auto route = router.calculateRoute(start_point, end_point);

  for (auto &point : route) {
    TSR_LOG_INFO("Point: ({}, {}, {})", point.x(), point.y(), point.z());
  }

  ASSERT_EQ(route.size(), 4);
  ASSERT_EQ(route.at(0), points.at(0));
  ASSERT_EQ(route.at(1), points.at(1));
  ASSERT_EQ(route.at(2), points.at(3));
  ASSERT_EQ(route.at(3), points.at(5));
}