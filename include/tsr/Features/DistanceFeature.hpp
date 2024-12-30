#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/TSRState.hpp"

#include <CGAL/Distance_3/Point_3_Point_3.h>
#include <boost/concept_check.hpp>
#include <cmath>

namespace tsr {

class DistanceFeature : public Feature<double> {
public:
  using Feature<double>::Feature;

  double calculateDistance(Point_3 p1, Point_3 p2) {

    return std::sqrt(CGAL::squared_distance(p1, p2));
  }

  double calculate(TSRState &state) override {

    return calculateDistance(state.current_vertex->point(),
                             state.next_vertex->point());
  }
};

} // namespace tsr