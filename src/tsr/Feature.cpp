#include "tsr/Feature.hpp"
#include "tsr/logging.hpp"

#include <stdexcept>

namespace tsr {

template <class DataType> Feature<DataType>::Feature(std::string name) {
  this->name = name;
  this->valid = false;
}

template <class DataType> void Feature<DataType>::set_valid() {
  this->valid = true;
}

template <class DataType> void Feature<DataType>::set_invalid() {
  this->valid = false;
}

template <class DataType> void Feature<DataType>::reset() {
  this->values.clear();
  this->valid = false;
}

template <class DataType>
void Feature<DataType>::add_value(unsigned int index, DataType &value) {
  // only add if the mesh is not already validated
  if (this->valid) {
    TSR_LOG_ERROR(
        "Cannot add to already valid mesh: reset or invalidate first");
    throw std::runtime_error("Cannot add to already valid mesh");
  }

  this->values.insert(index, value);
}

template <class DataType>
DataType &Feature<DataType>::get_value(unsigned int index) {
  // Return the value at the index or throw out_of_range exception
  return this->values.at(index);
}

template <class DataType>
template <class OtherDataType>
bool Feature<DataType>::isEqual(Feature<OtherDataType> &otherFeature) {
  // Equality determined by feature name
  return this->name == otherFeature.name;
}

} // namespace tsr