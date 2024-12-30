#include <CGAL/IO/write_VTU.h>
#include <CGAL/boost/graph/IO/OBJ.h>
#include <cmath>
#include <fstream>
#include <ios>
#include <sstream>
#include <string>
#include <vector>

#include <CGAL/boost/graph/IO/VTK.h>

#include "tsr/Delaunay_3.hpp"

#include "tsr/IO/MeshIO.hpp"
#include "tsr/Mesh.hpp"
#include "tsr/Point_2.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/logging.hpp"

namespace tsr::IO {

void write_mesh_to_obj(std::string filepath, Mesh mesh) {
  std::ofstream ofile(filepath, std::ios_base::binary);
  CGAL::IO::set_binary_mode(ofile);

  CGAL::IO::write_OBJ(ofile, mesh);
  ofile.close();
};

Delaunay_3 loadCDTFromFile(std::string filepath) {
  std::ifstream file(filepath, std::ios::binary);

  if (!file) {
    TSR_LOG_ERROR("failed to open CDT file");
    throw std::runtime_error("failed to open CDT file");
  }

  Delaunay_3 cdt;
  file >> cdt;

  return cdt;
}

bool write_CDT_to_file(std::string filepath, const Delaunay_3 &cdt) {
  std::ofstream file(filepath, std::ios::binary);

  if (!file) {
    TSR_LOG_ERROR("failed to open CDT file");
    return false;
  }

  if (!cdt.is_valid()) {
    TSR_LOG_TRACE("CDT INVALID");
  }

  file << std::setprecision(18) << cdt;

  file.close();

  return true;
}

// TODO: Delete direct access of DEM from file
std::vector<Point_3> load_dem_from_file(std::string filepath) {

  // TODO: error checking

  TSR_LOG_TRACE("loading points from file");

  // Read points from file
  std::ifstream ifile(filepath, std::ios_base::binary);
  std::vector<Point_3> points;

  double x, y, z;
  while (ifile >> x >> y >> z) {
    points.push_back(Point_3(round(x), round(y), round(z)));
  }

  return points;
}

void load_vector_contours_from_file(
    std::string filepath, std::vector<std::vector<Point_2>> &contours) {

  std::ifstream iFile(filepath, std::ios_base::binary);
  if (!iFile) {
    TSR_LOG_ERROR("failed to open contour file");
    throw std::runtime_error("failed to open contour file");
  }

  std::vector<std::vector<Point_2>> loadedContours;
  std::string line;
  while (std::getline(iFile, line)) {
    std::istringstream stream(line);
    std::vector<Point_2> row;
    Point_2 value;
    while (stream >> value) {
      row.push_back(value);
    }
    loadedContours.push_back(row);
  }

  iFile.close();
  contours.swap(loadedContours);
}

void write_vector_contours_to_file(
    std::string filepath, const std::vector<std::vector<Point_2>> &contours) {

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