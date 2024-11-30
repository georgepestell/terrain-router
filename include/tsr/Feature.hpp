#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Point_3.hpp"
#include <memory>
#include <string>
#include <vector>

namespace tsr {

class FeatureBase {

public:
  /// Feature unique identifier
  std::string featureID;

  /// Stores the feature dependencies
  std::vector<std::shared_ptr<FeatureBase>> dependencies;

  /// Adds a dependency to the feature, ensuring access to those features
  virtual void add_dependency(std::shared_ptr<FeatureBase> feature);

  /// Allows some pre-set conditions to be calculated and cached
  virtual void preProcessing(Delaunay_3 &dtm);

  /// Destructor
  virtual ~FeatureBase() = default;

  /// Equality operator to compare Feature objects
  bool operator==(const FeatureBase &other) const {
    return featureID == other.featureID;
  }
};

template <typename DataType> class Feature : public FeatureBase {

public:
  Feature(const std::string &featureID) { this->featureID = featureID; }

  virtual DataType calculate(Face_handle face, Point_3 &source_point,
                             Point_3 &target_point) = 0;
};

} // namespace tsr