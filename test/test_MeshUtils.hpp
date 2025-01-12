#include <gtest/gtest.h>

#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/GeometryUtils.hpp"
#include "tsr/MapUtils.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/PointProcessor.hpp"

#include "tsr/IO/MeshIO.hpp"

#include "tsr/Point3.hpp"

using namespace tsr;

TEST(MeshUtilsTests, TestInitialize) {

  // Initialize mesh boundary
  Point3 sourcePointWGS84(56.317649, -2.816415, 0);
  Point3 targetPointWGS84(56.329492, -2.782974, 0);

  Point3 sourcePoint = TranslateWgs84PointToUtm(sourcePointWGS84);
  Point3 targetPoint = TranslateWgs84PointToUtm(targetPointWGS84);

  MeshBoundary boundary(sourcePoint, targetPoint, 1);
  auto tin =
      InitializeTinFromBoundary(boundary, "0f789809fed28dc634c8d75695d0cc5c");

  SurfaceMesh mesh;
  ConvertTinToSurfaceMesh(tin, mesh);

  IO::WriteMeshToObj("test.obj", mesh);

  SUCCEED();
}

TEST(MeshUtilsTest, TestStoreAndRetrieveMesh) {

  Tin tin;

  Point3 a(0, 3, 5);
  Point3 b(3, 1, 5);
  Point3 c(9, 2, 5);
  Point3 d(1, 9, 5);

  tin.insert(a);
  tin.insert(b);
  tin.insert(c);
  tin.insert(d);
  tin.insert_constraint(a, c);

  std::string FILENAME = "./testStoreAndRetrieveMesh.mesh";

  IO::WriteTinToFile(FILENAME, tin);

  IO::WriteTinToFile("./testSToreAndRetrieveMesh.mesh", tin);

  Tin loadCDT = IO::loadCDTFromFile(FILENAME);

  ASSERT_EQ(loadCDT.number_of_vertices(), 4);
  ASSERT_FALSE(loadCDT.constrained_edges().empty());
}