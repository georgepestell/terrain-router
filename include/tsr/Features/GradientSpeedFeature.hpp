#pragma once

#include "tsr/Feature.hpp"
#include "tsr/TsrState.hpp"
#include <algorithm>
#include <boost/multiprecision/detail/min_max.hpp>
#include <cmath>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

namespace tsr {

class GradientSpeedFeature : public Feature<double> {
private:
  std::vector<double> upwards_coefficients;
  std::vector<double> downwards_coefficients;

  enum DEPENDENCIES { X };

  static inline std::vector<double> DEFAULT_UPWARDS_COEFFS = {1, -2.7, -34.83,
                                                              200.63, -292.06};
  static inline std::vector<double> DEFAULT_DOWNWARDS_COEFFS = {
      1, -0.01, 79.31, 1164.83, 4622.34, 5737.68};

public:
  GradientSpeedFeature(std::string name,
                       std::vector<double> upwards_coefficients,
                       std::vector<double> downwards_coefficients)
      : Feature<double>(name), upwards_coefficients(upwards_coefficients),
        downwards_coefficients(downwards_coefficients) {};

  GradientSpeedFeature(std::string name)
      : GradientSpeedFeature(name, DEFAULT_UPWARDS_COEFFS,
                             DEFAULT_DOWNWARDS_COEFFS) {}

  static double solvePolynomial(double x, std::vector<double> &coefficients) {

    double y = 0;
    for (uint degree = 0; degree < coefficients.size(); degree++) {
      y += std::pow(x, degree);
    }

    return y;
  }

  double Calculate(TsrState &state) override {

    // Solve the polynomial with the given input
    auto inputFeature = dynamic_pointer_cast<Feature<double>>(
        this->dependencies[DEPENDENCIES::X]);

    // Get the dependency value
    double gradient = inputFeature->Calculate(state);

    double speedInfluence;
    if (gradient > 0) {
      speedInfluence = solvePolynomial(gradient, this->upwards_coefficients);
    } else {
      speedInfluence = solvePolynomial(gradient, this->downwards_coefficients);
    }

    // Cap the speedInfluence to a minimum of 0x speed
    double cappedSpeedInfluence = fmax(0.0, speedInfluence);

    if (cappedSpeedInfluence <= 0) {
      AddWarning(state, "Untraversable gradient", 10);
    } else if (cappedSpeedInfluence < 0.5) {
      AddWarning(state, "Steep Gradient", 2);
    } else if (cappedSpeedInfluence < 0.8) {
      AddWarning(state, "Slight Gradient", 1);
    }

    return cappedSpeedInfluence;
  }
};

}; // namespace tsr