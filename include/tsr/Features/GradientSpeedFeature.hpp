#pragma once

#include "tsr/Feature.hpp"
#include "tsr/TsrState.hpp"

#include <string>
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

  static double solvePolynomial(double x, std::vector<double> &coefficients);

  double Calculate(TsrState &state) override;
};

}; // namespace tsr