#pragma once

#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"

#include <vector>

namespace tsr {

class MeshBoundary {
public:
  double width;
  double height;
  double angle;
  double radii_multiplier;
  Point2 ur;
  Point2 ll;
  Point2 midpoint;

  MeshBoundary(const Point3 &source_point, const Point3 &target_point,
               const double radii_multiplier);

  static Point3 rotatePoint(const Point3 &p, const Point2 &midpoint,
                            double angle);
  static Point2 rotatePoint(const Point2 &p, const Point2 &midpoint,
                            double angle);

  bool IsBounded(const Point3 &p) const;
  bool IsBounded(const Point2 &p) const;
  bool IsBoundedSafe(const Point3 &p) const;

  void FilterPointsOutsideBoundary(std::vector<Point3> points) const;

  Point2 GetLowerLeftPoint() const;
  Point2 GetUpperRightPoint() const;
};

} // namespace tsr