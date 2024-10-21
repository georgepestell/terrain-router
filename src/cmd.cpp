#include "tsr/DTM.hpp"
#include "tsr/IO.hpp"
#include "tsr/logging.hpp"

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#define DEFAULT_DEM_FILE SOURCE_ROOT "/data/benNevis_COP30.xyz"

using namespace std;

namespace tsr {

bool tsr_run(vector<string> args) {

  unique_ptr<DTM> dtm;

  // Chcek if a DEM filepath was specified
  if (args.size() == 1) {

    string filepath = path_to_absolute(args[0].c_str());

    // Construct the DTM from filepath
    TSR_LOG_TRACE("loading DEM file: ({:s})", filepath);
    dtm = make_unique<DTM>(load_points_from_file(filepath.c_str()));

  } else {

    // Load default DEM file
    TSR_LOG_TRACE("loading default DEM ({:s})", DEFAULT_DEM_FILE);
    dtm = make_unique<DTM>(load_points_from_file(DEFAULT_DEM_FILE));
  }

  // Fetch a reference to the topology mesh
  Delaunay_3D &topology_mesh = dtm->get_topology();
  TSR_LOG_INFO("topology vertices: {:d}", topology_mesh.number_of_vertices());

  // Simplify large regions
  dtm->simplify_3d_feature(topology_mesh, topology_mesh);

  // Simplify small regions
  // simplify_3d_mesh(topology_mesh, topology_mesh, 0.70, 0.50, 0.95, 0.5);

  TSR_LOG_INFO("simplified topology vertices: {:d}",
               topology_mesh.number_of_vertices());

  write_mesh_to_obj("dtm.obj", dtm->get_topology());

  return EXIT_SUCCESS;
}

} // namespace tsr

int main(int argc, char **argv) {

  vector<string> args;
  for (int i = 1; i < argc; i++) {
    args.push_back(argv[i]);
  }

  return tsr::tsr_run(std::move(args));
}