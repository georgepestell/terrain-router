#include "tsr/Router.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/logging.hpp"

#include <CGAL/circulator.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace tsr {
double calculateXYDistance(Point_3 p1, Point_3 p2) {

  double dx = p1.x() - p2.x();
  double dy = p1.y() - p2.y();

  return std::hypot(dx, dy);
}

Vertex_handle Router::nearestVertexToPoint(Delaunay_3 &dtm, Point_3 &point) {
  Face_handle face = dtm.locate(point);

  if (face == nullptr || !face->is_valid() || dtm.is_infinite(face)) {
    TSR_LOG_ERROR("Point outside DTM domain");
    throw std::runtime_error("Point outside DTM domain");
  }

  Vertex_handle vertex = face->vertex(0);
  double minDistance = calculateXYDistance(face->vertex(0)->point(), point);
  for (int i = 1; i < 3; i++) {
    double distance = calculateXYDistance(face->vertex(i)->point(), point);
    if (distance < minDistance) {
      vertex = face->vertex(i);
      minDistance = distance;
    }
  }

  return vertex;
}

std::vector<Point_3> Router::calculateRoute(Delaunay_3 &dtm, FeatureManager &fm,
                                            Point_3 &start_point,
                                            Point_3 &end_point) {

  TSR_LOG_TRACE("Routing");

  // TSR_LOG_DEBUG("Deleting outside vertices");
  // delete_nodes_outside_rectangle(*this->dtm, start_point, end_point);
  // TSR_LOG_DEBUG("Kept: {} vertices", this->dtm->number_of_vertices());

  Vertex_handle startVertex = nearestVertexToPoint(dtm, start_point);
  Vertex_handle endVertex = nearestVertexToPoint(dtm, end_point);

  // Setup the queue of gCosts to calculate and CLOSED set
  std::priority_queue<Node, std::vector<Node>, CompareNode> cost_queue;
  std::unordered_map<Vertex_handle, Node> best_routes;

  // Initialize the start node
  Node startNode(startVertex);
  startNode.gCost = 0;
  cost_queue.push(startNode);

  /**
   * From the current node, calculate the cost of traversing to the
   * connected nodes
   * - We need the face, the adjacent face connecting the next node, and the
   * vertices
   */
  TSR_LOG_TRACE("Starting search");
  do {

    // Select best node from queue
    Node current_node = cost_queue.top();
    cost_queue.pop();

    // Check if this route is already beaten
    if (best_routes.contains(current_node.handle)) {
      continue;
    }

    // Check if the route is possible
    if (current_node.gCost == std::numeric_limits<double>::infinity()) {
      TSR_LOG_FATAL("Could not find safe path");
      throw std::runtime_error("Could not find safe path");
    }

    // Add this as the best_route to that node
    best_routes[current_node.handle] = current_node;

    /**
     * Calculate the costs of the adjacent not-closed nodes
     */

    if (!dtm.is_valid()) {
      TSR_LOG_FATAL("Invalid DTM detected");
      throw std::runtime_error("Invalid DTM detected");
    }

    // Fetch each vertices
    auto faceCirculator = current_node.handle->incident_faces();
    if (faceCirculator != nullptr) {
      auto faceCirculatorEnd = faceCirculator;
      do {

        auto face = faceCirculator;
        if (dtm.is_infinite(face)) {
          continue;
        }

        // Get vertices of adjacent face
        for (int i = 0; i < 3; i++) {
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
                       calculateTrivialCost(fm, faceHandle, current_node.handle,
                                            connectedVertex);
          node.parent = current_node.handle;

          // Add the node to the priority queue
          cost_queue.push(node);
        }
      } while (++faceCirculator != faceCirculatorEnd);
    }
  } while (!best_routes.contains(endVertex));

  // Get the end node point
  std::vector<Point_3> route;
  Node current_node = best_routes[endVertex];
  TSR_LOG_TRACE("cost: {}", current_node.gCost);
  route.push_back(end_point);

  while (current_node != startNode) {
    current_node = best_routes[current_node.parent];
    route.push_back(current_node.handle->point());
    TSR_LOG_TRACE("cost: {}", current_node.gCost);
  }

  std::reverse(route.begin(), route.end());

  TSR_LOG_TRACE("Cost queue has {} nodes skipped", cost_queue.size());
  TSR_LOG_TRACE("Sucessfully analysed {} nodes", best_routes.size());

  TSR_LOG_TRACE("Completed!");
  return route;
}

double Router::calculateTrivialCost(FeatureManager &fm, Face_handle face,
                                    Vertex_handle vertex_start,
                                    Vertex_handle vertex_end) {
  return fm.calculateCost(face, vertex_start->point(), vertex_end->point());
}

} // namespace tsr