#include "tsr/Feature.hpp"
#include "tsr/TSRState.hpp"
#include "tsr/logging.hpp"
#include <limits>
#include <memory>

namespace tsr {

template <typename inDataType, typename outDataType>
class InverseFeature : public Feature<outDataType> {
private:
  enum DEPENDENCIES { VALUE };

public:
  using Feature<outDataType>::Feature;

  outDataType calculate(TSRState &state);
};

template <> inline bool InverseFeature<bool, bool>::calculate(TSRState &state) {
  auto feature =
      std::dynamic_pointer_cast<Feature<bool>>(this->dependencies[VALUE]);
  return !feature->calculate(state);
}
template <>
inline double InverseFeature<bool, double>::calculate(TSRState &state) {
  auto feature =
      std::dynamic_pointer_cast<Feature<bool>>(this->dependencies[VALUE]);
  if (!feature->calculate(state)) {
    return 1;
  } else {
    return std::numeric_limits<double>::infinity();
  }
}

template <>
inline double InverseFeature<double, double>::calculate(TSRState &state) {

  auto feature =
      std::dynamic_pointer_cast<Feature<double>>(this->dependencies[VALUE]);

  double value = feature->calculate(state);

  if (value == 0) {
    return std::numeric_limits<double>::infinity();
  }

  return 1 / value;
}

} // namespace tsr