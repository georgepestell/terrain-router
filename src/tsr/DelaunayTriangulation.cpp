#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/Mesh.hpp"
#include "tsr/MeshUtils.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/Vector_3.hpp"
#include "tsr/logging.hpp"

#include <CGAL/Kernel/global_functions_3.h>
#include <CGAL/Named_function_parameters.h>
#include <CGAL/Polygon_mesh_processing/region_growing.h>
#include <CGAL/Polygon_mesh_processing/remesh_planar_patches.h>
#include <CGAL/Polygon_mesh_processing/repair.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/boost/graph/graph_traits_Constrained_Delaunay_triangulation_2.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/grid_simplify_point_set.h>
#include <CGAL/hierarchy_simplify_point_set.h>
#include <CGAL/jet_smooth_point_set.h>
#include <CGAL/property_map.h>
#include <CGAL/tags.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>

#include <boost/property_map/vector_property_map.hpp>
#include <cmath>
#include <cstddef>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <sys/types.h>
#include <vector>

namespace tsr {

void convert_surface_mesh_to_tin(Mesh const &source, Delaunay_3 &target) {
  for (auto v : source.vertices()) {
    target.insert(source.point(v));
  }
}

double calculateAngle(const Point_3 &p1, const Point_3 &p2) {
  return std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
}

// Function to calculate distance between two points
double calculateDistance(const Point_3 &p1, const Point_3 &p2) {
  return std::sqrt(std::pow(p2.x() - p1.x(), 2) + std::pow(p2.y() - p1.y(), 2));
}

// Function to rotate a point around a center by an angle
Point_3 rotatePoint(const Point_3 &center, const Point_3 &p, double angle) {
  double s = std::sin(angle);
  double c = std::cos(angle);

  // Translate point to origin
  double x = p.x() - center.x();
  double y = p.y() - center.y();

  // Rotate point
  double newX = x * c - y * s;
  double newY = x * s + y * c;

  // Translate back
  return Point_3(newX + center.x(), newY + center.y(), p.z());
}

void convert_tin_to_surface_mesh(Delaunay_3 const &source, Mesh &target) {
  CGAL::copy_face_graph(source, target);
}

// Function to check if a point is inside a rectangle
bool isPointInRectangle(const Point_3 &center, double width, double height,
                        double angle, const Point_3 &point) {
  // Rotate the point back (inverse rotation)
  Point_3 rotatedPoint = rotatePoint(center, point, -angle);

  // Check if the rotated point is within the axis-aligned rectangle
  double halfWidth = width / 2.0;
  double halfHeight = height / 2.0;
  return (rotatedPoint.x() >= center.x() - halfWidth &&
          rotatedPoint.x() <= center.x() + halfWidth &&
          rotatedPoint.y() >= center.y() - halfHeight &&
          rotatedPoint.y() <= center.y() + halfHeight);
}

Delaunay_3 create_tin_from_points(std::vector<Point_3> &points,
                                                   Point_3 source_point,
                                                   Point_3 target_point,
                                                   double radiiMultiplier) {
  TSR_LOG_TRACE("creating TIN from point cloud");

  // validate point set contains points
  if (points.empty()) {
    TSR_LOG_WARN("TIN point vector is empty");
    throw std::invalid_argument("TIN point vector is empty");
  }

  // triangulate points using 2.5D Delaunay triangulation
  Delaunay_3 tin;

  // Calculate domain boundary

  double distance = calculateDistance(source_point, target_point);
  Point_3 midpoint = Point_3((source_point.x() + target_point.x()) / 2.0,
                             (source_point.y() + target_point.y()) / 2.0, 0);

  double radius = (distance / 2.0) * radiiMultiplier;

  double angle = calculateAngle(source_point, target_point);

  double boundaryWidth = distance + 2 * radius;
  double boundaryHeight = 2 * radius;

  // Add points inside boundary
  int d = 0;
  for (auto p : points) {
    bool inRectangle =
        isPointInRectangle(midpoint, boundaryWidth, boundaryHeight, angle, p);
    if (inRectangle) {
      tin.insert(p);
    } else {
      d++;
    }
  }

  TSR_LOG_TRACE("Discarded {} points", d);

  // validate triangulated mesh
  if (!tin.is_valid()) {
    TSR_LOG_ERROR("initalized DTM invalid");
    throw std::runtime_error("DTM Delaunay Triangulated mesh invalid");
  }

  TSR_LOG_TRACE("TIN initialized with {:d} vertices",
                tin.number_of_vertices());

  return tin;
}

void add_contour_constraint(Delaunay_3 &dtm, std::vector<Point_2> contour,
                            double max_segment_length) {

  for (auto vertexIt = contour.begin(); vertexIt != contour.end(); ++vertexIt) {
    auto vertexNextIt = std::next(vertexIt);
    if (vertexNextIt == contour.end())
      break;

    const double x = vertexIt->x();
    const double y = vertexIt->y();

    Delaunay_3::Face_handle vertexFace = dtm.locate(Point_3(x, y, 0));

    if (vertexFace == nullptr || dtm.is_infinite(vertexFace)) {
      // TSR_LOG_WARN("Point outside boundary x: {} y: {}", x, y);
      continue;
    }

    double z = interpolate_z(vertexFace->vertex(0)->point(),
                             vertexFace->vertex(1)->point(),
                             vertexFace->vertex(2)->point(), x, y);

    Point_3 vertex(vertexIt->x(), vertexIt->y(), z);

    const double next_x = vertexNextIt->x();
    const double next_y = vertexNextIt->y();

    Delaunay_3::Face_handle vertexNextFace =
        dtm.locate(Point_3(next_x, next_y, 0));

    if (vertexNextFace == nullptr || dtm.is_infinite(vertexNextFace)) {
      // TSR_LOG_WARN("Point outside boundary x: {} y: {}", next_x, next_y);
      continue;
    }

    double next_z = interpolate_z(
        vertexNextFace->vertex(0)->point(), vertexNextFace->vertex(1)->point(),
        vertexNextFace->vertex(2)->point(), next_x, next_y);

    Point_3 vertexPoint(x, y, z);
    Point_3 vertexNextPoint(next_x, next_y, next_z);

    // Calculate the Euclidean distance in the XY plane
    double dx = next_x - x;
    double dy = next_y - y;
    double length = sqrt(dx * dx + dy * dy);

    if (length > max_segment_length) {
      // Calculate the number of splits required
      double splits = floor(length / max_segment_length);

      std::vector<Point_3> splitPoints;
      for (double split = 1; split <= splits; split++) {
        double split_x = round(x + (dx / splits) * split);
        double split_y = round(y + (dy / splits) * split);

        Delaunay_3::Face_handle vertexSplitFace =
            dtm.locate(Point_3(split_x, split_y, 0));

        if (dtm.is_infinite(vertexSplitFace)) {
          // TSR_LOG_ERROR("Point outside boundary x: {} y: {}", split_x,
          // split_y);
          return;
        }

        double split_z = interpolate_z(vertexSplitFace->vertex(0)->point(),
                                       vertexSplitFace->vertex(1)->point(),
                                       vertexSplitFace->vertex(2)->point(),
                                       split_x, split_y);
        splitPoints.push_back(Point_3(split_x, split_y, split_z));
      }

      // Add the constraints
      dtm.insert_constraint(vertexPoint, splitPoints[0]);
      ushort splitIndex;
      for (splitIndex = 1; splitIndex < splitPoints.size(); splitIndex++) {
        dtm.insert_constraint(splitPoints[splitIndex - 1],
                              splitPoints[splitIndex]);
      }
      dtm.insert_constraint(splitPoints[splitIndex - 1], vertexNextPoint);

    } else {
      dtm.insert_constraint(vertexPoint, vertexNextPoint);
    }
  }
}

void simplify_tin(Delaunay_3 const &source_mesh, Delaunay_3 &target_mesh,
                  float cosine_max_angle_regions, float max_distance_regions,
                  float cosine_max_angle_corners, float max_distance_corners) {

  // Convert the mesh to a Surface_Mesh
  Mesh sourceSurfaceMesh;
  convert_tin_to_surface_mesh(source_mesh, sourceSurfaceMesh);

  if (!CGAL::is_valid_polygon_mesh(sourceSurfaceMesh)) {
    TSR_LOG_ERROR("Source mesh is invalid!");
    return;
  }

  // Required simplification information
  std::vector<size_t> regionIdMap(CGAL::num_faces(sourceSurfaceMesh));
  std::vector<size_t> cornerIdMap(CGAL::num_vertices(sourceSurfaceMesh), -1);
  std::vector<bool> ecm(CGAL::num_edges(sourceSurfaceMesh), false);
  boost::vector_property_map<CGAL::Epick::Vector_3> normalMap;

  TSR_LOG_TRACE("detecting mesh regions for simplification");

  size_t nbRegions =
      CGAL::Polygon_mesh_processing::region_growing_of_planes_on_faces(
          sourceSurfaceMesh, CGAL::make_random_access_property_map(regionIdMap),
          CGAL::parameters::cosine_of_maximum_angle(cosine_max_angle_regions)
              .region_primitive_map(normalMap)
              .maximum_distance(max_distance_regions));
  TSR_LOG_TRACE("Detected {} regions", nbRegions);

  TSR_LOG_TRACE("detecting mesh corners for simplification");

  size_t nbCorners = CGAL::Polygon_mesh_processing::detect_corners_of_regions(
      sourceSurfaceMesh, CGAL::make_random_access_property_map(regionIdMap),
      nbRegions, CGAL::make_random_access_property_map(cornerIdMap),
      CGAL::parameters::cosine_of_maximum_angle(cosine_max_angle_corners)
          .maximum_distance(max_distance_corners)
          .edge_is_constrained_map(CGAL::make_random_access_property_map(ecm)));
  TSR_LOG_TRACE("Detected {} corners", nbCorners);

  Mesh targetSurfaceMesh;
  try {
    TSR_LOG_TRACE("simplifying mesh");
    CGAL::Polygon_mesh_processing::remesh_almost_planar_patches(
        sourceSurfaceMesh, targetSurfaceMesh, nbRegions, nbCorners,
        CGAL::make_random_access_property_map(regionIdMap),
        CGAL::make_random_access_property_map(cornerIdMap),
        CGAL::make_random_access_property_map(ecm),
        CGAL::parameters::patch_normal_map(normalMap));
    TSR_LOG_TRACE("Simplified mesh has {} vertices",
                  targetSurfaceMesh.number_of_vertices());
  } catch (std::exception &e) {
    TSR_LOG_ERROR("simplification failed: {}", e.what());
    throw e;
  }

  if (targetSurfaceMesh.number_of_vertices() == 0) {
    TSR_LOG_WARN("Empty target surface mesh");
  }

  // copy the mesh to the target mesh
  // exceptions here result in an undefined memory state
  try {
    TSR_LOG_TRACE("writing to target mesh");
    target_mesh.clear();
    convert_surface_mesh_to_tin(targetSurfaceMesh, target_mesh);
  } catch (std::exception &e) {
    TSR_LOG_FATAL("copying simplified mesh to target mesh failed");
    throw e;
  }
}

void simplify_tin(Delaunay_3 const &source_mesh, Delaunay_3 &target_mesh) {
  TSR_LOG_TRACE("calling simplify mesh with defaults");
  return simplify_tin(
      source_mesh, target_mesh, DEFAULT_COSINE_MAX_ANGLE_REGIONS,
      DEFAULT_MAX_DISTANCE_REGIONS, DEFAULT_COSINE_MAX_ANGLE_CORNERS,
      DEFAULT_MAX_DISTANCE_CORNERS);
}

} // namespace tsr