#include <gtest/gtest.h>

#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point3.hpp"

#include "tsr/Router.hpp"
#include "tsr/Tin.hpp"

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

  std::vector<Point3> points;

  points.push_back(Point3(0, 5, 0));
  points.push_back(Point3(5, 0, 0));
  points.push_back(Point3(10, 5, 0));
  points.push_back(Point3(15, 0, 0));
  points.push_back(Point3(20, 5, 0));
  points.push_back(Point3(25, 0, 0));

  TSR_LOG_TRACE("Initializing Tin");
  auto tin = create_tin_from_pointset(points);

  FeatureManager fm;
  fm.setOutputFeature(std::make_shared<GradientFeature>("GRADIENT"));

  ASSERT_NO_THROW(Router router);
}

TEST(TestRouter, routerDistanceCheck) {

  /*
   *
   * (s) -- (1) -- (2)
   *   \  /    \ /   \
   *   (x) -- (x) -- (e)
   *
   */

  std::vector<Point3> points;

  points.push_back(Point3(0, 5, 0));
  points.push_back(Point3(5, 0, 0));
  points.push_back(Point3(10, 5, 0));
  points.push_back(Point3(15, 0, 0));
  points.push_back(Point3(20, 5, 0));
  points.push_back(Point3(25, 0, 0));

  auto tin = create_tin_from_pointset(points);

  TSR_LOG_TRACE("Initializing Tin");

  FeatureManager featureManager;
  auto distanceFeature = std::make_shared<DistanceFeature>("DISTANCE_FEATURE");

  featureManager.setOutputFeature(distanceFeature);

  Router router;

  // Get the vertex handle of a point
  Point3 start_point(0, 5, 0);
  Point3 end_point(25, 0, 0);

  TSR_LOG_TRACE("Calculating route");
  auto route =
      router.calculateRoute(tin, featureManager, start_point, end_point);

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

  std::vector<Point3> points;

  points.push_back(Point3(0, 5, 0));
  points.push_back(Point3(5, 0, 0));
  points.push_back(Point3(10, 5, 200));
  points.push_back(Point3(15, 0, 0));
  points.push_back(Point3(20, 5, 0));
  points.push_back(Point3(25, 0, 0));

  TSR_LOG_TRACE("Initializing Tin");

  auto tin = create_tin_from_pointset(points);

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

  Router router;

  // Get the vertex handle of a point
  Point3 start_point(0, 5, 0);
  Point3 end_point(25, 0, 0);

  TSR_LOG_TRACE("Calculating route");
  auto route =
      router.calculateRoute(tin, feature_manager, start_point, end_point);

  for (auto &point : route) {
    TSR_LOG_INFO("Point: ({}, {}, {})", point.x(), point.y(), point.z());
  }

  ASSERT_EQ(route.size(), 4);
  ASSERT_EQ(route.at(0), points.at(0));
  ASSERT_EQ(route.at(1), points.at(1));
  ASSERT_EQ(route.at(2), points.at(3));
  ASSERT_EQ(route.at(3), points.at(5));
}