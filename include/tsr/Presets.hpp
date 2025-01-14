#include "tsr/FeatureManager.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Tin.hpp"

namespace tsr {

FeatureManager SetupTimePreset(Tin &tin, const MeshBoundary &boundary);
FeatureManager SetupTimeWithSwimmingPreset(Tin &tin,
                                           const MeshBoundary &boundary);
FeatureManager SetupSimpleDistancePreset(Tin &tin,
                                         const MeshBoundary &boundary);
// FeatureManager SetupTerrainTypePreset(Tin &tin, const MeshBoundary
// &boundary);
FeatureManager SetupSimpleGradientPreset(Tin &tin,
                                         const MeshBoundary &boundary);

} // namespace tsr