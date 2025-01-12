#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include "tsr/Point3.hpp"

#include <string>
#include <vector>

namespace tsr::IO {

void writeSuccessStateToKML(const std::string &filepath, TsrState &state);
void writeFailureStateToKML(const std::string &filepath, TsrState &state);

std::string GenerateKmlFaces(std::vector<Face_handle> &faces);

std::string GenerateKmlWarnings(const TsrState &state);

std::string GenerateKmlRoute(const std::vector<Point3> &route);

std::string GenerateKmlDocument(const std::string &inner_kml);

} // namespace tsr::IO