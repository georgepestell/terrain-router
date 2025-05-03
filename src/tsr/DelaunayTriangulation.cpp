#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/GeometryUtils.hpp"
#include "tsr/Logging.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"
#include "tsr/SurfaceMesh.hpp"
#include "tsr/Tin.hpp"

#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Shape_detection/Region_growing/Region_growing.h>
#include <CGAL/Polygon_mesh_processing/remesh_planar_patches.h>
#include <CGAL/Polygon_mesh_processing/repair.h>
#include <CGAL/Polygon_mesh_processing/repair_degeneracies.h>
#include <CGAL/Polygon_mesh_processing/repair_polygon_soup.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/boost/graph/copy_face_graph.h>
#include <CGAL/boost/graph/graph_traits_Constrained_Delaunay_triangulation_2.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/property_map.h>
#include <CGAL/tags.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/property_map/vector_property_map.hpp>

#include <cmath>
#include <exception>
#include <gdal.h>
#include <iterator>

#include <memory>
#include <oneapi/tbb/flow_graph.h>
#include <set>
#include <string>
#include <tbb/flow_graph.h>
#include <tbb/parallel_for.h>

#include "tsr/ChunkInfo.hpp"
#include "tsr/ChunkManager.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/PointProcessor.hpp"

#include "tsr/IO/ChunkCache.hpp"
#include "tsr/IO/MapIO.hpp"

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace tsr {

#define TIN_FEATURE_ID "topography"

void ConvertSurfaceMeshToTin(SurfaceMesh const &source, Tin &target) {
  for (auto v : source.vertices()) {
    target.insert(source.point(v));
  }
}

void ConvertTinToSurfaceMesh(Tin const &source, SurfaceMesh &target) {
  CGAL::copy_face_graph(source, target);
}

Tin CreateTinFromPoints(const std::vector<Point3> &points) {
  if (points.empty()) {
    TSR_LOG_WARN("empty Tin created");
  }

  Tin tin;

  for (auto p : points) {
    if (p.x() == 0 && p.y() == 0 && p.z() == 0) {
      TSR_LOG_TRACE("point is zero zero");
    }
    tin.insert(p);
  }

  // tin.insert(points.begin(), points.end());

  return tin;
}

std::set<std::pair<Point3, Point3>>
AddContourConstraint(Tin &tin, std::vector<Point2> contour,
                     double max_segment_length) {

  std::set<std::pair<Point3, Point3>> constraints;

  for (auto vertexIt = contour.begin(); vertexIt != contour.end(); ++vertexIt) {
    auto vertexNextIt = std::next(vertexIt);
    if (vertexNextIt == contour.end())
      break;

    const double x = vertexIt->x();
    const double y = vertexIt->y();

    Tin::Face_handle vertexFace = tin.locate(Point3(x, y, 0));

    if (vertexFace == nullptr || tin.is_infinite(vertexFace)) {
      // TSR_LOG_WARN("Point outside boundary x: {} y: {}", x, y);
      continue;
    }

    double z = InterpolateZ(vertexFace->vertex(0)->point(),
                            vertexFace->vertex(1)->point(),
                            vertexFace->vertex(2)->point(), x, y);

    Point3 vertex(vertexIt->x(), vertexIt->y(), z);

    const double next_x = vertexNextIt->x();
    const double next_y = vertexNextIt->y();

    Tin::Face_handle vertexNextFace = tin.locate(Point3(next_x, next_y, 0));

    if (vertexNextFace == nullptr || tin.is_infinite(vertexNextFace)) {
      // TSR_LOG_WARN("Point outside boundary x: {} y: {}", next_x, next_y);
      continue;
    }

    double next_z = InterpolateZ(
        vertexNextFace->vertex(0)->point(), vertexNextFace->vertex(1)->point(),
        vertexNextFace->vertex(2)->point(), next_x, next_y);

    Point3 vertexPoint(x, y, z);
    Point3 vertexNextPoint(next_x, next_y, next_z);

    // Calculate the Euclidean distance in the XY plane
    double dx = next_x - x;
    double dy = next_y - y;
    double length = sqrt(dx * dx + dy * dy);

    if (length > max_segment_length) {
      // Calculate the number of splits required
      double splits = floor(length / max_segment_length);

      std::vector<Point3> splitPoints;
      for (double split = 1; split <= splits; split++) {
        double split_x = round(x + (dx / splits) * split);
        double split_y = round(y + (dy / splits) * split);

        Tin::Face_handle vertexSplitFace =
            tin.locate(Point3(split_x, split_y, 0));

        if (tin.is_infinite(vertexSplitFace)) {
          // TSR_LOG_ERROR("Point outside boundary x: {} y: {}", split_x,
          // split_y);
          continue;
        }

        double split_z =
            InterpolateZ(vertexSplitFace->vertex(0)->point(),
                         vertexSplitFace->vertex(1)->point(),
                         vertexSplitFace->vertex(2)->point(), split_x, split_y);
        splitPoints.push_back(Point3(split_x, split_y, split_z));
      }

      // Add the constraints
      constraints.insert({vertexPoint, splitPoints[0]});
      ushort splitIndex;
      for (splitIndex = 1; splitIndex < splitPoints.size(); splitIndex++) {
        constraints.insert(
            {splitPoints[splitIndex - 1], splitPoints[splitIndex]});
      }
      constraints.insert({splitPoints[splitIndex - 1], vertexNextPoint});

    } else {
      constraints.insert({vertexPoint, vertexNextPoint});
    }
  }

  for (auto c : constraints) {
    tin.insert_constraint(c.first, c.second);
  }

  return constraints;
}

void SimplifyTin(Tin const &source_mesh, Tin &target_mesh,
                 float cosine_max_angle_regions, float max_distance_regions,
                 float cosine_max_angle_corners, float max_distance_corners) {

  // Convert the mesh to a Surface_Mesh
  SurfaceMesh sourceSurfaceMesh;
  ConvertTinToSurfaceMesh(source_mesh, sourceSurfaceMesh);

  CGAL::Polygon_mesh_processing::remove_degenerate_faces(sourceSurfaceMesh);
  CGAL::Polygon_mesh_processing::remove_almost_degenerate_faces(
      sourceSurfaceMesh);
  CGAL::Polygon_mesh_processing::remove_degenerate_edges(sourceSurfaceMesh);
  CGAL::Polygon_mesh_processing::triangulate_faces(sourceSurfaceMesh);

  // typedef boost::graph_traits<SurfaceMesh>::vertex_descriptor
  // vertex_descriptor; typedef
  // boost::graph_traits<SurfaceMesh>::halfedge_descriptor halfedge_descriptor;

  if (!CGAL::is_valid_polygon_mesh(sourceSurfaceMesh)) {
    TSR_LOG_ERROR("Source mesh is invalid!");
    return;
  }

  // // Required simplification information
  // std::vector<size_t> regionIdMap(CGAL::num_faces(sourceSurfaceMesh));
  // std::vector<size_t> cornerIdMap(CGAL::num_vertices(sourceSurfaceMesh), -1);
  // std::vector<bool> ecm(CGAL::num_edges(sourceSurfaceMesh), false);
  // boost::vector_property_map<CGAL::Epick::Vector_3> normalMap;

  // TSR_LOG_TRACE("detecting mesh regions for simplification");

  // size_t nbRegions =
  //     CGAL::Polygon_mesh_processing::region_growing_of_planes_on_faces(
  //         sourceSurfaceMesh,
  //         CGAL::make_random_access_property_map(regionIdMap),
  //         CGAL::parameters::cosine_of_maximum_angle(cosine_max_angle_regions)
  //             .region_primitive_map(normalMap)
  //             .maximum_distance(max_distance_regions));
  // TSR_LOG_TRACE("Detected {} regions", nbRegions);

  // TSR_LOG_TRACE("detecting mesh corners for simplification");

  // size_t nbCorners =
  // CGAL::Polygon_mesh_processing::detect_corners_of_regions(
  //     sourceSurfaceMesh, CGAL::make_random_access_property_map(regionIdMap),
  //     nbRegions, CGAL::make_random_access_property_map(cornerIdMap),
  //     CGAL::parameters::cosine_of_maximum_angle(cosine_max_angle_corners)
  //         .maximum_distance(max_distance_corners)
  //         .edge_is_constrained_map(CGAL::make_random_access_property_map(ecm)));
  // TSR_LOG_TRACE("Detected {} corners", nbCorners);

  // SurfaceMesh targetSurfaceMesh;
  // try {
  //   TSR_LOG_TRACE("simplifying mesh");

  //   // typedef boost::property_map<SurfaceMesh, CGAL::vertex_index_t>::type
  //   //     VertexIndexMap;
  //   // VertexIndexMap vim = get(CGAL::vertex_index, sourceSurfaceMesh);

  //   // typedef boost::property_map<SurfaceMesh,
  //   CGAL::face_patch_id_t<int>>::type
  //   //     FacePatchIdMap;
  //   // FacePatchIdMap patch_id_map =
  //   //     get(CGAL::face_patch_id_t<int>(), sourceSurfaceMesh);

  //   // bool res =
  //   CGAL::Polygon_mesh_processing::remesh_almost_planar_patches(
  //   //     sourceSurfaceMesh, targetSurfaceMesh, nbRegions, nbCorners,
  //   //     CGAL::make_random_access_property_map(regionIdMap),
  //   //     CGAL::make_random_access_property_map(cornerIdMap),
  //   //     CGAL::make_random_access_property_map(ecm),
  //   //     CGAL::parameters::patch_normal_map(normalMap));
  //   // if (!res) {
  //   //   throw std::runtime_error("return value indicates failure");
  //   // }
  //   TSR_LOG_TRACE("Simplified mesh has {} vertices",
  //                 sourceSu.number_of_vertices());
  // } catch (std::exception &e) {
  //   TSR_LOG_ERROR("simplification failed: {}", e.what());
  //   throw e;
  // }

  if (sourceSurfaceMesh.number_of_vertices() == 0) {
    TSR_LOG_WARN("Empty target surface mesh");
  }

  // copy the mesh to the target mesh
  // exceptions here result in an undefined memory state
  try {
    TSR_LOG_TRACE("writing to target mesh");
    target_mesh.clear();
    ConvertSurfaceMeshToTin(sourceSurfaceMesh, target_mesh);
  } catch (std::exception &e) {
    TSR_LOG_FATAL("copying simplified mesh to target mesh failed");
    throw e;
  }
}

void SimplifyTin(Tin const &source_mesh, Tin &target_mesh) {
  TSR_LOG_TRACE("calling simplify mesh with defaults");
  return SimplifyTin(source_mesh, target_mesh, DEFAULT_COSINE_MAX_ANGLE_REGIONS,
                     DEFAULT_MAX_DISTANCE_REGIONS,
                     DEFAULT_COSINE_MAX_ANGLE_CORNERS,
                     DEFAULT_MAX_DISTANCE_CORNERS);
}

Tin InitializeTinFromBoundary(MeshBoundary boundary, std::string api_key,
                              std::string url_format) {

  std::vector<ChunkInfo> apiTilesRequired;
  std::vector<ChunkInfo> cachedTilesRequired;

  const double TILE_SIZE = 0.1;

  TSR_LOG_TRACE("initializing DEM ChunkManager");
  ChunkManager chunkManager(url_format, TILE_SIZE, {0, 1, 2, 3}, api_key);

  TSR_LOG_TRACE("getting required DEM chunks");
  auto chunks = chunkManager.GetRequiredChunks(boundary);

  // Loop over the x chunks
  TSR_LOG_TRACE("checking chunk cache");
  for (auto chunk : chunks) {
    if (IO::IsChunkCached(TIN_FEATURE_ID, chunk)) {
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
  Tin masterTIN;
  for (auto chunk : cachedTilesRequired) {
    // Add mesh points in domain to master mesh
    try {
      Tin chunkMesh;
      IO::GetChunkFromCache<Tin>(TIN_FEATURE_ID, chunk, chunkMesh);

      TSR_LOG_TRACE("merging chunk {} {} {} {}", chunk.minLat, chunk.minLng,
                    chunk.maxLat, chunk.maxLng);
      MergeTinPointsInBoundary(boundary, chunkMesh, masterTIN);
    } catch (std::exception &e) {
      TSR_LOG_ERROR("Cached chunk corrupted");
      TSR_LOG_ERROR("{}", e.what());

      // Delete the corrupted file
      IO::DeleteChunkFromCache(TIN_FEATURE_ID, chunk);

      // Add the chunk to the API required tiles
      apiTilesRequired.push_back(chunk);
    }
  }

  TSR_LOG_TRACE("master TIN vertices: {}", masterTIN.number_of_vertices());

  /*
   * Try to fetch and generate the required meshes from the API
   *
   */

  tbb::flow::graph flowGraph;

  // Step 1: Fetch data from the API

  struct ParallelChunkData {
    ChunkInfo chunkInfo;
    std::vector<Point3> points;
    GDALDatasetH dataset;
    std::shared_ptr<Tin> tin;
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
      [&chunkManager](ParallelChunkData data) -> ParallelChunkData {
        // TSR_LOG_TRACE("API node");
        try {
          auto response = chunkManager.FetchRasterChunk(data.chunkInfo);
          data.dataset = response.dataset;
          boost::filesystem::remove(response.filename);
        } catch (std::exception &e) {
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
            //  TSR_LOG_TRACE("Point Extractor node");
            try {
              data.points = IO::ExtractGdalDatasetPoints(data.dataset, 1);
            } catch (std::exception &e) {
              TSR_LOG_ERROR("failed extracting DTM points from dataset");
              TSR_LOG_TRACE("{}", e.what());
              GDALReleaseDataset(data.dataset);
              throw e;
            }

            GDALReleaseDataset(data.dataset);

            try {
              SimplifyPoints(data.points);
            } catch (std::exception &e) {
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
            // TSR_LOG_TRACE("TIN node");
            try {
              data.tin =
                  std::make_shared<Tin>(CreateTinFromPoints(data.points));
            } catch (std::exception &e) {
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
          SimplifyTin(*data.tin, *data.tin);
        } catch (std::exception &e) {
          TSR_LOG_ERROR("failed to simplify mesh");
          TSR_LOG_TRACE("{}", e.what());
          throw e;
        }
        return data;
      });

  tbb::flow::function_node<ParallelChunkData, ParallelChunkData> cache_node(
      flowGraph, tbb::flow::unlimited,
      [](ParallelChunkData data) -> ParallelChunkData {
        // TSR_LOG_TRACE("Cache node");
        try {
          IO::CacheChunk(TIN_FEATURE_ID, data.chunkInfo, *data.tin);
        } catch (std::exception &e) {
          TSR_LOG_ERROR("caching chunk failed");
          TSR_LOG_TRACE("{}", e.what());
        }
        return data;
      });

  tbb::flow::function_node<ParallelChunkData> collect_node(
      flowGraph, tbb::flow::serial,
      [&boundary, &masterTIN](ParallelChunkData data) {
        // TSR_LOG_TRACE("Collector node");
        MergeTinPointsInBoundary(boundary, *data.tin, masterTIN);
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
    } catch (std::exception &e) {
      TSR_LOG_ERROR("{}", e.what());
      flowGraph.cancel();
      flowGraph.wait_for_all();
      throw e;
    }
  }

  TSR_LOG_TRACE("master TIN vertices: {}", masterTIN.number_of_vertices());
  return masterTIN;
}

void MergeTinPointsInBoundary(MeshBoundary &boundary, const Tin &srcTIN,
                              Tin &dstTIN) {
  for (auto vertex = srcTIN.all_vertices_begin();
       vertex != srcTIN.all_vertices_end(); ++vertex) {

    // Skip infinite vertices
    if (srcTIN.is_infinite(vertex)) {
      continue;
    }

    Point3 point = vertex->point();
    if (boundary.IsBounded(point)) {
      dstTIN.insert(point);
    }
  }
}

} // namespace tsr
