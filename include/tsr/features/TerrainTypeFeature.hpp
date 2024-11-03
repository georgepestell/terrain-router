#pragma once

#include "tsr/Feature.hpp"

namespace tsr::features {

enum class TERRAIN_TYPE {
    FLAT,
    HILL,
    MOUNTAIN,
    WATER,
    FOREST,
    URBAN
};

/**
 * @brief Feature representing the terrain type of a face.
 * 
 */
class TerrainTypeFeature : public Feature<TERRAIN_TYPE> {};

} // namespace tsr
