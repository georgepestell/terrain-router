#include "tsr/Presets.hpp"
#include "tsr/Logging.hpp"
#include "tsr/IO.hpp"
#include "tsr/Core.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include <cfenv>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>
#include <vector>

#include "tsr/Config.hpp"

namespace po = boost::program_options;

#define DEFAULT_DEM_FILE SOURCE_ROOT "/data/benNevis_DEM.xyz"
#define RADII_MULTIPLIER 1.5
#define DEM_API_KEY OPENTOP_KEY

/// DEBUG: remove DEBUG_TIME
#define DEBUG_TIME

#include <sys/resource.h>

namespace tsr {

#ifdef DEBUG_TIME
#include <chrono>
#include <ratio>
#endif

bool tsr_run(double sLat, double sLon, double eLat, double eLon) {

#ifdef DEBUG_TIME
  log_set_global_logstream(tsr::LogStream::STDERR);
#endif

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
  TSR_LOG_DEBUG("DEBUG: TIME");
  using std::chrono::duration;
  using std::chrono::duration_cast;
  using std::chrono::high_resolution_clock;
  using std::chrono::milliseconds;

  auto timer_initial_tin_start = high_resolution_clock::now();
#endif

  TSR_LOG_INFO("Initializing TIN");
  Tin tin = InitializeTinFromBoundary(boundary, OPENTOP_KEY);

#ifdef DEBUG_TIME
  auto timer_features_setup = high_resolution_clock::now();
#endif

  TSR_LOG_TRACE("Vertices: {}", tin.number_of_vertices());

  TSR_LOG_INFO("Preparing Feature Manager");
  FeatureManager fm = SetupTimePreset(tin, boundary);

#ifdef DEBUG_TIME
  auto timer_routing_start = high_resolution_clock::now();
#endif

  TSR_LOG_DEBUG("Finished setup");

  TSR_LOG_TRACE("final vertex count: {}", tin.number_of_vertices());

  /// DEBUG: Write mesh to obj
  SurfaceMesh tmpMesh;
  ConvertTinToSurfaceMesh(tin, tmpMesh);
  IO::WriteMeshToObj("test.obj", tmpMesh);

  TSR_LOG_TRACE("Preparing router");
  Router router;

  // Calculate the optimal route
  std::vector<Point3> route;

  TSR_LOG_INFO("Routing");

  bool routeStatus = EXIT_SUCCESS;
  try {
    route = router.Route(tin, fm, boundary, startPoint, endPoint);
  } catch (std::exception &e) {
    // Continue
    routeStatus = EXIT_FAILURE;
  }

#ifdef DEBUG_TIME
  auto timer_routing_end = high_resolution_clock::now();
#endif

  // // Convert the points to WGS84
  // std::vector<Point3> routeWGS84;
  // for (auto &point : route) {
  //   Point3 pointWGS84 = TranslateUtmPointToWgs84(point, 30, true);
  //   routeWGS84.push_back(pointWGS84);
  // }

  // // Output the route as a gpx file
  // IO::WriteDataToFile("route.gpx", IO::FormatRouteAsGpx(routeWGS84));

// Output the timing information
#ifdef DEBUG_TIME
  duration<double> s_initial_tin =
      timer_features_setup - timer_initial_tin_start;
  duration<double> s_features_setup =
      timer_routing_start - timer_features_setup;
  duration<double> s_routing = timer_routing_end - timer_routing_start;

  log_set_global_logstream(LogStream::STDOUT);
  TSR_LOG_INFO("'{}, {}','{}, {}',{},{},{},{},{},{},{}", sLat, sLon, eLat, eLon,
               CalculateXYDistance(startPoint, endPoint),
               tin.number_of_vertices(), s_initial_tin.count(),
               s_features_setup.count(), s_routing.count(), routeStatus == 0,
               IsCacheEnabled());

  log_set_global_logstream(LogStream::STDERR);

#endif

  return routeStatus;
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
    TSR_LOG_ERROR("{}", e.what());
    throw e;
    return 1;
  }
}
