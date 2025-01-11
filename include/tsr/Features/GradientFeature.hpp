#pragma once

#include "tsr/Feature.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include <boost/concept_check.hpp>

namespace tsr {

class GradientFeature : public Feature<double> {

public:
  using Feature<double>::Feature;

  static double CalculateGradient(const Point3 &p1, const Point3 &p2) {

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

  double Calculate(TsrState &state) {
    return CalculateGradient(state.current_vertex->point(),
                             state.next_vertex->point());
  }
};

} // namespace tsr
