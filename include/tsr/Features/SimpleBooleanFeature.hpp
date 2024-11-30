#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"

namespace tsr {

class SimpleBooleanFeature : public Feature<bool> {

public:
  using Feature<bool>::Feature;

  bool calculate(Face_handle face, Point_3 &source_point,
                 Point_3 &target_point) {
    return false;
  }
};

} // namespace tsr
