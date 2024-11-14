#include "tsr/DTM.hpp"
#include "tsr/IO/FileIOHandler.hpp"
#include "tsr/IO/ImageProcesor.hpp"
#include "tsr/logging.hpp"

#include "tsr/Point_2.hpp"

#include "gtest/gtest.h"

#include <memory>

using namespace tsr;
using namespace tsr::IO;

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

TEST(TestDTM, test_simplify_mesh_tin) {
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
  ASSERT_NO_THROW(simplify_mesh(dtm, dtm));

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
  simplify_mesh(dtm, dtm);

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
  simplify_mesh(dtm, dtm_simple, DEFAULT_COSINE_MAX_ANGLE_REGIONS, 10000,
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
  simplify_mesh(dtm, dtm, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
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
  simplify_mesh(dtm, dtm, DEFAULT_COSINE_MAX_ANGLE_REGIONS, 10000,
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
  simplify_mesh(dtm, dtm, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
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

  /** DEBUG BEGIN
      TODO: Delete writing test to obj file
  */
  Surface_mesh surface_mesh;
  convert_tin_to_surface_mesh(dtm, surface_mesh);
  /** DEBUG END */

  write_mesh_to_obj("test.obj", surface_mesh);

  // sqrt(300^2 + 200^2) = 360.555...
  // 360.555... / 20 - 1 = 17 additional vertices

  ASSERT_EQ(dtm.number_of_vertices(), 22);
}

TEST(TestDTM, DISABLED_testRealConstraints) {

  TSR_LOG_TRACE("Loading points");
  auto points = load_dem_from_file("../data/benNevis_DEM.xyz");

  TSR_LOG_TRACE("point count: {}", points->size());

  // TSR_LOG_TRACE("Smoothing");
  // jet_smooth_points(*points);

  TSR_LOG_TRACE("Denoising");
  denoise_points(*points);

  TSR_LOG_TRACE("Simplifying");
  simplify_points(*points);

  TSR_LOG_TRACE("point count: {}", points->size());

  TSR_LOG_TRACE("Triangulating");
  auto dtm = create_tin_from_points(*points);

  TSR_LOG_TRACE("Simplifying");
  simplify_mesh(dtm, dtm);

  TSR_LOG_TRACE("Getting water contours");
  auto water_image = load_image_from_file("../data/benNevis_water.tiff");

  auto water_feature = extract_feature_contours(*water_image, 0.001, 369953.765,
                                                6304359.542, 3, 3);

  TSR_LOG_TRACE("Adding {} constraints", water_feature->size());
  for (auto constraint : *water_feature) {
    add_contour_constraint(dtm, constraint, 22);
  }

  TSR_LOG_TRACE("Getting terrain type contours");
  auto terrain_types_image =
      load_image_from_file("../data/benNevis_terrain_types.tiff");
  auto terrain_type_feature = extract_feature_contours(
      *terrain_types_image, 0.03, 369953.765, 6304359.542, 10, 10);

  TSR_LOG_TRACE("Adding {} constraints", terrain_type_feature->size());
  for (auto constraint : *terrain_type_feature) {
    add_contour_constraint(dtm, constraint, 22);
  }

  TSR_LOG_INFO("final vertices: {}", dtm.number_of_vertices());

  TSR_LOG_TRACE("Writing to obj file");
  Surface_mesh surface_mesh;
  convert_tin_to_surface_mesh(dtm, surface_mesh);
  write_mesh_to_obj("testBigConstraints.obj", surface_mesh);
}