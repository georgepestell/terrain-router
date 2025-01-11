#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/Features/BoolWaterFeature.hpp"
#include "tsr/Features/CEHTerrainFeature.hpp"
#include "tsr/Features/PathFeature.hpp"
#include "tsr/Features/SimpleBooleanFeature.hpp"
#include "tsr/Features/SimpleBooleanToDoubleFeature.hpp"
#include "tsr/GeometryUtils.hpp"
#include "tsr/IO/MeshIO.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point3.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Tin.hpp"
#include <gdal/gdal.h>
#include <gtest/gtest.h>

// DEBUG
#include "tsr/IO/MapIO.hpp"
#include "tsr/TsrState.hpp"

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
  TsrState state;
  ASSERT_FALSE(boolFeature->Calculate(state));
}

TEST(testFeature, testSimpleBoolToIntegerFeature) {

  auto boolFeature = std::make_shared<SimpleBooleanFeature>("Feature", false);

  double pos_value = 10;
  double neg_value = -10;

  auto boolToIntFeature = std::make_shared<SimpleBooleanToDoubleFeature>(
      "BOOL_TO_DOUBLE", pos_value, neg_value);

  boolToIntFeature->AddDependency(boolFeature);

  TsrState state;
  ASSERT_EQ(boolToIntFeature->Calculate(state), neg_value);
}

TEST(TestFeature, testCEHFeatureInitialization) {

  Point3 src(56.317649, -2.816415, 0);
  Point3 tgt(56.329492, -2.782974, 0);

  auto srcUTM = WGS84_point_to_UTM(src);
  auto tgtUTM = WGS84_point_to_UTM(tgt);

  MeshBoundary boundary(srcUTM, tgtUTM, 1);

  Tin tin =
      InitializeTinFromBoundary(boundary, "0f789809fed28dc634c8d75695d0cc5c");

  CEHTerrainFeature ceh = CEHTerrainFeature("CEH", 0.1);
  ceh.Initialize(tin, boundary);

  BoolWaterFeature waterFeature = BoolWaterFeature("WATER", 0.1);
  waterFeature.Initialize(tin, boundary);

  PathFeature pathFeature = PathFeature("paths", 0.1);
  pathFeature.Initialize(tin, boundary);

  SurfaceMesh mesh;
  convertTINToMesh(tin, mesh);

  IO::write_mesh_to_obj("test_featureMesh.obj", mesh);

  // ceh.Tag(dtm);
}