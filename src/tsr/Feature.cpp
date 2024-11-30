#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"

#include "tsr/logging.hpp"

namespace tsr {

void FeatureBase::add_dependency(std::shared_ptr<FeatureBase> feature) {
  this->dependencies.push_back(feature);
}

void FeatureBase::preProcessing(Delaunay_3 &dtm) {
  TSR_LOG_TRACE("No pre-processing required for feature {}", featureID);
};

} // namespace tsr