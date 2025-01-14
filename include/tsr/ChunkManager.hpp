#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "tsr/ChunkInfo.hpp"
#include "tsr/DataFile.hpp"
#include "tsr/MeshBoundary.hpp"

#include <gdal/gdal.h>

namespace tsr {

class ChunkManager {

  /// The API url format that will be inserted with the lat,lng and API key
  std::string url;

  /// Size of tiles in lat/lng degrees
  double tile_size;

  /// Optional API key
  std::string api_key;

  std::vector<int> position_order;

public:
  /// Base constructor
  ChunkManager(std::string url, double tile_size,
               std::vector<int> position_order, std::string api_key)
      : url(url), tile_size(tile_size), api_key(api_key),
        position_order(position_order) {}

  /// Constuctor enabling APIs with no API key to be initialized
  ChunkManager(std::string url, double tile_size,
               std::vector<int> position_order)
      : ChunkManager(url, tile_size, position_order, NULL) {}

  /// Fetch vector data from the API and automatically warp it to UTM
  DataFile FetchVectorChunk(const ChunkInfo &chunk) const;

  DataFile FetchAndRasterizeVectorChunk(const ChunkInfo &chunk,
                                        double pixel_resolution) const;

  /// Fetch raster data from the API and automatically warp it to UTM
  DataFile FetchRasterChunk(const ChunkInfo &chunk) const;

  /// Converts a chunk to the relevant API address
  std::string FormatUrl(const ChunkInfo &chunk) const;

  /// Gets the chunk bounding a particular latitude/longitude
  ChunkInfo GetChunkInfo(double lat, double lng) const;

  // Gets the list of chunks required to cover an area
  std::vector<ChunkInfo> GetRequiredChunks(const MeshBoundary &boundary) const;

  bool IsAvailableInCache(const std::string &feature_id,
                          const ChunkInfo &chunk);
};

void CacheSetEnabled(bool isEnabled);
bool IsCacheEnabled();

} // namespace tsr