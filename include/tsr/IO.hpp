#pragma once

#include "tsr/DTM.hpp"

namespace tsr {

Point_set_3 load_points_from_file(const char *filepath);

void write_mesh_to_obj(char const *filepath, Delaunay_3D const &mesh);

const char *path_to_absolute(const char *filepath);

} // namespace tsr