#include "tsr/DTM.hpp"
#include "tsr/logging.hpp"

#include <CGAL/Kernel/global_functions_3.h>
#include <CGAL/Polygon_mesh_processing/region_growing.h>
#include <CGAL/Polygon_mesh_processing/remesh_planar_patches.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/boost/graph/graph_traits_Constrained_Delaunay_triangulation_2.h>
#include <CGAL/hierarchy_simplify_point_set.h>
#include <CGAL/tags.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>

#include <CGAL/jet_smooth_point_set.h>

#include <CGAL/grid_simplify_point_set.h>

#include <stdexcept>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Vector_3.h>

#include <GeographicLib/Geodesic.hpp>

namespace tsr {

typedef CGAL::Vector_3<CGAL::Exact_predicates_inexact_constructions_kernel> Vector_3;
typedef CGAL::Parallel_if_available_tag Concurrency_tag;

Delaunay_3 &DTM::get_mesh() const { return *this->mesh; }

std::unique_ptr<Delaunay_3> create_tin_from_points(std::vector<Point_3> &points) {
  TSR_LOG_TRACE("creating TIN from point cloud");

  // validate point set contains points
  if (points.empty()) {
    TSR_LOG_WARN("TIN point vector is empty");
    throw std::invalid_argument("TIN point vector is empty");
  }

  // triangulate points using 2.5D Delaunay triangulation
  auto tin = std::make_unique<Delaunay_3>();
  tin->insert(points.begin(), points.end());

  // validate triangulated mesh
  if (!tin->is_valid()) {
    TSR_LOG_ERROR("initalized DTM invalid");
    throw std::runtime_error("DTM Delaunay Triangulated mesh invalid");
  }

  TSR_LOG_TRACE("TIN initialized with {:d} vertices",
                tin->number_of_vertices());
 
 return tin;

}

void convert_surface_mesh_to_tin(Surface_mesh const &source,
                                      Delaunay_3 &target) {
  for (auto v : source.vertices()) {
    target.insert(source.point(v));
  }
}

double interpolate_z(const Point_3& p1, const Point_3& p2, const Point_3& p3, const double x, const double y) {
      // Compute two edge vectors
    Vector_3 v1 = p2 - p1;
    Vector_3 v2 = p3 - p1;
    
    Vector_3 normal = CGAL::cross_product(v1, v2);

    // The plane equation is A * x + B * y + C * z + D = 0
    // Where (A, B, C) is the normal vector
    double A = normal[0];
    double B = normal[1];
    double C = normal[2];
    
    // Compute D using one of the triangle vertices (e.g., p1)
    double D = - (A * p1[0] + B * p1[1] + C * p1[2]);
    
    // Now we can solve for z: z = (-A * x - B * y - D) / C
    return (-A * x - B * y - D) / C;
}

void DTM::add_contour_constraint(std::vector<Point_2> contour, double max_segment_length) {

  for (auto vertexIt = contour.begin(); vertexIt != contour.end(); ++vertexIt) {
    auto vertexNextIt = std::next(vertexIt);
    if (vertexNextIt == contour.end()) break;

    const double x = vertexIt->x();
    const double y = vertexIt->y();

    Delaunay_3::Face_handle vertexFace = this->mesh->locate(Point_3(x, y, 0));

    if (vertexFace == nullptr || this->mesh->is_infinite(vertexFace)) {
      TSR_LOG_WARN("Point outside boundary x: {} y: {}", x, y);
      continue;
    }

    double z = interpolate_z(vertexFace->vertex(0)->point(), vertexFace->vertex(1)->point(), vertexFace->vertex(2)->point(), x ,y);

    Point_3 vertex(vertexIt->x(), vertexIt->y(), z);

    const double next_x = vertexNextIt->x();
    const double next_y = vertexNextIt->y();

    Delaunay_3::Face_handle vertexNextFace = this->mesh->locate(Point_3(next_x, next_y, 0));

    if (vertexNextFace == nullptr || this->mesh->is_infinite(vertexNextFace)) {
      TSR_LOG_WARN("Point outside boundary x: {} y: {}", next_x, next_y);
      continue;
    }

    double next_z = interpolate_z(vertexNextFace->vertex(0)->point(), vertexNextFace->vertex(1)->point(), vertexNextFace->vertex(2)->point(), next_x ,next_y);

    Point_3 vertexPoint(x, y, z);
    Point_3 vertexNextPoint(next_x, next_y, next_z);
    
    // Calculate the Euclidean distance in the XY plane
    double dx = next_x - x;
    double dy = next_y - y;
    double length = sqrt(dx * dx + dy * dy);

    std::cout << "length: " << length << std::endl;
    TSR_LOG_WARN("length {}", length);

    if  (length > max_segment_length) {
      // Calculate the number of splits required
      double splits = floor(length / max_segment_length);

      std::vector<Point_3> splitPoints;
      for (double split = 1; split <= splits; split++) {
        double split_x = round(x + (dx / splits) * split);
        double split_y = round(y + (dy / splits) * split);

        Delaunay_3::Face_handle vertexSplitFace = this->mesh->locate(Point_3(split_x, split_y, 0));

        if (this->mesh->is_infinite(vertexSplitFace)) {
          TSR_LOG_ERROR("Point outside boundary x: {} y: {}", split_x, split_y);
          return;
        }

        double split_z = interpolate_z(vertexSplitFace->vertex(0)->point(), vertexSplitFace->vertex(1)->point(), vertexSplitFace->vertex(2)->point(), split_x, split_y);
        splitPoints.push_back(Point_3(split_x, split_y, split_z));
      }

      // Add the constraints
      this->mesh->insert_constraint(vertexPoint, splitPoints[0]);
      ushort splitIndex;
      for (splitIndex = 1; splitIndex < splitPoints.size(); splitIndex++)  {
        this->mesh->insert_constraint(splitPoints[splitIndex - 1], splitPoints[splitIndex]);
      }
      this->mesh->insert_constraint(splitPoints[splitIndex - 1], vertexNextPoint);

    } else {
      this->mesh->insert_constraint(vertexPoint, vertexNextPoint);
    }

  }
}



void convert_tin_to_surface_mesh(Delaunay_3 const &source, Surface_mesh &target) {
  CGAL::copy_face_graph(source, target);
}

void denoise_points(std::vector<Point_3> &points) {

  std::cout << points.size() << " input points" << std::endl;
  std::vector<std::size_t> indices(points.size());
  for(std::size_t i = 0; i < points.size(); ++i){
    indices[i] = i;
  }
  // simplification by clustering using erase-remove idiom
  double cell_size = 22;
  std::vector<std::size_t>::iterator end;
  end = CGAL::grid_simplify_point_set(indices,
                                      cell_size,
                                      CGAL::parameters::point_map (CGAL::make_property_map(points)));
  std::size_t k = end - indices.begin();
  std::cerr << "Keep " << k << " of " << indices.size() <<  " indices" << std::endl;
  {
    std::vector<Point_3> tmp_points(k);
    for(std::size_t i=0; i<k; ++i){
      tmp_points[i] = points[indices[i]];
    }
    points.swap(tmp_points);
  }
}

void jet_smooth_points(std::vector<Point_3> &points) {
  const unsigned int nb_neighbors = 8; // default is 24 for real-life point sets
  CGAL::jet_smooth_point_set<Concurrency_tag>(points, nb_neighbors);
}

void simplify_points(std::vector<Point_3> &points) {

    points.erase(CGAL::hierarchy_simplify_point_set(points,
                                                  CGAL::parameters::size(2.2)// Max cluster size
                                                                   .maximum_variation(0.001)), // Max surface variation
               points.end());

  //parameters
  const double retain_percentage = 50;   // percentage of points to retain.
  const double neighbor_radius = 0.5;   // neighbors size.
 
  std::vector<Point_3> points_simplified;

  CGAL::wlop_simplify_and_regularize_point_set<Concurrency_tag>
    (points, std::back_inserter(points_simplified),
     CGAL::parameters::select_percentage(retain_percentage).
     neighbor_radius (neighbor_radius));

     points = points_simplified;

}

void simplify_mesh(Surface_mesh const &source_mesh, Surface_mesh &target_mesh,
                           float cosine_max_angle_regions,
                           float max_distance_regions,
                           float cosine_max_angle_corners,
                           float max_distance_corners) {

  // Required simplification information
  std::vector<size_t> region_ids(CGAL::num_faces(source_mesh));
  std::vector<size_t> corner_id_map(CGAL::num_vertices(source_mesh), -1);
  std::vector<bool> ecm(CGAL::num_edges(source_mesh), false);
  boost::vector_property_map<CGAL::Epick::Vector_3> normal_map;

  TSR_LOG_TRACE("detecting mesh regions for simplification");

  size_t nb_regions =
      CGAL::Polygon_mesh_processing::region_growing_of_planes_on_faces(
          source_mesh,
          CGAL::make_random_access_property_map(region_ids),
          CGAL::parameters::cosine_of_maximum_angle(cosine_max_angle_regions)
              .region_primitive_map(normal_map)
              .maximum_distance(max_distance_regions));

  TSR_LOG_TRACE("detecting mesh corners for simplification");

  size_t nb_corners = CGAL::Polygon_mesh_processing::detect_corners_of_regions(
      source_mesh, CGAL::make_random_access_property_map(region_ids),
      nb_regions, CGAL::make_random_access_property_map(corner_id_map),
      CGAL::parameters::cosine_of_maximum_angle(cosine_max_angle_corners)
          .maximum_distance(max_distance_corners)
          .edge_is_constrained_map(CGAL::make_random_access_property_map(ecm)));

  try {
    TSR_LOG_TRACE("simplifying mesh");
    CGAL::Polygon_mesh_processing::remesh_almost_planar_patches(
        source_mesh, target_mesh, nb_regions, nb_corners,
        CGAL::make_random_access_property_map(region_ids),
        CGAL::make_random_access_property_map(corner_id_map),
        CGAL::make_random_access_property_map(ecm),
        CGAL::parameters::patch_normal_map(normal_map));
  } catch (std::exception &e) {
    TSR_LOG_ERROR("simplification failed: {}", e.what());
    throw e;
  }
}

void simplify_mesh(Surface_mesh const &source_mesh, Surface_mesh &target_mesh) {
  simplify_mesh(source_mesh, target_mesh, DEFAULT_COSINE_MAX_ANGLE_REGIONS, DEFAULT_MAX_DISTANCE_REGIONS, DEFAULT_COSINE_MAX_ANGLE_CORNERS, DEFAULT_MAX_DISTANCE_CORNERS);
}

void simplify_mesh(Delaunay_3 const &source_mesh,
                              Delaunay_3 &target_mesh,
                              float cosine_max_angle_regions,
                              float max_distance_regions,
                              float cosine_max_angle_corners,
                              float max_distance_corners) {

  // Convert the mesh to a Surface_Mesh
  auto source_surface_mesh = std::make_unique<Surface_mesh>();
  convert_tin_to_surface_mesh(source_mesh, *source_surface_mesh);

  auto target_surface_mesh = std::make_unique<Surface_mesh>();

  simplify_mesh(*source_surface_mesh, *target_surface_mesh, cosine_max_angle_regions, max_distance_regions, cosine_max_angle_corners, max_distance_corners);

  // copy the mesh to the target mesh
  // exceptions here result in an undefined memory state
  try {
    TSR_LOG_TRACE("writing to target mesh");
    target_mesh.clear();
    convert_surface_mesh_to_tin(*target_surface_mesh, target_mesh);
  } catch (std::exception &e) {
    TSR_LOG_FATAL("copying simplified mesh to target mesh failed");
    throw e;
  }
}

void simplify_mesh(Delaunay_3 const &source_mesh,
                              Delaunay_3 &target_mesh) {
  TSR_LOG_TRACE("calling simplify mesh with defaults");
  return simplify_mesh(
      source_mesh, target_mesh, DEFAULT_COSINE_MAX_ANGLE_REGIONS,
      DEFAULT_MAX_DISTANCE_REGIONS, DEFAULT_COSINE_MAX_ANGLE_CORNERS,
      DEFAULT_MAX_DISTANCE_CORNERS);
}

} // namespace tsr