#pragma once
#include "tsr/Feature.hpp"

#include <memory>

namespace tsr {

class FeatureManager {
private:
  bool
  has_dependency_cycle(std::shared_ptr<FeatureBase> current_feature,
                       std::unordered_set<std::string> &preexisting_features);

public:
  // Final cost output feature
  std::shared_ptr<Feature<double>> outputFeature;

  void setOutputFeature(std::shared_ptr<Feature<double>> feature);

  bool has_dependency_cycle();

  double calculateCost(Face_handle face, Point_3 &source_point,
                       Point_3 &target_point);
};

} // namespace tsr