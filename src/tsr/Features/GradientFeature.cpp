#include "tsr/Features/GradientFeature.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point3.hpp"
#include <cmath>

namespace tsr {

double GradientFeature::CalculateGradient(const Point3 &p1, const Point3 &p2) {
  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  double distance = std::hypot(dx, dy);

  /// Prevents divide by zero errors
  if (distance == 0) {
    TSR_LOG_WARN("Cannot calculate gradient - requires zero distance division");
    return 0;
  }

  double dz = p2.z() - p1.z();
  return dz / distance;
}

} // namespace tsr