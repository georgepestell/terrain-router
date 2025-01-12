
#include "tsr/TsrState.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Logging.hpp"

namespace tsr {

void TsrState::ProcessWarnings() {

  TSR_LOG_TRACE("processing warnings");

  std::unordered_map<Face_handle, size_t> processedWarnings;

  RouteNode currentNode = this->routes.at(end_vertex);
  while (currentNode.vertex != start_vertex) {
    currentNode = this->routes.at(currentNode.parent);

    // // From the end point to the start point, add the warnings on the route
    // // itself
    // auto warningID = this->warnings[currentNode.face];
    // if (warningID > 0) {
    //   // TODO: better showing warnings on route
    //   // processedWarnings[currentNode.face] = (warningID);
    // }

    // Otherwise, add an adjacent warning if required
    short maxPriority = 0;
    Face_handle maxWarning;

    auto fC = currentNode.vertex->incident_faces();
    auto fCEnd = fC;
    do {

      // Check if there is a warning for the face
      auto adjacentWarningID = this->warnings[fC];

      if (adjacentWarningID != 0) {
        // Check if the priority is greater than the current max
        auto adjacentWarningPriority =
            this->warning_priorities[adjacentWarningID];
        if (adjacentWarningPriority > maxPriority) {
          maxPriority = adjacentWarningPriority;
          maxWarning = fC;
        }
      }
    } while (++fC != fCEnd);

    // Add the warning if it is greater than a desired priority
    unsigned short MIN_PRIORITY = 10;
    if (maxPriority >= MIN_PRIORITY) {
      processedWarnings[maxWarning] = this->warnings[maxWarning];
    }
  }

  this->warnings.swap(processedWarnings);
}

std::vector<Point3> TsrState::fetchRoute() const {

  // Get the end node point
  std::vector<Point3> route;
  RouteNode current_node = this->routes.at(this->end_vertex);
  TSR_LOG_TRACE("cost: {}", current_node.gCost);
  route.push_back(this->end_vertex->point());

  while (current_node.vertex != this->start_vertex) {
    current_node = this->routes.at(current_node.parent);
    route.push_back(current_node.vertex->point());
    TSR_LOG_TRACE("cost: {}", current_node.gCost);
  }

  std::reverse(route.begin(), route.end());
  return route;
}

  size_t TsrState::AddWarning(const std::string &warning, const short priority) {
    if (!warning_index.contains(warning)) {
      size_t index = warning_messages.size();
      warning_messages.push_back(warning);

      warning_index[warning] = index;
      warning_priorities[index] = priority;

      return index;
    } else {
      return warning_index[warning];
    }
  }

} // namespace tsr