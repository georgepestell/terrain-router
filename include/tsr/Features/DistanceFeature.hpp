#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"
#include "tsr/Point_3.hpp"

#include <CGAL/Distance_3/Point_3_Point_3.h>
#include <cmath>

namespace tsr {

class DistanceFeature : public Feature<double> {
public:
  using Feature<double>::Feature;

  double calculateDistance(Point_3 p1, Point_3 p2) {

    return std::sqrt(CGAL::squared_distance(p1, p2));
  }

  double calculate(Face_handle face, Point_3 &source_point,
                   Point_3 &target_point) override {

    return calculateDistance(source_point, target_point);
  }
};

} // namespace tsr