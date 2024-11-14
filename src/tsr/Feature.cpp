#include "tsr/Feature.hpp"
#include "tsr/logging.hpp"

#include <stdexcept>

namespace tsr {

Feature::Feature(std::string name) {
  this->name = name;
  this->valid = false;
}

void Feature::set_valid() { this->valid = true; }

void Feature::set_invalid() { this->valid = false; }

void Feature::reset() { this->valid = false; }

bool Feature::isEqual(Feature &otherFeature) {
  // Equality determined by feature name
  return this->name == otherFeature.name;
}

} // namespace tsr