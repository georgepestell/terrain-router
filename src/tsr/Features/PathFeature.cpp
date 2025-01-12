#include "tsr/Features/PathFeature.hpp"
#include "tsr/ChunkInfo.hpp"
#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/IO/ChunkCache.hpp"
#include "tsr/IO/JSONParser.hpp"
#include "tsr/Logging.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"
#include <cstddef>
#include <exception>
#include <gdal.h>
#include <string>
#include <utility>
#include <vector>

namespace tsr {

std::size_t EdgeHash::operator()(const std::pair<Point2, Point2> &edge) const {
  std::size_t seed = 0;
  boost::hash_combine(seed, edge.first);
  boost::hash_combine(seed, edge.second);
  return seed;
}

std::string PathFeature::URL =
    "https://lz4.overpass-api.de/api/"
    "interpreter?data=%5Bout%3Axml%5D%5Btimeout%3A25%5D%3B%28way%5B%"
    "22highway%22~%22footway%7Cpath%7Cpedestrian%7Ctrack%7Ccycleway%22%5D%"
    "28{},{},{},{}%29%3Bway%5B%22highway%22%5D%5B%22foot%22~%22yes%"
    "7Cdesignated%"
    "7Cpermissive%7Cno%22%5D%28{},{},{},{}%29%3Bway%5B%22highway%22%5D%5B%"
    "22access%"
    "22~%22yes%7Cdesignated%7Cpermissive%22%5D%28{},{},{},{}%29%3Bway%5B%"
    "22sidewalk%"
    "22~%22yes%7Cboth%7Cleft%7Cright%22%5D%28{},{},{},{}%29%3Bway%5B%"
    "22bridge%22%5D%"
    "5B%22sidewalk%22~%22yes%7Cboth%7Cleft%7Cright%22%5D%28{},{},{},{}%29%"
    "3Bway%5B%"
    "22bridge%22%5D%5B%22passenger_lines%22%5D%28if%3At%5B%22passenger_lines%"
    "22%5D%20%3E%200%29%28{},{},{},{}%29%3B%29%3Bout+body%3B%0A{}";

std::pair<Point2, Point2> PathFeature::NormalizeSegmentOrder(Point2 p1, Point2 p2) {

  if (p1 < p2) {
    return {p2, p1};
  } else {
    return {p1, p2};
  }
}

void PathFeature::Initialize(Tin &tin, const MeshBoundary &boundary) {
  auto chunks = chunkManager.GetRequiredChunks(boundary);
  const double MAX_SEGMENT_SIZE = 22;

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
      GDALReleaseDataset(data.dataset);

      contours = IO::LoadContoursFromJsonFile(data.filename, "features");

      // Cache Contours
      try {
        IO::CacheChunk(this->feature_id, chunk, contours);
      } catch (std::exception e) {
        TSR_LOG_WARN("failed to cache path contours");
      }
    }

    for (auto contour : contours) {
      AddContourConstraint(tin, contour, MAX_SEGMENT_SIZE);

      for (unsigned int i = 0; i < contour.size() - 1; i++) {
        this->paths.insert(NormalizeSegmentOrder(contour[i], contour[i + 1]));
      }
    }
  }
}

bool PathFeature::Calculate(TsrState &state) {

  const Point3 source_point = state.current_vertex->point();
  const Point3 target_point = state.next_vertex->point();

  const std::pair<Point2, Point2> segment =
      NormalizeSegmentOrder(Point2(source_point.x(), source_point.y()),
                       Point2(target_point.x(), target_point.y()));

  return this->paths.contains(segment);
}

} // namespace tsr