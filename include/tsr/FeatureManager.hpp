#pragma once

#include "tsr/Feature.hpp"

#include <any>
#include <memory>
#include <unordered_map>
#include <vector>

namespace tsr {
class FeatureManager {
private:
  std::vector<Feature<std::any>> features;
  std::unordered_map<int, std::vector<int>> dependency_graph;

  bool has_cycle();

public:
  void add_feature(Feature<std::any> &feature);
  void add_dependency(Feature<std::any> &feature,
                      Feature<std::any> &dependency);

  // TODO: Add delete_feature and delete_dependency functions
  // void delete_feature(Feature<std::any> &feature);
  // void delete_dependency(Feature<std::any> &feature,
  //                        Feature<std::any> &dependency);
};
} // namespace tsr