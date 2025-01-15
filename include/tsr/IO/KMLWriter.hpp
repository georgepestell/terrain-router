#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"

#include <string>
#include <utility>
#include <vector>

namespace tsr::IO {

void writeSuccessStateToKML(const std::string &filepath, TsrState &state);
void writeFailureStateToKML(const std::string &filepath, TsrState &state);

std::string GenerateKmlFaces(std::vector<Face_handle> &faces, std::string name);

std::string GenerateKmlWarnings(const TsrState &state);

std::string GenerateKmlRoute(const std::vector<Point3> &route);

std::string GenerateKmlDocument(const std::string &inner_kml);

std::string GenerateKmlLine(std::pair<Point3, Point3> line);

} // namespace tsr::IO