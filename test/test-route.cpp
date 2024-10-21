

#include "tsr/DTM.hpp"
#include "tsr/IO.hpp"
#include "gtest/gtest.h"
#include <CGAL/Segment_2.h>

#ifndef TSR_DEBUG
#define TSR_DEBUG
#endif

#include "tsr/logging.hpp"


using namespace tsr;

class Environment : public ::testing::Environment {
  public:
    void SetUp() override {
      log_set_global_loglevel(LogLevel::TRACE);
      log_set_global_logstream(LogStream::STDERR);    
    }
};

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
   *    _*
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

TEST(TestDTM, testBinaryFeatureEdgeCount) {

  auto points_3d = load_points_from_file("../data/benNevis_filtered_water.xyz"); 

  vector<Point_2> points;
  for (auto p : points_3d.points()) {
    points.push_back(Point_2(p.x(), p.y()));
  }


  TSR_LOG_TRACE("Initializing for {:d} points", points.size());

  auto feature_water = create_binary_feature("water", points);

  TSR_LOG_TRACE("vertices: {:d}", feature_water->number_of_vertices());
  TSR_LOG_TRACE("edges {:d} points", feature_water->all_edges().size());

  using Segment_2 = CGAL::Segment_2<Kernel>;

  vector<Segment_2> good_edges;
  for(auto edge : feature_water->finite_edges()) {
    Segment_2 s = feature_water->segment(edge);
    const Point_2 &p1 = s.point(0);
    const Point_2 &p2 = s.point(1);

    const double height = p1.y() - p2.y();
    const double width = p1.x() - p2.x();

    const double length = pow(height, 2) + pow(width, 2);

    const double maxLength = pow(32, 2);

    if (length <= maxLength) {
      good_edges.push_back(s);
    }
  }

  TSR_LOG_INFO("edges between water: {}", good_edges.size());

}

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    // gtest takes ownership of the TestEnvironment ptr - we don't delete it.
    ::testing::AddGlobalTestEnvironment(new Environment);
    return RUN_ALL_TESTS();
}