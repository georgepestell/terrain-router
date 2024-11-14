#include <gtest/gtest.h>

#include "tsr/logging.hpp"

#include "tsr/Delaunay_3.hpp"
#include "tsr/Router.hpp"

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

  Delaunay_3 tin;
  tin.insert(points.begin(), points.end());

  TSR_LOG_TRACE("Initializing TIN");
  Router router(tin);
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
  Router router(tin);

  // Get the vertex handle of a point
  Point_3 start_point(0, 5, 0);
  Point_3 end_point(25, 0, 0);

  TSR_LOG_TRACE("Calculating route");
  auto route = router.calculate_route(start_point, end_point);

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

  Delaunay_3 tin;
  tin.insert(points.begin(), points.end());

  TSR_LOG_TRACE("Initializing TIN");
  Router router(tin);

  // Get the vertex handle of a point
  Point_3 start_point(0, 5, 0);
  Point_3 end_point(25, 0, 0);

  TSR_LOG_TRACE("Calculating route");
  auto route = router.calculate_route(start_point, end_point);

  for (auto &point : route) {
    TSR_LOG_INFO("Point: ({}, {}, {})", point.x(), point.y(), point.z());
  }

  ASSERT_EQ(route.size(), 4);
  ASSERT_EQ(route.at(0), points.at(0));
  ASSERT_EQ(route.at(1), points.at(1));
  ASSERT_EQ(route.at(2), points.at(3));
  ASSERT_EQ(route.at(3), points.at(5));
}