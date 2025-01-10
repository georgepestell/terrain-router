#include "tsr/Router.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/Features/WaterFeature.hpp"
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
double calculateXYDistance(Point3 p1, Point3 p2) {

  double dx = p1.x() - p2.x();
  double dy = p1.y() - p2.y();

  return std::hypot(dx, dy);
}

Vertex_handle Router::nearestVertexToPoint(Tin &dtm, Point3 &point) {
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

std::vector<Point3> Router::calculateRoute(Tin &dtm, FeatureManager &fm,
                                           MeshBoundary &boundary,
                                           Point3 &start_point,
                                           Point3 &end_point) {

  double minX = 100000000;
  double maxX = -100000000;
  double minY = 100000000;
  double maxY = -100000000;
  for (auto v : dtm.all_vertex_handles()) {
    if (dtm.is_infinite(v)) {
      continue;
    }
    Point3 p = v->point();

    if (p.x() > maxX)
      maxX = p.x();
    if (p.x() < minX)
      minX = p.x();

    if (p.y() > maxY)
      maxY = p.y();
    if (p.y() < minY)
      minY = p.y();
  }

  TSR_LOG_TRACE("minX: {}, maxX: {}, minY: {}, maxY: {}", minX, maxX, minY,
                maxY);

  TSR_LOG_TRACE("Routing");

  // TSR_LOG_DEBUG("Deleting outside vertices");
  // delete_nodes_outside_rectangle(*this->dtm, start_point, end_point);
  // TSR_LOG_DEBUG("Kept: {} vertices", this->dtm->number_of_vertices());

  this->state.start_vertex = nearestVertexToPoint(dtm, start_point);
  this->state.end_vertex = nearestVertexToPoint(dtm, end_point);

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
          if (!boundary.isBoundedSafe(connectedVertex->point())) {
            continue;
          }

          // Calculate the cost
          // TODO: Face_handle neighbourFace = face->neighbor(edgeIndex);
          RouteNode node(connectedVertex, face);
          this->state.next_vertex = connectedVertex;
          node.gCost =
              current_node.gCost + calculateTrivialCost(fm, this->state);
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

double Router::calculateTrivialCost(const FeatureManager &fm, TsrState &state) {
  return fm.calculateCost(state);
}

} // namespace tsr