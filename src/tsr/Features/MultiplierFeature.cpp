#include "tsr/Features/MultiplierFeature.hpp"
#include "tsr/Feature.hpp"
#include "tsr/Logging.hpp"
#include "tsr/TsrState.hpp"
#include <boost/concept_check.hpp>
#include <limits>
#include <memory>
#include <stdexcept>

namespace tsr {

double MultiplierFeature::Calculate(TsrState &state) {

  double total = 1;
  for (auto f : this->dependencies) {
    switch (this->dependency_types[f->feature_id]) {
    case INT: {
      auto feature = std::dynamic_pointer_cast<Feature<int>>(f);
      int value = feature->Calculate(state);

      if (total == std::numeric_limits<double>::infinity() ||
          ((double)value) == std::numeric_limits<double>::infinity()) {
        total = std::numeric_limits<double>::infinity();
        break;
      }

      total *= (double)value;
      break;
    }
    case DOUBLE: {
      auto feature = std::dynamic_pointer_cast<Feature<double>>(f);
      double value = feature->Calculate(state);

      if (total == std::numeric_limits<double>::infinity() ||
          value == std::numeric_limits<double>::infinity()) {
        total = std::numeric_limits<double>::infinity();
        break;
      }

      total *= value;
      break;
    }
    case BOOL: {
      auto feature = std::dynamic_pointer_cast<Feature<bool>>(f);
      bool value = feature->Calculate(state);
      if (value) {
        break;
      } else {
        return 0;
      }
      break;
    }
    default:
      TSR_LOG_ERROR("Multiplier feature type invalid");
      throw std::runtime_error("multiplier feature type invalid");
    }

    if (total == 0) {
      break;
    }
  }

  return total;
}

void MultiplierFeature::AddDependency(std::shared_ptr<FeatureBase> feature) {
  boost::ignore_unused_variable_warning(feature);
  TSR_LOG_ERROR(
      "Multiplier AddDependency requires specifying the feature type");
  throw std::runtime_error(
      "Multiplier AddDependency requires specifying the feature type");
}

void MultiplierFeature::AddDependency(std::shared_ptr<FeatureBase> feature,
                                      DEPENDENCY_TYPE type) {
  this->dependencies.push_back(feature);
  this->dependency_types[feature->feature_id] = type;
}

} // namespace tsr