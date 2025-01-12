#pragma once

#include "tsr/Features/APIFeature.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Tin.hpp"

#include "tsr/TsrState.hpp"

#include <string>
#include <unordered_map>

namespace tsr {

class BoolWaterFeature : public APIFeature<bool> {

private:
  enum DEPENDENCIES { DISTANCE };

  static std::string URL;
  inline static int NODATA_VALUE = -9999;
  enum WATER_STATUS { NODATA, WATER, LAND };

  std::unordered_map<Face_handle, WATER_STATUS> waterMap;

public:
  BoolWaterFeature(std::string name, double tile_size)
      : APIFeature(name, URL, tile_size, {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
                                          0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3}) {
        };

  void Initialize(Tin &tin, const MeshBoundary &boundary) override;

  void Tag(const Tin &tin) override;

  bool Calculate(TsrState &state) override;

  void writeWaterMapToKML();
};

} // namespace tsr