#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "tsr/ChunkInfo.hpp"
#include "tsr/DataFile.hpp"
#include "tsr/MeshBoundary.hpp"

#include <gdal/gdal.h>

namespace tsr {

class Chunker {

  /// The API url format that will be inserted with the lat,lng and API key
  std::string url;

  /// Size of tiles in lat/lng degrees
  double tile_size;

  /// Optional API key
  std::string api_key;

  std::vector<int> position_order;

public:
  /// Base constructor
  Chunker(std::string url, double tile_size, std::vector<int> position_order,
          std::string api_key)
      : url(url), tile_size(tile_size), api_key(api_key),
        position_order(position_order) {}

  /// Constuctor enabling APIs with no API key to be initialized
  Chunker(std::string url, double tile_size, std::vector<int> position_order)
      : Chunker(url, tile_size, position_order, NULL) {}

  /// Fetch vector data from the API and automatically warp it to UTM
  DataFile fetchVectorChunk(const ChunkInfo &chunk) const;

  DataFile fetchVectorChunkAndRasterize(const ChunkInfo &chunk,
                                        double pixel_resolution) const;

  /// Fetch raster data from the API and automatically warp it to UTM
  DataFile fetchRasterChunk(const ChunkInfo &chunk) const;

  /// Converts a chunk to the relevant API address
  std::string formatURL(const ChunkInfo &chunk) const;

  /// Gets the chunk bounding a particular latitude/longitude
  ChunkInfo getChunkInfo(double lat, double lng) const;

  // Gets the list of chunks required to cover an area
  std::vector<ChunkInfo> getRequiredChunks(const MeshBoundary &boundary) const;
};

} // namespace tsr