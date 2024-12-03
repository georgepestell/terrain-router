#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/Features/DistanceFeature.hpp"
#include "tsr/Features/GradientFeature.hpp"
#include "tsr/Features/GradientSpeedFeature.hpp"
#include "tsr/Features/InverseFeature.hpp"
#include "tsr/Features/MultiplierFeature.hpp"
#include "tsr/Features/WaterFeature.hpp"
#include "tsr/Mesh.hpp"
#include "tsr/PointProcessor.hpp"

#include "tsr/IO/ImageIO.hpp"
#include "tsr/IO/MapIO.hpp"

#include "tsr/Point_3.hpp"
#include "tsr/logging.hpp"

#include "tsr/IO/FileIO.hpp"
#include "tsr/IO/GPXFormatter.hpp"
#include "tsr/IO/MeshIO.hpp"
#include "tsr/MeshUtils.hpp"

#include "tsr/Router.hpp"

#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
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

namespace tsr {

bool tsr_run(double sLat, double sLon, double eLat, double eLon) {

  log_set_global_loglevel(LogLevel::TRACE);

  // Convert points to UTM
  Point_3 startPoint = WGS84_point_to_UTM(Point_3(sLat, sLon, 0));
  Point_3 endPoint = WGS84_point_to_UTM(Point_3(eLat, eLon, 0));

  TSR_LOG_TRACE("Loading points");
  auto points = IO::load_dem_from_file("../data/benNevis_DEM.xyz");

  TSR_LOG_TRACE("Filtering domain");

  double DEFAULT_RADII_MULTIPLIER = 1;
  filter_points_domain(points, startPoint, endPoint, DEFAULT_RADII_MULTIPLIER);

  // TSR_LOG_TRACE("Smoothing");
  // jet_smooth_points(points);

  TSR_LOG_TRACE("Simplifying");
  simplify_points(points);

  TSR_LOG_TRACE("point count: {}", points.size());

  TSR_LOG_TRACE("Triangulating");
  auto dtm = create_tin_from_points(points);
  // auto dtm = create_tin_from_points(points);

  TSR_LOG_TRACE("Simplfying");
  simplify_tin(dtm, dtm);

  // Add water contours
  TSR_LOG_TRACE("Preparing water contours");
  auto waterImage = IO::load_image_from_file("../data/benNevis_water.tiff");

  auto waterRGB = IO::convert_grayscale_image_to_rgb(waterImage);

  auto contours = IO::extract_feature_contours(waterRGB, 0.001, 369951.000,
                                               6304362.000, 3.0, 3.0);

  TSR_LOG_TRACE("Vertices: {}", dtm.number_of_vertices());

  TSR_LOG_TRACE("Adding water contours");
  for (auto contour : *contours) {
    add_contour_constraint(dtm, contour, 30);
  }

  Mesh tmpMesh;
  convert_tin_to_surface_mesh(dtm, tmpMesh);

  IO::write_mesh_to_obj("test.obj", tmpMesh);

  // Feature Manager Configuration
  TSR_LOG_TRACE("Setting up feature manager");
  FeatureManager fm;

  auto gradientFeature = std::make_shared<GradientFeature>("gradient");
  auto distance = std::make_shared<DistanceFeature>("distance");
  auto gradientSpeedInfluence =
      std::make_shared<GradientSpeedFeature>("gradient_speed");
  gradientSpeedInfluence->add_dependency(gradientFeature);

  auto waterDataset =
      IO::load_gdal_dataset_from_file("../data/benNevis_water.tiff");
  std::shared_ptr<BoolWaterFeature> waterFeature;
  try {
    waterFeature =
        std::make_shared<BoolWaterFeature>("water", dtm, *waterDataset);
  } catch (std::exception e) {
    GDALClose(*waterDataset);
    throw e;
  }

  GDALClose(*waterDataset);

  waterFeature->writeWaterMapToKML();

  auto waterSpeedInfluence = std::make_shared<InverseFeature<bool, bool>>("water_speed");
  waterSpeedInfluence->add_dependency(waterFeature);

  auto speedFeature = std::make_shared<MultiplierFeature>("speed");
  speedFeature->add_dependency(gradientSpeedInfluence, MultiplierFeature::DOUBLE);
  speedFeature->add_dependency(waterSpeedInfluence, MultiplierFeature::BOOL);

  auto inverseSpeedFeature = std::make_shared<InverseFeature<double, double>>("inverse_speed");
  inverseSpeedFeature->add_dependency(speedFeature);

  auto timeFeature = std::make_shared<MultiplierFeature>("time");
  timeFeature->add_dependency(distance, MultiplierFeature::DOUBLE);
  timeFeature->add_dependency(inverseSpeedFeature,
  MultiplierFeature::DOUBLE);

  fm.setOutputFeature(timeFeature);

  TSR_LOG_TRACE("Preparing router");
  Router router;

  // Calculate the optimal route
  TSR_LOG_TRACE("Calculating Route");
  auto route = router.calculateRoute(dtm, fm, startPoint, endPoint);

  // Convert the points to WGS84
  std::vector<Point_3> routeWGS84;
  for (auto &point : route) {
    Point_3 pointWGS84 = UTM_point_to_WGS84(point, 30, true);
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
            .style(po::command_line_style::long_allow_next |
                   po::command_line_style::allow_long_disguise)

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
