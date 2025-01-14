#pragma once

#include "tsr/Tin.hpp"
#include <functional>
#include <limits>

namespace tsr {

class RouteNode {
public:
  double gCost;
  bool closed;
  Vertex_handle vertex;
  Face_handle face;
  Vertex_handle parent;

  RouteNode() = default;
  RouteNode(Vertex_handle vertex, Face_handle face)
      : gCost(std::numeric_limits<double>::infinity()), closed(false),
        vertex(vertex), face(face), parent(nullptr) {}

  bool operator==(const RouteNode &other) const {
    return this->vertex == other.vertex;
  }
};

struct CompareNode {
  bool operator()(const RouteNode &node1, const RouteNode &node2) {
    return node1.gCost > node2.gCost;
  }
};

struct HashNode {
  bool operator()(const RouteNode &node1) {
    return std::hash<Vertex_handle>()(node1.vertex);
  }
};

} // namespace tsr