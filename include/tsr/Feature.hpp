#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "tsr/Delaunay_3.hpp"

namespace tsr {

class State {
  public:
    Delaunay_3 dtm;
    Vertex_handle startVertex; 
    Vertex_handle endVertex;
    Face_handle face;
    double x_from_endVertex;
    double y_from_endVertex;
};


class Feature {
private:
  bool valid; /// Whether the feature tags are up to date
  std::string name;

public:
  virtual double
  get_cost(State &state,
           std::vector<std::shared_ptr<Feature>> &dependencies) = 0;
  virtual void tag(State &state, std::vector<std::shared_ptr<Feature>> &dependencies) = 0;

  /**
   * @brief Resets the valid flag, allowing value updates
   *
   */
  void reset();
  void set_invalid();
  void set_valid();

  Feature(std::string name);

  bool isEqual(Feature &otherFeature);
};

} // namespace tsr