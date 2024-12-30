#include <string>

#include "tsr/Delaunay_3.hpp"
#include "tsr/Mesh.hpp"
#include "tsr/Point_2.hpp"

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

bool write_CDT_to_file(std::string filepath, const Delaunay_3 &cdt);
Delaunay_3 loadCDTFromFile(std::string);

void write_vector_contours_to_file(
    std::string filepath, const std::vector<std::vector<Point_2>> &contours);

void load_vector_contours_from_file(
    std::string filepath, std::vector<std::vector<Point_2>> &contours);

} // namespace tsr::IO