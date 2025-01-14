#pragma once

#include <boost/concept_check.hpp>
#include <gdal/gdal_priv.h>

#include <memory>
#include <string>
#include <vector>

#include "tsr/MeshBoundary.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"

namespace tsr {

class FeatureBase {

public:
  /// Feature unique identifier
  std::string feature_id;

  /// Stores the feature dependencies
  std::vector<std::shared_ptr<FeatureBase>> dependencies;

  /// Features can override or re-define with their own initialization functions
  FeatureBase(const std::string &feature_id) : feature_id(feature_id) {}

  // Explicitly define the default destructor
  virtual ~FeatureBase() = default;

  /// Features are equal if their id is the same
  bool operator==(const FeatureBase &other) const {
    return feature_id == other.feature_id;
  }

  /// Adds a dependency to the feature, ensuring access to those features
  virtual void AddDependency(std::shared_ptr<FeatureBase> feature);

  static void AddWarning(TsrState &state, const std::string &warning,
                         const unsigned short priority);
};

template <typename DataType> class Feature : public FeatureBase {

public:
  using FeatureBase::FeatureBase;

  // Default initialization behaviour does nothing
  virtual void Initialize(Tin &tin, const MeshBoundary &boundary) {
    boost::ignore_unused_variable_warning(tin);
    boost::ignore_unused_variable_warning(boundary);
  };
  // Default tagging behaviour does nothing
  virtual void Tag(const Tin &tin) {
    boost::ignore_unused_variable_warning(tin);
  };

  // Features must implement their own calculate logic
  virtual DataType Calculate(TsrState &state) = 0;
};

} // namespace tsr