#include "tsr/DelaunayTriangulation.hpp"

#include "tsr/logging.hpp"

#include "tsr/Point_2.hpp"

#include "gtest/gtest.h"

#include "tsr/Router.hpp"

using namespace tsr;

TEST(TestDTM, test_initializeEmptyThrows) {
  // Create empty point set
  std::vector<Point_3> empty_points;

  // Initialize empty DTM
  EXPECT_THROW(create_tin_from_points(empty_points), std::invalid_argument);
}

TEST(TestDTM, test_initalizeDoesNotThrow) {
  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 1));
  points.push_back(Point_3(5, 0, 3));
  points.push_back(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  ASSERT_NO_THROW(create_tin_from_points(points));
}

TEST(TestDTM, test_getTopology) {
  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 1));
  points.push_back(Point_3(5, 0, 3));
  points.push_back(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = create_tin_from_points(points);

  ASSERT_NO_THROW(dtm.number_of_vertices());
}

TEST(TestDTM, test_initalizeVertexCountMatchesDEM) {
  // Create point set
  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 1));
  points.push_back(Point_3(5, 0, 3));
  points.push_back(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = create_tin_from_points(points);

  TSR_LOG_TRACE("verifying vertex count");

  ASSERT_EQ(dtm.number_of_vertices(), points.size());
}

TEST(TestDTM, test_simplify_tin_tin) {
  // Create mesh
  // Create point set
  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 1));
  points.push_back(Point_3(5, 0, 3));
  points.push_back(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = create_tin_from_points(points);

  TSR_LOG_TRACE("simplifying dtm");

  // Simplify
  ASSERT_NO_THROW(simplify_tin(dtm, dtm));

  ASSERT_TRUE(dtm.is_valid());
}

TEST(TestDTM, test_simplify_tin_mesh_flat_plane) {

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

  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 0));
  points.push_back(Point_3(1, 2, 0)); // Additional vertice
  points.push_back(Point_3(2, 4, 0));
  points.push_back(Point_3(4, 0, 0));

  auto dtm = create_tin_from_points(points);

  // Simplify
  simplify_tin(dtm, dtm);

  ASSERT_EQ(dtm.number_of_vertices(), 3);
}

TEST(TestDTM, test_simplify_tin_mesh_small_angles) {

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

  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 0));
  points.push_back(Point_3(0.9, 2, 0)); // Additional vertice
  points.push_back(Point_3(2, 4, 0));
  points.push_back(Point_3(4, 0, 0));

  auto dtm = create_tin_from_points(points);

  Delaunay_3 dtm_simple;

  // Simplify, ensuring distance is not the limiting factor
  simplify_tin(dtm, dtm_simple, DEFAULT_COSINE_MAX_ANGLE_REGIONS, 10000,
               DEFAULT_COSINE_MAX_ANGLE_CORNERS, 10000);

  ASSERT_EQ(dtm_simple.number_of_vertices(), 3);
}

TEST(TestDTM, test_simplify_tin_mesh_small_distances) {

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

  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 0));
  points.push_back(Point_3(0.9, 2, 0)); // Additional vertice
  points.push_back(Point_3(2, 4, 0));
  points.push_back(Point_3(4, 0, 0));

  auto dtm = create_tin_from_points(points);

  // Simplify, ensuring angle is not the limiting factor
  simplify_tin(dtm, dtm, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
               DEFAULT_MAX_DISTANCE_CORNERS);

  ASSERT_EQ(dtm.number_of_vertices(), 3);
}

TEST(TestDTM, test_simplify_tin_mesh_keeps_sharp_edges) {

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

  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 0));
  points.push_back(Point_3(1, 2, 100)); // Additional vertice
  points.push_back(Point_3(2, 4, 0));
  points.push_back(Point_3(4, 0, 0));

  auto dtm = create_tin_from_points(points);

  // Simplify, ensuring distance is not the limiting factor
  simplify_tin(dtm, dtm, DEFAULT_COSINE_MAX_ANGLE_REGIONS, 10000,
               DEFAULT_COSINE_MAX_ANGLE_CORNERS, 10000);

  ASSERT_EQ(dtm.number_of_vertices(), 4);
}

TEST(TestDTM, test_simplify_tin_mesh_keeps_long_distances) {

  /**
   * @brief Creates a flat plane with an extra vertice along an edge which
   * should be removed upon simplification
   *
   *    _*
   *   /   \
   *  *     \
   *  |      \
   *  *- - - -*
   *
   */

  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 0));
  points.push_back(Point_3(0, 20000, 0)); // Additional vertice
  points.push_back(Point_3(20000, 40000, 0));
  points.push_back(Point_3(40000, 0, 0));

  auto dtm = create_tin_from_points(points);

  // Simplify, ensuring angle is not the limiting factor
  simplify_tin(dtm, dtm, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
               DEFAULT_MAX_DISTANCE_CORNERS);

  ASSERT_EQ(dtm.number_of_vertices(), 4);
}

TEST(TestDTM, testConstraintAdd) {

  /**
   * @brief Creates a flat plane with an extra vertice along an edge which
   * should be removed upon simplification
   *
   *      *
   *     / \
   *    * _-\
   *   /-    \
   *  *- - - -*
   *
   */

  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 0));
  points.push_back(Point_3(1, 2, 0)); // Additional vertice
  points.push_back(Point_3(2, 4, 0));
  points.push_back(Point_3(4, 0, 0));

  std::vector<Point_2> contour;
  contour.push_back(Point_2(0, 0));
  contour.push_back(Point_2(3, 2));

  auto dtm = create_tin_from_points(points);

  // Add constraints
  add_contour_constraint(dtm, contour, 22);

  ASSERT_EQ(dtm.number_of_vertices(), 5);
  ASSERT_EQ(dtm.number_of_faces(), 3);
}

// TEST(TestRoute, testIOGPX) {
//   std::vector<Point_3> points;
//   points.push_back(Point_3(0, 0, 0));
//   points.push_back(Point_3(1, 2, 0)); // Additional vertice
//   points.push_back(Point_3(2, 4, 0));
//   points.push_back(Point_3(4, 0, 0));

//   write_data_to_file("test.gpx", formatPointsAsGPXRoute(points));
// }

TEST(TestDTM, testConstraintAddSplit) {

  /**
   * @brief Creates a flat plane with an extra vertice along an edge which
   * should be removed upon simplification
   *
   *      *
   *     / \
   *    * _-\
   *   /-    \
   *  *- - - -*
   *
   */

  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 0));
  points.push_back(Point_3(10, 20, 0)); // Additional vertice
  points.push_back(Point_3(20, 40, 0));
  points.push_back(Point_3(40, 0, 0));

  std::vector<Point_2> contour;
  contour.push_back(Point_2(0, 0));
  contour.push_back(Point_2(30, 20));

  auto dtm = create_tin_from_points(points);

  // Add constraints
  add_contour_constraint(dtm, contour, 2);

  /**
   * distance  = sqrt(30^2 + 20^2) = 36.05...
   *
   * splits    = (36.05... / 2) - 1 = 17.02...
   *           = 18 additional vertices
   *
   * verticies = 18 + 4
   *           = 22
   */

  ASSERT_EQ(dtm.number_of_vertices(), 22);
}