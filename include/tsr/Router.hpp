#pragma once

#include <limits>

#include "tsr/Delaunay_3.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/Point_3.hpp"

namespace tsr {

class Node {
public:
  double gCost;
  bool closed;
  Vertex_handle handle;
  Vertex_handle parent;

  Node() = default;
  Node(Vertex_handle handle)
      : gCost(std::numeric_limits<double>::infinity()), closed(false),
        handle(handle), parent(nullptr) {}

  bool operator==(const Node &other) const {
    return this->handle == other.handle;
  }
};

struct CompareNode {
  bool operator()(const Node &node1, const Node &node2) {
    return node1.gCost > node2.gCost;
  }
};

struct HashNode {
  bool operator()(const Node &node1) {
    return std::hash<Vertex_handle>()(node1.handle);
  }
};

/**
 * @brief Accepts a DTM and two points, returns the optimal route between them
 * using the D* algorithm.
 *
 */
class Router {
private:
  std::shared_ptr<Delaunay_3> dtm;
  FeatureManager feature_manager;

  std::unordered_map<Vertex_handle, Node> nodes;
  double calculateTrivialCost(Face_handle face, Vertex_handle vertex_start,
                              Vertex_handle vertex_end);

public:
  Router(Delaunay_3 &dtm, FeatureManager &feature_manager);

  void propagate_costs();

  Vertex_handle nearestVertexToPoint(Point_3 &point);

  std::vector<Point_3> calculateRoute(Point_3 &start_point, Point_3 &end_point);
};

} // namespace tsr