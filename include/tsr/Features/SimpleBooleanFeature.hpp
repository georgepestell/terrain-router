#pragma once

#include "tsr/Features/ConstantFeature.hpp"
#include <boost/concept_check.hpp>

namespace tsr {

class SimpleBooleanFeature : public ConstantFeature<bool> {

public:
  SimpleBooleanFeature(const std::string &name, bool value)
      : ConstantFeature<bool>(name, value) {};
};

} // namespace tsr
