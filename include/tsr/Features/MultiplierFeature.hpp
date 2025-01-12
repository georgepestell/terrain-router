#pragma once

#include "tsr/Feature.hpp"
#include "tsr/TsrState.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace tsr {

class MultiplierFeature : public Feature<double> {
public:
  enum DEPENDENCY_TYPE { INT, DOUBLE, BOOL };

  std::unordered_map<std::string, DEPENDENCY_TYPE> dependency_types;

  using Feature<double>::Feature;

  double Calculate(TsrState &state) override;

  void AddDependency(std::shared_ptr<FeatureBase> feature) override;

  void AddDependency(std::shared_ptr<FeatureBase> feature,
                     DEPENDENCY_TYPE type);
};

} // namespace tsr