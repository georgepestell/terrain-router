#include "tsr/Feature.hpp"
#include "tsr/logging.hpp"
#include <limits>
#include <memory>

namespace tsr {

template <typename DataType> class InverseFeature : public Feature<DataType> {
private:
  enum DEPENDENCIES { VALUE };

public:
  using Feature<DataType>::Feature;

  DataType calculate(Face_handle face, Point_3 &source_point,
                     Point_3 &target_point);
};

template <>
inline bool InverseFeature<bool>::calculate(Face_handle face,
                                            Point_3 &source_point,
                                            Point_3 &target_point) {
  auto feature =
      std::dynamic_pointer_cast<Feature<bool>>(this->dependencies[VALUE]);
  return !feature->calculate(face, source_point, target_point);
}

template <>
inline double InverseFeature<double>::calculate(Face_handle face,
                                                Point_3 &source_point,
                                                Point_3 &target_point) {

  auto feature =
      std::dynamic_pointer_cast<Feature<double>>(this->dependencies[VALUE]);

  double value = feature->calculate(face, source_point, target_point);

  if (value == 0) {
    return std::numeric_limits<double>::infinity();
  }

  return 1 / value;
}

} // namespace tsr