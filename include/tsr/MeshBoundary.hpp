#pragma once

#include "tsr/Point_2.hpp"
#include "tsr/Point_3.hpp"

#include <vector>

namespace tsr {

class MeshBoundary {
public:
  double width;
  double height;
  double angle;
  double radii_multiplier;
  Point_2 ur;
  Point_2 ll;
  Point_2 midpoint;

  MeshBoundary(const Point_3 &source_point, const Point_3 &target_point,
               const double radii_multiplier);

  static Point_3 rotatePoint(const Point_3 &p, const Point_2 &midpoint,
                             double angle);
  static Point_2 rotatePoint(const Point_2 &p, const Point_2 &midpoint,
                             double angle);

  bool isBounded(Point_3 p) const;
  bool isBoundedSafe(Point_3 p) const;

  void filterPointsOutsideBoundary(std::vector<Point_3> points) const;

  Point_2 getLLCorner() const;
  Point_2 getURCorner() const;
};

} // namespace tsr