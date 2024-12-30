#pragma once

#include "tsr/Delaunay_3.hpp"
#include <functional>
#include <limits>

namespace tsr {

class Node {
public:
  double gCost;
  bool closed;
  Vertex_handle vertex;
  Face_handle face;
  Vertex_handle parent;

  Node() = default;
  Node(Vertex_handle vertex, Face_handle face)
      : gCost(std::numeric_limits<double>::infinity()), closed(false),
        vertex(vertex), face(face), parent(nullptr) {}

  bool operator==(const Node &other) const {
    return this->vertex == other.vertex;
  }
};

struct CompareNode {
  bool operator()(const Node &node1, const Node &node2) {
    return node1.gCost > node2.gCost;
  }
};

struct HashNode {
  bool operator()(const Node &node1) {
    return std::hash<Vertex_handle>()(node1.vertex);
  }
};

} // namespace tsr