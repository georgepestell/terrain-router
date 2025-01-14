#include "tsr/Point3.hpp"

#include <string>

namespace tsr {
std::string toString(const Point3 &p) {
  std::string s = "(";
  s += std::to_string(p.x()) + ", " + std::to_string(p.y()) + ", " +
       std::to_string(p.z()) + ")";
  return s;
}
} // namespace tsr