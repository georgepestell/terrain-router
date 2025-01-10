#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include <string>
#include <vector>
namespace tsr::IO {

void writeSuccessStateToKML(const std::string &filepath, TsrState &state);
void writeFailureStateToKML(const std::string &filepath, TsrState &state);

std::string generateKMLFaces(std::vector<Face_handle> &faces);

std::string generateKMLWarnings(const TsrState &state);

std::string generateKMLRoute(const std::vector<Point3> &route);

std::string generateKMLForAllRoutes(const TsrState &state);

std::string generateKMLDocument(const std::string &inner_kml);

} // namespace tsr::IO