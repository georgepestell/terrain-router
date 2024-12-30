#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/Features/CEHTerrainFeature.hpp"
#include "tsr/Features/PathFeature.hpp"
#include "tsr/Features/SimpleBooleanFeature.hpp"
#include "tsr/Features/SimpleBooleanToDoubleFeature.hpp"
#include "tsr/Features/WaterFeature.hpp"
#include "tsr/IO/MeshIO.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/MeshUtils.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Point_3.hpp"
#include <gdal/gdal.h>
#include <gtest/gtest.h>

// DEBUG
#include "tsr/IO/MapIO.hpp"
#include "tsr/TSRState.hpp"

#include <memory>

using namespace tsr;

TEST(testFeature, testFeatureIsEqualsFalse) {
  SimpleBooleanFeature feature1("feature1", true);
  SimpleBooleanFeature feature2("feature2", true);
  ASSERT_NE(feature1, feature2);
}

TEST(testFeature, testFeatureIsEqualsSameObject) {
  SimpleBooleanFeature feature1("feature1", true);
  ASSERT_EQ(feature1, feature1);
}
TEST(testFeature, testFeatureIsEqualsSameName) {
  SimpleBooleanFeature feature1("feature1", true);
  SimpleBooleanFeature feature2("feature1", false);
  ASSERT_EQ(feature1, feature2);
}

TEST(testFeature, testSimpleBooleanFeature) {

  auto boolFeature =
      std::make_shared<SimpleBooleanFeature>("SimpleBoolFeature", false);

  Face_handle tmpFace;
  TSRState state;
  ASSERT_FALSE(boolFeature->calculate(state));
}

TEST(testFeature, testSimpleBoolToIntegerFeature) {

  auto boolFeature = std::make_shared<SimpleBooleanFeature>("Feature", false);

  double pos_value = 10;
  double neg_value = -10;

  auto boolToIntFeature = std::make_shared<SimpleBooleanToDoubleFeature>(
      "BOOL_TO_DOUBLE", pos_value, neg_value);

  boolToIntFeature->add_dependency(boolFeature);

  TSRState state;
  ASSERT_EQ(boolToIntFeature->calculate(state), neg_value);
}

TEST(TestFeature, testCEHFeatureInitialization) {

  Point_3 src(56.317649, -2.816415, 0);
  Point_3 tgt(56.329492, -2.782974, 0);

  auto srcUTM = WGS84_point_to_UTM(src);
  auto tgtUTM = WGS84_point_to_UTM(tgt);

  MeshBoundary boundary(srcUTM, tgtUTM, 1);

  Delaunay_3 cdt = initializeMesh(boundary, "0f789809fed28dc634c8d75695d0cc5c");

  CEHTerrainFeature ceh = CEHTerrainFeature("CEH", 0.1);
  ceh.initialize(cdt, boundary);

  BoolWaterFeature waterFeature = BoolWaterFeature("WATER", 0.1);
  waterFeature.initialize(cdt, boundary);

  PathFeature pathFeature = PathFeature("paths", 0.1);
  pathFeature.initialize(cdt, boundary);

  Mesh mesh;
  convert_tin_to_surface_mesh(cdt, mesh);

  IO::write_mesh_to_obj("test_featureMesh.obj", mesh);

  // ceh.tag(dtm);
}