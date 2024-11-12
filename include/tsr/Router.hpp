#include <limits>

#include "tsr/Delaunay_3.hpp"
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

class Face {
public:
  double constant_weight;
};

/**
 * @brief Accepts a DTM and two points, returns the optimal route between them
 * using the D* algorithm.
 *
 */
class Router {
private:
  std::unique_ptr<Delaunay_3> dtm;
  std::unordered_map<Face_handle, double> constant_weights;
  std::unordered_map<Vertex_handle, Node> nodes;

  double calculate_constant_face_cost(Face_handle face);

  // double calculate_face_cost(Face_handle face);

  double calculate_distance(Point_3 &p1, Point_3 &p2);

  double calculate_trivial_cost(Face_handle &face, Vertex_handle &vertex_start,
                                Vertex_handle &vertex_end);

  // double calculate_indirect_cost(Face_handle &face, Vertex_handle
  // &vertex_start,
  //                                Point_3 &point_middle,
  //                                Vertex_handle &vertex_end);

  // double calculate_direct_cost(Face_handle &face, Vertex_handle
  // &vertex_start,
  //                              Point_3 &point_middle_1, Point_3
  //                              &point_middle_2, Vertex_handle &vertex_end);

public:
  Router(Delaunay_3 &dtm);

  void propagate_costs();

  std::vector<Point_3> calculate_route(Vertex_handle &start,
                                       Vertex_handle &end);
};

} // namespace tsr