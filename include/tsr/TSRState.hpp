#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/RouterNode.hpp"
#include "tsr/logging.hpp"
#include <unordered_map>

namespace tsr {

struct TSRState {

  Vertex_handle start_vertex;
  Vertex_handle end_vertex;

  std::unordered_map<Vertex_handle, Node> routes;
  Vertex_handle current_vertex;
  Vertex_handle next_vertex;
  Face_handle current_face;

  /// A set of warning texts which are indexed by the warnings, reduces
  /// redundant string storage

  std::vector<std::string> warning_messages = {"NONE"};
  std::unordered_map<size_t, short> warning_priorities = {{0, 0}};
  std::unordered_map<std::string, size_t> warning_index;

  /// Stores warnings for faces by their index into the warning_messages and
  /// warning_priority set
  std::unordered_map<Face_handle, size_t> warnings;

  size_t addWarning(const std::string &warning, const short priority) {
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

  void processWarnings();

  std::vector<Point_3> fetchRoute() const;
};

} // namespace tsr