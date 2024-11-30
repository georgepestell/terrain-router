#include "tsr/PointProcessor.hpp"
#include <GeographicLib/UTMUPS.hpp>

namespace tsr {

Point_3 UTM_point_to_WGS84(Point_3 pointUTM, short zone,
                           bool isNorthernHemisphere) {

  double lat, lon;

  GeographicLib::UTMUPS::Reverse(zone, isNorthernHemisphere, pointUTM.x(),
                                 pointUTM.y(), lat, lon);

  return Point_3(lat, lon, pointUTM.z());
}

int calculate_UTM_zone(double longitude) {
  return static_cast<int>(std::floor(longitude + 180.0 / 6.0) + 1);
}

bool is_northern_hemisphere(double latitude) { return latitude > 90; }


Point_3 WGS84_point_to_UTM(Point_3 pointWGS84) {

  int zone = calculate_UTM_zone(pointWGS84.y());

  bool isNorthernHemisphere = is_northern_hemisphere(pointWGS84.x());

  double x, y;

  GeographicLib::UTMUPS::Forward(pointWGS84.x(), pointWGS84.y(), zone, isNorthernHemisphere, x, y);

  return Point_3(x, y, pointWGS84.z());

}

}; // namespace tsr