#include "tsr/Features/CEHTerrainFeature.hpp"
#include "tsr/API/GDALHandler.hpp"
#include "tsr/ChunkInfo.hpp"
#include "tsr/DataFile.hpp"
#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/IO/ChunkCache.hpp"
#include "tsr/Logging.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"

#include <CGAL/Kernel/global_functions_3.h>
#include <cpl_error.h>
#include <cstdint>
#include <exception>
#include <gdal/gdal.h>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace tsr {

std::string CEHTerrainFeature::URL =
    "https://catalogue.ceh.ac.uk/maps/"
    "4728dd2d-064f-4532-be85-ecafc283bdcf?language=eng&SERVICE=WMS&"
    "VERSION=1.3.0&REQUEST=GetMap&CRS=CRS:84&LAYERS=LC.10m.GB&STYLE="
    "default&BBOX={},{},{},{}&FORMAT=image/"
    "tiff&WIDTH=2048&HEIGHT=2048{}";

std::map<uint32_t, CEH_TERRAIN_TYPE> CEHTerrainFeature::TERRAIN_COLOURS = {
    {0xFF0000, BROADLEAVED_MIXED_AND_YEW_WOODLAND},
    {0x006600, CONIFEROUS_WOODLAND},
    {0x732600, ARABLE_AND_HORTICULTURE},
    {0x00FF00, IMRPOVED_GRASSLAND},
    {0x7FE57F, NEUTRAL_GRASSLAND},
    {0x70A800, CALCAREOUS_GRASSLAND},
    {0x998100, ACID_GRASSLAND},
    {0xFFFF00, FEN_MARSH_AND_SWAMP},
    {0x801A80, HEATHER},
    {0xE68CA6, HEATHER_GRASSLAND},
    {0x008073, BOG},
    {0xD2D2FF, INLAND_ROCK},
    {0xCCB300, SUPRALITTORAL_ROCK_AND_SEDIMENT},
    {0xFFFF80, LITTORAL_ROCK_AND_SEDIMENT},
    {0x8080FF, SALTMARSH},
    {0x000000, URBAN},
    {0x808080, SUBURBAN},
    {0xFFFFFF, NO_DATA},
};

CEH_TERRAIN_TYPE
CEHTerrainFeature::interpretCEHTerrainColour(std::vector<double> colourValues) {

  uint32_t colour = (int)colourValues[0] << 16 | (int)colourValues[1] << 8 |
                    (int)colourValues[2];

  if (TERRAIN_COLOURS.contains(colour)) {
    return TERRAIN_COLOURS[colour];
  } else {
    TSR_LOG_WARN("terrain colour not recognized ({})", colour);
    return CEH_TERRAIN_TYPE::NO_DATA;
  }
}

std::vector<double> CEHTerrainFeature::GetPixelColour(GDALDatasetH dataset,
                                                      int x, int y) {
  int rasterXSize = GDALGetRasterXSize(dataset);
  int rasterYSize = GDALGetRasterYSize(dataset);

  if (x < 0 || x >= rasterXSize || y < 0 || y >= rasterYSize) {
    TSR_LOG_ERROR("point outside raster dataset {} {}", x, y);
    return std::vector<double>({255, 255, 255});
  }

  std::vector<double> colourValues(3);

  for (int b = 1; b <= 3; b++) {
    GDALRasterBandH band = GDALGetRasterBand(dataset, b);
    if (band == nullptr) {
      TSR_LOG_ERROR("CEH terrain data band not found");
      throw std::runtime_error("CEH terrain data band not found");
    }

    if (GDALRasterIO(band, GF_Read, x, y, 1, 1, &colourValues[b - 1], 1, 1,
                     GDT_Byte, 0, 0) != CE_None) {
      TSR_LOG_ERROR("failed to get terrain value");
      throw std::runtime_error("failed to get terrain value");
    }
  }

  return colourValues;
}

void CEHTerrainFeature::Initialize(Tin &tin, const MeshBoundary &boundary) {
  TSR_LOG_TRACE("initializing {}", this->feature_id);

  // Fetch the data from the api
  auto chunks = chunkManager.GetRequiredChunks(boundary);

  std::string dataCacheID = this->feature_id + "/data";
  std::string contourCacheID = this->feature_id + "/contours";

  TSR_LOG_TRACE("fetching contours");
  std::vector<std::vector<Point2>> contours;

  const double MAX_SEGMENT_SIZE = 22.0;

  for (auto chunk : chunks) {

    std::vector<std::vector<Point2>> contours;
    if (chunkManager.IsAvailableInCache(contourCacheID, chunk)) {
      // TODO: Add contours from cache
      IO::GetChunkFromCache<std::vector<std::vector<Point2>>>(contourCacheID,
                                                              chunk, contours);
    } else {

      // Fetch the dataset from either the cache or API
      DataFile data(nullptr, "");
      if (IO::IsChunkCached(dataCacheID, chunk)) {

        data.filename = IO::GetChunkFilepath(dataCacheID, chunk);
        IO::GetChunkFromCache<GDALDatasetH>(dataCacheID, chunk, data.dataset);

      } else {

        // Fech chunk from API
        TSR_LOG_TRACE("fetching chunk from API");
        data = chunkManager.FetchRasterChunk(chunk);

        // Cache dataset
        TSR_LOG_TRACE("caching chunk");
        try {
          IO::CacheChunk(dataCacheID, chunk, data.dataset);
        } catch (std::exception e) {
          TSR_LOG_ERROR("failed to cache data");
          throw e;
        }
      }

      // Release dataset

      double adfGeotransform[6];
      GDALGetGeoTransform(data.dataset, adfGeotransform);
      GDALReleaseDataset(data.dataset);

      contours =
          API::ExtractFeatureContours(data.filename, adfGeotransform, 0.5);

      // TODO: Cache contours
      TSR_LOG_TRACE("Caching contours");
      try {
        IO::CacheChunk(contourCacheID, chunk, contours);
      } catch (std::exception e) {
        TSR_LOG_WARN("failed to cache CEH contours");
      }
    }

    TSR_LOG_TRACE("CEH Contours: {}", contours.size());

    // Add contours to the mesh
    for (auto contour : contours) {
      AddContourConstraint(tin, contour, MAX_SEGMENT_SIZE);
    }
  }
};

void CEHTerrainFeature::Tag(const Tin &tin) {
  TSR_LOG_TRACE("Tagging CEH terrain type feature");

  std::string dataCacheID = this->feature_id + "/data";

  GDALDatasetH dataset = nullptr;
  ChunkInfo dataset_chunk;
  for (Face_handle face : tin.all_face_handles()) {

    if (tin.is_infinite(face)) {
      continue;
    }

    auto p0 = face->vertex(0)->point();
    auto p1 = face->vertex(1)->point();
    auto p2 = face->vertex(2)->point();

    Point3 center = CGAL::circumcenter(p0, p1, p2);

    Point3 centerWGS84;
    try {
      centerWGS84 = TranslateUtmPointToWgs84(center, 30, true);
    } catch (std::exception e) {
      continue;
    }

    ChunkInfo chunk =
        chunkManager.GetChunkInfo(centerWGS84.x(), centerWGS84.y());

    if (chunk != dataset_chunk && IO::IsChunkCached(dataCacheID, chunk)) {

      if (dataset != nullptr) {
        GDALReleaseDataset(dataset);
      }

      IO::GetChunkFromCache<GDALDatasetH>(dataCacheID, chunk, dataset);
      dataset_chunk = chunk;
    } else {
      continue;
    }

    if (dataset == nullptr) {
      TSR_LOG_ERROR("CEH terrain type dataset empty");
      throw std::runtime_error("CEH terrain type dataset empty");
    }

    // Ensure there are RGB bands
    // TSR_LOG_TRACE("Checking feature dataset RGB");
    int bandCount = GDALGetRasterCount(dataset);
    if (bandCount < 3) {
      TSR_LOG_ERROR("CEH terrain dataset not RGB");
      throw std::runtime_error("CEH terrain dataset not RGB");
    }

    // TSR_LOG_TRACE("getting dataset information");

    // Tag each face with a terrain feature
    double geotransform[6];
    if (GDALGetGeoTransform(dataset, geotransform) != CE_None) {
      TSR_LOG_ERROR("failed to get terrain type GeoTransform");
      throw std::runtime_error("failed to get terrain type GeoTransform");
    }

    int pixel_x =
        static_cast<int>((center.x() - geotransform[0]) / geotransform[1]);
    int pixel_y =
        static_cast<int>((center.y() - geotransform[3]) / geotransform[5]);

    // TSR_LOG_TRACE("getting colour value");
    auto colourValues = GetPixelColour(dataset, pixel_x, pixel_y);

    // TSR_LOG_TRACE("interpreting and setting colour value");
    this->terrain_map[face] = interpretCEHTerrainColour(colourValues);
  }

  if (dataset != nullptr) {
    GDALReleaseDataset(dataset);
  }
}

double CEHTerrainFeature::Calculate(TsrState &state) {

  CEH_TERRAIN_TYPE type;

  if (this->terrain_map.contains(state.current_face)) {
    type = this->terrain_map[state.current_face];
  } else {
    type = CEH_TERRAIN_TYPE::NO_DATA;
  }

  switch (type) {
  case BROADLEAVED_MIXED_AND_YEW_WOODLAND:
  case CONIFEROUS_WOODLAND:
    AddWarning(state, "Woodland", 3);
    return 0.4;
  case ARABLE_AND_HORTICULTURE:
    return 0.7;
  case IMRPOVED_GRASSLAND:
    return 0.85;
  case NEUTRAL_GRASSLAND:
  case ACID_GRASSLAND:
  case CALCAREOUS_GRASSLAND:
    return 0.85;
  case HEATHER:
    AddWarning(state, "Heather", 10);
    return 0;
  case HEATHER_GRASSLAND:
    AddWarning(state, "Potential heather", 7);
    return 0.35;
  case SALTMARSH:
  case FEN_MARSH_AND_SWAMP:
    AddWarning(state, "Saltmarsh / Fen / Marsh / Swamp", 10);
    return 0;
  case BOG:
    AddWarning(state, "Bog", 10);
    return 0;
  case URBAN:
  case SUBURBAN:
    return 1;
  default:
    return 0.80;
  }
}

} // namespace tsr