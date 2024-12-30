#pragma once

#include <gdal/gdal_priv.h>

#include <memory>
#include <string>
#include <vector>

#include "tsr/TSRState.hpp"

namespace tsr {

class FeatureBase {

public:
  /// Feature unique identifier
  std::string featureID;

  /// Stores the feature dependencies
  std::vector<std::shared_ptr<FeatureBase>> dependencies;

  /// Adds a dependency to the feature, ensuring access to those features
  virtual void add_dependency(std::shared_ptr<FeatureBase> feature);

  /// Features can override or re-define with their own initialization functions
  FeatureBase(const std::string &featureID) : featureID(featureID) {}

  static void addWarning(TSRState &state, const std::string &warning,
                         const unsigned short priority);

  /// Default destructor. Features with malloced attributes should overrite this
  virtual ~FeatureBase() = default;

  /// Equality operator to compare Feature objects
  bool operator==(const FeatureBase &other) const {
    return featureID == other.featureID;
  }
};

template <typename DataType> class Feature : public FeatureBase {

public:
  using FeatureBase::FeatureBase;

  virtual DataType calculate(TSRState &state) = 0;
};

} // namespace tsr