#pragma once

#include "tsr/Point_2.hpp"
#include <vector>

namespace tsr::IO {

std::vector<std::vector<Point_2>> load_contours_from_file(std::string filepath,
                                                          std::string layerID);

}