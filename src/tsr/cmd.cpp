#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/Features/CEHTerrainFeature.hpp"
#include "tsr/Features/ConditionalFeature.hpp"
#include "tsr/Features/ConstantFeature.hpp"
#include "tsr/Features/DistanceFeature.hpp"
#include "tsr/Features/GradientFeature.hpp"
#include "tsr/Features/GradientSpeedFeature.hpp"
#include "tsr/Features/InverseFeature.hpp"
#include "tsr/Features/MultiplierFeature.hpp"
#include "tsr/Features/PathFeature.hpp"
#include "tsr/Features/WaterFeature.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/SurfaceMesh.hpp"

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
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

#define DEFAULT_DEM_FILE SOURCE_ROOT "/data/benNevis_DEM.xyz"
#define RADII_MULTIPLIER 1.5
#define DEM_API_KEY "0f789809fed28dc634c8d75695d0cc5c"

#include <sys/resource.h>

namespace tsr {

bool tsr_run(double sLat, double sLon, double eLat, double eLon) {

  struct rlimit limit;
  getrlimit(RLIMIT_STACK, &limit);
  limit.rlim_cur = 16 * 1024 * 1024;
  setrlimit(RLIMIT_STACK, &limit);

  fesetround(FE_TONEAREST);

  log_set_global_loglevel(LogLevel::TRACE);

  // Convert points to UTM
  Point3 startPoint = WGS84_point_to_UTM(Point3(sLat, sLon, 0));
  Point3 endPoint = WGS84_point_to_UTM(Point3(eLat, eLon, 0));

  MeshBoundary boundary(startPoint, endPoint, RADII_MULTIPLIER);

  TSR_LOG_TRACE("Initializing mesh");
  Tin tin = InitializeTinFromBoundary(boundary, DEM_API_KEY);

  TSR_LOG_TRACE("Vertices: {}", tin.number_of_vertices());

  // Feature Manager Configuration
  TSR_LOG_TRACE("Setting up feature manager");
  FeatureManager fm;

  auto gradientFeature = std::make_shared<GradientFeature>("gradient");
  auto distance = std::make_shared<DistanceFeature>("distance");
  auto gradientSpeedInfluence =
      std::make_shared<GradientSpeedFeature>("gradient_speed");
  gradientSpeedInfluence->add_dependency(gradientFeature);

  auto terrainFeature =
      std::make_shared<CEHTerrainFeature>("terrain_type", 0.1);
  terrainFeature->initialize(tin, boundary);

  auto waterFeature = std::make_shared<BoolWaterFeature>("water", 0.1);

  auto pathFeature = std::make_shared<PathFeature>("paths", 0.1);

  auto waterSpeedInfluence =
      std::make_shared<InverseFeature<bool, bool>>("water_speed");
  waterSpeedInfluence->add_dependency(waterFeature);

  auto speedFeature = std::make_shared<MultiplierFeature>("speed");
  speedFeature->add_dependency(waterSpeedInfluence, MultiplierFeature::BOOL);
  speedFeature->add_dependency(terrainFeature, MultiplierFeature::DOUBLE); //
  // DO NOT MERGE: Not using terrain type
  speedFeature->add_dependency(gradientSpeedInfluence,
                               MultiplierFeature::DOUBLE);

  // DO NOT MEGE : Using constant path speed instead of gradient/terrain based
  auto pathSpeed = std::make_shared<MultiplierFeature>("path_speed");
  pathSpeed->add_dependency(gradientSpeedInfluence, MultiplierFeature::DOUBLE);

  // auto pathSpeed =
  // std::make_shared<ConstantFeature<double>>("path_speed", 1.0);

  auto speedWithPathFeature =
      std::make_shared<ConditionalFeature<double>>("speed_with_path");
  speedWithPathFeature->add_dependency(pathFeature);
  speedWithPathFeature->add_dependency(pathSpeed);
  speedWithPathFeature->add_dependency(speedFeature);

  auto inverseSpeedFeature =
      std::make_shared<InverseFeature<double, double>>("inverse_speed");
  inverseSpeedFeature->add_dependency(speedWithPathFeature);

  auto timeFeature = std::make_shared<MultiplierFeature>("time");
  timeFeature->add_dependency(distance, MultiplierFeature::DOUBLE);
  timeFeature->add_dependency(inverseSpeedFeature, MultiplierFeature::DOUBLE);

  fm.setOutputFeature(timeFeature);

  TSR_LOG_TRACE("initializing features");

  waterFeature->initialize(tin, boundary);
  pathFeature->initialize(tin, boundary);
  terrainFeature->initialize(tin, boundary);

  TSR_LOG_TRACE("final vertex count: {}", tin.number_of_vertices());

  /// DEBUG: Write mesh to obj
  SurfaceMesh tmpMesh;
  convertTINToMesh(tin, tmpMesh);
  IO::write_mesh_to_obj("test.obj", tmpMesh);

  TSR_LOG_TRACE("tagging features");

  terrainFeature->tag(tin);
  waterFeature->tag(tin);
  pathFeature->tag(tin);

  /// DEBUG: Write watermap to KML
  waterFeature->writeWaterMapToKML();

  TSR_LOG_TRACE("Preparing router");
  Router router;

  // Calculate the optimal route
  TSR_LOG_TRACE("Calculating Route");
  TSR_LOG_TRACE("ll: {} {}", boundary.getLLCorner().x(),
                boundary.getLLCorner().y());
  TSR_LOG_TRACE("ur: {} {}", boundary.getURCorner().x(),
                boundary.getURCorner().y());
  auto route = router.calculateRoute(tin, fm, boundary, startPoint, endPoint);

  // Convert the points to WGS84
  std::vector<Point3> routeWGS84;
  for (auto &point : route) {
    Point3 pointWGS84 = UTM_point_to_WGS84(point, 30, true);
    routeWGS84.push_back(pointWGS84);
  }

  // Output the route as a gpx file
  IO::write_data_to_file("route.gpx", IO::formatPointsAsGPXRoute(routeWGS84));

  return EXIT_SUCCESS;
}

} // namespace tsr

int main(int argc, char **argv) {
  try {
    // Define allowed options
    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "Print help message")(
        "example", "Run an example with hardcoded coordinates")(
        "start_lat", po::value<double>(), "Starting latitude")(
        "start_lon", po::value<double>(), "Starting longitude")(
        "end_lat", po::value<double>(),
        "Ending latitude")("end_lon", po::value<double>(), "Ending longitude");

    // Map positional arguments to options
    po::positional_options_description positionalArgs;
    positionalArgs.add("start_lat", 1)
        .add("start_lon", 1)
        .add("end_lat", 1)
        .add("end_lon", 1);

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
      std::cout
          << "Usage: ./tsr-route <start_lat> <start_lon> <end_lat> <end_lon>\n"
          << "Example: ./tsr-route 56.777800 -5.024737 56.809481 -5.025113"
          << std::endl;
      return 0;
    }

    if (vm.count("example")) {
      return tsr::tsr_run(56.777800, -5.024737, 56.809481, -5.025113);
    }

    // Validate positional arguments
    if (!vm.count("start_lat") || !vm.count("start_lon") ||
        !vm.count("end_lat") || !vm.count("end_lon")) {
      std::cerr << "Parsed values:" << std::endl;
      if (!vm.count("start_lat"))
        std::cerr << "  Missing: start_lat" << std::endl;
      if (!vm.count("start_lon"))
        std::cerr << "  Missing: start_lon" << std::endl;
      if (!vm.count("end_lat"))
        std::cerr << "  Missing: end_lat" << std::endl;
      if (!vm.count("end_lon"))
        std::cerr << "  Missing: end_lon" << std::endl;
      throw std::runtime_error(
          "Must provide all four positional arguments: start_lat, start_lon, "
          "end_lat, end_lon");
    }

    // Parse arguments
    double start_lat = vm["start_lat"].as<double>();
    double start_lon = vm["start_lon"].as<double>();
    double end_lat = vm["end_lat"].as<double>();
    double end_lon = vm["end_lon"].as<double>();

    // Debugging: Print parsed values
    std::cout << "Parsed values:" << std::endl;
    std::cout << "  start_lat=" << start_lat << std::endl;
    std::cout << "  start_lon=" << start_lon << std::endl;
    std::cout << "  end_lat=" << end_lat << std::endl;
    std::cout << "  end_lon=" << end_lon << std::endl;

    return tsr::tsr_run(start_lat, start_lon, end_lat, end_lon);

  } catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
}
