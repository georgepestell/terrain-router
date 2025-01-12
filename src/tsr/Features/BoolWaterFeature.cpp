#include "tsr/Features/BoolWaterFeature.hpp"
#include "tsr/API/GDALHandler.hpp"
#include "tsr/ChunkInfo.hpp"
#include "tsr/DelaunayTriangulation.hpp"
#include "tsr/IO/ChunkCache.hpp"
#include "tsr/IO/FileIO.hpp"
#include "tsr/IO/KMLWriter.hpp"
#include "tsr/Logging.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Tin.hpp"
#include "tsr/TsrState.hpp"

#include <CGAL/Kernel/global_functions_3.h>
#include <cpl_error.h>
#include <exception>
#include <gdal.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace tsr {

std::string BoolWaterFeature::URL =
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

void BoolWaterFeature::Initialize(Tin &tin, const MeshBoundary &boundary) {
  auto chunks = chunkManager.GetRequiredChunks(boundary);

  std::string dataFeatureID = this->feature_id + "/data";
  std::string contourFeatureID = this->feature_id + "/contour";

  const double MAX_SEGMENT_LENGTH = 22;

  // For each chunk, either fetch from the API or cache
  for (auto chunk : chunks) {

    std::vector<std::vector<Point2>> contours;
    if (IO::IsChunkCached(contourFeatureID, chunk)) {
      TSR_LOG_TRACE("reading cached contours");

      IO::GetChunkFromCache<std::vector<std::vector<Point2>>>(contourFeatureID,
                                                              chunk, contours);

      TSR_LOG_TRACE("cache chunk contours: {}", contours.size());

    } else {
      TSR_LOG_TRACE("fetching contours from API");

      // Fetch data from api
      auto data = chunkManager.FetchAndRasterizeVectorChunk(chunk, 0.00001);

      // Cache data
      IO::CacheChunk(dataFeatureID, chunk, data.dataset);

      // Fetch the geotransform for the dataset
      double adfGeotransform[6];
      GDALGetGeoTransform(data.dataset, adfGeotransform);

      // Close the dataset to enable the extractor to read
      GDALReleaseDataset(data.dataset);

      TSR_LOG_TRACE("contour file: {}", data.filename);

      // Extract contours usign OpenCV
      const double SIMPLIFICATION_FACTOR = 0.01;
      contours = API::ExtractFeatureContours(data.filename, adfGeotransform,
                                             SIMPLIFICATION_FACTOR);

      TSR_LOG_TRACE("chunk contours: {}", contours.size());

      // Cache contours
      IO::CacheChunk(contourFeatureID, chunk, contours);
    }

    // add contours to mesh
    for (auto contour : contours) {
      AddContourConstraint(tin, contour, MAX_SEGMENT_LENGTH);
    }
  }
}

void BoolWaterFeature::Tag(const Tin &tin) {
  TSR_LOG_TRACE("Tagging water feature");

  const std::string dataFeatureID = this->feature_id + "/data";

  // Mark whether a face is water or not
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
      centerWGS84 = TranslateUtmPointToWgs84(center, 30, true);
    } catch (std::exception e) {
      continue;
    }

    // Get the required chunk
    ChunkInfo chunk =
        chunkManager.GetChunkInfo(centerWGS84.x(), centerWGS84.y());

    GDALDatasetH dataset = nullptr;

    if (datasets.contains(chunk)) {
      dataset = datasets[chunk];
    } else if (chunkManager.IsAvailableInCache(dataFeatureID, chunk)) {
      IO::GetChunkFromCache<GDALDatasetH>(dataFeatureID, chunk, dataset);
      datasets[chunk] = dataset;
    } else {
      this->waterMap[face] = NODATA;
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
      continue;
    }

    // Read the value at the specified pixel
    float value;
    if (GDALRasterIO(band, GF_Read, pixel_x, pixel_y, 1, 1, &value, 1, 1,
                     GDT_Float32, 0, 0) != CE_None) {
      TSR_LOG_ERROR("Failed to read pixel value from water dataset");
      throw std::runtime_error("failed to read pixel value from water dataset");
    }

    if (value == NODATA_VALUE) {
      this->waterMap[face] = NODATA;
    } else if (value == 0) {
      this->waterMap[face] = LAND;
    } else {
      this->waterMap[face] = WATER;
    }
  }
}

void BoolWaterFeature::WriteWaterToKml() {
  std::vector<Face_handle> faces;

  for (auto f : this->waterMap) {
    if (f.second == WATER) {
      faces.push_back(f.first);
    }
  }

  std::string waterKML = IO::GenerateKmlFaces(faces);

  auto kml = IO::GenerateKmlDocument(waterKML);

  IO::WriteDataToFile("water.kml", kml);
}

bool BoolWaterFeature::Calculate(TsrState &state) {

  if (!waterMap.contains(state.current_face)) {
    AddWarning(state, "water data unavailable", 11);
    return true;
  }

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
} // namespace tsr