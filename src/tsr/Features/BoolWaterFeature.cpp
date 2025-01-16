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
    "https://lz4.overpass-api.de/api/"
    "interpreter?data=%5Bout%3Axml%5D%5Btimeout%3A25%5D%3B(nwr%5B%22natural%"
    "22%3D%22bay%22%5D%28{}%2C{}%2C{}%2C{}%29%3Bnwr%5B%22natural%22%3D%22water%"
    "22%5D%28{}%2C{}%2C{}%2C{}%29%3Bnwr%5B%22natural%22%3D%22coastline%22%5D%"
    "28{}%2C{}%2C{}%2C{}%29%3B);%28._%3B%3E%3B%29%3Bout%20body%3B%0A{}";

void BoolWaterFeature::Initialize(Tin &tin, const MeshBoundary &boundary) {
  auto chunks = chunkManager.GetRequiredChunks(boundary);

  std::string dataFeatureID = this->feature_id + "/data";
  std::string contourFeatureID = this->feature_id + "/contour";

  const double MAX_SEGMENT_LENGTH = 22;

  // For each chunk, either fetch from the API or cache
  for (auto chunk : chunks) {

    std::vector<std::vector<Point2>> contours;
    if (chunkManager.IsAvailableInCache(contourFeatureID, chunk)) {

      IO::GetChunkFromCache<std::vector<std::vector<Point2>>>(contourFeatureID,
                                                              chunk, contours);

    } else {
      // Fetch data from api
      DataFile data;
      try {
        data = chunkManager.FetchAndRasterizeVectorChunk(chunk, 0.0001);
      } catch (std::exception &e) {
        TSR_LOG_WARN("Failed to fetch chunk or it is empty.");
        continue;
      }

      // Cache data
      IO::CacheChunk(dataFeatureID, chunk, data.dataset);

      // Fetch the geotransform for the dataset
      double adfGeotransform[6];
      GDALGetGeoTransform(data.dataset, adfGeotransform);

      // Close the dataset to enable the extractor to read
      GDALReleaseDataset(data.dataset);

      TSR_LOG_TRACE("contour file: {}", data.filename);

      // Extract contours usign OpenCV
      const double SIMPLIFICATION_FACTOR = 0.001;
      TSR_LOG_DEBUG("Extracting Contours");
      contours = API::ExtractFeatureContours(data.filename, adfGeotransform,
                                             SIMPLIFICATION_FACTOR);

      TSR_LOG_TRACE("chunk contours: {}", contours.size());

      // Cache contours
      TSR_LOG_DEBUG("Caching contours ");
      IO::CacheChunk(contourFeatureID, chunk, contours);
    }

    TSR_LOG_TRACE("Water Contours: {}", contours.size());

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
  ChunkInfo dataset_chunk;
  GDALDatasetH dataset = nullptr;
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
    } catch (std::exception &e) {
      continue;
    }

    // Get the required chunk
    ChunkInfo chunk =
        chunkManager.GetChunkInfo(centerWGS84.x(), centerWGS84.y());

    if (chunk != dataset_chunk) {
      if (!IO::IsChunkCached(dataFeatureID, chunk)) {
        this->waterMap[face] = NODATA;
        TSR_LOG_WARN("Value not available in cache");
        continue;
      }

      if (dataset != nullptr) {
        GDALReleaseDataset(dataset);
      }

      IO::GetChunkFromCache<GDALDatasetH>(dataFeatureID, chunk, dataset);
      dataset_chunk = chunk;
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

  if (dataset != nullptr) {
    GDALReleaseDataset(dataset);
  }
}

void BoolWaterFeature::WriteWaterToKml() {
  std::vector<Face_handle> waterFaces;
  std::vector<Face_handle> nodataFaces;

  for (auto f : this->waterMap) {
    if (f.second == WATER) {
      waterFaces.push_back(f.first);
    } else if (f.second == NODATA) {
      nodataFaces.push_back(f.first);
    }
  }

  std::string waterKML = IO::GenerateKmlFaces(waterFaces, "Water Faces");
  std::string noDataFaces = IO::GenerateKmlFaces(nodataFaces, "No Water Data");

  auto kml = IO::GenerateKmlDocument(waterKML + noDataFaces);

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