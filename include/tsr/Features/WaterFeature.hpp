#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Feature.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/Point_3.hpp"

#include "tsr/logging.hpp"

#include <CGAL/Kernel/global_functions_3.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "tsr/IO/KMLWriter.hpp"

#include <gdal/gdal_priv.h>

namespace tsr {

class BoolWaterFeature : public Feature<bool> {

private:
  std::unordered_map<Face_handle, bool> waterMap;
  double max_traversable_distance = 0;

  enum DEPENDENCIES { DISTANCE };

public:
  BoolWaterFeature(std::string name, Delaunay_3 &dtm, GDALDatasetH &waterData)
      : Feature(name) {

    GDALDataset *dataset = (GDALDataset *)waterData;
    if (dataset == nullptr) {
      TSR_LOG_ERROR("Water dataset empty");
      throw std::runtime_error("water dataset empty");
    }

    // Find the point corresponding to the center of the
    // Get the GeoTransform
    double geotransform[6];
    if (dataset->GetGeoTransform(geotransform) != CE_None) {
      TSR_LOG_ERROR("Failed to get water GeoTransform");
      throw std::runtime_error("failed to get water GeoTransform");
    }

    // Check if the coordinates are within the raster bounds
    int raster_x_size = dataset->GetRasterXSize();
    int raster_y_size = dataset->GetRasterYSize();

    // Get the raster band (assuming a single-band GeoTIFF)
    GDALRasterBand *band =
        dataset->GetRasterBand(1); // Use appropriate band if multiple
    if (band == nullptr) {
      TSR_LOG_ERROR("Water dataset band not found");
      throw std::runtime_error("water dataset band not found");
    }

    // Mark whether a face is water or not
    int waterCount = 0;
    for (Face_handle face : dtm.all_face_handles()) {

      // Get the circumcenter poing
      auto p0 = face->vertex(0)->point();
      auto p1 = face->vertex(1)->point();
      auto p2 = face->vertex(2)->point();

      Point_3 center = CGAL::circumcenter(p0, p1, p2);

      // Convert UTM coordinates to pixel coordinates
      int pixel_x =
          static_cast<int>((center.x() - geotransform[0]) / geotransform[1]);
      int pixel_y =
          static_cast<int>((center.y() - geotransform[3]) / geotransform[5]);

      if (pixel_x < 0 || pixel_x >= raster_x_size || pixel_y < 0 ||
          pixel_y >= raster_y_size) {
        TSR_LOG_WARN("Point outside water dataset bounds {} {}", center.x(),
                     center.y());
        continue;
      }

      // Read the value at the specified pixel
      float value;
      if (band->RasterIO(GF_Read, pixel_x, pixel_y, 1, 1, &value, 1, 1,
                         GDT_Float32, 0, 0) != CE_None) {
        TSR_LOG_ERROR("Failed to read pixel value from water dataset");
        throw std::runtime_error(
            "failed to read pixel value from water dataset");
      }

      this->waterMap[face] = value > 0.0 ? true : false;
      if (!this->waterMap[face]) {
        waterCount++;
      }
    }
    TSR_LOG_TRACE("faces with water: {}", waterCount);
  }

  void writeWaterMapToKML() {
    std::vector<Face_handle> faces;

    for (auto f : this->waterMap) {
      if (f.second) {
        faces.push_back(f.first);
      }
    }

    std::string kml = IO::generateKML(faces);

    IO::write_data_to_file("water.kml", kml);
  }

  bool calculate(Face_handle face, Point_3 &source_point,
                 Point_3 &target_point) override {

    if (!waterMap.contains(face)) {
      TSR_LOG_WARN("Water value not set for face");
      return true;
    }

    // auto distanceFeature = std::dynamic_pointer_cast<Feature<double>>(
    //     this->dependencies[DEPENDENCIES::DISTANCE]);

    // double distance =
    //     distanceFeature->calculate(face, source_point, target_point);

    return this->waterMap[face];
  }
};

} // namespace tsr