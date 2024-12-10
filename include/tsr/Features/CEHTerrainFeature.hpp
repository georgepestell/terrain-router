#include "tsr/Feature.hpp"
#include "tsr/logging.hpp"

#include "tsr/Point_3.hpp"

#include <gdal/gdal_priv.h>

#include <random>

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

class CEHTerrainFeature : public Feature<double> {
private:
  inline static std::map<uint32_t, CEH_TERRAIN_TYPE> TERRAIN_COLOURS = {
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

  std::unordered_map<Face_handle, CEH_TERRAIN_TYPE> terrain_map;

  static CEH_TERRAIN_TYPE
  interpretCEHTerrainColour(std::vector<double> colourValues) {

    uint32_t colour = (int)colourValues[0] << 16 | (int)colourValues[1] << 8 |
                      (int)colourValues[2];

    if (TERRAIN_COLOURS.contains(colour)) {
      return TERRAIN_COLOURS[colour];
    } else {
      TSR_LOG_WARN("terrain colour not recognized ({})", colour);
      return CEH_TERRAIN_TYPE::NO_DATA;
    }
  }

  static std::vector<double> getColourAtPoint(GDALDataset *dataset, int x,
                                              int y) {
    int rasterXSize = dataset->GetRasterXSize();
    int rasterYSize = dataset->GetRasterYSize();

    if (x < 0 || x >= rasterXSize || y < 0 || y >= rasterYSize) {
      TSR_LOG_ERROR("point outside raster dataset {} {}", x, y);
      return std::vector<double>({255, 255, 255});
    }

    std::vector<double> colourValues(3);

    for (int b = 1; b <= 3; b++) {
      GDALRasterBand *band = dataset->GetRasterBand(b);
      if (band == nullptr) {
        TSR_LOG_ERROR("CEH terrain data band not found");
        throw std::runtime_error("CEH terrain data band not found");
      }

      if (band->RasterIO(GF_Read, x, y, 1, 1, &colourValues[b - 1], 1, 1,
                         GDT_Byte, 0, 0) != CE_None) {
        TSR_LOG_ERROR("failed to get terrain value");
        throw std::runtime_error("failed to get terrain value");
      }
    }

    return colourValues;
  }

public:
  CEHTerrainFeature(std::string name, Delaunay_3 &dtm,
                    GDALDatasetH &terrainData)
      : Feature(name) {
    TSR_LOG_TRACE("Setting up CEH terrain type feautre");

    // Fetch the dataset pointer from the handle
    GDALDataset *dataset = static_cast<GDALDataset *>(terrainData);
    if (dataset == nullptr) {
      TSR_LOG_ERROR("CEH terrain type dataset empty");
      throw std::runtime_error("CEH terrain type dataset empty");
    }

    // Tag each face with a terrain feature
    double geotransform[6];
    if (dataset->GetGeoTransform(geotransform) != CE_None) {
      TSR_LOG_ERROR("failed to get terrain type GeoTransform");
      throw std::runtime_error("failed to get terrain type GeoTransform");
    }

    // Ensure there are RGB bands
    TSR_LOG_TRACE("Checking feature dataset RGB");
    int bandCount = dataset->GetRasterCount();
    if (bandCount < 3) {
      TSR_LOG_ERROR("CEH terrain dataset not RGB");
      throw std::runtime_error("CEH terrain dataset not RGB");
    }

    for (Face_handle face : dtm.all_face_handles()) {
      auto p0 = face->vertex(0)->point();
      auto p1 = face->vertex(1)->point();
      auto p2 = face->vertex(2)->point();

      Point_3 center = CGAL::circumcenter(p0, p1, p2);

      int pixel_x =
          static_cast<int>((center.x() - geotransform[0]) / geotransform[1]);
      int pixel_y =
          static_cast<int>((center.y() - geotransform[3]) / geotransform[5]);

      auto colourValues = getColourAtPoint(dataset, pixel_x, pixel_y);

      this->terrain_map[face] = interpretCEHTerrainColour(colourValues);
    }
  }

  double calculate(Face_handle face, Point_3 &source_point,
                   Point_3 &target_point) override {

    CEH_TERRAIN_TYPE type;

    if (this->terrain_map.contains(face)) {
      type = this->terrain_map[face];
    } else {
      type = CEH_TERRAIN_TYPE::NO_DATA;
    }

    switch (type) {
    case BROADLEAVED_MIXED_AND_YEW_WOODLAND | CONIFEROUS_WOODLAND:
      return 0.4;
    case ARABLE_AND_HORTICULTURE:
      return 0.7;
    case IMRPOVED_GRASSLAND:
      return 0.85;
    case NEUTRAL_GRASSLAND | ACID_GRASSLAND | CALCAREOUS_GRASSLAND:
      return 0.90;
    case HEATHER:
      return 0.2;
    case HEATHER_GRASSLAND:
      return 0.35;
    case SALTMARSH | FEN_MARSH_AND_SWAMP:
      return 0;
    case BOG:
      return 0;
    default:
      return 1;
    }
  }
};

} // namespace tsr