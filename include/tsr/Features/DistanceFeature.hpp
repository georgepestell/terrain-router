#pragma once

#include "tsr/Feature.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"

#include <CGAL/Distance_3/Point_3_Point_3.h>
#include <boost/concept_check.hpp>
#include <cmath>

namespace tsr {

class DistanceFeature : public Feature<double> {
public:
  using Feature<double>::Feature;

  double calculateDistance(Point3 p1, Point3 p2) {

    return std::sqrt(CGAL::squared_distance(p1, p2));
  }

  double calculate(TsrState &state) override {

    return calculateDistance(state.current_vertex->point(),
                             state.next_vertex->point());
  }
};

} // namespace tsr