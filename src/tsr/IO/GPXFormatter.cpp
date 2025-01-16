#include "tsr/IO/GPXFormatter.hpp"
#include "tsr/Point3.hpp"
#include <fmt/core.h>
#include <string>
#include <vector>

namespace tsr::IO {

std::string FormatRouteAsGpx(std::vector<Point3> points) {
  const std::string GPX_HEADER =
      "<?xml version='1.0' encoding='UTF-8'?>\n <gpx version='1.1' "
      "creator='tsr-route' xmlns='http://www.topografix.com/GPX/1/1'>\n "
      "\t<rte>\n\t\t<name>Test Route</name>\n";
  const std::string GPX_FOOTER = "\t</rte>\n </gpx>\n";

  std::string gpxRouteString = GPX_HEADER;

  for (auto point : points) {
    gpxRouteString.append(
        ::fmt::format("\t\t<rtept lat='{}' "
                      "lon='{}'>\n\t\t\t<name>Waypoint</name>\n\t\t</rtept>\n",
                      point.x(), point.y()));
  }

  gpxRouteString.append(GPX_FOOTER);
  return gpxRouteString;
}

} // namespace tsr::IO