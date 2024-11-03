#pragma once

#include <unordered_map>

namespace tsr {

template <class DataType>
class Feature {
private:

bool valid; /// Whether the feature map is valid

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

virtual ~Feature() = default;

};

}