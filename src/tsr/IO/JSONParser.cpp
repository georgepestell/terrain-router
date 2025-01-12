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
  TSR_LOG_TRACE("Opened JSON file");

  auto doc = parser.iterate(json);
  TSR_LOG_TRACE("Loaded JSON");

  auto layerArray = doc.find_field(layerID).get_array();

  std::vector<std::vector<Point2>> contours;
  for (auto feature : layerArray) {

    auto coordinates =
        feature.find_field("geometry").find_field("coordinates").get_array();

    std::vector<Point2> coordVector;
    for (auto coord : coordinates) {

      auto coordIt = coord.begin();

      Point2 p((double)*coordIt.value(), (double)*++coordIt.value());

      coordVector.push_back(p);
    }

    contours.push_back(coordVector);
  }

  return contours;
}

} // namespace tsr::IO