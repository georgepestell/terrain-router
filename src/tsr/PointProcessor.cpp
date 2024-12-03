#include "tsr/PointProcessor.hpp"
#include "tsr/logging.hpp"
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

double calculateAngle(const Point_3 &p1, const Point_3 &p2) {
  return std::atan2(p2.y() - p1.y(), p2.x() - p1.x());
}

// Function to calculate distance between two points
double calculateDistance(const Point_3 &p1, const Point_3 &p2) {
  return std::sqrt(std::pow(p2.x() - p1.x(), 2) + std::pow(p2.y() - p1.y(), 2));
}

// Function to rotate a point around a center by an angle
Point_3 rotatePoint(const Point_3 &center, const Point_3 &p, double angle) {
  double s = std::sin(angle);
  double c = std::cos(angle);

  // Translate point to origin
  double x = p.x() - center.x();
  double y = p.y() - center.y();

  // Rotate point
  double newX = x * c - y * s;
  double newY = x * s + y * c;

  // Translate back
  return Point_3(newX + center.x(), newY + center.y(), p.z());
}

// Function to check if a point is inside a rotated rectangle
bool isPointInRectangle(const Point_3 &center, double width, double height,
                        double angle, const Point_3 &point) {
  // Rotate the point back (inverse rotation)
  Point_3 rotatedPoint = rotatePoint(center, point, -angle);

  // Check if the rotated point is within the axis-aligned rectangle
  double halfWidth = width / 2.0;
  double halfHeight = height / 2.0;

  bool withinBounds = (rotatedPoint.x() >= center.x() - halfWidth &&
                       rotatedPoint.x() <= center.x() + halfWidth &&
                       rotatedPoint.y() >= center.y() - halfHeight &&
                       rotatedPoint.y() <= center.y() + halfHeight);

  return withinBounds;
}

void filter_points_domain(std::vector<Point_3> &points, Point_3 &source_point,
                          Point_3 &target_point, double radii_multiplier) {

  // validate point set contains points
  if (points.empty()) {
    TSR_LOG_WARN("Points empty");
    return;
  }

  // triangulate points using 2.5D Delaunay triangulation
  std::vector<Point_3> filteredPoints;

  // Calculate domain boundary

  double distance = calculateDistance(source_point, target_point);
  Point_3 midpoint = Point_3((source_point.x() + target_point.x()) / 2.0,
                             (source_point.y() + target_point.y()) / 2.0, 0);

  double radius = (distance / 2.0) * radii_multiplier;

  double angle = calculateAngle(source_point, target_point);

  double boundaryWidth = distance + 2 * radius;
  double boundaryHeight = 2 * radius;

  // Add points inside boundary
  int d = 0;
  for (auto p : points) {
    bool inRectangle =
        isPointInRectangle(midpoint, boundaryWidth, boundaryHeight, angle, p);
    if (inRectangle) {
      filteredPoints.push_back(p);
    } else {
      d++;
    }
  }

  TSR_LOG_TRACE("Discarded {} points", d);
  points.swap(filteredPoints);
}

}; // namespace tsr