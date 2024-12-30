#include "tsr/IO/KMLWriter.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/TSRState.hpp"
#include "tsr/logging.hpp"
#include <CGAL/Kernel/global_functions_3.h>
#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace tsr::IO {

void writeSuccessStateToKML(const std::string &filepath, TSRState &state) {

  // TODO: Draw the route
  state.processWarnings();

  auto route = state.fetchRoute();

  auto routeKML = generateKMLRoute(route);

  auto warningsKML = generateKMLWarnings(state);

  auto kml = generateKMLDocument(routeKML + warningsKML);

  IO::write_data_to_file(filepath, kml);
}

void writeFailureStateToKML(const std::string &filepath, TSRState &state) {
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
    Point_3 p1_UTM = face->vertex(0)->point();
    Point_3 p2_UTM = face->vertex(1)->point();
    Point_3 p3_UTM = face->vertex(2)->point();

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

std::string generateKMLWarnings(const TSRState &state) {

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
    Point_3 centerWGS84;
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

std::string generateKMLRoute(const std::vector<Point_3> &route) {

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
  kml += "<Placemark>\n";
  kml += "<name>route</name>\n";
  kml += "<styleUrl>routeStyle</styleUrl>\n";
  kml += "<LineString>\n";
  kml += "<altitudeMode>clampToGround</altitudeMode>\n";
  kml += "<coordinates>\n";

  for (const auto &point : route) {

    auto pointWGS84 = UTM_point_to_WGS84(point, 30, true);

    kml += std::to_string(pointWGS84.y()) + "," +
           std::to_string(pointWGS84.x()) + "," + "0\n";
  }

  kml += "</coordinates>\n";
  kml += "</LineString>\n";
  kml += "</Placemark>\n";
  kml += "</Folder>\n";

  return kml;
}

std::string generateKMLForAllRoutes(const TSRState &state) {

  // TODO: generate KML route mesh
  return "";
}

} // namespace tsr::IO