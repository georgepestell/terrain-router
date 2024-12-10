#pragma once

#include "tsr/Feature.hpp"
#include "tsr/Point_2.hpp"
#include <unordered_map>

namespace tsr {

class PathFeature : public Feature<bool> {
private:
  std::vector<std::vector<Point_2>> paths;

public:
  PathFeature(std::string name, std::vector<std::vector<Point_2>> paths)
      : Feature(name), paths(paths) {}

  bool calculate(Face_handle face, Point_3 &source_point,
                 Point_3 &target_point) override {

    for (auto path : paths) {
      auto contour = path.begin();
      auto nextContour = contour++;
      while (nextContour != path.end()) {

        if (contour->x() == source_point.x() &&
            contour->y() == source_point.y() &&
            nextContour->x() == target_point.x() &&
            nextContour->y() == target_point.y()) {
          return true;
        } else if (contour->x() == target_point.x() &&
                   contour->y() == target_point.y() &&
                   nextContour->x() == source_point.x() &&
                   nextContour->y() == source_point.y()) {
          return true;
        }

        contour++;
        nextContour++;
      }
    }
    return false;
  }
};

} // namespace tsr