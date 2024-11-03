#include "tsr/DTM.hpp"
#include "tsr/IO/FileIOHandler.hpp"
#include "tsr/logging.hpp"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#define DEFAULT_DEM_FILE SOURCE_ROOT "/data/benNevis_UTM.xyz"

namespace tsr {

bool tsr_run(std::vector<std::string> args) {

  std::string filepath = args.size() == 1 ? IO::path_to_absolute(args[0]) : DEFAULT_DEM_FILE;

    // Construct the DTM from filepath
  TSR_LOG_TRACE("loading dem file ({:s})", filepath);
  auto points = IO::load_dem_from_file(filepath);

  TSR_LOG_TRACE("creating dtm from point cloud");
  auto dtm = std::make_unique<DTM>(*points);

  return EXIT_SUCCESS;
}

} // namespace tsr

int main(int argc, char **argv) {

  std::vector<std::string> args;
  for (int i = 1; i < argc; i++) {
    args.push_back(argv[i]);
  }

  return tsr::tsr_run(std::move(args));
}