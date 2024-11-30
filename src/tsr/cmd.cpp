#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/Features/DistanceFeature.hpp"
#include "tsr/Features/GradientFeature.hpp"
#include "tsr/Features/GradientSpeedFeature.hpp"
#include "tsr/Features/InverseFeature.hpp"
#include "tsr/Features/MultiplierFeature.hpp"
#include "tsr/PointProcessor.hpp"

#include "tsr/IO/ImageIO.hpp"

#include "tsr/Point_3.hpp"
#include "tsr/logging.hpp"

#include "tsr/IO/FileIO.hpp"
#include "tsr/IO/GPXFormatter.hpp"
#include "tsr/IO/MeshIO.hpp"
#include "tsr/MeshUtils.hpp"

#include "tsr/Router.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <cstdlib>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

#define DEFAULT_DEM_FILE SOURCE_ROOT "/data/benNevis_DEM.xyz"

namespace tsr {

bool tsr_run(double start_lat, double start_lon, double end_lat,
             double end_lon) {

  log_set_global_loglevel(LogLevel::TRACE);

  // Convert points to UTM
  Point_3 startPoint = WGS84_point_to_UTM(Point_3(start_lat, start_lon, 0));
  Point_3 endPoint = WGS84_point_to_UTM(Point_3(end_lat, end_lon, 0));

  TSR_LOG_TRACE("Loading points");
  auto points = IO::load_dem_from_file("../data/benNevis_DEM.xyz");

  TSR_LOG_TRACE("point count: {}", points.size());

  TSR_LOG_TRACE("Triangulating");
  auto dtm = create_tin_from_points(points, startPoint, endPoint, 1);

  TSR_LOG_TRACE("Simplfying");
  simplify_tin(dtm, dtm);

  // Add water contours
  TSR_LOG_TRACE("Preparing water contours");
  auto waterImage = IO::load_image_from_file("../data/benNevis_water.tiff");

  auto waterRGB = IO::convert_grayscale_image_to_rgb(waterImage);

  auto contours = IO::extract_feature_contours(waterRGB, 0.001, 369951.000,
                                               6304362.000, 3.0, 3.0);

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
  auto distanceFeature = std::make_shared<DistanceFeature>("distance");
  auto speedFeature = std::make_shared<GradientSpeedFeature>("gradient_speed");
  auto timeFeature = std::make_shared<MultiplierFeature>("time");

  speedFeature->add_dependency(gradientFeature);

  auto inverseSpeedFeature =
      std::make_shared<InverseFeature<double>>("inverse_speed");

  inverseSpeedFeature->add_dependency(speedFeature);

  timeFeature->add_dependency(distanceFeature, MultiplierFeature::DOUBLE);
  timeFeature->add_dependency(inverseSpeedFeature, MultiplierFeature::DOUBLE);

  fm.setOutputFeature(timeFeature);

  TSR_LOG_TRACE("Preparing router");
  Router router(dtm, fm);

  // Calculate the optimal route
  TSR_LOG_TRACE("Calculating Route");
  auto route = router.calculateRoute(startPoint, endPoint);

  // Convert the points to WGS84
  std::vector<Point_3> routeWGS84;
  for (auto &point : route) {
    Point_3 pointWGS84 = UTM_point_to_WGS84(point, 30, true);
    routeWGS84.push_back(pointWGS84);
  }

  // Output the route as a gpx file
  IO::write_data_to_file("route.gpx", IO::formatPointsAsGPXRoute(routeWGS84));

  // TSR_LOG_TRACE("Writing to obj file");
  // Surface_mesh surface_mesh;
  // convert_tin_to_surface_mesh(dtm, surface_mesh);
  // write_mesh_to_obj("testBigConstraints.obj", surface_mesh);
  return EXIT_SUCCESS;
}

} // namespace tsr

int main(int argc, char **argv) {

  try {

    // if (argc != 5) {
    //   throw std::runtime_error("Must provide start and end lat/lon values");
    // }

    po::options_description desc("Allowed options");

    desc.add_options()("help,h", "print help message")(
        "example", po::value<bool>(), "Run an example with start: , end: ");

    po::positional_options_description positionalArgs;
    positionalArgs.add("start_lat", 1)
        .add("start_lon", 1)
        .add("end_lat", 1)
        .add("end_lon", 1);

    po::variables_map vm;

    po::store(po::command_line_parser(argc, argv)
                  .options(desc)
                  .positional(positionalArgs)
                  .run(),
              vm);
    po::notify(vm);

    if (vm.count("help")) {
      TSR_LOG_INFO(
          "Usage: ./tsr-route <start_lat> <start_lon> <end_lat> <end_lon>");
      return 0;
    }

    if (vm.count("example")) {
      return tsr::tsr_run(56.777800, -5.024737, 56.809481, -5.025113);
    }

    double start_lat(vm["start_lat"].as<double>());
    double start_lon(vm["start_lon"].as<double>());
    double end_lat(vm["end_lat"].as<double>());
    double end_lon(vm["end_lon"].as<double>());

    return tsr::tsr_run(start_lat, start_lon, end_lat, end_lon);

  } catch (const std::exception &e) {
    TSR_LOG_ERROR("ERROR: {}", e.what());
    return 1;
  }
}