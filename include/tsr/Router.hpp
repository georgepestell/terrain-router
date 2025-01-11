#pragma once

#include "tsr/FeatureManager.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"

#include "tsr/TsrState.hpp"

namespace tsr {

/**
 * @brief Accepts a DTM and two points, returns the optimal route between them
 * using Dijkstra's shortest path search algorithm with a custom cost function defined in FeatureManager.
 *
 */
class Router {
private:
  TsrState state;
  double calculateTrivialCost(const FeatureManager &fm, TsrState &state);

public:
  Vertex_handle nearestVertexToPoint(Tin &dtm, Point3 &point);

  std::vector<Point3> calculateRoute(Tin &dtm, FeatureManager &fm,
                                     MeshBoundary &boundary,
                                     Point3 &start_point, Point3 &end_point);
};

} // namespace tsr