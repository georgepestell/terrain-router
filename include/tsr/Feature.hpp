#pragma once

#include <memory>
#include <string>
#include <unordered_map>

namespace tsr {

class DTM;

enum FEATURE_TYPE { BOOLEAN, CONTINUOUS, DISCREET };

template <typename DataType> class Feature {
private:
  bool valid; /// Whether the feature tags are up to date
  std::string name;
  std::unordered_map<unsigned int, DataType> values;

public:
  virtual DataType &get_value(unsigned int index);
  virtual void add_value(unsigned int index, DataType &value);

  /**
   * @brief Resets the valid flag, allowing value updates
   *
   */
  void reset();
  void set_invalid();
  void set_valid();

  Feature(std::string name);

  template <class OtherDataType>
  bool isEqual(Feature<OtherDataType> &otherFeature);

  virtual void tag(std::unique_ptr<DTM> &dtm);
};

} // namespace tsr