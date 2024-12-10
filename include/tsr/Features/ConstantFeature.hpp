#include "tsr/Feature.hpp"

namespace tsr {

class ConstantFeature : public Feature<double> {
private:
  double constant;

public:
  // Disable the default
  ConstantFeature(std::string name, double constant)
      : Feature<double>(name), constant(constant) {}

  double calculate(Face_handle face, Point_3 &source_point,
                   Point_3 &target_point) override {
    return this->constant;
  }
};

} // namespace tsr