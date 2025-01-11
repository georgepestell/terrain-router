#pragma once

#include "tsr/Feature.hpp"

#include "tsr/Features/SimpleBooleanFeature.hpp"
#include "tsr/TsrState.hpp"
#include <memory>

namespace tsr {

class SimpleBooleanToDoubleFeature : public Feature<double> {
private:
  double pos_value;
  double neg_value;

public:
  SimpleBooleanToDoubleFeature(const std::string &name, double pos_value,
                               double neg_value)
      : Feature(name), pos_value(pos_value), neg_value(neg_value) {};

  enum DEPENDENCIES { SIMPLE_BOOLEAN };

  double Calculate(TsrState &state) override {

    auto boolFeature = std::dynamic_pointer_cast<SimpleBooleanFeature>(
        this->dependencies.at(DEPENDENCIES::SIMPLE_BOOLEAN));
    bool boolValue = boolFeature->Calculate(state);

    return boolValue ? this->pos_value : this->neg_value;
  }
};

} // namespace tsr