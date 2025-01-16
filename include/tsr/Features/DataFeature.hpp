#pragma once

#include <string>
#include <vector>

#include "tsr/ChunkManager.hpp"
#include "tsr/Feature.hpp"
#include "tsr/TsrState.hpp"

namespace tsr {

template <typename DataType> class DataFeature : public Feature<DataType> {
public:
  /// Raster API
  ChunkManager chunkManager;

  DataFeature(std::string name, std::string url, double tile_size,
              std::vector<int> position_order, std::string api_key)
      : Feature<DataType>(name),
        chunkManager(ChunkManager(url, tile_size, position_order, api_key)) {};

  DataFeature(std::string name, std::string url, double tile_size,
              std::vector<int> position_order)
      : DataFeature(name, url, tile_size, position_order, "") {}

  virtual DataType Calculate(TsrState &state) override = 0;
};

} // namespace tsr