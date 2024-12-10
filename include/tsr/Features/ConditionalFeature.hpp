#include "tsr/Feature.hpp"
#include <memory>

namespace tsr {

template <typename DataType>
class ConditionalFeature : public Feature<DataType> {
private:
  enum DEPENDENCIES { CONDITIONAL, A, B };

public:
  using Feature<DataType>::Feature;

  DataType calculate(Face_handle face, Point_3 &source_point,
                     Point_3 &target_point) override {

    auto conditionalFeature = std::dynamic_pointer_cast<Feature<bool>>(
        this->dependencies[CONDITIONAL]);

    if (conditionalFeature->calculate(face, source_point, target_point)) {
      auto feature =
          std::dynamic_pointer_cast<Feature<double>>(this->dependencies[A]);
      return feature->calculate(face, source_point, target_point);
    } else {
      auto feature =
          std::dynamic_pointer_cast<Feature<double>>(this->dependencies[B]);
      return feature->calculate(face, source_point, target_point);
    }
  }
};

} // namespace tsr