#pragma once

#include "tsr/Feature.hpp"
#include "tsr/Point3.hpp"
#include "tsr/TsrState.hpp"

namespace tsr {

class GradientFeature : public Feature<double> {

public:
  using Feature<double>::Feature;

  static double CalculateGradient(const Point3 &p1, const Point3 &p2);

  double Calculate(TsrState &state) {
    return CalculateGradient(state.current_vertex->point(),
                             state.next_vertex->point());
  }
};

} // namespace tsr
