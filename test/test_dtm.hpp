
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
  EXPECT_THROW(make_unique<DTM>(empty_points), std::invalid_argument);
}

TEST(TestDTM, test_initalizeDoesNotThrow) {
  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 1));
  points.push_back(Point_3(5, 0, 3));
  points.push_back(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  ASSERT_NO_THROW(make_unique<DTM>(points));
}

TEST(TestDTM, test_getTopology) {
  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 1));
  points.push_back(Point_3(5, 0, 3));
  points.push_back(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = make_unique<DTM>(points);

  ASSERT_NO_THROW(dtm->get_mesh().number_of_vertices());
}

TEST(TestDTM, test_initalizeVertexCountMatchesDEM) {
  // Create point set
  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 1));
  points.push_back(Point_3(5, 0, 3));
  points.push_back(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = make_unique<DTM>(points);

  TSR_LOG_TRACE("verifying vertex count");

  ASSERT_EQ(dtm->get_mesh().number_of_vertices(), points.size());
}

TEST(TestDTM, test_simplify_mesh_tin) {
  // Create mesh
  // Create point set
  std::vector<Point_3> points;
  points.push_back(Point_3(0, 0, 1));
  points.push_back(Point_3(5, 0, 3));
  points.push_back(Point_3(2.5, 5, 2));

  TSR_LOG_TRACE("constructing dtm");

  auto dtm = make_unique<DTM>(points);

  TSR_LOG_TRACE("simplifying dtm");

  // Simplify
  Delaunay_3 &mesh = dtm->get_mesh();
  ASSERT_NO_THROW(simplify_mesh(mesh, mesh));

  ASSERT_TRUE(dtm->get_mesh().is_valid());
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

  auto dtm = make_unique<DTM>(points);

  Delaunay_3 &meshPtr = dtm->get_mesh();

  // Simplify
  simplify_mesh(meshPtr, meshPtr);

  ASSERT_EQ(meshPtr.number_of_vertices(), 3);
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

  auto dtm = make_unique<DTM>(points);

  Delaunay_3 &meshPtr = dtm->get_mesh();

  Delaunay_3 mesh2;

  // Simplify, ensuring distance is not the limiting factor
  simplify_mesh(meshPtr, mesh2, DEFAULT_COSINE_MAX_ANGLE_REGIONS,
                           10000, DEFAULT_COSINE_MAX_ANGLE_CORNERS, 10000);

  ASSERT_EQ(mesh2.number_of_vertices(), 3);
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

  auto dtm = make_unique<DTM>(points);

  Delaunay_3 &meshPtr = dtm->get_mesh();

  // Simplify, ensuring angle is not the limiting factor
  simplify_mesh(meshPtr, meshPtr, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
                           DEFAULT_MAX_DISTANCE_CORNERS);

  ASSERT_EQ(meshPtr.number_of_vertices(), 3);
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

  auto dtm = make_unique<DTM>(points);

  Delaunay_3 &meshPtr = dtm->get_mesh();

  // Simplify, ensuring distance is not the limiting factor
  simplify_mesh(meshPtr, meshPtr, DEFAULT_COSINE_MAX_ANGLE_REGIONS,
                           10000, DEFAULT_COSINE_MAX_ANGLE_CORNERS, 10000);

  ASSERT_EQ(meshPtr.number_of_vertices(), 4);
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

  auto dtm = make_unique<DTM>(points);

  Delaunay_3 &meshPtr = dtm->get_mesh();

  // Simplify, ensuring angle is not the limiting factor
  simplify_mesh(meshPtr, meshPtr, 0, DEFAULT_MAX_DISTANCE_REGIONS, 0,
                           DEFAULT_MAX_DISTANCE_CORNERS);

  ASSERT_EQ(meshPtr.number_of_vertices(), 4);
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
  contour.push_back(Point_2(0,0));
  contour.push_back(Point_2(3, 2));

  auto dtm = make_unique<DTM>(points);

  // Add constraints
  dtm->add_contour_constraint(contour, 22);

  ASSERT_EQ(dtm->get_mesh().number_of_vertices(), 5);
  ASSERT_EQ(dtm->get_mesh().number_of_faces(), 3);
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
  contour.push_back(Point_2(0,0));
  contour.push_back(Point_2(30, 20));

  auto dtm = make_unique<DTM>(points);

  // Add constraints
  dtm->add_contour_constraint(contour, 2);
  

  /** DEBUG BEGIN 
      TODO: Delete writing test to obj file
  */
  Surface_mesh surface_mesh;
  convert_tin_to_surface_mesh(dtm->get_mesh(), surface_mesh);
  /** DEBUG END */

  write_mesh_to_obj("test.obj", surface_mesh);

  // sqrt(300^2 + 200^2) = 360.555...
  // 360.555... / 20 - 1 = 17 additional vertices

  ASSERT_EQ(dtm->get_mesh().number_of_vertices(), 22);
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
  auto dtm = std::make_unique<DTM>(*points);

  TSR_LOG_TRACE("Simplifying");
  simplify_mesh(dtm->get_mesh(), dtm->get_mesh());
  

  TSR_LOG_TRACE("Getting water contours");
  auto water_image = load_image_from_file("../data/benNevis_water.tiff");

  auto water_feature = extract_feature_contours(*water_image, 0.001,-5.1273611, 56.8648611, 3, 3);

  TSR_LOG_TRACE("Adding {} constraints", water_feature->size());
  for (auto constraint : *water_feature) {
    dtm->add_contour_constraint(constraint, 0.001);
  }

  TSR_LOG_TRACE("Getting terrain type contours");
  auto terrain_types_image = load_image_from_file("../data/benNevis_terrain_types.tiff");
  auto terrain_type_feature = extract_feature_contours(*terrain_types_image, 0.03, -5.1273611, 56.8648611, 10, 10);

  TSR_LOG_TRACE("Adding {} constraints", terrain_type_feature->size());
  for (auto constraint : *terrain_type_feature) {
    dtm->add_contour_constraint(constraint, 22);
  }

  TSR_LOG_INFO("final vertices: {}", dtm->get_mesh().number_of_vertices());

  TSR_LOG_TRACE("Writing to obj file");
  Surface_mesh surface_mesh;
  convert_tin_to_surface_mesh(dtm->get_mesh(), surface_mesh);
  write_mesh_to_obj("testBigConstraints.obj", surface_mesh);

}