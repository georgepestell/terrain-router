#pragma once
#include <string>

#include "tsr/ChunkManager.hpp"
#include "tsr/Feature.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"

namespace tsr {

template <typename DataType> class APIFeature : public Feature<DataType> {
public:
  /// Raster API
  ChunkManager chunker;

  APIFeature(std::string name, std::string url, double tile_size,
             std::vector<int> position_order, std::string api_key)
      : Feature<DataType>(name),
        chunker(ChunkManager(url, tile_size, position_order, api_key)) {};

  APIFeature(std::string name, std::string url, double tile_size,
             std::vector<int> position_order)
      : APIFeature(name, url, tile_size, position_order, "") {}

  /// Often used to add contours to the tin and cache API data
  virtual void initialize(Tin &tin, const MeshBoundary &boundary) {};

  /// Called once the tin is fully preparaed
  virtual void tag(const Tin &tin) {};

  virtual DataType calculate(TsrState &state) override = 0;
};

} // namespace tsr