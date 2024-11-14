#pragma once

#include "tsr/Feature.hpp"

namespace tsr::features {
class GradientFeature : public Feature {
public:
  virtual double
  get_cost(State &state,
           std::vector<std::shared_ptr<Feature>> &dependencies) override;
  virtual void
  tag(Delaunay_3 &dtm,
      std::vector<std::shared_ptr<Feature>> &dependencies) override;
};
} // namespace tsr::features