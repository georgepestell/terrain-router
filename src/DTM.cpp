#include "tsr/DTM.hpp"
#include "tsr/logging.hpp"

#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_mesh_processing/region_growing.h>
#include <CGAL/Polygon_mesh_processing/remesh_planar_patches.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/boost/graph/graph_traits_Delaunay_triangulation_2.h>

#include <algorithm>
#include <stdexcept>

namespace tsr {

Delaunay_3D &DTM::get_topology() const { return *this->topology_mesh; }

void DTM::initialize_dtm(Point_set_3 points) {
  TSR_LOG_TRACE("initialising DTM");

  // validate point set contains points
  if (points.empty()) {
    TSR_LOG_WARN("DTM point set empty");
    throw std::invalid_argument("DTM initialized with empty pointset");
  }

  // triangulate points using 2.5D Delaunay triangulation
  Delaunay_3D dt(points.points().begin(), points.points().end());

  // validate triangulated mesh
  if (!dt.is_valid()) {
    TSR_LOG_ERROR("initalized DTM invalid");
    throw std::runtime_error("DTM Delaunay Triangulated mesh invalid");
  }

  TSR_LOG_TRACE("DTM initialization complete with {:d} vertices",
                dt->number_of_vertices());

  // convert the delaunay mesh to surface mesh
  *this->topology_mesh = dt;
}

void DTM::simplify_3d_feature(Delaunay_3D const &source_mesh,
                              Delaunay_3D &target_mesh,
                              float cosine_max_angle_regions,
                              float max_distance_regions,
                              float cosine_max_angle_corners,
                              float max_distance_corners) {

  // Convert the mesh to a Surface_Mesh
  auto source_surface_mesh = make_unique<Surface_mesh>();
  CGAL::copy_face_graph(source_mesh, *source_surface_mesh);

  // Required simplification information
  vector<size_t> region_ids(CGAL::num_faces(*source_surface_mesh));
  vector<size_t> corner_id_map(CGAL::num_vertices(*source_surface_mesh), -1);
  vector<bool> ecm(CGAL::num_edges(*source_surface_mesh), false);
  boost::vector_property_map<CGAL::Epick::Vector_3> normal_map;

  TSR_LOG_TRACE("detecting mesh regions for simplification");

  size_t nb_regions =
      CGAL::Polygon_mesh_processing::region_growing_of_planes_on_faces(
          *source_surface_mesh,
          CGAL::make_random_access_property_map(region_ids),
          CGAL::parameters::cosine_of_maximum_angle(cosine_max_angle_regions)
              .region_primitive_map(normal_map)
              .maximum_distance(max_distance_regions));

  TSR_LOG_TRACE("detecting mesh corners for simplification");

  size_t nb_corners = CGAL::Polygon_mesh_processing::detect_corners_of_regions(
      *source_surface_mesh, CGAL::make_random_access_property_map(region_ids),
      nb_regions, CGAL::make_random_access_property_map(corner_id_map),
      CGAL::parameters::cosine_of_maximum_angle(cosine_max_angle_corners)
          .maximum_distance(max_distance_corners)
          .edge_is_constrained_map(CGAL::make_random_access_property_map(ecm)));

  auto target_surface_mesh = make_unique<Surface_mesh>();
  try {
    TSR_LOG_TRACE("simplifying mesh");
    CGAL::Polygon_mesh_processing::remesh_almost_planar_patches(
        *source_surface_mesh, *target_surface_mesh, nb_regions, nb_corners,
        CGAL::make_random_access_property_map(region_ids),
        CGAL::make_random_access_property_map(corner_id_map),
        CGAL::make_random_access_property_map(ecm),
        CGAL::parameters::patch_normal_map(normal_map));
  } catch (exception &e) {
    TSR_LOG_ERROR("simplification failed: {}", e.what());
    throw e;
  }

  // copy the mesh to the target mesh
  // exceptions here result in an undefined memory state
  try {
    TSR_LOG_TRACE("writing to target mesh");
    target_mesh.clear();
    convert_surface_mesh_to_delaunay(*target_surface_mesh, target_mesh);
  } catch (exception &e) {
    TSR_LOG_FATAL("copying simplified mesh to target mesh failed");
    throw e;
  }
}

void convert_surface_mesh_to_delaunay(Surface_mesh const &source,
                                      Delaunay_3D &target) {
  for (auto v : source.vertices()) {
    target.insert(source.point(v));
  }
}

void DTM::simplify_3d_feature(Delaunay_3D const &source_mesh,
                              Delaunay_3D &target_mesh) {
  TSR_LOG_TRACE("calling simplify mesh with defaults");
  return simplify_3d_feature(
      source_mesh, target_mesh, DEFAULT_COSINE_MAX_ANGLE_REGIONS,
      DEFAULT_MAX_DISTANCE_REGIONS, DEFAULT_COSINE_MAX_ANGLE_CORNERS,
      DEFAULT_MAX_DISTANCE_CORNERS);
}

unique_ptr<Delaunay_2D> create_binary_feature(char const *layer_id,
                                              vector<Point_2> &points) {
  TSR_LOG_INFO("initialising binary feature mesh");

  // validate point set contains points
  if (points.size() == 0) {
    TSR_LOG_WARN("point set empty");
    throw std::invalid_argument("feature mesh initialized with empty pointset");
  }

  // triangulate points using 2D Delaunay triangulation
  auto dt = make_unique<Delaunay_2D>(points.begin(), points.end());

  // validate triangulated mesh
  if (!dt->is_valid()) {
    TSR_LOG_ERROR("initalized DTM invalid");
    throw std::runtime_error("DTM Delaunay Triangulated mesh invalid");
  }

  TSR_LOG_TRACE("DTM initialization complete with {:d} vertices",
                dt->number_of_vertices());

  return dt;
}

} // namespace tsr