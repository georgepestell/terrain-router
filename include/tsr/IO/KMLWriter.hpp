#include "tsr/Delaunay_3.hpp"
#include "tsr/TSRState.hpp"
#include <string>
#include <vector>
namespace tsr::IO {

void writeSuccessStateToKML(const std::string &filepath, TSRState &state);
void writeFailureStateToKML(const std::string &filepath, TSRState &state);

std::string generateKMLFaces(std::vector<Face_handle> &faces);

std::string generateKMLWarnings(const TSRState &state);

std::string generateKMLRoute(const std::vector<Point_3> &route);

std::string generateKMLForAllRoutes(const TSRState &state);

std::string generateKMLDocument(const std::string &inner_kml);

} // namespace tsr::IO