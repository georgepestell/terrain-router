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
void WriteMeshToObj(std::string filepath, SurfaceMesh mesh);

std::vector<Point3> LoadPointsFromXYZFile(std::string filepath);

bool WriteTinToFile(std::string filepath, const Tin &tin);
Tin loadCDTFromFile(std::string);

void WriteContoursToFile(
    std::string filepath, const std::vector<std::vector<Point2>> &contours);

void LoadContoursFromFile(std::string filepath,
                                    std::vector<std::vector<Point2>> &contours);

} // namespace tsr::IO