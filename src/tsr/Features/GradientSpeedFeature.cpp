#include "tsr/Features/GradientSpeedFeature.hpp"
#include "tsr/Feature.hpp"
#include "tsr/TsrState.hpp"
#include <cmath>
#include <memory>
#include <sys/types.h>
#include <vector>

namespace tsr {

double
GradientSpeedFeature::solvePolynomial(double x,
                                      std::vector<double> &coefficients) {

  double y = 0;
  for (uint degree = 0; degree < coefficients.size(); degree++) {
    y += std::pow(x, degree);
  }

  return y;
}

double GradientSpeedFeature::Calculate(TsrState &state) {
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

} // namespace tsr