#pragma once
#include "tsr/Feature.hpp"
#include "tsr/TsrState.hpp"

#include <memory>

namespace tsr {

/**
 * @brief Represents the cost function. Setup through defining the output feature. Calls the output feature's Feature.Calculate function, which then manages it's own dependencies.
 * 
 */
class FeatureManager {
public:
  bool HasDependencyCycle(
      std::shared_ptr<FeatureBase> current_feature,
      std::unordered_set<std::string> &preexisting_features) const;

  // Final cost output feature
  std::shared_ptr<Feature<double>> outputFeature;

  void SetOutputFeature(std::shared_ptr<Feature<double>> feature);

  bool HasDependencyCycle() const;

  double Calculate(TsrState &state) const;
};

} // namespace tsr