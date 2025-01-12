#include "tsr/Tin.hpp"
#include "tsr/version_info.hpp"

#include "tsr/ChunkManager.hpp"
#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/SurfaceMesh.hpp"

#include "tsr/Presets.hpp"

#include "tsr/IO/ImageIO.hpp"
#include "tsr/IO/MapIO.hpp"

#include "tsr/Logging.hpp"
#include "tsr/Point3.hpp"

#include "tsr/GeometryUtils.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/IO/GPXFormatter.hpp"
#include "tsr/IO/MeshIO.hpp"

#include "tsr/Router.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <cfenv>
#include <cstdlib>
#include <exception>
#include <gdal/gdal.h>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

#define DEFAULT_DEM_FILE SOURCE_ROOT "/data/benNevis_DEM.xyz"
#define RADII_MULTIPLIER 1.5
#define DEM_API_KEY "0f789809fed28dc634c8d75695d0cc5c"

#include <sys/resource.h>

#ifdef DEBUG_TIME
log_set_global_logstream(LogStream::stderr);
#endif

namespace tsr {

bool tsr_run(double sLat, double sLon, double eLat, double eLon) {

  struct rlimit limit;
  getrlimit(RLIMIT_STACK, &limit);
  limit.rlim_cur = 16 * 1024 * 1024;
  setrlimit(RLIMIT_STACK, &limit);

  fesetround(FE_TONEAREST);

  log_set_global_loglevel(LogLevel::TRACE);

  // Convert points to UTM
  Point3 startPoint = TranslateWgs84PointToUtm(Point3(sLat, sLon, 0));
  Point3 endPoint = TranslateWgs84PointToUtm(Point3(eLat, eLon, 0));

  MeshBoundary boundary(startPoint, endPoint, RADII_MULTIPLIER);

  TSR_LOG_TRACE("Initializing mesh");

#ifdef DEBUG_TIME
  TSR_LOG_TRACE("DEBUG: TIME");
  using std::chrono::duration;
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;
  using std::chrono::miliseconds;

  auto timer_initial_tin_start = high_resolution_clock::now();
#endif

  Tin tin = InitializeTinFromBoundary(boundary, DEM_API_KEY);

#ifdef DEBUG_TIME
  auto timer_features_setup = high_resolution_clock::now();
#endif

  TSR_LOG_TRACE("Vertices: {}", tin.number_of_vertices());

  FeatureManager fm = SetupTimePreset(tin, boundary);

#ifdef DEBUG_TIME
  auto timer_routing_start = high_resolution_clock::now();
#endif

  TSR_LOG_TRACE("final vertex count: {}", tin.number_of_vertices());

  // /// DEBUG: Write mesh to obj
  // SurfaceMesh tmpMesh;
  // ConvertTinToSurfaceMesh(tin, tmpMesh);
  // IO::WriteMeshToObj("test.obj", tmpMesh);

  TSR_LOG_TRACE("Preparing router");
  Router router;

  // Calculate the optimal route
  TSR_LOG_TRACE("Calculating Route");
  TSR_LOG_TRACE("ll: {} {}", boundary.GetLowerLeftPoint().x(),
                boundary.GetLowerLeftPoint().y());
  TSR_LOG_TRACE("ur: {} {}", boundary.GetUpperRightPoint().x(),
                boundary.GetUpperRightPoint().y());
  auto route = router.Route(tin, fm, boundary, startPoint, endPoint);

#ifdef DEBUG_TIME
  auto timer_routing_end = high_resolution_clock::now();
#endif

  // Convert the points to WGS84
  std::vector<Point3> routeWGS84;
  for (auto &point : route) {
    Point3 pointWGS84 = TranslateUtmPointToWgs84(point, 30, true);
    routeWGS84.push_back(pointWGS84);
  }

  // Output the route as a gpx file
  IO::WriteDataToFile("route.gpx", IO::FormatRouteAsGpx(routeWGS84));

// Output the timing information
#ifdef DEBUG_TIME
  duration<double, std::milli> ms_initial_tin =
      timer_initial_tin_start - timer_features_setup;
  duration<double, std::milli> ms_features_setup =
      timer_routing_start - timer_features_setup;
  duration<double, std::milli> ms_routing =
      timer_routing_end - timer_routing_start;

  log_set_global_logstream(LogStream::stdout);
  TSR_LOG_INFO("{},{},{}", ms_initial_tin, ms_features_setup, ms_routing);
  log_set_global_logstream(LogStream::stderr);

#endif

  return EXIT_SUCCESS;
}

void PrintUsage() {
  std::cout
      << "Usage: ./tsr-route <start-lat> <start-lon> <end-lat> <end-lon>\n"
      << "Example: ./tsr-route 56.777800 -5.024737 56.809481 -5.025113"
      << std::endl;
}

} // namespace tsr

int main(int argc, char **argv) {
  try {
    // Define allowed options
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "Print help message")(
        "example", "Run an example with hardcoded coordinates")(
        "disable-cache", "Disables data cache")(
        "start-lat", po::value<double>(), "Starting latitude")(
        "start-lon", po::value<double>(), "Starting longitude")(
        "end-lat", po::value<double>(),
        "Ending latitude")("end-lon", po::value<double>(), "Ending longitude");

    // Map positional arguments to options
    po::positional_options_description positionalArgs;
    positionalArgs.add("start-lat", 1)
        .add("start-lon", 1)
        .add("end-lat", 1)
        .add("end-lon", 1);

    po::variables_map vm;

    // Parse command-line arguments
    po::parsed_options parsed =
        po::command_line_parser(argc, argv)
            .options(desc)
            .positional(positionalArgs)
            .style(po::command_line_style::allow_long |
                   boost::program_options::command_line_style::long_allow_next)
            .run();

    po::store(parsed, vm);
    po::notify(vm);

    if (vm.count("help")) {
      tsr::PrintUsage();
      return 1;
    }

    if (vm.count("disable-cache")) {
      tsr::CacheSetEnabled(false);
    }

    if (vm.count("example")) {
      return tsr::tsr_run(56.777800, -5.024737, 56.809481, -5.025113);
    }
    // Validate positional arguments
    if (!vm.count("start-lat") || !vm.count("start-lon") ||
        !vm.count("end-lat") || !vm.count("end-lon")) {
      tsr::PrintUsage();
      return 1;
    }

    // Parse arguments
    double start_lat = vm["start-lat"].as<double>();
    double start_lon = vm["start-lon"].as<double>();
    double end_lat = vm["end-lat"].as<double>();
    double end_lon = vm["end-lon"].as<double>();

    return tsr::tsr_run(start_lat, start_lon, end_lat, end_lon);

  } catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
}
