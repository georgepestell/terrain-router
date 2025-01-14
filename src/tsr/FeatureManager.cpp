#include "tsr/FeatureManager.hpp"
#include "tsr/Feature.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace tsr {

bool FeatureManager::HasDependencyCycle() const {
  std::unordered_set<std::string> existingFeatures = {
      this->outputFeature->feature_id};

  return this->HasDependencyCycle(this->outputFeature, existingFeatures);
}


/**
* @brief Recursively determines if a dependency cycle exists, by keeping a set of preexisting features, and following each branch fully.
* 
* @param current_feature 
* @param preexisting_features 
* @return true A dependency cycle exists. The DAG is invalid.
* @return false No dependency cycles exist. The DAG is valid.
*/
bool FeatureManager::HasDependencyCycle(
    std::shared_ptr<FeatureBase> current_feature,
    std::unordered_set<std::string> &preexisting_features) const {

  if (current_feature == nullptr) {
    return false;
  }

  // Add it's dependencies to the previous dependencies and
  // calculate the dependencies of the next

  for (auto dep : current_feature->dependencies) {

    if (preexisting_features.find(dep->feature_id) !=
        preexisting_features.end()) {
      return true;
    }

    std::unordered_set<std::string> childFeatures = preexisting_features;
    childFeatures.insert(dep->feature_id);

    if (this->HasDependencyCycle(dep, childFeatures)) {
      return true;
    }
  }

  return false;
}

void FeatureManager::SetOutputFeature(
    std::shared_ptr<Feature<double>> feature) {
  this->outputFeature = feature;

  if (HasDependencyCycle()) {
    this->outputFeature = nullptr;
    TSR_LOG_ERROR("Feature graph has dependency cycle");
    throw std::runtime_error("Feature graph has dependency cycle");
  }
}

double FeatureManager::Calculate(TsrState &state) const {

  // const auto Pc = state.current_vertex->point();
  // const auto Pn = state.next_vertex->point();

  // TSR_LOG_TRACE("\ncurrent position: {} {} {}", Pc.x(), Pc.y(), Pc.z());
  // TSR_LOG_TRACE("next position: {} {} {}", Pn.x(), Pn.y(), Pn.z());
  // TSR_LOG_TRACE("face id: {}", (void *)&state.current_face);

  return this->outputFeature->Calculate(state);

  // TSR_LOG_TRACE("cost: {}\n", cost);
  // return cost;
}

} // namespace tsr