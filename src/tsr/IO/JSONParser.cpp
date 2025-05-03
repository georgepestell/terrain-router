#include "tsr/Logging.hpp"
#include "tsr/Point2.hpp"
#include <string>
#include <vector>

#include <simdjson.h>

#include "tsr/IO/JSONParser.hpp"

#include "tsr/PointProcessor.hpp"

namespace tsr::IO {

std::vector<std::vector<Point2>> LoadContoursFromJsonFile(std::string filepath,
                                                          std::string layerID) {

  // Open file
  simdjson::ondemand::parser parser;

  auto json = simdjson::padded_string::load(filepath);
  TSR_LOG_TRACE("Opened JSON file {}", filepath);

  auto doc = parser.iterate(json);
  TSR_LOG_TRACE("Loaded JSON");

  auto layerArray = doc.find_field(layerID).get_array();

  TSR_LOG_TRACE("layer {}", layerArray.count_elements().value());

  std::vector<std::vector<Point2>> contours;
  for (auto feature : layerArray) {

    auto geometry = feature.find_field("geometry");

    if (geometry.error() != simdjson::SUCCESS) {
      TSR_LOG_TRACE("failed to find geometry");
      continue;
    }

    auto coordinates = geometry.find_field("coordinates").get_array();

    if (coordinates.error() != simdjson::SUCCESS) {
      TSR_LOG_TRACE("failed to find coordinates");
      continue;
    }

    std::vector<Point2> coordVector;
    for (auto coord : coordinates) {

      auto coordIt = coord.begin();

      if (coordIt == coord.end()) {
        continue;
      }

      double x = (double)*coordIt.value();

      if (++coordIt == coord.end()) {
        continue;
      }
      double y = (double)*coordIt.value();

      Point2 p(x, y);

      coordVector.push_back(p);
    }

    contours.push_back(coordVector);
  }

  return contours;
}

} // namespace tsr::IO
