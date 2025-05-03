#include "tsr/Feature.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Presets.hpp"
#include "tsr/TsrState.hpp"

#include <cstddef>
#include <memory>
#include <string>

namespace tsr {

void FeatureBase::AddDependency(std::shared_ptr<FeatureBase> feature) {
  this->dependencies.push_back(feature);
}

void FeatureBase::AddWarning(TsrState &state, const std::string &warning,
                             const unsigned short priority) {

  auto face = state.current_face;

  if (state.warnings.contains(face)) {
    size_t existingIndex = state.warnings[face];
    const unsigned short existingPriority =
        state.warning_priorities[existingIndex];

    if (existingPriority > priority) {
      return;
    }
  }

  // Either no warning already exists, or the new priority is higher than old
  size_t newIndex = state.AddWarning(warning, priority);

  state.warnings[face] = newIndex;
}

} // namespace tsr
