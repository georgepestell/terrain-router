#include "tsr/IO.hpp"
#include "tsr/DTM.hpp"
#include "tsr/logging.hpp"

#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/boost/graph/graph_traits_Delaunay_triangulation_2.h>

#include <filesystem>

#include <CGAL/boost/graph/IO/OBJ.h>

namespace tsr {

Point_set_3 load_points_from_file(char const *filepath) {

  // TODO: error checking

  TSR_LOG_TRACE("loading points from file");

  // Read points from file
  std::ifstream ifile(filepath, std::ios_base::binary);
  CGAL::Point_set_3<Point_3> points;
  ifile >> points;

  return points;
}

const char *path_to_absolute(char const *path_string) {

  filesystem::path filepath(path_string);

  // Add pwd to relative path
  if (filepath.is_relative()) {
    // Get the current working directory
    filepath = filesystem::current_path() / filepath;
  }

  return filepath.c_str();
}

void write_mesh_to_obj(char const *filepath, Delaunay_3D const &mesh) {
  std::ofstream ofile(filepath, ios_base::binary);
  CGAL::IO::set_binary_mode(ofile);

  Surface_mesh surface_mesh;
  CGAL::copy_face_graph(mesh, surface_mesh);

  CGAL::IO::write_OBJ(ofile, surface_mesh);
  ofile.close();
}

} // namespace tsr