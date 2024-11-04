#pragma once

#include "tsr/Feature.hpp"

namespace tsr::features {

/**
 * @brief Feature representing whether a face is a water feature.
 *
 */
class BooleanWaterFeature : Feature<bool> {};

} // namespace tsr::features