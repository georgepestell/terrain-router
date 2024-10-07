#define CGAL_PMP_USE_CERES_SOLVER

#include "terrain_parser.hpp"

#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/IO/io.h>
#include <CGAL/IO/write_xyz_points.h>
#include <CGAL/Point_2.h>
#include <CGAL/Point_3.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/angle_and_area_smoothing.h>
#include <CGAL/Polygon_mesh_processing/detect_features.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/remesh_planar_patches.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Segment_3.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Surface_mesh/Surface_mesh.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_count_ratio_stop_predicate.h>
#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Triangulation_2.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/boost/graph/graph_traits_Delaunay_triangulation_2.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/boost/graph/iterator.h>
#include <CGAL/property_map.h>
#include <boost/graph/graph_traits.hpp>
#include <eigen3/Eigen/Dense>

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/random_perturbation.h>
#include <CGAL/Polygon_mesh_processing/region_growing.h>
#include <CGAL/Polygon_mesh_processing/remesh_planar_patches.h>

#include <boost/property_map/vector_property_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <fstream>

using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Projection_traits = CGAL::Projection_traits_xy_3<Kernel>;
using Point_2 = Kernel::Point_2;
using Point_3 = Kernel::Point_3;
using Segment_3 = Kernel::Segment_3;

// Triangulated Irregular Network
using TIN = CGAL::Delaunay_triangulation_2<Projection_traits>;

using namespace std;

namespace terrainParserLib {

void parseFile(string filepath) {

  /**
   * @brief For each cell/DEPTH in the ASCIIGrid file, we take a measurement,
   * and use that as the centrepoint of the triangle.
   *
   */
  cout << filepath << endl;

  /** DEBUG BEGIN
   * - Testing fade25d 2.5D Delaunay Triangulation Library
   */

  // Read points
  cout << "Reading Points" << endl;
  std::ifstream ifile(filepath, std::ios_base::binary);
  CGAL::Point_set_3<Point_3> points;
  ifile >> points;

  cout << "Creating TIN" << endl;
  auto dsm = make_unique<TIN>(points.points().begin(), points.points().end());

  // Empty the points variable
  points.clear();

  cout << "Creating Mesh" << endl;

  using Mesh = CGAL::Surface_mesh<Point_3>;
  Mesh dsm_mesh;
  CGAL::copy_face_graph(*dsm, dsm_mesh);
  cout << "initial vertices: " << dsm_mesh.number_of_vertices() << endl;


// Simplify -- using a value of 25m for collapsable edges because that is the
  // range at which terrain doesn't change much
  cout << "Simplifying 1" << endl;

  cout << "- detecting regions and corners" << endl;
  vector<size_t> region_ids(CGAL::num_faces(dsm_mesh));
  vector<size_t> corner_id_map(CGAL::num_vertices(dsm_mesh), -1);
  vector<bool> ecm(CGAL::num_edges(dsm_mesh), false);
  boost::vector_property_map<CGAL::Epick::Vector_3> normal_map;

  size_t nb_regions =
      CGAL::Polygon_mesh_processing::region_growing_of_planes_on_faces(
          dsm_mesh, CGAL::make_random_access_property_map(region_ids),
          CGAL::parameters::cosine_of_maximum_angle(0.60)
              .region_primitive_map(normal_map)
              .maximum_distance(5.0));

  size_t nb_corners = CGAL::Polygon_mesh_processing::detect_corners_of_regions(
      dsm_mesh, CGAL::make_random_access_property_map(region_ids), nb_regions,
      CGAL::make_random_access_property_map(corner_id_map),
      CGAL::parameters::cosine_of_maximum_angle(0.90)
          .maximum_distance(3.0)
          .edge_is_constrained_map(CGAL::make_random_access_property_map(ecm)));

  cout << "- simplifying through remeshing almost planar patches" << endl;
  Mesh out;
  CGAL::Polygon_mesh_processing::remesh_almost_planar_patches(
      dsm_mesh, out, nb_regions, nb_corners,
      CGAL::make_random_access_property_map(region_ids),
      CGAL::make_random_access_property_map(corner_id_map),
      CGAL::make_random_access_property_map(ecm),
      CGAL::parameters::patch_normal_map(normal_map));


  dsm_mesh.clear();
  CGAL::copy_face_graph(out, dsm_mesh);

  // Smoothing
  cout << "Smoothing" << endl;

  cout << "- Detecting sharp edges" << endl;
  // Constrain edges with a dihedral angle over 60°
  typedef boost::property_map<Mesh, CGAL::edge_is_feature_t>::type EIFMap;
  EIFMap eif = get(CGAL::edge_is_feature, dsm_mesh);
  CGAL::Polygon_mesh_processing::detect_sharp_edges(dsm_mesh, 80, eif);

  cout << "- Smoothing angles and areas" << endl;
  const unsigned int nb_iterations = 10;
  CGAL::Polygon_mesh_processing::angle_and_area_smoothing(
      dsm_mesh, CGAL::parameters::number_of_iterations(nb_iterations)
                    .use_safety_constraints(true)
                    .edge_is_constrained_map(eif)
                    .use_area_smoothing(true));

  // Simplify -- using a value of 25m for collapsable edges because that is the
  // range at which terrain doesn't change much
  cout << "Simplifying 2" << endl;

  cout << "- detecting regions and corners" << endl;
  vector<size_t> region_ids_2(CGAL::num_faces(dsm_mesh));
  vector<size_t> corner_id_map_2(CGAL::num_vertices(dsm_mesh), -1);
  vector<bool> ecm_2(CGAL::num_edges(dsm_mesh), false);
  boost::vector_property_map<CGAL::Epick::Vector_3> normal_map_2;

  nb_regions =
      CGAL::Polygon_mesh_processing::region_growing_of_planes_on_faces(
          dsm_mesh, CGAL::make_random_access_property_map(region_ids_2),
          CGAL::parameters::cosine_of_maximum_angle(0.70)
              .region_primitive_map(normal_map_2)
              .maximum_distance(0.5));

  nb_corners = CGAL::Polygon_mesh_processing::detect_corners_of_regions(
      dsm_mesh, CGAL::make_random_access_property_map(region_ids_2), nb_regions,
      CGAL::make_random_access_property_map(corner_id_map_2),
      CGAL::parameters::cosine_of_maximum_angle(0.95)
          .maximum_distance(0.5)
          .edge_is_constrained_map(CGAL::make_random_access_property_map(ecm_2)));

  cout << "- simplifying through remeshing almost planar patches" << endl;
  Mesh out2;
  CGAL::Polygon_mesh_processing::remesh_almost_planar_patches(
      dsm_mesh, out2, nb_regions, nb_corners,
      CGAL::make_random_access_property_map(region_ids_2),
      CGAL::make_random_access_property_map(corner_id_map_2),
      CGAL::make_random_access_property_map(ecm_2),
      CGAL::parameters::patch_normal_map(normal_map_2));

  dsm_mesh.clear();
  // Print new vertex count
  cout << "DONE, new vertices: " << out2.number_of_vertices() << endl;

  cout << "Writing .obj" << endl;
  std::ofstream dsm_ofile("dsm.obj", ios_base::binary);
  CGAL::IO::set_binary_mode(dsm_ofile);
  CGAL::IO::write_OBJ(dsm_ofile, out2);
  dsm_ofile.close();

  /** DEBUG END */
}

} // namespace terrainParserLib

int main(int argc, char *args[]) {

  // Pass absolute path in directly, or fetch the PWD
  if (argc == 2 && args[1][0] == '/') {
    // Absolute path
    terrainParserLib::parseFile((string)args[1]);
  } else {

    // Relative path

    // Use default if no argument given
    string relative_filepath = (argc == 1) ? DEFAULT_TERRAIN_FILE : args[1];

    filesystem::path fp = filesystem::path(filesystem::current_path());

    fp.append(relative_filepath);

    terrainParserLib::parseFile(fp);
  }

  return 0;
}
