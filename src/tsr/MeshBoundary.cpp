#include "tsr/MeshBoundary.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"
#include <cmath>
#include <vector>

#include "tsr/Logging.hpp"
#include "tsr/PointProcessor.hpp"

namespace tsr {

MeshBoundary::MeshBoundary(const Point3 &source_point,
                           const Point3 &target_point,
                           const double radii_multiplier) {

  const double distance = CalculateXYDistance(source_point, target_point);
  const double radius = (distance / 2.0) * radii_multiplier;

  this->midpoint = Point2((source_point.x() + target_point.x()) / 2.0,
                          (source_point.y() + target_point.y()) / 2.0);

  this->angle = CalculateXYAngle(source_point, target_point);

  this->width = distance + 2 * radius;
  this->height = 2 * radius;

  double bboxMinX = this->midpoint.x() - this->width / 2;
  double bboxMaxX = this->midpoint.x() + this->width / 2;
  double bboxMinY = this->midpoint.y() - this->height / 2;
  double bboxMaxY = this->midpoint.y() + this->height / 2;

  Point2 p1 =
      rotatePoint(Point2(bboxMinX, bboxMinY), this->midpoint, this->angle);
  Point2 p2 =
      rotatePoint(Point2(bboxMaxX, bboxMinY), this->midpoint, this->angle);
  Point2 p3 =
      rotatePoint(Point2(bboxMinX, bboxMaxY), this->midpoint, this->angle);
  Point2 p4 =
      rotatePoint(Point2(bboxMaxX, bboxMaxY), this->midpoint, this->angle);

  double minX = std::min({p1.x(), p2.x(), p3.x(), p4.x()});
  double maxX = std::max({p1.x(), p2.x(), p3.x(), p4.x()});
  double minY = std::min({p1.y(), p2.y(), p3.y(), p4.y()});
  double maxY = std::max({p1.y(), p2.y(), p3.y(), p4.y()});

  TSR_LOG_TRACE("b min x: {}", minX);
  TSR_LOG_TRACE("b max x: {}", maxX);
  TSR_LOG_TRACE("b min y: {}", minY);
  TSR_LOG_TRACE("b max y: {}", maxY);

  this->ll = Point2(minX, maxY);
  this->ur = Point2(maxX, minY);
}

Point2 MeshBoundary::rotatePoint(const Point2 &p, const Point2 &midpoint,
                                 double angle) {
  double s = std::sin(angle);
  double c = std::cos(angle);

  // Translate point to origin
  double x = p.x() - midpoint.x();
  double y = p.y() - midpoint.y();

  // Rotate point
  double newX = x * c - y * s;
  double newY = x * s + y * c;

  // Translate back
  return Point2(newX + midpoint.x(), newY + midpoint.y());
}

Point3 MeshBoundary::rotatePoint(const Point3 &p, const Point2 &midpoint,
                                 double angle) {
  double s = std::sin(angle);
  double c = std::cos(angle);

  // Translate point to origin
  double x = p.x() - midpoint.x();
  double y = p.y() - midpoint.y();

  // Rotate point
  double newX = x * c - y * s;
  double newY = x * s + y * c;

  // Translate back
  return Point3(newX + midpoint.x(), newY + midpoint.y(), p.z());
}

bool MeshBoundary::IsBounded(Point3 p) const {
  // Rotate the point back (inverse rotation)
  Point3 rotatedPoint = rotatePoint(p, this->midpoint, -this->angle);

  // Check if the rotated point is within the axis-aligned rectangle
  double halfWidth = width / 2.0;
  double halfHeight = height / 2.0;

  bool withinBounds = (rotatedPoint.x() >= this->midpoint.x() - halfWidth &&
                       rotatedPoint.x() <= this->midpoint.x() + halfWidth &&
                       rotatedPoint.y() >= this->midpoint.y() - halfHeight &&
                       rotatedPoint.y() <= this->midpoint.y() + halfHeight);

  return withinBounds;
}
bool MeshBoundary::IsBoundedSafe(Point3 p) const {
  // Rotate the point back (inverse rotation)
  Point3 rotatedPoint = rotatePoint(p, this->midpoint, -this->angle);

  // Check if the rotated point is within the axis-aligned rectangle
  double halfWidth = width / 2.0;
  double halfHeight = height / 2.0;

  double SAFE_DISTANCE_M = 30;

  bool withinBounds =
      (rotatedPoint.x() >= this->midpoint.x() - halfWidth + SAFE_DISTANCE_M &&
       rotatedPoint.x() <= this->midpoint.x() + halfWidth - SAFE_DISTANCE_M &&
       rotatedPoint.y() >= this->midpoint.y() - halfHeight + SAFE_DISTANCE_M &&
       rotatedPoint.y() <= this->midpoint.y() + halfHeight - SAFE_DISTANCE_M);

  return withinBounds;
}

void MeshBoundary::FilterPointsOutsideBoundary(
    std::vector<Point3> points) const {

  std::vector<Point3> filteredPoints;
  for (auto p : points) {
    if (this->IsBounded(p)) {
      filteredPoints.push_back(p);
    }
  }

  TSR_LOG_TRACE("Discarded {} points outside boundary",
                points.size() - filteredPoints.size());

  points.swap(filteredPoints);
}

Point2 MeshBoundary::GetLowerLeftPoint() const {
  // Calculate the lower left corner
  return this->ll;
}

Point2 MeshBoundary::GetUpperRightPoint() const {
  // Calculate the upper right corner
  return this->ur;
}

} // namespace tsr