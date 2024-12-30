#pragma once
#include <string>

#include "tsr/Chunker.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/TSRState.hpp"

namespace tsr {

template <typename DataType> class APIFeature : public Feature<DataType> {
public:
  /// Raster API
  Chunker chunker;

  APIFeature(std::string name, std::string url, double tile_size,
             std::vector<int> position_order, std::string api_key)
      : Feature<DataType>(name),
        chunker(Chunker(url, tile_size, position_order, api_key)) {};

  APIFeature(std::string name, std::string url, double tile_size,
             std::vector<int> position_order)
      : APIFeature(name, url, tile_size, position_order, "") {}

  /// Often used to add contours to the cdt and cache API data
  virtual void initialize(Delaunay_3 &cdt, const MeshBoundary &boundary) {};

  /// Called once the cdt is fully preparaed
  virtual void tag(const Delaunay_3 &cdt) {};

  virtual DataType calculate(TSRState &state) override = 0;
};

} // namespace tsr