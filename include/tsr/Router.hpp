#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/FeatureManager.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point_3.hpp"

#include "tsr/TSRState.hpp"

namespace tsr {

/**
 * @brief Accepts a DTM and two points, returns the optimal route between them
 * using the D* algorithm.
 *
 */
class Router {
private:
  TSRState state;
  double calculateTrivialCost(const FeatureManager &fm, TSRState &state);

public:
  Vertex_handle nearestVertexToPoint(Delaunay_3 &dtm, Point_3 &point);

  std::vector<Point_3> calculateRoute(Delaunay_3 &dtm, FeatureManager &fm,
                                      MeshBoundary &boundary,
                                      Point_3 &start_point, Point_3 &end_point);
};

} // namespace tsr