#include <gtest/gtest.h>

#include "tsr/DTM.hpp"
#include "tsr/logging.hpp"

#include "tsr/Delaunay_3.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/Router.hpp"

#include "tsr/IO/FileIOHandler.hpp"

#include <vector>

using namespace tsr;

TEST(TestRouter, routerInitiailizeTest) {

  /**

  () -- () -- ()
    \  /  \  /  \
     () -- () -- ()

   */

  std::vector<Point_3> points;

  points.push_back(Point_3(0, 5, 0));
  points.push_back(Point_3(5, 0, 0));
  points.push_back(Point_3(10, 5, 0));
  points.push_back(Point_3(15, 0, 0));
  points.push_back(Point_3(20, 5, 0));
  points.push_back(Point_3(25, 0, 0));

  Delaunay_3 tin;
  tin.insert(points.begin(), points.end());

  TSR_LOG_TRACE("Initializing TIN");
  Router router(tin);

  // Get the vertex handle of a point
  Point_3 start_point(0, 5, 0);
  Point_3 end_point(25, 0, 0);
  auto start_face = tin.locate(start_point);
  auto end_face = tin.locate(end_point);

  Vertex_handle start_vertex = start_face->vertex(0);
  Vertex_handle end_vertex = end_face->vertex(1);

  TSR_LOG_TRACE("START: {}, {}, {}", start_vertex->point().x(),
                start_vertex->point().y(), start_vertex->point().z());
  TSR_LOG_TRACE("END: {}, {}, {}", end_vertex->point().x(),
                end_vertex->point().y(), end_vertex->point().z());

  TSR_LOG_TRACE("Calculating route");
  router.calculate_route(start_vertex, end_vertex);

  Surface_mesh mesh;
  convert_tin_to_surface_mesh(tin, mesh);

  IO::write_mesh_to_obj("testingRoute.obj", mesh);
}