#include "tsr/Router.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/logging.hpp"

#include <CGAL/circulator.h>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <unordered_set>
#include <vector>

#include <CGAL/squared_distance_3.h>

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
    return node1.gCost < node2.gCost;
  }
};

struct HashNode {
  bool operator()(const Node &node1) {
    return std::hash<Vertex_handle>()(node1.handle);
  }
};

// TODO: Implement constnat face cost
double Router::calculate_constant_face_cost(Face_handle face) { return 1.0; }

std::vector<Point_3> Router::calculate_route(Vertex_handle &start,
                                             Vertex_handle &end) {

  // Setup the queue of gCosts to calculate and CLOSED set
  std::priority_queue<Node, std::vector<Node>, CompareNode> cost_queue;
  std::unordered_map<Vertex_handle, Node> best_routes;

  // Initialize the start node
  Node startNode(start);
  startNode.gCost = 0;
  cost_queue.push(startNode);

  TSR_LOG_INFO("DTM Vertices: {}", this->dtm->number_of_vertices());

  /**
   * From the current node, calculate the cost of traversing to the
   * connected nodes
   * - We need the face, the adjacent face connecting the next node, and the
   * vertices
   */
  while (best_routes.size() < this->dtm->number_of_vertices()) {

    TSR_LOG_TRACE("Getting next node");

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

    // Loop over the incident faces around the vertex
    TSR_LOG_TRACE("Getting next node costs");
    auto faceCirculator = current_node.handle->incident_faces();
    auto faceCirculatorEnd = faceCirculator;
    do {

      Face_handle face = faceCirculator;

      // Get the index of the current node vertex for this face
      int vertexFaceIndex = -1;
      for (int i = 0; i < 3; i++) {
        if (face->vertex(i) == current_node.handle) {
          vertexFaceIndex = i;
          break;
        }
      }

      TSR_LOG_TRACE("current index: {}", vertexFaceIndex);

      if (vertexFaceIndex == -1) {
        TSR_LOG_FATAL("Couldn't find vertex on face");
        throw std::runtime_error("Couldn't find vertex on face");
      }

      // Get the neighboring face adjacent to current vertex and the others
      for (int i = 1; i <= 2; i++) {
        int edgeIndex = (vertexFaceIndex + i) % 3;

        TSR_LOG_TRACE("checking index: {}", edgeIndex);

        Vertex_handle connectedVertex = face->vertex(edgeIndex);

        if (this->dtm->is_infinite(connectedVertex)) {
          TSR_LOG_TRACE("skipping infinite vertex");
          continue;
        }

        if (connectedVertex == current_node.handle) {
          TSR_LOG_FATAL("Selected own vertex");
          throw std::runtime_error("Selected own vertex");
        }

        // Skip vertices already searched
        if (best_routes.contains(connectedVertex)) {
          continue;
        }

        // Calculate the cost
        // TODO: Face_handle neighbourFace = face->neighbor(edgeIndex);
        Node node(connectedVertex);
        node.gCost =
            current_node.gCost +
            calculate_trivial_cost(face, current_node.handle, connectedVertex);
        TSR_LOG_TRACE(
            "FROM: {} {} {}, TO: {} {} {}", current_node.handle->point().x(),
            current_node.handle->point().y(), current_node.handle->point().z(),
            connectedVertex->point().x(), connectedVertex->point().y(),
            connectedVertex->point().z());
        TSR_LOG_TRACE("Cost calculated: {}", node.gCost);
        node.parent = current_node.handle;

        // Add the node to the priority queue
        cost_queue.push(node);
      }
    } while (++faceCirculator != faceCirculatorEnd);
  } 

  // Get the end node point
  std::vector<Node> route;
  Node current_node = best_routes[end];
  route.push_back(current_node);

  while (current_node.handle != start) {
    current_node = best_routes[current_node.parent];
    route.push_back(current_node);
  }

  TSR_LOG_TRACE("Starting Route:");
  for (int i = route.size(); i > 0; i--) {
    Node n = route[i - 1];
    Point_3 p = n.handle->point();
    TSR_LOG_TRACE("- x: {}, y: {}, z: {}. cost: {}", p.x(), p.y(), p.z(),
                  n.gCost);
  }

  TSR_LOG_TRACE("Cost queue has {} nodes skipped", cost_queue.size());
  TSR_LOG_TRACE("Sucessfully analysed {} nodes", best_routes.size());

  TSR_LOG_TRACE("Completed!");
  return std::vector<Point_3>();
}

// TODO: Trivial cost more than just distance
double Router::calculate_trivial_cost(Face_handle &face,
                                      Vertex_handle &vertex_start,
                                      Vertex_handle &vertex_end) {
  return this->calculate_distance(vertex_start->point(), vertex_end->point());
}

double Router::calculate_distance(Point_3 &p1, Point_3 &p2) {

  return sqrt(CGAL::squared_distance(p1, p2));
}

} // namespace tsr