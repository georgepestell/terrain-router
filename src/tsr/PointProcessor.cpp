#include "tsr/PointProcessor.hpp"
#include "tsr/Logging.hpp"
#include "tsr/MeshBoundary.hpp"
#include <GeographicLib/UTMUPS.hpp>
#include <cmath>

#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"

#include "tsr/MapUtils.hpp"

namespace tsr {

Point3 TranslateUtmPointToWgs84(Point3 pointUTM, short zone,
                          bool isNorthernHemisphere) {

  double lat, lon;

  GeographicLib::UTMUPS::Reverse(zone, isNorthernHemisphere, pointUTM.x(),
                                 pointUTM.y(), lat, lon);

  return Point3(lat, lon, pointUTM.z());
}

Point2 TranslateUtmPointToWgs84(Point2 pointUTM, short zone,
                          bool isNorthernHemisphere) {

  double lat, lon;

  GeographicLib::UTMUPS::Reverse(zone, isNorthernHemisphere, pointUTM.x(),
                                 pointUTM.y(), lat, lon);

  return Point2(lat, lon);
}

Point2 TranslateWgs84PointToUtm(Point2 pointWGS84) {

  int zone = CalculateUtmZone(pointWGS84.y());

  bool isNorthernHemisphere = IsNorthernHemisphere(pointWGS84.x());

  double x, y;

  GeographicLib::UTMUPS::Forward(pointWGS84.x(), pointWGS84.y(), zone,
                                 isNorthernHemisphere, x, y);

  return Point2(x, y);
}

Point3 TranslateWgs84PointToUtm(Point3 pointWGS84) {

  int zone = CalculateUtmZone(pointWGS84.y());

  bool isNorthernHemisphere = IsNorthernHemisphere(pointWGS84.x());

  double x, y;

  GeographicLib::UTMUPS::Forward(pointWGS84.x(), pointWGS84.y(), zone,
                                 isNorthernHemisphere, x, y);

  return Point3(x, y, pointWGS84.z());
}

void JetSmoothPoints(std::vector<Point3> &points) {
  const unsigned int nb_neighbors = 8; // default is 24 for real-life point sets
  CGAL::jet_smooth_point_set<CGAL::Parallel_if_available_tag>(points,
                                                              nb_neighbors);
}

void SimplifyPoints(std::vector<Point3> &points) {

  points.erase(CGAL::hierarchy_simplify_point_set(
                   points,
                   CGAL::parameters::size(22)      // Max cluster size
                       .maximum_variation(0.001)), // Max surface variation
               points.end());

  // parameters
  const double retain_percentage = 70; // percentage of points to retain.
  const double neighbor_radius = 30;   // neighbors size.

  std::vector<Point3> points_simplified;

  CGAL::wlop_simplify_and_regularize_point_set<CGAL::Parallel_if_available_tag>(
      points, std::back_inserter(points_simplified),
      CGAL::parameters::select_percentage(retain_percentage)
          .neighbor_radius(neighbor_radius));

  points = points_simplified;
}

double CalculateXYAngle(const Point3 &p1, const Point3 &p2) {
  return std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
}

// Function to calculate distance between two points
double CalculateXYDistance(const Point3 &p1, const Point3 &p2) {
  return std::sqrt(std::pow(p2.x() - p1.x(), 2) + std::pow(p2.y() - p1.y(), 2));
}

}; // namespace tsr