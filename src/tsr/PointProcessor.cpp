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

  GeographicLib::UTMUPS::Forward(pointWGS84.x(), pointWGS84.y(), zone,
                                 isNorthernHemisphere, x, y);

  return Point_3(x, y, pointWGS84.z());
}

void jet_smooth_points(std::vector<Point_3> &points) {
  const unsigned int nb_neighbors = 8; // default is 24 for real-life point sets
  CGAL::jet_smooth_point_set<CGAL::Parallel_if_available_tag>(points,
                                                              nb_neighbors);
}

void simplify_points(std::vector<Point_3> &points) {

  points.erase(CGAL::hierarchy_simplify_point_set(
                   points,
                   CGAL::parameters::size(22)      // Max cluster size
                       .maximum_variation(0.001)), // Max surface variation
               points.end());

  // parameters
  const double retain_percentage = 70; // percentage of points to retain.
  const double neighbor_radius = 30;   // neighbors size.

  std::vector<Point_3> points_simplified;

  CGAL::wlop_simplify_and_regularize_point_set<CGAL::Parallel_if_available_tag>(
      points, std::back_inserter(points_simplified),
      CGAL::parameters::select_percentage(retain_percentage)
          .neighbor_radius(neighbor_radius));

  points = points_simplified;
}

}; // namespace tsr