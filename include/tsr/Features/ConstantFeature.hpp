#include "tsr/Feature.hpp"
#include "tsr/TSRState.hpp"
#include <boost/concept_check.hpp>

namespace tsr {

template <typename DataType> class ConstantFeature : public Feature<DataType> {
private:
  const DataType constant;

public:
  // Disable the default
  ConstantFeature(std::string name, DataType constant)
      : Feature<DataType>(name), constant(constant) {}

  DataType calculate(TSRState &state) override {

    // Ignore compiler unused variable warnings. Keeps the template for
    // calculate the same as other features
    boost::ignore_unused_variable_warning(state);

    return this->constant;
  }
};

} // namespace tsr