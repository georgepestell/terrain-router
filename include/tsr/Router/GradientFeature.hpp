#pragma once

#include "tsr/Feature.hpp"

namespace tsr {
class GradientFeature : public Feature {
public:
  virtual double
  get_cost(State &state,
           std::vector<std::shared_ptr<Feature>> &dependencies) override;
  virtual void
  tag(State &state,
      std::vector<std::shared_ptr<Feature>> &dependencies) override;
};
} // namespace tsr