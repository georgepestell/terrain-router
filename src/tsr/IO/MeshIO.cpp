#include <CGAL/IO/write_VTU.h>
#include <CGAL/boost/graph/IO/OBJ.h>
#include <cmath>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

#include <CGAL/boost/graph/IO/VTK.h>

#include "tsr/Tin.hpp"

#include "tsr/IO/MeshIO.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"
#include "tsr/SurfaceMesh.hpp"

namespace tsr::IO {

void write_mesh_to_obj(std::string filepath, SurfaceMesh mesh) {
  std::ofstream ofile(filepath, std::ios_base::binary);
  CGAL::IO::set_binary_mode(ofile);

  CGAL::IO::write_OBJ(ofile, mesh);
  ofile.close();
};

Tin loadCDTFromFile(std::string filepath) {
  std::ifstream file(filepath, std::ios::binary);

  if (!file) {
    TSR_LOG_ERROR("failed to open TIN file");
    throw std::runtime_error("failed to open TIN file");
  }

  Tin tin;
  file >> tin;

  return tin;
}

bool write_CDT_to_file(std::string filepath, const Tin &tin) {
  std::ofstream file(filepath, std::ios::binary);

  if (!file) {
    TSR_LOG_ERROR("failed to open TIN file");
    return false;
  }

  if (!tin.is_valid()) {
    TSR_LOG_TRACE("TIN INVALID");
  }

  file << std::setprecision(18) << tin;

  file.close();

  return true;
}

// TODO: Delete direct access of DEM from file
std::vector<Point3> load_dem_from_file(std::string filepath) {

  // TODO: error checking

  TSR_LOG_TRACE("loading points from file");

  // Read points from file
  std::ifstream ifile(filepath, std::ios_base::binary);
  std::vector<Point3> points;

  double x, y, z;
  while (ifile >> x >> y >> z) {
    points.push_back(Point3(round(x), round(y), round(z)));
  }

  return points;
}

void load_vector_contours_from_file(
    std::string filepath, std::vector<std::vector<Point2>> &contours) {

  std::ifstream iFile(filepath, std::ios_base::binary);
  if (!iFile) {
    TSR_LOG_ERROR("failed to open contour file");
    throw std::runtime_error("failed to open contour file");
  }

  std::vector<std::vector<Point2>> loadedContours;
  std::string line;
  while (std::getline(iFile, line)) {
    std::istringstream stream(line);
    std::vector<Point2> row;
    Point2 value;
    while (stream >> value) {
      row.push_back(value);
    }
    loadedContours.push_back(row);
  }

  iFile.close();
  contours.swap(loadedContours);
}

void write_vector_contours_to_file(
    std::string filepath, const std::vector<std::vector<Point2>> &contours) {

  std::ofstream oFile(filepath);
  if (!oFile) {
    TSR_LOG_ERROR("failed to open vector contour file");
    throw std::runtime_error("failed to open vector contour file");
  }

  for (const auto &row : contours) {
    for (const auto elem : row) {
      oFile << elem << " ";
    }
    oFile << "\n";
  }

  oFile.close();
}

} // namespace tsr::IO