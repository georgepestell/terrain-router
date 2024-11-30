#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"
#include "tsr/Point_3.hpp"
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

  static inline std::vector<double> DEFAULT_UPWARDS_COEFFS = {
      1, -2.3, 7.61, -20.66, -8.11, 22.46};
  static inline std::vector<double> DEFAULT_DOWNWARDS_COEFFS = {
      0.99, 4.37, -21.08, -4.87, 20.59};

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

  double calculate(Face_handle face, Point_3 &source_point,
                   Point_3 &target_point) override {

    // Solve the polynomial with the given input
    auto inputFeature = dynamic_pointer_cast<Feature<double>>(
        this->dependencies[DEPENDENCIES::X]);

    // Get the dependency value
    double gradient = inputFeature->calculate(face, source_point, target_point);

    if (gradient > 0) {
      return solvePolynomial(gradient, this->upwards_coefficients);
    } else {
      return solvePolynomial(gradient, downwards_coefficients);
    }
  }
};

}; // namespace tsr