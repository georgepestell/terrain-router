#include "tsr/Feature.hpp"
#include "tsr/TsrState.hpp"
#include <memory>

namespace tsr {

template <typename DataType>
class ConditionalFeature : public Feature<DataType> {
private:
  enum DEPENDENCIES { CONDITIONAL, A, B };

public:
  using Feature<DataType>::Feature;

  DataType Calculate(TsrState &state) override {

    auto conditionalFeature = std::dynamic_pointer_cast<Feature<bool>>(
        this->dependencies[CONDITIONAL]);

    if (conditionalFeature->Calculate(state)) {
      auto feature =
          std::dynamic_pointer_cast<Feature<double>>(this->dependencies[A]);
      return feature->Calculate(state);
    } else {
      auto feature =
          std::dynamic_pointer_cast<Feature<double>>(this->dependencies[B]);
      return feature->Calculate(state);
    }
  }
};

} // namespace tsr