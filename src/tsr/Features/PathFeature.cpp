#include "tsr/Features/PathFeature.hpp"
#include "tsr/ChunkInfo.hpp"
#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/Features/BoolWaterFeature.hpp"
#include "tsr/IO/ChunkCache.hpp"
#include "tsr/IO/JSONParser.hpp"
#include "tsr/Logging.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include <cmath>
#include <cstddef>
#include <exception>
#include <gdal.h>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace tsr {

std::size_t EdgeHash::operator()(const std::pair<Point3, Point3> &edge) const {
  size_t seed = 0;
  boost::hash_combine(seed, edge.first.x());
  boost::hash_combine(seed, edge.first.y());
  boost::hash_combine(seed, edge.second.x());
  boost::hash_combine(seed, edge.second.y());
  return seed;
}

std::string PathFeature::URL =
    "https://lz4.overpass-api.de/api/"
    "interpreter?data=%5Bout%3Axml%5D%5Btimeout%3A25%5D%3B%28way%5B%22highway%"
    "22%5D%28{},{},{},{}%29%3B%29%3B%28._%3B%3E%3B%29%3Bout+body%"
    "3B%0A{}";

std::pair<Point3, Point3> PathFeature::NormalizeSegmentOrder(Point3 p1,
                                                             Point3 p2) {

  // Lexicographical ordering on coords
  bool retainOrder;
  if (p1.x() < p2.x()) {
    retainOrder = true;
  } else if (p1.x() > p2.x()) {
    retainOrder = false;
  } else if (p1.y() < p2.y()) {
    retainOrder = true;
  } else if (p1.y() > p2.y()) {
    retainOrder = false;
  } else if (p1.z() < p2.z()) {
    retainOrder = true;
  } else {
    retainOrder = false;
  }

  if (retainOrder) {
    return {p1, p2};
  } else {
    return {p2, p1};
  }
}

void PathFeature::Initialize(Tin &tin, const MeshBoundary &boundary) {
  auto chunks = chunkManager.GetRequiredChunks(boundary);
  const double MAX_SEGMENT_SIZE = 15;

  // Check if the paths are cached
  std::vector<std::vector<Point2>> contours;
  for (auto chunk : chunks) {

    if (chunkManager.IsAvailableInCache(this->feature_id, chunk)) {
      // Contours are cached
      IO::GetChunkFromCache<std::vector<std::vector<Point2>>>(this->feature_id,
                                                              chunk, contours);
    } else {
      // Download from API
      auto data = chunkManager.FetchVectorChunk(chunk);
      // IO::CacheChunk("test", chunk, data.dataset);
      GDALReleaseDataset(data.dataset);

      contours = IO::LoadContoursFromJsonFile(data.filename, "features");

      // Cache Contours
      try {
        IO::CacheChunk(this->feature_id, chunk, contours);
      } catch (std::exception &e) {
        TSR_LOG_WARN("failed to cache path contours");
      }
    }

    TSR_LOG_TRACE("Path Contours: {}", contours.size());

    for (auto contour : contours) {
      auto constraints = AddContourConstraint(tin, contour, MAX_SEGMENT_SIZE);

      for (auto constraint : constraints) {
        this->paths.insert(
            NormalizeSegmentOrder(constraint.first, constraint.second));
      }
    }
  }

  TSR_LOG_TRACE("Total Paths: {}", paths.size());
}

bool PathFeature::Calculate(TsrState &state) {

  const Point3 source_point = state.current_vertex->point();
  const Point3 target_point = state.next_vertex->point();

  const std::pair<Point3, Point3> segment =
      NormalizeSegmentOrder(source_point, target_point);

  if (this->paths.contains(segment)) {
    return true;
  } else {
    return false;
  }
}

} // namespace tsr