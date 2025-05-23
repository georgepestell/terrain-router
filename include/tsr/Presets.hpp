#include "tsr/FeatureManager.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Tin.hpp"

namespace tsr {

FeatureManager SetupTimePreset(Tin &tin, const MeshBoundary &boundary);
FeatureManager SetupTimeWithSwimmingPreset(Tin &tin,
                                           const MeshBoundary &boundary);
FeatureManager SetupTimeRestrictSwimmingPreset(Tin &tin,
                                               const MeshBoundary &boundary);

/// Simply minimizes the overall distance travelled
FeatureManager SetupSimpleDistancePreset();

/// This feature minimizes the overall elevation change (i.e. distance *
/// abs(gradient))
FeatureManager SetupSimpleGradientPreset();

} // namespace tsr