#ifndef TSR_DEBUG
#define TSR_DEBUG
#include "tsr/IO.hpp"
#include <CGAL/Point_2.h>
#include <CGAL/Point_set_2.h>
#include <CGAL/Point_set_3.h>
#endif

#include "tsr/DTM.hpp"
#include "tsr/logging.hpp"
#include "gtest/gtest.h"

using namespace tsr;

TEST(TestDTM, test_initializeEmptyThrows) {
  // Create empty point set
  Point_set_3 empty_points;

  // Initialize empty DTM
  EXPECT_THROW(make_unique<DTM>(empty_points), invalid_argument);
}

TEST(TestDTM, test_initalizeDoesNotThrow) {
  Point_set_3 points;
  points.insert(Point_3(0, 0, 1));
  points.insert(Point_3(5, 0, 3));
  points.insert(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  ASSERT_NO_THROW(make_unique<DTM>(points));
}

TEST(TestDTM, test_getTopology) {
  Point_set_3 points;
  points.insert(Point_3(0, 0, 1));
  points.insert(Point_3(5, 0, 3));
  points.insert(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = make_unique<DTM>(points);

  ASSERT_NO_THROW(dtm->get_topology().number_of_vertices());
}

TEST(TestDTM, test_initalizeVertexCountMatchesDEM) {
  // Create point set
  Point_set_3 points;
  points.insert(Point_3(0, 0, 1));
  points.insert(Point_3(5, 0, 3));
  points.insert(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = make_unique<DTM>(points);

  TSR_LOG_TRACE("verifying vertex count");

  ASSERT_EQ(dtm->get_topology().number_of_vertices(), points.size());
}

TEST(TestDTM, test_simplify_3d_feature) {
  // Create mesh
  // Create point set
  Point_set_3 points;
  points.insert(Point_3(0, 0, 1));
  points.insert(Point_3(5, 0, 3));
  points.insert(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = make_unique<DTM>(points);

  TSR_LOG_TRACE("simplifying dtm");

  // Simplify
  Delaunay_3D &mesh = dtm->get_topology();
  ASSERT_NO_THROW(dtm->simplify_3d_feature(mesh, mesh));

  ASSERT_TRUE(dtm->get_topology().is_valid());
}

TEST(TestDTM, test_simplify_3d_feature_simplifies_flat_plane) {

  /**
   * @brief Creates a flat plane with an extra vertice along an edge which
   * should be removed upon simplification
   *
   *      *
   *     / \
   *    *   \
   *   /     \
   *  *- - - -*
   *
   */

  Point_set_3 points;
  points.insert(Point_3(0, 0, 0));
  points.insert(Point_3(1, 2, 0)); // Additional vertice
  points.insert(Point_3(2, 4, 0));
  points.insert(Point_3(4, 0, 0));

  auto dtm = make_unique<DTM>(points);

  Delaunay_3D &meshPtr = dtm->get_topology();

  // Simplify
  dtm->simplify_3d_feature(meshPtr, meshPtr);

  ASSERT_EQ(meshPtr.number_of_vertices(), 3);
}

TEST(TestDTM, test_simplify_3d_feature_simplifies_small_angles) {

  /**
   * @brief Creates a flat plane with an extra vertice along an edge which
   * should be removed upon simplification
   *
   *      *
   *     / \
   *    *   \
   *   /     \
   *  *- - - -*
   *
   */

  Point_set_3 points;
  points.insert(Point_3(0, 0, 0));
  points.insert(Point_3(0.9, 2, 0)); // Additional vertice
  points.insert(Point_3(2, 4, 0));
  points.insert(Point_3(4, 0, 0));

  auto dtm = make_unique<DTM>(points);

  Delaunay_3D &meshPtr = dtm->get_topology();

  Delaunay_3D mesh2;

  // Simplify, ensuring distance is not the limiting factor
  dtm->simplify_3d_feature(meshPtr, mesh2, DEFAULT_COSINE_MAX_ANGLE_REGIONS,
                           10000, DEFAULT_COSINE_MAX_ANGLE_CORNERS, 10000);

  ASSERT_EQ(mesh2.number_of_vertices(), 3);
}

TEST(TestDTM, test_simplify_3d_feature_simplifies_small_distances) {

  /**
   * @brief Creates a flat plane with an extra vertice along an edge which
   * should be removed upon simplification
   *
   *      *
   *     / \
   *   *    \
   *  /      \
   *  *- - - -*
   *
   */

  Point_set_3 points;
  points.insert(Point_3(0, 0, 0));
  points.insert(Point_3(0.9, 2, 0)); // Additional vertice
  points.insert(Point_3(2, 4, 0));
  points.insert(Point_3(4, 0, 0));

  auto dtm = make_unique<DTM>(points);

  Delaunay_3D &meshPtr = dtm->get_topology();

  // Simplify, ensuring angle is not the limiting factor
  dtm->simplify_3d_feature(meshPtr, meshPtr, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
                           DEFAULT_MAX_DISTANCE_CORNERS);

  ASSERT_EQ(meshPtr.number_of_vertices(), 3);
}

TEST(TestDTM, test_simplify_3d_feature_does_not_simplify_sharp_edges) {

  /**
   * @brief Creates a flat polygon, with one very elevated edge along the edge
   * of the triangle which should not be removed as the angle is too great
   *
   *   _--*
   *  *_   \
   *  | \_  \
   *  |   \ _\
   *  *- - - -*
   *
   */

  Point_set_3 points;
  points.insert(Point_3(0, 0, 0));
  points.insert(Point_3(1, 2, 100)); // Additional vertice
  points.insert(Point_3(2, 4, 0));
  points.insert(Point_3(4, 0, 0));

  auto dtm = make_unique<DTM>(points);

  Delaunay_3D &meshPtr = dtm->get_topology();

  // Simplify, ensuring distance is not the limiting factor
  dtm->simplify_3d_feature(meshPtr, meshPtr, DEFAULT_COSINE_MAX_ANGLE_REGIONS,
                           10000, DEFAULT_COSINE_MAX_ANGLE_CORNERS, 10000);

  ASSERT_EQ(meshPtr.number_of_vertices(), 4);
}

TEST(TestDTM, test_simplify_3d_feature_does_not_simplify_long_distances) {

  /**
   * @brief Creates a flat plane with an extra vertice along an edge which
   * should be removed upon simplification
   *
   *    /*
   *   /   \
   *  *     \
   *  |      \
   *  *- - - -*
   *
   */

  Point_set_3 points;
  points.insert(Point_3(0, 0, 0));
  points.insert(Point_3(0, 20000, 0)); // Additional vertice
  points.insert(Point_3(20000, 40000, 0));
  points.insert(Point_3(40000, 0, 0));

  auto dtm = make_unique<DTM>(points);

  Delaunay_3D &meshPtr = dtm->get_topology();

  // Simplify, ensuring angle is not the limiting factor
  dtm->simplify_3d_feature(meshPtr, meshPtr, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
                           DEFAULT_MAX_DISTANCE_CORNERS);

  ASSERT_EQ(meshPtr.number_of_vertices(), 4);
}

TEST(TestDTM, testCreateBinaryFeature) {

  vector<Point_2> points;
  points.push_back(Point_2(0, 0));
  points.push_back(Point_2(0, 20000)); // Additional vertice
  points.push_back(Point_2(20000, 40000));
  points.push_back(Point_2(40000, 0));

  TSR_LOG_TRACE("Initializing for {:d} points", points.size());

  auto feature_water = create_binary_feature("water", points);

  ASSERT_EQ(feature_water->number_of_vertices(), 4);
}
