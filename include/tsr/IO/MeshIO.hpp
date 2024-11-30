#include <string>

#include "tsr/Mesh.hpp"

namespace tsr::IO {

/**
 * @brief Writes a Surface mesh object to an OBJ file. Useful for blender
 * visualization of meshes.
 *
 * @param filepath Output OBJ file path. Will be overriden if already exists.
 * @param mesh Surface mesh to write to the file
 */
void write_mesh_to_obj(std::string filepath, Mesh mesh);

std::vector<Point_3> load_dem_from_file(std::string filepath);

} // namespace tsr::IO