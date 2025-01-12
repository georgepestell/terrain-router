#include "tsr/Router.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/Features/BoolWaterFeature.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/IO/KMLWriter.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"

#include <CGAL/circulator.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace tsr {
double calculateXYDistance(const Point3 p1, const Point3 p2) {

  double dx = p1.x() - p2.x();
  double dy = p1.y() - p2.y();

  return std::hypot(dx, dy);
}

Vertex_handle Router::CalculateNearestVertexToPoint(const Tin &dtm,
                                           const Point3 &point) {
  Face_handle face = dtm.locate(point);

  if (face == nullptr || !face->is_valid() || dtm.is_infinite(face)) {
    TSR_LOG_ERROR("Point outside DTM domain");
    TSR_LOG_TRACE("point: {} {}", point.x(), point.y());
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

std::vector<Point3> Router::Route(const Tin &dtm, FeatureManager &fm,
                                           const MeshBoundary &boundary,
                                           const Point3 &start_point,
                                           const Point3 &end_point) {

  TSR_LOG_TRACE("Routing");

  // Fetch the nearest search node to the given points
  this->state.start_vertex = CalculateNearestVertexToPoint(dtm, start_point);
  this->state.end_vertex = CalculateNearestVertexToPoint(dtm, end_point);

  // Setup the queue of gCosts to calculate and CLOSED set
  std::priority_queue<RouteNode, std::vector<RouteNode>, CompareNode>
      cost_queue;

  // Initialize the start node
  RouteNode startNode(this->state.start_vertex, nullptr);
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
    RouteNode current_node = cost_queue.top();
    cost_queue.pop();

    // Check if this route is already beaten
    if (this->state.routes.contains(current_node.vertex)) {
      continue;
    }

    // Check if the route is possible
    if (current_node.gCost == std::numeric_limits<double>::infinity()) {
      TSR_LOG_FATAL("Could not find safe path");
      IO::writeFailureStateToKML("failure.kml", state);
      throw std::runtime_error("Could not find safe path");
    }

    // Add this as the best_route to that node
    this->state.routes[current_node.vertex] = current_node;
    this->state.current_vertex = current_node.vertex;

    /**
     * Calculate the costs of the adjacent not-closed nodes
     */

    if (!dtm.is_valid()) {
      TSR_LOG_FATAL("Invalid DTM detected");
      throw std::runtime_error("Invalid DTM detected");
    }

    // Fetch each vertices
    auto faceCirculator = current_node.vertex->incident_faces();
    if (faceCirculator != nullptr) {
      auto faceCirculatorEnd = faceCirculator;
      do {

        auto face = faceCirculator;
        if (dtm.is_infinite(face)) {
          continue;
        }

        this->state.current_face = face;

        // Get vertices of adjacent face
        for (int i = 0; i < 3; i++) {
          Vertex_handle connectedVertex = face->vertex(i);
          if (connectedVertex == current_node.vertex) {
            continue;
          }

          // Skip vertices already searched
          if (this->state.routes.contains(connectedVertex)) {
            continue;
          }

          // Check the point is bounded
          if (!boundary.IsBoundedSafe(connectedVertex->point())) {
            continue;
          }

          // Calculate the cost
          // TODO: Face_handle neighbourFace = face->neighbor(edgeIndex);
          RouteNode node(connectedVertex, face);
          this->state.next_vertex = connectedVertex;
          node.gCost =
              current_node.gCost + CalculateTrivialCost(fm, this->state);
          node.parent = current_node.vertex;

          // Add the node to the priority queue
          cost_queue.push(node);
        }
      } while (++faceCirculator != faceCirculatorEnd);
    }
  } while (!this->state.routes.contains(this->state.end_vertex));

  auto route = this->state.fetchRoute();

  TSR_LOG_TRACE("Cost queue has {} nodes skipped", cost_queue.size());
  TSR_LOG_TRACE("Sucessfully analysed {} nodes", this->state.routes.size());

  // Filter the warnings along the route
  IO::writeSuccessStateToKML("success.kml", state);

  TSR_LOG_TRACE("Completed!");
  return route;
}

double Router::CalculateTrivialCost(const FeatureManager &fm, TsrState &state) {
  return fm.Calculate(state);
}

} // namespace tsr