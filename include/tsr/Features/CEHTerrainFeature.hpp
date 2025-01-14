#include "tsr/Features/APIFeature.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"

#include <cstdint>
#include <gdal/gdal.h>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace tsr {

enum CEH_TERRAIN_TYPE {
  BROADLEAVED_MIXED_AND_YEW_WOODLAND,
  CONIFEROUS_WOODLAND,
  ARABLE_AND_HORTICULTURE,
  IMRPOVED_GRASSLAND,
  NEUTRAL_GRASSLAND,
  CALCAREOUS_GRASSLAND,
  ACID_GRASSLAND,
  FEN_MARSH_AND_SWAMP,
  HEATHER,
  HEATHER_GRASSLAND,
  BOG,
  INLAND_ROCK,
  SUPRALITTORAL_ROCK_AND_SEDIMENT,
  LITTORAL_ROCK_AND_SEDIMENT,
  SALTMARSH,
  URBAN,
  SUBURBAN,
  NO_DATA
};

class CEHTerrainFeature : public APIFeature<double> {
private:
  static std::map<uint32_t, CEH_TERRAIN_TYPE> TERRAIN_COLOURS;

  std::unordered_map<Face_handle, CEH_TERRAIN_TYPE> terrain_map;

  static CEH_TERRAIN_TYPE
  interpretCEHTerrainColour(std::vector<double> colourValues);

  static std::vector<double> GetPixelColour(GDALDatasetH dataset, int x,
                                              int y);
  static std::string URL;

public:
  CEHTerrainFeature(std::string name, double tile_size)
      : APIFeature(name, URL, tile_size, {1, 0, 3, 2}) {};

  void Initialize(Tin &tin, const MeshBoundary &boundary) override;

  void Tag(const Tin &tin) override;

  double Calculate(TsrState &state) override;
};
} // namespace tsr