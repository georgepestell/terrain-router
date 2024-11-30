#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"

#include "tsr/Features/SimpleBooleanFeature.hpp"
#include "tsr/Point_3.hpp"
#include <memory>

namespace tsr {

class SimpleBooleanToDoubleFeature : public Feature<double> {
public:
  using Feature<double>::Feature;

  enum DEPENDENCIES { SIMPLE_BOOLEAN };

  double calculate(Face_handle face, Point_3 &source_point,
                   Point_3 &target_point) override {

    auto boolFeature = std::dynamic_pointer_cast<SimpleBooleanFeature>(
        this->dependencies.at(DEPENDENCIES::SIMPLE_BOOLEAN));
    bool boolValue = boolFeature->calculate(face, source_point, target_point);

    return boolValue ? 10.0 : -10.0;
  }
};

} // namespace tsr