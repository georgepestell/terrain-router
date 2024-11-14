#include "tsr/Router.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/logging.hpp"

#include <CGAL/circulator.h>
#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <unordered_set>
#include <vector>

namespace tsr {

Router::Router(Delaunay_3 &dtm) {
  this->dtm = std::make_unique<Delaunay_3>(dtm);

  // Calculate the constant weights for each face
  for (auto face = this->dtm->finite_faces_begin();
       face != this->dtm->finite_faces_end(); face++) {
    this->constant_weights[face] = this->calculate_constant_face_cost(face);
  }
}

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

// TODO: Implement constnat face cost
double Router::calculate_constant_face_cost(Face_handle face) { return 1.0; }

std::vector<Point_3> Router::calculate_route(Point_3 &start_point,
                                             Point_3 &end_point) {

  auto start_face = this->dtm->locate(start_point);
  auto end_face = this->dtm->locate(end_point);

  Vertex_handle start = start_face->vertex(0);
  Vertex_handle end = end_face->vertex(1);

  // Setup the queue of gCosts to calculate and CLOSED set
  std::priority_queue<Node, std::vector<Node>, CompareNode> cost_queue;
  std::unordered_map<Vertex_handle, Node> best_routes;

  // Initialize the start node
  Node startNode(start);
  startNode.gCost = 0;
  cost_queue.push(startNode);

  /**
   * From the current node, calculate the cost of traversing to the
   * connected nodes
   * - We need the face, the adjacent face connecting the next node, and the
   * vertices
   */
  while (best_routes.size() < this->dtm->number_of_vertices()) {

    // Select best node from queue
    Node current_node = cost_queue.top();
    cost_queue.pop();

    // Check if the best_route gCost is better than the current best_route gCost
    if (best_routes.contains(current_node.handle)) {
      continue;
    }

    // Add this as the best_route to that node
    best_routes[current_node.handle] = current_node;

    /**
     * Calculate the costs of the adjacent not-closed nodes
     */

    if (!this->dtm->is_valid()) {
      TSR_LOG_FATAL("Invalid DTM detected");
      throw std::runtime_error("Invalid DTM detected");
    }

    // Loop over the incident faces around the vertex
    auto faceCirculator = current_node.handle->incident_faces();

    if (faceCirculator != nullptr) {
      auto faceCirculatorEnd = faceCirculator;
      do {

        auto face = faceCirculator;

        if (this->dtm->is_infinite(face)) {
          continue;
        }

        // Get the neighboring face adjacent to current vertex and the others
        for (int i = 0; i <= 2; i++) {
          Vertex_handle connectedVertex = face->vertex(i);
          if (connectedVertex == current_node.handle) {
            continue;
          }

          // Skip vertices already searched
          if (best_routes.contains(connectedVertex)) {
            continue;
          }

          // Calculate the cost
          // TODO: Face_handle neighbourFace = face->neighbor(edgeIndex);
          Node node(connectedVertex);
          Face_handle faceHandle = face;
          node.gCost = current_node.gCost +
                       calculate_trivial_cost(faceHandle, current_node.handle,
                                              connectedVertex);
          node.parent = current_node.handle;

          // TSR_LOG_TRACE(
          //     "FROM: {} {} {}, TO: {} {} {}",
          //     current_node.handle->point().x(),
          //     current_node.handle->point().y(),
          //     current_node.handle->point().z(), connectedVertex->point().x(),
          //     connectedVertex->point().y(), connectedVertex->point().z());
          // TSR_LOG_TRACE("Cost calculated: {}", node.gCost);

          // Add the node to the priority queue
          cost_queue.push(node);
        }
      } while (++faceCirculator != faceCirculatorEnd);
    }
  }

  // Get the end node point
  std::vector<Point_3> route;
  Node current_node = best_routes[end];
  route.push_back(current_node.handle->point());

  while (current_node.handle != start) {
    current_node = best_routes[current_node.parent];
    route.push_back(current_node.handle->point());
  }
  std::reverse(route.begin(), route.end());

  TSR_LOG_TRACE("Cost queue has {} nodes skipped", cost_queue.size());
  TSR_LOG_TRACE("Sucessfully analysed {} nodes", best_routes.size());

  TSR_LOG_TRACE("Completed!");
  return route;
}

double calculate_gradient(Point_3 &p1, Point_3 &p2) {

  // TSR_LOG_TRACE("p1, p2: {} {} {}, {} {} {}", p1.x(), p1.y(), p1.z(), p2.x(),
  //                p2.y(), p2.z());

  double dz = p2.z() - p1.z();

  double dx = p2.x() - p1.x();
  double dy = p2.y() - p1.y();
  double distance = sqrt(dx * dx + dy * dy);

  if (distance == 0) {
    return 0;
  }

  double gradient = dz / distance;

  return gradient;
}

// TODO: Trivial cost more than just distance
double Router::calculate_trivial_cost(Face_handle &face,
                                      Vertex_handle &vertex_start,
                                      Vertex_handle &vertex_end) {
  // TODO: Use features to determine the cost

  // TODO: DELETE JUST GETTING THE GRADIENT
  double distance =
      this->calculate_distance(vertex_start->point(), vertex_end->point());

  // Calculate the gradient TODO: Cost curves
  double gradient =
      abs(calculate_gradient(vertex_start->point(), vertex_end->point()));

  // DEBUG: Print start and end vertex points and the distance and gradients
  // TSR_LOG_TRACE("Start: {} {} {}, End: {} {} {}, Distance: {}, Gradient: {}",
  //               vertex_start->point().x(), vertex_start->point().y(),
  //               vertex_start->point().z(), vertex_end->point().x(),
  //               vertex_end->point().y(), vertex_end->point().z(), distance,
  //               gradient);

  // Simple gradient curve TODO: Proper Gradient cost curve
  double gradientInfluence = gradient * 10 + 1;

  return gradientInfluence * distance;
}

double Router::calculate_distance(Point_3 &p1, Point_3 &p2) {
  // calculate the absolute distance between the two points
  return sqrt(pow(p2.x() - p1.x(), 2) + pow(p2.y() - p1.y(), 2) +
              pow(p2.z() - p1.z(), 2));
}

} // namespace tsr