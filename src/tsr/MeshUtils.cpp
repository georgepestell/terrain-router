#include "tsr/MeshUtils.hpp"
#include "tsr/API/GDALHandler.hpp"
#include "tsr/ChunkInfo.hpp"
#include "tsr/Chunker.hpp"
#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/IO/ChunkCache.hpp"
#include "tsr/MapUtils.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Point_3.hpp"

#include <boost/filesystem.hpp>

#include "tsr/IO/MapIO.hpp"
#include "tsr/MeshBoundary.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Kernel/global_functions_3.h>
#include <CGAL/Vector_3.h>

#include <boost/container/vector.hpp>
#include <gdal.h>
#include <gdal_priv.h>
#include <memory>

#include <tbb/concurrent_vector.h>
#include <tbb/flow_graph.h>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

#include "tsr/logging.hpp"

namespace tsr {

#define CDT_ID "cdt"
#define DEM_URL_FORMAT                                                         \
  "https://portal.opentopography.org/API/"                                     \
  "globaldem?demtype=COP30&south={}&west={}&north="                            \
  "{}&east={}&outputFormat=GeoTiff&API_Key={}"

double interpolate_z(const Point_3 &p1, const Point_3 &p2, const Point_3 &p3,
                     const double x, const double y) {
  // Compute two edge vectors

  typedef CGAL::Vector_3<CGAL::Exact_predicates_inexact_constructions_kernel>
      Vector_3;

  Vector_3 v1 = p2 - p1;
  Vector_3 v2 = p3 - p1;

  Vector_3 normal = CGAL::cross_product(v1, v2);

  // The plane equation is A * x + B * y + C * z + D = 0
  // Where (A, B, C) is the normal vector
  double A = normal[0];
  double B = normal[1];
  double C = normal[2];

  // Compute D using one of the triangle vertices (e.g., p1)
  double D = -(A * p1[0] + B * p1[1] + C * p1[2]);

  // Now we can solve for z: z = (-A * x - B * y - D) / C
  return (-A * x - B * y - D) / C;
}

void mergeBoundedPointsFromCDT(MeshBoundary &boundary, const Delaunay_3 &srcCDT,
                               Delaunay_3 &dstCDT) {
  for (auto vertex = srcCDT.all_vertices_begin();
       vertex != srcCDT.all_vertices_end(); ++vertex) {

    // Skip infinite vertices
    if (srcCDT.is_infinite(vertex)) {
      continue;
    }

    Point_3 point = vertex->point();
    if (boundary.isBounded(point)) {
      dstCDT.insert(point);
    }
  }
}

Delaunay_3 initializeMesh(MeshBoundary boundary, std::string api_key) {

  // TODO: get the tiles required
  std::vector<ChunkInfo> apiTilesRequired;
  std::vector<ChunkInfo> cachedTilesRequired;

  const double TILE_SIZE = 0.1;

  TSR_LOG_TRACE("initializing DEM Chunker");
  Chunker chunker(DEM_URL_FORMAT, TILE_SIZE, {0, 1, 2, 3}, api_key);

  TSR_LOG_TRACE("getting required DEM chunks");
  auto chunks = chunker.getRequiredChunks(boundary);

  // Loop over the x chunks
  TSR_LOG_TRACE("checking chunk cache");
  for (auto chunk : chunks) {
    if (IO::isChunkCached(CDT_ID, chunk)) {
      cachedTilesRequired.push_back(chunk);
    } else {
      apiTilesRequired.push_back(chunk);
    }
  }

  TSR_LOG_TRACE("DEM api tiles: {}", apiTilesRequired.size());
  TSR_LOG_TRACE("DEM cache tiles: {}", cachedTilesRequired.size());

  /*
   * Try to fetch all of the meshes from the cache
   *
   */

  TSR_LOG_TRACE("Fetching chunks from cache");
  Delaunay_3 masterCDT;
  for (auto chunk : cachedTilesRequired) {
    // Add mesh points in domain to master mesh
    try {
      Delaunay_3 chunkMesh;
      IO::getChunkFromCache<Delaunay_3>(CDT_ID, chunk, chunkMesh);

      TSR_LOG_TRACE("merging chunk {} {} {} {}", chunk.minLat, chunk.minLng,
                    chunk.maxLat, chunk.maxLng);
      mergeBoundedPointsFromCDT(boundary, chunkMesh, masterCDT);
    } catch (std::exception e) {
      TSR_LOG_ERROR("Cached chunk corrupted");
      TSR_LOG_ERROR("{}", e.what());

      // Delete the corrupted file
      IO::deleteCachedChunk(CDT_ID, chunk);

      // Add the chunk to the API required tiles
      apiTilesRequired.push_back(chunk);
    }
  }

  TSR_LOG_TRACE("master CDT vertices: {}", masterCDT.number_of_vertices());

  /*
   * Try to fetch and generate the required meshes from the API
   *
   */

  tbb::flow::graph flowGraph;

  // Step 1: Fetch data from the API

  struct ParallelChunkData {
    ChunkInfo chunkInfo;
    std::vector<Point_3> points;
    GDALDatasetH dataset;
    std::shared_ptr<Delaunay_3> cdt;
  };

  tbb::flow::input_node<ParallelChunkData> input_node(
      flowGraph,
      [&apiTilesRequired](tbb::flow_control &fc) -> ParallelChunkData {
        // TSR_LOG_TRACE("Input node");
        static unsigned int count = 0;
        if (count < apiTilesRequired.size()) {
          ParallelChunkData data;
          data.chunkInfo = apiTilesRequired[count];
          ++count;
          return data;
        } else {
          fc.stop();
        }
        return {};
      });

  tbb::flow::function_node<ParallelChunkData, ParallelChunkData> api_node(
      flowGraph, tbb::flow::unlimited,
      [&chunker](ParallelChunkData data) -> ParallelChunkData {
        // TSR_LOG_TRACE("API Node");
        try {
          auto response = chunker.fetchRasterChunk(data.chunkInfo);
          data.dataset = response.dataset;
          boost::filesystem::remove(response.filename);
        } catch (std::exception e) {
          TSR_LOG_ERROR("{}", e.what());
          throw e;
        }
        return data;
      });

  // Step 2: Extract points from the datasets
  tbb::flow::function_node<ParallelChunkData, ParallelChunkData>
      point_extractor_node(
          flowGraph, tbb::flow::unlimited,
          [](ParallelChunkData data) -> ParallelChunkData {
            //  TSR_LOG_TRACE("Point Extractor Node");
            try {
              data.points = IO::extractPointsFromGDALDataset(data.dataset, 1);
            } catch (std::exception e) {
              TSR_LOG_ERROR("failed extracting DTM points from dataset");
              TSR_LOG_TRACE("{}", e.what());
              GDALReleaseDataset(data.dataset);
              throw e;
            }

            GDALReleaseDataset(data.dataset);

            try {
              simplify_points(data.points);
            } catch (std::exception e) {
              TSR_LOG_ERROR("failed to simplify points");
              TSR_LOG_TRACE("{}", e.what());
              throw e;
            }

            return data;
          });

  tbb::flow::function_node<ParallelChunkData, ParallelChunkData>
      delaunay_triangulation_node(
          flowGraph, tbb::flow::unlimited,
          [](ParallelChunkData data) -> ParallelChunkData {
            // TSR_LOG_TRACE("CDT Node");
            try {
              data.cdt = std::make_shared<Delaunay_3>(
                  create_tin_from_points(data.points));
            } catch (std::exception e) {
              TSR_LOG_ERROR("{}", e.what());
              throw e;
            }
            data.points = {};
            return data;
          });

  tbb::flow::function_node<ParallelChunkData, ParallelChunkData> simplify_node(
      flowGraph, tbb::flow::unlimited,
      [](ParallelChunkData data) -> ParallelChunkData {
        try {
          simplify_tin(*data.cdt, *data.cdt);
        } catch (std::exception e) {
          TSR_LOG_ERROR("failed to simplify mesh");
          TSR_LOG_TRACE("{}", e.what());
          throw e;
        }
        return data;
      });

  tbb::flow::function_node<ParallelChunkData, ParallelChunkData> cache_node(
      flowGraph, tbb::flow::unlimited,
      [](ParallelChunkData data) -> ParallelChunkData {
        // TSR_LOG_TRACE("Cache Node");
        try {
          IO::cacheChunk(CDT_ID, data.chunkInfo, *data.cdt);
        } catch (std::exception e) {
          TSR_LOG_ERROR("caching chunk failed");
          TSR_LOG_TRACE("{}", e.what());
        }
        return data;
      });

  tbb::flow::function_node<ParallelChunkData> collect_node(
      flowGraph, tbb::flow::serial,
      [&boundary, &masterCDT](ParallelChunkData data) {
        // TSR_LOG_TRACE("Collector Node");
        mergeBoundedPointsFromCDT(boundary, *data.cdt, masterCDT);
      });

  tbb::flow::make_edge(input_node, api_node);
  tbb::flow::make_edge(api_node, point_extractor_node);
  tbb::flow::make_edge(point_extractor_node, delaunay_triangulation_node);
  tbb::flow::make_edge(delaunay_triangulation_node, simplify_node);
  tbb::flow::make_edge(simplify_node, cache_node);
  tbb::flow::make_edge(cache_node, collect_node);

  if (apiTilesRequired.size() > 0) {
    try {
      input_node.activate();
      flowGraph.wait_for_all();
    } catch (std::exception e) {
      TSR_LOG_ERROR("{}", e.what());
      flowGraph.cancel();
      flowGraph.wait_for_all();
      throw e;
    }
  }

  TSR_LOG_TRACE("master CDT vertices: {}", masterCDT.number_of_vertices());
  return masterCDT;
}

} // namespace tsr