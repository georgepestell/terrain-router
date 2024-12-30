#pragma once

#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/Features/APIFeature.hpp"
#include "tsr/IO/ChunkCache.hpp"
#include "tsr/IO/JSONParser.hpp"
#include "tsr/Point_2.hpp"
#include "tsr/Point_3.hpp"
#include "tsr/TSRState.hpp"
#include "tsr/logging.hpp"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <boost/concept_check.hpp>
#include <boost/functional/hash.hpp>
#include <boost/type_index/stl_type_index.hpp>
#include <gdal/gdal.h>

namespace tsr {

struct EdgeHash {
  std::size_t operator()(const std::pair<Point_2, Point_2> &edge) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, edge.first);
    boost::hash_combine(seed, edge.second);
    return seed;
  }
};

class PathFeature : public APIFeature<bool> {
private:
  std::unordered_set<std::pair<Point_2, Point_2>, EdgeHash> paths;

  inline static std::string URL =
      "https://lz4.overpass-api.de/api/"
      "interpreter?data=%5Bout%3Axml%5D%5Btimeout%3A25%5D%3B%0A%28%0A%20%20way%"
      "5B%27highway%27~%27trunk%7Csecondary%7Cunclassified%7Cresidential%"
      "7Ctertiary%7Ctrunk_link%7Cprimary_link%7Csecondary_link%7Ctertiary_link%"
      "7Cliving_street%7Csteps%7Croad%7Cfootway%7Cvia_ferrata%7Csidewalk%"
      "7Ccrossing%7Ccycleway%7Ctraffic_island%7Cpath%7Cpedestrian%7Ctrack%"
      "7Cservice%27%5D%5B%27foot%27%21~%27no%27%5D%5B%27route%27%21~%27ferry%"
      "27%5D%28{}%2C%20{}%2C%20{}%2C%20{}%29%3B%0A%20%20way%5B%27foot%27%3D%"
      "27yes%27%5D%5B%27route%27%21~%27ferry%27%5D%28{}%2C%20{}%2C%20{}%2C%20{}"
      "%29%3B%0A%20%20way%5B%27bridge%27%3D%27yes%27%5D%5B%27foot%27%3D%27yes%"
      "27%5D%28{}%2C%20{}%2C%20{}%2C%20{}%29%3B%0A%29%3B%0A%28._%3B%3E%3B%29%"
      "3Bout+body%3B{}";

  std::pair<Point_2, Point_2> normalizeSegment(Point_2 p1, Point_2 p2) {
    if (p1 < p2) {
      return {p2, p1};
    } else {
      return {p1, p2};
    }
  }

public:
  PathFeature(std::string name, double tile_size)
      : APIFeature(name, URL, tile_size, {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3}) {
  }

  void initialize(Delaunay_3 &dtm, const MeshBoundary &boundary) override {

    auto chunks = chunker.getRequiredChunks(boundary);
    const double MAX_SEGMENT_SIZE = 22;

    // Check if the paths are cached
    std::vector<std::vector<Point_2>> contours;
    for (auto chunk : chunks) {

      if (IO::isChunkCached(this->featureID, chunk)) {
        // Contours are cached
        IO::getChunkFromCache<std::vector<std::vector<Point_2>>>(
            this->featureID, chunk, contours);
      } else {
        // Download from API
        auto data = chunker.fetchVectorChunk(chunk);
        GDALReleaseDataset(data.dataset);

        contours = IO::load_contours_from_file(data.filename, "features");

        // Cache Contours
        try {
          IO::cacheChunk(this->featureID, chunk, contours);
        } catch (std::exception e) {
          TSR_LOG_WARN("failed to cache path contours");
        }
      }

      for (auto contour : contours) {
        add_contour_constraint(dtm, contour, MAX_SEGMENT_SIZE);

        for (unsigned int i = 0; i < contour.size() - 1; i++) {
          this->paths.insert(normalizeSegment(contour[i], contour[i + 1]));
        }
      }
    }
  }

  bool calculate(TSRState &state) override {

    const Point_3 source_point = state.current_vertex->point();
    const Point_3 target_point = state.next_vertex->point();

    const std::pair<Point_2, Point_2> segment =
        normalizeSegment(Point_2(source_point.x(), source_point.y()),
                         Point_2(target_point.x(), target_point.y()));

    return this->paths.contains(segment);
  }
};

} // namespace tsr