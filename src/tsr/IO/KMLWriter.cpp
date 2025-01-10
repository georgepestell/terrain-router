#include "tsr/IO/KMLWriter.hpp"
#include "tsr/Features/GradientSpeedFeature.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point3.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include <CGAL/Kernel/global_functions_3.h>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>

#include "tsr/Features/GradientFeature.hpp"

namespace tsr::IO {

void writeSuccessStateToKML(const std::string &filepath, TsrState &state) {

  state.ProcessWarnings();

  auto route = state.fetchRoute();

  auto routeKML = generateKMLRoute(route);

  auto warningsKML = generateKMLWarnings(state);

  auto kml = generateKMLDocument(routeKML + warningsKML);

  IO::write_data_to_file(filepath, kml);
}

void writeFailureStateToKML(const std::string &filepath, TsrState &state) {
  // TODO: Draw all routes
  auto routesKML = generateKMLForAllRoutes(state);

  auto warningsKML = generateKMLWarnings(state);

  auto kml = generateKMLDocument(routesKML + warningsKML);

  IO::write_data_to_file(filepath, kml);
}

std::string generateKMLDocument(const std::string &inner_kml) {

  std::string kml;

  kml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  kml += "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
  kml += "<Document>\n";
  kml += "  <name>TSR Router</name>\n";

  kml += inner_kml;

  kml += "</Document>\n";
  kml += "</kml>\n";

  return kml;
}

std::string generateKMLFaces(std::vector<Face_handle> &faces) {
  std::string kml;

  for (uint f = 0; f < faces.size(); f++) {

    Face_handle face = faces[f];
    Point3 p1_UTM = face->vertex(0)->point();
    Point3 p2_UTM = face->vertex(1)->point();
    Point3 p3_UTM = face->vertex(2)->point();

    auto p1 = UTM_point_to_WGS84(p1_UTM, 30, true);
    auto p2 = UTM_point_to_WGS84(p2_UTM, 30, true);
    auto p3 = UTM_point_to_WGS84(p3_UTM, 30, true);

    kml += "<Placemark>\n";
    kml += "  <name>Triangle " + std::to_string(f) + "</name>\n";
    kml += "  <styleUrl>#blueStyle</styleUrl>\n";
    kml += "  <Polygon>\n";
    kml += "    <altitudeMode>absolute</altitudeMode>\n"; // Ensure the altitude
                                                          // is interpreted as
                                                          // absolute height
                                                          // above sea level
    kml += "    <outerBoundaryIs>\n";
    kml += "      <LinearRing>\n";
    kml += "        <coordinates>\n";

    kml += "          " + std::to_string(p1.y()) + "," +
           std::to_string(p1.x()) + "," + std::to_string((p1.z() + 1)) + "\n";
    kml += "          " + std::to_string(p2.y()) + "," +
           std::to_string(p2.x()) + "," + std::to_string((p2.z() + 1)) + "\n";
    kml += "          " + std::to_string(p3.y()) + "," +
           std::to_string(p3.x()) + "," + std::to_string((p3.z() + 1)) + "\n";
    kml += "          " + std::to_string(p1.y()) + "," +
           std::to_string(p1.x()) + "," + std::to_string((p1.z() + 1)) + "\n";

    kml += "        </coordinates>\n";
    kml += "      </LinearRing>\n";
    kml += "    </outerBoundaryIs>\n";
    kml += "  </Polygon>\n";
    kml += "</Placemark>\n";
  }

  return kml;
}

std::string generateKMLWarnings(const TsrState &state) {

  TSR_LOG_TRACE("generate warnings KML");
  TSR_LOG_TRACE("warning count: {}", state.warnings.size());

  std::string kml;
  kml += "<Folder>\n";
  kml += "<name>Warnings</name>\n";

  // For each warning, add a pin
  for (const auto &warning : state.warnings) {

    // Skip empty warnings
    if (warning.second == 0) {
      continue;
    }

    const std::string message = state.warning_messages[warning.second];

    auto p1 = warning.first->vertex(0)->point();
    auto p2 = warning.first->vertex(1)->point();
    auto p3 = warning.first->vertex(2)->point();

    // Get the center of the triangle
    auto center = CGAL::circumcenter(p1, p2, p3);

    // Convert the center to WGS84
    Point3 centerWGS84;
    try {
      centerWGS84 = UTM_point_to_WGS84(center, 30, true);
    } catch (std::exception e) {
      continue;
    }

    kml += "<Placemark>\n";
    kml += "<altitudeMode>clampToGround</altitudeMode>\n";
    kml += "<name>" + message + "</name>\n";
    kml += "<description>" + message + "</description>\n";
    kml += "<Point>\n";
    kml += "<coordinates>";
    kml += std::to_string(centerWGS84.y()) + "," +
           std::to_string(centerWGS84.x()) + "," + "0";
    kml += "</coordinates>\n";
    kml += "</Point>\n";
    kml += "</Placemark>\n";
  }

  kml += "</Folder>\n";

  return kml;
}

std::string generateKMLRoute(const std::vector<Point3> &route) {

  // Used to show gradient
  GradientSpeedFeature Fg("gradient");

  if (route.empty()) {
    TSR_LOG_WARN("Route empty");
    return "";
  }

  std::string kml;

  auto startPointWGS84 = UTM_point_to_WGS84(route[0], 30, true);
  auto endPointWGS84 = UTM_point_to_WGS84(route[route.size() - 1], 30, true);

  kml += "<Folder>\n";
  kml += "<name>Route</name>\n";

  kml += "<Style "
         "id=\"routeStyle\"><IconStyle><color>#ff61ffb8</color>";
  kml += "<Icon><href>http://maps.google.com/mapfiles/kml/paddle/"
         "blu-blank.png</href></Icon>";
  kml += "</"
         "IconStyle><LineStyle><color>#ff61ffb8</color><width>4</width></"
         "LineStyle></Style>";

  kml += "<Style "
         "id=\"slightGradientWarning\"><LineStyle><color>#ffffa500</"
         "color><width>4</width></"
         "LineStyle></Style>";
  kml += "<Style "
         "id=\"steepGradientWarning\"><LineStyle><color>#ffff4500</"
         "color><width>4</width></"
         "LineStyle></Style>";

  // Add the waypoints for start and end point
  kml += "<Placemark>\n";
  kml += "<altitudeMode>clampToGround</altitudeMode>\n";
  kml += "<styleUrl>routeStyle</styleUrl>\n";
  kml += "<name>Start Point</name>\n";
  kml += "<Point>\n";
  kml += "<coordinates>";
  kml += std::to_string(startPointWGS84.y()) + "," +
         std::to_string(startPointWGS84.x()) + ",0";
  kml += "</coordinates>\n";
  kml += "</Point>\n";
  kml += "</Placemark>\n";

  kml += "<Placemark>\n";
  kml += "<altitudeMode>clampToGround</altitudeMode>\n";
  kml += "<styleUrl>routeStyle</styleUrl>\n";
  kml += "<name>End Point</name>\n";
  kml += "<Point>\n";
  kml += "<coordinates>";
  kml += std::to_string(endPointWGS84.y()) + "," +
         std::to_string(endPointWGS84.x()) + ",0";
  kml += "</coordinates>\n";
  kml += "</Point>\n";
  kml += "</Placemark>\n";

  // Add the route
  for (unsigned int i = 0; i < route.size() - 1; i++) {

    // Get the gradient from current point to next point
    auto sourcePoint = route.at(i);
    auto targetPoint = route.at(i + 1);

    double gradient =
        GradientFeature::calculate_gradient(sourcePoint, targetPoint);

    kml += "<Placemark>\n";
    kml += "<name>route segment</name>\n";

    if (gradient < -0.2 || gradient > 0.2) {
      if (gradient < -0.3 || gradient > 0.3) {
        kml += "<styleUrl>steepGradientWarning</styleUrl>\n";
      } else {
        kml += "<styleUrl>slightGradientWarning</styleUrl>\n";
      }
    } else {
      kml += "<styleUrl>routeStyle</styleUrl>\n";
    }

    kml += "<LineString>\n";
    kml += "<altitudeMode>clampToGround</altitudeMode>\n";
    kml += "<coordinates>\n";

    auto sourcePointWGS84 = UTM_point_to_WGS84(sourcePoint, 30, true);
    auto targetPointWGS84 = UTM_point_to_WGS84(targetPoint, 30, true);

    kml += std::to_string(sourcePointWGS84.y()) + "," +
           std::to_string(sourcePointWGS84.x()) + "," + "0\n";
    kml += std::to_string(targetPointWGS84.y()) + "," +
           std::to_string(targetPointWGS84.x()) + "," + "0\n";

    kml += "</coordinates>\n";
    kml += "</LineString>\n";
    kml += "</Placemark>\n";
  }

  kml += "</Folder>\n";

  return kml;
}

std::string generateKMLForAllRoutes(const TsrState &state) {

  // TODO: generate KML route mesh
  return "";
}

} // namespace tsr::IO