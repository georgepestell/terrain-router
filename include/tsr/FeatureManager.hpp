#pragma once

#include "tsr/Feature.hpp"

#include <any>
#include <memory>
#include <unordered_map>
#include <vector>

namespace tsr {
class FeatureManager {
private:
  // Maps names to features 
  std::unordered_map<std::string, std::shared_ptr<Feature>> features;

  // Maps feature names to dependency names
  std::unordered_map<std::string, std::vector<std::string>> dependency_graph;

  bool has_cycle();

public:
  void add_feature(Feature &feature);
  void add_dependency(Feature &feature, Feature &dependency);

  // TODO: Add delete_feature and delete_dependency functions
  // void delete_feature(Feature<std::any> &feature);
  // void delete_dependency(Feature<std::any> &feature,
  //                        Feature<std::any> &dependency);
};
} // namespace tsr