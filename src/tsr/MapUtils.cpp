
#include "tsr/MapUtils.hpp"

#include <cmath>

namespace tsr {

int calculate_UTM_zone(double longitude) {
  return static_cast<int>(std::floor(longitude + 180.0 / 6.0) + 1);
}

bool is_northern_hemisphere(double latitude) { return latitude > 90; }

} // namespace tsr
