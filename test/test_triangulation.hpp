#include "tsr/DelaunayTriangulation.hpp"

#include "tsr/Logging.hpp"

#include "tsr/Point2.hpp"

#include "gtest/gtest.h"

#include "tsr/Router.hpp"

using namespace tsr;

TEST(TestDTM, test_initializeEmptyThrows) {
  // Create empty point set
  std::vector<Point3> empty_points;

  // Initialize empty DTM
  EXPECT_THROW(CreateTinFromPoints(empty_points), std::invalid_argument);
}

TEST(TestDTM, test_initalizeDoesNotThrow) {
  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 1));
  points.push_back(Point3(5, 0, 3));
  points.push_back(Point3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  ASSERT_NO_THROW(CreateTinFromPoints(points));
}

TEST(TestDTM, test_getTopology) {
  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 1));
  points.push_back(Point3(5, 0, 3));
  points.push_back(Point3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = CreateTinFromPoints(points);

  ASSERT_NO_THROW(dtm.number_of_vertices());
}

TEST(TestDTM, test_initalizeVertexCountMatchesDEM) {
  // Create point set
  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 1));
  points.push_back(Point3(5, 0, 3));
  points.push_back(Point3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = CreateTinFromPoints(points);

  TSR_LOG_TRACE("verifying vertex count");

  ASSERT_EQ(dtm.number_of_vertices(), points.size());
}

TEST(TestDTM, test_simplify_tin_tin) {
  // Create mesh
  // Create point set
  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 1));
  points.push_back(Point3(5, 0, 3));
  points.push_back(Point3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = CreateTinFromPoints(points);

  TSR_LOG_TRACE("simplifying dtm");

  // Simplify
  ASSERT_NO_THROW(SimplifyTin(dtm, dtm));

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

  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 0));
  points.push_back(Point3(1, 2, 0)); // Additional vertice
  points.push_back(Point3(2, 4, 0));
  points.push_back(Point3(4, 0, 0));

  auto dtm = CreateTinFromPoints(points);

  // Simplify
  SimplifyTin(dtm, dtm);

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

  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 0));
  points.push_back(Point3(0.9, 2, 0)); // Additional vertice
  points.push_back(Point3(2, 4, 0));
  points.push_back(Point3(4, 0, 0));

  auto dtm = CreateTinFromPoints(points);

  Tin dtm_simple;

  // Simplify, ensuring distance is not the limiting factor
  SimplifyTin(dtm, dtm_simple, DEFAULT_COSINE_MAX_ANGLE_REGIONS, 10000,
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

  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 0));
  points.push_back(Point3(0.9, 2, 0)); // Additional vertice
  points.push_back(Point3(2, 4, 0));
  points.push_back(Point3(4, 0, 0));

  auto dtm = CreateTinFromPoints(points);

  // Simplify, ensuring angle is not the limiting factor
  SimplifyTin(dtm, dtm, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
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

  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 0));
  points.push_back(Point3(1, 2, 100)); // Additional vertice
  points.push_back(Point3(2, 4, 0));
  points.push_back(Point3(4, 0, 0));

  auto dtm = CreateTinFromPoints(points);

  // Simplify, ensuring distance is not the limiting factor
  SimplifyTin(dtm, dtm, DEFAULT_COSINE_MAX_ANGLE_REGIONS, 10000,
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

  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 0));
  points.push_back(Point3(0, 20000, 0)); // Additional vertice
  points.push_back(Point3(20000, 40000, 0));
  points.push_back(Point3(40000, 0, 0));

  auto dtm = CreateTinFromPoints(points);

  // Simplify, ensuring angle is not the limiting factor
  SimplifyTin(dtm, dtm, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
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

  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 0));
  points.push_back(Point3(1, 2, 0)); // Additional vertice
  points.push_back(Point3(2, 4, 0));
  points.push_back(Point3(4, 0, 0));

  std::vector<Point2> contour;
  contour.push_back(Point2(0, 0));
  contour.push_back(Point2(3, 2));

  auto dtm = CreateTinFromPoints(points);

  // Add constraints
  AddContourConstraint(dtm, contour, 22);

  ASSERT_EQ(dtm.number_of_vertices(), 5);
  ASSERT_EQ(dtm.number_of_faces(), 3);
}

// TEST(TestRoute, testIOGPX) {
//   std::vector<Point3> points;
//   points.push_back(Point3(0, 0, 0));
//   points.push_back(Point3(1, 2, 0)); // Additional vertice
//   points.push_back(Point3(2, 4, 0));
//   points.push_back(Point3(4, 0, 0));

//   WriteDataToFile("test.gpx", FormatRouteAsGpx(points));
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

  std::vector<Point3> points;
  points.push_back(Point3(0, 0, 0));
  points.push_back(Point3(10, 20, 0)); // Additional vertice
  points.push_back(Point3(20, 40, 0));
  points.push_back(Point3(40, 0, 0));

  std::vector<Point2> contour;
  contour.push_back(Point2(0, 0));
  contour.push_back(Point2(30, 20));

  auto dtm = CreateTinFromPoints(points);

  // Add constraints
  AddContourConstraint(dtm, contour, 2);

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