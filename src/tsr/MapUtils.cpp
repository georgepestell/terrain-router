
#include "tsr/MapUtils.hpp"

#include <cmath>

namespace tsr {

int CalculateUtmZone(double longitude) {
  return static_cast<int>(std::floor(longitude + 180.0 / 6.0) + 1);
}

bool IsNorthernHemisphere(double latitude) { return latitude > 90; }

} // namespace tsr
