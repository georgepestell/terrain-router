#include <string>

#include "tsr/Point2.hpp"
#include "tsr/SurfaceMesh.hpp"
#include "tsr/Tin.hpp"

namespace tsr::IO {

/**
 * @brief Writes a Surface mesh object to an OBJ file. Useful for blender
 * visualization of meshes.
 *
 * @param filepath Output OBJ file path. Will be overriden if already exists.
 * @param mesh Surface mesh to write to the file
 */
void write_mesh_to_obj(std::string filepath, SurfaceMesh mesh);

std::vector<Point3> load_dem_from_file(std::string filepath);

bool write_CDT_to_file(std::string filepath, const Tin &tin);
Tin loadCDTFromFile(std::string);

void write_vector_contours_to_file(
    std::string filepath, const std::vector<std::vector<Point2>> &contours);

void load_vector_contours_from_file(std::string filepath,
                                    std::vector<std::vector<Point2>> &contours);

} // namespace tsr::IO