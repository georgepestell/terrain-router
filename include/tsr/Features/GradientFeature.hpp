#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/logging.hpp"

namespace tsr {

class GradientFeature : public Feature<double> {

public:
  using Feature<double>::Feature;

  static double calculate_gradient(Point_3 &p1, Point_3 &p2) {

    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    double distance = std::hypot(dx, dy);

    if (distance == 0) {
      TSR_LOG_WARN(
          "Cannot calculate gradient - requires zero distance division");
      return 0;
    }

    double dz = p2.z() - p1.z();
    return dz / distance;
  }

  double calculate(Face_handle face, Point_3 &source_point,
                   Point_3 &target_point) {
    return calculate_gradient(source_point, target_point);
  }
};

} // namespace tsr
