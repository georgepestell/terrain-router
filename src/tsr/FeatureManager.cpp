#include "tsr/FeatureManager.hpp"
#include "tsr/Feature.hpp"

namespace tsr {

void FeatureManager::add_feature(Feature<std::any> &feature) {

  // Check for duplicate feature
  for (auto existing_feature : this->features) {

    if (feature.isEqual(existing_feature)) {
      throw std::invalid_argument("Feature already exists");
    }
  }
  this->features.push_back(feature);
}

} // namespace tsr