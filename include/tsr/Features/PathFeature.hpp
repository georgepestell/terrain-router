#pragma once

#include "tsr/Features/APIFeature.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include <cstddef>
#include <string>
#include <unordered_set>
#include <utility>

namespace tsr {

struct EdgeHash {
  std::size_t operator()(const std::pair<Point2, Point2> &edge) const;
};

class PathFeature : public APIFeature<bool> {
private:
  std::unordered_set<std::pair<Point2, Point2>, EdgeHash> paths;

  static std::string URL;

  std::pair<Point2, Point2> normalizeSegment(Point2 p1, Point2 p2);

public:
  PathFeature(std::string name, double tile_size)
      : APIFeature(name, URL, tile_size, {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
                                          0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3}) {
  }

  void Initialize(Tin &tin, const MeshBoundary &boundary) override;

  bool Calculate(TsrState &state) override;
};

} // namespace tsr

// TODO: VARIABLE SIMPLIFICATION OF PATHS BY VERTEX COUNT