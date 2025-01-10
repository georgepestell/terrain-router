#pragma once

#include "tsr/Point2.hpp"
#include <vector>

namespace tsr::IO {

std::vector<std::vector<Point2>> load_contours_from_file(std::string filepath,
                                                         std::string layerID);

}