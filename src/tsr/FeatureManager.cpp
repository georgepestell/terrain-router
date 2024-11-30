#include "tsr/FeatureManager.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/logging.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace tsr {

bool FeatureManager::has_dependency_cycle() {
  std::unordered_set<std::string> existingFeatures = {
      this->outputFeature->featureID};

  return this->has_dependency_cycle(this->outputFeature, existingFeatures);
}

bool FeatureManager::has_dependency_cycle(
    std::shared_ptr<FeatureBase> current_feature,
    std::unordered_set<std::string> &preexisting_features) {

  if (current_feature == nullptr) {
    return false;
  }

  // Add it's dependencies to the previous dependencies and
  // calculate the dependencies of the next

  for (auto dep : current_feature->dependencies) {

    if (preexisting_features.find(dep->featureID) !=
        preexisting_features.end()) {
      return true;
    }

    std::unordered_set<std::string> childFeatures = preexisting_features;
    childFeatures.insert(dep->featureID);

    if (this->has_dependency_cycle(dep, childFeatures)) {
      return true;
    }
  }

  return false;
}

void FeatureManager::setOutputFeature(
    std::shared_ptr<Feature<double>> feature) {
  this->outputFeature = feature;

  if (has_dependency_cycle()) {
    this->outputFeature = nullptr;
    TSR_LOG_ERROR("Feature graph has dependency cycle");
    throw std::runtime_error("Feature graph has dependency cycle");
  }
}

double FeatureManager::calculateCost(Face_handle face, Point_3 &source_point,
                                     Point_3 &target_point) {
  return this->outputFeature->calculate(face, source_point, target_point);
}

} // namespace tsr