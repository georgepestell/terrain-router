#include "tsr/features/GradientFeature.hpp"

#include <CGAL/distance.h>

namespace tsr::features {

double
GradientFeature::get_cost(State &state,
                          std::vector<std::shared_ptr<Feature>> &dependencies) : Feature::get_cost(state, dependencies) {

  // Calculate the trivial gradient cost
  Point_3 start = state.startVertex->point();
  Point_3 end = state.endVertex->point();
  
  // Calculate the distance between the start and end vertices  
   = state.endVertex->point() + Vector(state.x_from_endVertex, state.y_from_endVertex, 0);
  auto distance = sqrt(CGAL::squared_distance(start, end));

  if (distance == 0) {
    return 0.0; // Avoid division by zero
  }

  double trivialGradient = (end.z() - start.z()) / distance;

  // If the direct gradient is beyond the threshold, return infinite cost
  if (trivialGradient > this->threshold) {
    return std::numeric_limits<double>::infinity();
  }

  // Check if indirect gradient is required
  double indirectGradient = 0;
  if (state.x_from_endVertex != 0) {

    // If the indirect gradient is beyond the threshold, return infinite cost
    if (indirectGradient > this->threshold) {
      return std::numeric_limits<double>::infinity();
    }
  }

  // Direct gradient
  double directGradient = 0;
  if(state.y_from_endVertex != 0) {

    // If the direct gradient is beyond the threshold, return infinite cost
    if (directGradient > this->threshold) {
      return std::numeric_limits<double>::infinity();
    }
  }

  // Calculate the combined average gradient
  




}

} // namespace tsr::features