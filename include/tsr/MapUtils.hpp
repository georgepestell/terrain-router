#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/MeshBoundary.hpp"

namespace tsr {

int calculate_UTM_zone(double longitude);
bool is_northern_hemisphere(double latitude);

} // namespace tsr