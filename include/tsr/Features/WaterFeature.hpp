#pragma once

#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/Features/APIFeature.hpp"
#include "tsr/IO/ChunkCache.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/IO/MapIO.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point3.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Tin.hpp"

#include "tsr/Logging.hpp"
#include "tsr/TsrState.hpp"

#include <CGAL/Kernel/global_functions_3.h>
#include <boost/concept_check.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "tsr/API/GDALHandler.hpp"

#include "tsr/IO/KMLWriter.hpp"

#include <gdal/gdal_priv.h>

namespace tsr {

class BoolWaterFeature : public APIFeature<bool> {

private:
  enum WATER_STATUS { NODATA, WATER, LAND };

  std::unordered_map<Face_handle, WATER_STATUS> waterMap;

  double max_traversable_distance = 0;

  enum DEPENDENCIES { DISTANCE };

  inline static std::string URL =
      "https://lz4.overpass-api.de/api/interpreter?data="
      "%5Bout%3Axml%5D%5Btimeout%3A25%5D%3B%0A%28node%5B%22natural%22%3D%"
      "22water%22%5D%5B%22type%22%3D%22multipolygon%22%5D%28{},{},{},{}%29%3B%"
      "0Away%5B%22natural%22%3D%22water%22%5D%5B%22type%22%3D%22multipolygon%"
      "22%5D%28{},{},{},{}%29%3B%0Arelation%5B%22natural%22%3D%22water%22%5D%"
      "5B%22type%22%3D%22multipolygon%22%5D%28{},{},{},{}%29%3B%0Anode%5B%"
      "22natural%22%3D%22water%22%5D%28{},{},{},{}%29%3B%0Away%5B%22natural%22%"
      "3D%22water%22%5D%28{},{},{},{}%29%3B%0Arelation%5B%22natural%22%3D%"
      "22water%22%5D%28{},{},{},{}%29%3B%29%3B%0A%28._%3B%3E%3B%29%3Bout+"
      "body%3B{}";

  inline static int NODATA_VALUE = -9999;

public:
  BoolWaterFeature(std::string name, double tile_size)
      : APIFeature(name, URL, tile_size, {0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3,
                                          0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3}) {
        };

  void initialize(Tin &tin, const MeshBoundary &boundary) override {
    // TODO: get datasets from cache/api
    auto chunks = chunker.getRequiredChunks(boundary);

    std::string dataFeatureID = this->feature_id + "/data";
    std::string contourFeatureID = this->feature_id + "/contour";

    const double MAX_SEGMENT_LENGTH = 22;

    // For each chunk, either fetch from the API or cache
    unsigned int total = 0;
    for (auto chunk : chunks) {

      std::vector<std::vector<Point2>> contours;
      if (IO::isChunkCached(contourFeatureID, chunk)) {
        TSR_LOG_TRACE("reading cached contours");

        // TODO: Fetch contours from cache
        IO::getChunkFromCache<std::vector<std::vector<Point2>>>(
            contourFeatureID, chunk, contours);

        TSR_LOG_TRACE("cache chunk contours: {}", contours.size());

      } else {
        TSR_LOG_TRACE("fetching contours from API");

        // Fetch data from api
        auto data = chunker.fetchVectorChunkAndRasterize(chunk, 0.00001);

        // Cache data
        IO::cacheChunk(dataFeatureID, chunk, data.dataset);

        // Fetch the geotransform for the dataset
        double adfGeotransform[6];
        GDALGetGeoTransform(data.dataset, adfGeotransform);

        // Close the dataset to enable the extractor to read
        GDALReleaseDataset(data.dataset);

        TSR_LOG_TRACE("contour file: {}", data.filename);

        // Extract contours usign OpenCV
        const double SIMPLIFICATION_FACTOR = 0.01;
        contours = API::extract_feature_contours(data.filename, adfGeotransform,
                                                 SIMPLIFICATION_FACTOR);

        TSR_LOG_TRACE("chunk contours: {}", contours.size());

        // Cache contours
        IO::cacheChunk(contourFeatureID, chunk, contours);
      }

      // add contours to mesh
      total += contours.size();
      for (auto contour : contours) {
        add_contour_constraint(tin, contour, MAX_SEGMENT_LENGTH);
      }
    }
    TSR_LOG_TRACE("contours: {}", total);
  }

  void tag(const Tin &tin) override {
    TSR_LOG_TRACE("Tagging water feature");

    // TODO: Get dataset from cache or API
    const std::string dataFeatureID = this->feature_id + "/data";

    // Mark whether a face is water or not
    int waterCount = 0;
    int landCount = 0;
    int noDataCount = 0;
    std::unordered_map<ChunkInfo, GDALDatasetH> datasets;
    for (Face_handle face : tin.all_face_handles()) {

      if (tin.is_infinite(face)) {
        continue;
      }

      // Get the circumcenter poing
      auto p0 = face->vertex(0)->point();
      auto p1 = face->vertex(1)->point();
      auto p2 = face->vertex(2)->point();

      Point3 center = CGAL::circumcenter(p0, p1, p2);

      Point3 centerWGS84;
      try {
        centerWGS84 = UTM_point_to_WGS84(center, 30, true);
      } catch (std::exception e) {
        continue;
      }

      // Get the required chunk
      ChunkInfo chunk = chunker.getChunkInfo(centerWGS84.x(), centerWGS84.y());

      GDALDatasetH dataset = nullptr;

      if (datasets.contains(chunk)) {
        dataset = datasets[chunk];
      } else if (IO::isChunkCached(dataFeatureID, chunk)) {
        IO::getChunkFromCache<GDALDatasetH>(dataFeatureID, chunk, dataset);
        datasets[chunk] = dataset;
      } else {
        this->waterMap[face] = NODATA;
        noDataCount++;
        continue;
      }

      if (dataset == nullptr) {
        TSR_LOG_ERROR("Water dataset empty");
        throw std::runtime_error("water dataset empty");
      }

      // Find the point corresponding to the center of the
      // Get the GeoTransform
      double geotransform[6];
      if (GDALGetGeoTransform(dataset, geotransform) != CE_None) {
        TSR_LOG_ERROR("Failed to get water GeoTransform");
        throw std::runtime_error("failed to get water GeoTransform");
      }

      // Check if the coordinates are within the raster bounds
      int raster_x_size = GDALGetRasterXSize(dataset);
      int raster_y_size = GDALGetRasterYSize(dataset);

      // Get the raster band (assuming a single-band GeoTIFF)
      GDALRasterBandH band =
          GDALGetRasterBand(dataset, 1); // Use appropriate band if multiple
      if (band == nullptr) {
        TSR_LOG_ERROR("Water dataset band not found");
        throw std::runtime_error("water dataset band not found");
      }

      // Convert UTM coordinates to pixel coordinates
      int pixel_x =
          static_cast<int>((center.x() - geotransform[0]) / geotransform[1]);
      int pixel_y =
          static_cast<int>((center.y() - geotransform[3]) / geotransform[5]);

      if (pixel_x < 0 || pixel_x >= raster_x_size || pixel_y < 0 ||
          pixel_y >= raster_y_size) {
        TSR_LOG_WARN("Point outside water dataset bounds {} {}", center.x(),
                     center.y());
        this->waterMap[face] = NODATA;
        noDataCount++;
        continue;
      }

      // Read the value at the specified pixel
      float value;
      if (GDALRasterIO(band, GF_Read, pixel_x, pixel_y, 1, 1, &value, 1, 1,
                       GDT_Float32, 0, 0) != CE_None) {
        TSR_LOG_ERROR("Failed to read pixel value from water dataset");
        throw std::runtime_error(
            "failed to read pixel value from water dataset");
      }

      if (value == NODATA_VALUE) {
        this->waterMap[face] = NODATA;
        noDataCount++;
      } else if (value == 0) {
        this->waterMap[face] = LAND;
        landCount++;
      } else {
        this->waterMap[face] = WATER;
        waterCount++;
      }
    }

    TSR_LOG_TRACE("faces with water: {}", waterCount);
    TSR_LOG_TRACE("faces with land: {}", landCount);
    TSR_LOG_TRACE("faces with no data: {}", noDataCount);
  }

  void writeWaterMapToKML() {
    std::vector<Face_handle> faces;

    for (auto f : this->waterMap) {
      if (f.second == WATER) {
        faces.push_back(f.first);
      }
    }

    std::string waterKML = IO::generateKMLFaces(faces);

    auto kml = IO::generateKMLDocument(waterKML);

    IO::write_data_to_file("water.kml", kml);
  }

  bool calculate(TsrState &state) override {

    if (!waterMap.contains(state.current_face)) {
      AddWarning(state, "water data unavailable", 11);
      return true;
    }

    // auto distanceFeature = std::dynamic_pointer_cast<Feature<double>>(
    //     this->dependencies[DEPENDENCIES::DISTANCE]);

    // double distance =
    //     distanceFeature->calculate(face, source_point, target_point);

    auto waterStatus = this->waterMap[state.current_face];

    if (waterStatus == WATER) {
      AddWarning(state, "Water", 11);
      return true;
    } else if (waterStatus == NODATA) {
      AddWarning(state, "water data unavailable", 11);
      return true;
    } else {
      return false;
    }
  }
};

} // namespace tsr