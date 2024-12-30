#include <gtest/gtest.h>

#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/MapUtils.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/MeshUtils.hpp"
#include "tsr/PointProcessor.hpp"

#include "tsr/IO/MeshIO.hpp"

#include "tsr/Point_3.hpp"

using namespace tsr;

TEST(MeshUtilsTests, TestInitialize) {

  // Initialize mesh boundary
  Point_3 sourcePointWGS84(56.317649, -2.816415, 0);
  Point_3 targetPointWGS84(56.329492, -2.782974, 0);

  Point_3 sourcePoint = WGS84_point_to_UTM(sourcePointWGS84);
  Point_3 targetPoint = WGS84_point_to_UTM(targetPointWGS84);

  MeshBoundary boundary(sourcePoint, targetPoint, 1);
  auto cdt = initializeMesh(boundary, "0f789809fed28dc634c8d75695d0cc5c");

  Mesh mesh;
  convert_tin_to_surface_mesh(cdt, mesh);

  IO::write_mesh_to_obj("test.obj", mesh);

  SUCCEED();
}

TEST(MeshUtilsTest, TestStoreAndRetrieveMesh) {

  Delaunay_3 cdt;

  Point_3 a(0, 3, 5);
  Point_3 b(3, 1, 5);
  Point_3 c(9, 2, 5);
  Point_3 d(1, 9, 5);

  cdt.insert(a);
  cdt.insert(b);
  cdt.insert(c);
  cdt.insert(d);
  cdt.insert_constraint(a, c);

  std::string FILENAME = "./testStoreAndRetrieveMesh.mesh";

  IO::write_CDT_to_file(FILENAME, cdt);

  IO::write_CDT_to_file("./testSToreAndRetrieveMesh.mesh", cdt);

  Delaunay_3 loadCDT = IO::loadCDTFromFile(FILENAME);

  ASSERT_EQ(loadCDT.number_of_vertices(), 4);
  ASSERT_FALSE(loadCDT.constrained_edges().empty());
}