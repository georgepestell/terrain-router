#include "tsr/Feature.hpp"
#include "tsr/TsrState.hpp"

#include <limits>
#include <memory>

namespace tsr {

template <typename inDataType, typename outDataType>
class InverseFeature : public Feature<outDataType> {
private:
  enum DEPENDENCIES { VALUE };

public:
  using Feature<outDataType>::Feature;

  outDataType Calculate(TsrState &state);
};

template <> inline bool InverseFeature<bool, bool>::Calculate(TsrState &state) {
  auto feature =
      std::dynamic_pointer_cast<Feature<bool>>(this->dependencies[VALUE]);
  return !feature->Calculate(state);
}
template <>
inline double InverseFeature<bool, double>::Calculate(TsrState &state) {
  auto feature =
      std::dynamic_pointer_cast<Feature<bool>>(this->dependencies[VALUE]);
  if (!feature->Calculate(state)) {
    return 1;
  } else {
    return std::numeric_limits<double>::infinity();
  }
}

template <>
inline double InverseFeature<double, double>::Calculate(TsrState &state) {

  auto feature =
      std::dynamic_pointer_cast<Feature<double>>(this->dependencies[VALUE]);

  double value = feature->Calculate(state);

  if (value == 0) {
    return std::numeric_limits<double>::infinity();
  }

  return 1 / value;
}

} // namespace tsr