#pragma once

#include "tsr/Features/APIFeature.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include <cstddef>
#include <string>
#include <unordered_set>
#include <utility>

#include "tsr/IO/KMLWriter.hpp"

namespace tsr {

struct EdgeHash {
  std::size_t operator()(const std::pair<Point3, Point3> &edge) const;
};

class PathFeature : public APIFeature<bool> {
private:
  std::unordered_set<std::pair<Point3, Point3>, EdgeHash> paths;

  static std::string URL;

  std::pair<Point3, Point3> NormalizeSegmentOrder(Point3 p1, Point3 p2);

public:
  PathFeature(std::string name, double tile_size)
      : APIFeature(name, URL, tile_size, {0, 1, 2, 3}) {}

  void Initialize(Tin &tin, const MeshBoundary &boundary) override;
  bool Calculate(TsrState &state) override;

  void WritePathsToKml() {
    std::string kml = "";

    for (auto path : this->paths) {
      kml += IO::GenerateKmlLine(path);
    }

    IO::WriteDataToFile("path.kml", IO::GenerateKmlDocument(kml));
  }
};

} // namespace tsr