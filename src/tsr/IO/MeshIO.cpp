#include <fstream>

#include "tsr/IO/MeshIO.hpp"
#include "tsr/logging.hpp"

namespace tsr::IO {

void write_mesh_to_obj(std::string filepath, Mesh mesh) {
  std::ofstream ofile(filepath, std::ios_base::binary);
  CGAL::IO::set_binary_mode(ofile);

  CGAL::IO::write_OBJ(ofile, mesh);
  ofile.close();
};

// TODO: Delete direct access of DEM from file
std::vector<Point_3> load_dem_from_file(std::string filepath) {

  // TODO: error checking

  TSR_LOG_TRACE("loading points from file");

  // Read points from file
  std::ifstream ifile(filepath, std::ios_base::binary);
  std::vector<Point_3> points;

  double x, y, z;
  while (ifile >> x >> y >> z) {
    points.push_back(Point_3(round(x), round(y), round(z)));
  }

  return points;
}

} // namespace tsr::IO