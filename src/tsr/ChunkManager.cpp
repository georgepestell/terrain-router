#include "tsr/ChunkManager.hpp"

#include "fmt/core.h"
#include "tsr/API/APICaller.hpp"
#include "tsr/API/GDALHandler.hpp"

#include <algorithm>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <exception>
#include <gdal/gdal.h>

#include "tsr/ChunkInfo.hpp"
#include "tsr/Logging.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Point2.hpp"
#include "tsr/PointProcessor.hpp"

#include "tsr/IO/ChunkCache.hpp"
#include "tsr/IO/MapIO.hpp"

#include <cmath>
#include <fmt/args.h>
#include <fmt/format.h>
#include <gdal.h>
#include <string>
#include <vector>

namespace tsr {

static bool g_cache_enabled = true;

void CacheSetEnabled(bool isEnabled) { g_cache_enabled = isEnabled; }
bool IsCacheEnabled() { return g_cache_enabled; }

ChunkInfo ChunkManager::GetChunkInfo(double lat, double lng) const {
  double minLat = std::floor(lat / tile_size) * this->tile_size;
  double minLng = std::floor(lng / tile_size) * this->tile_size;
  double maxLat = minLat + this->tile_size;
  double maxLng = minLng + this->tile_size;
  return {minLat, minLng, maxLat, maxLng};
}
std::vector<ChunkInfo>
ChunkManager::GetRequiredChunks(const MeshBoundary &boundary) const {

  // Convert the boundary to WGS84
  const Point2 ur =
      TranslateUtmPointToWgs84(boundary.GetUpperRightPoint(), 30, true);
  const Point2 ll =
      TranslateUtmPointToWgs84(boundary.GetLowerLeftPoint(), 30, true);

  const double minLat = std::min(ur.x(), ll.x());
  const double maxLat = std::max(ur.x(), ll.x());

  const double minLng = std::min(ur.y(), ll.y());
  const double maxLng = std::max(ur.y(), ll.y());

  TSR_LOG_TRACE("looping through required chunks");
  TSR_LOG_TRACE("tile size: {}", this->tile_size);

  // Fetch the required chunks
  std::vector<ChunkInfo> chunks;
  for (double lat = minLat; lat < maxLat + this->tile_size;
       lat += this->tile_size) {
    for (double lng = minLng; lng < maxLng + this->tile_size;
         lng += this->tile_size) {

      ChunkInfo chunk = this->GetChunkInfo(lat, lng);
      chunks.push_back(chunk);
    }
  }

  TSR_LOG_TRACE("chunks: {}", chunks.size());

  return chunks;
}

std::string ChunkManager::FormatUrl(const ChunkInfo &chunkInfo) const {

  auto ds = fmt::dynamic_format_arg_store<fmt::format_context>();
  for (auto i : this->position_order) {
    if (i == 0) {

      ds.push_back(chunkInfo.minLat);
    } else if (i == 1) {
      ds.push_back(chunkInfo.minLng);
    } else if (i == 2) {
      ds.push_back(chunkInfo.maxLat);
    } else if (i == 3) {
      ds.push_back(chunkInfo.maxLng);
    }
  }

  ds.push_back(this->api_key);

  std::string formattedURL = fmt::vformat(this->url, ds);
  return formattedURL;
}

DataFile ChunkManager::FetchVectorChunk(const ChunkInfo &chunk) const {

  // Parse the API chunk URL
  std::string chunkURL = this->FormatUrl(chunk);

  // Create a temporary file name
  boost::filesystem::path dir = boost::filesystem::temp_directory_path();
  boost::filesystem::path filepath =
      boost::filesystem::unique_path(dir / "tempfile-%%%%%.tmp");

  // Fetch the response from the API
  GDALDatasetH dataset;
  try {
    API::APICaller caller;
    caller.FetchDataFromAPI(chunkURL, filepath.string());
    IO::LoadVectorGdalDatasetFromFile(filepath.string(), dataset);
  } catch (std::exception &e) {
    TSR_LOG_TRACE("{}", e.what());
    TSR_LOG_ERROR("No response from API");
    throw e;
  }

  // Warp dataset
  GDALDatasetH warpedDS;

  boost::filesystem::path warpedFilepath =
      boost::filesystem::unique_path(dir / "tempfile-%%%%%.tmp");

  try {
    warpedDS = API::WarpVectorDatasetToUtm(dataset, warpedFilepath.string());
  } catch (std::exception &e) {
    TSR_LOG_ERROR("failed to warp dataset");
    TSR_LOG_TRACE("{}", e.what());
    GDALReleaseDataset(dataset);
    // boost::filesystem::remove(filepath);
    throw e;
  }

  // Delete the dataset
  GDALReleaseDataset(dataset);
  // boost::filesystem::remove(filepath);
  return DataFile(warpedDS, warpedFilepath.string());
}

DataFile
ChunkManager::FetchAndRasterizeVectorChunk(const ChunkInfo &chunk,
                                           double pixel_resolution) const {

  // Parse the API chunk URL
  std::string chunkURL = this->FormatUrl(chunk);

  // Create a temporary file name
  boost::filesystem::path dir = boost::filesystem::temp_directory_path();
  boost::filesystem::path filepath =
      boost::filesystem::unique_path(dir / "tempfile-%%%%%.tmp");

  // Fetch the response from the API
  GDALDatasetH dataset;
  try {
    API::APICaller caller;
    caller.FetchDataFromAPI(chunkURL, filepath.string());
    IO::LoadVectorGdalDatasetFromFile(filepath.string(), dataset);
  } catch (std::exception &e) {
    TSR_LOG_TRACE("{}", e.what());
    TSR_LOG_ERROR("No response from API");
    boost::filesystem::remove(filepath);
    throw e;
  }

  // Create a temporary file name
  boost::filesystem::path raster_filepath =
      boost::filesystem::unique_path(dir / "tempfile-%%%%%.tmp");

  // Rasterize dataset
  GDALDatasetH rasterDataset;
  try {
    rasterDataset = API::RasterizeDataset(dataset, raster_filepath.string(),
                                          chunk, pixel_resolution);
  } catch (std::exception &e) {
    TSR_LOG_TRACE("{}", e.what());
    TSR_LOG_ERROR("failed to rasterize vector dataset");
    boost::filesystem::remove(filepath);
    GDALReleaseDataset(dataset);
    throw e;
  }

  GDALReleaseDataset(dataset);
  boost::filesystem::remove(filepath);

  // Warp dataset
  GDALDatasetH warpedDS;
  boost::filesystem::path warpedFilepath =
      boost::filesystem::unique_path(dir / "tempfile-%%%%%.tmp");

  TSR_LOG_TRACE("raster filepath: {}", raster_filepath.string());

  TSR_LOG_TRACE("warping dataset");
  TSR_LOG_DEBUG("Warping dataset");
  try {
    warpedDS =
        API::WarpRasterDatasetToUtm(rasterDataset, warpedFilepath.string());
  } catch (std::exception &e) {
    TSR_LOG_ERROR("failed to warp dataset");
    TSR_LOG_TRACE("{}", e.what());
    GDALReleaseDataset(rasterDataset);
    boost::filesystem::remove(raster_filepath);
    throw e;
  }

  // Delete the dataset
  GDALReleaseDataset(rasterDataset);
  boost::filesystem::remove(raster_filepath);
  return {warpedDS, warpedFilepath.string()};
}

DataFile ChunkManager::FetchRasterChunk(const ChunkInfo &chunk) const {

  TSR_LOG_TRACE("getting for: {} {} {} {}", chunk.minLat, chunk.minLng,
                chunk.maxLat, chunk.maxLng);

  // Parse the API chunk URL
  TSR_LOG_TRACE("formatting url");
  std::string chunkURL = this->FormatUrl(chunk);

  TSR_LOG_TRACE("creating temporary filename");

  // Create a temporary file name
  boost::filesystem::path dir = boost::filesystem::temp_directory_path();
  boost::filesystem::path filepath =
      boost::filesystem::unique_path(dir / "tempfile-%%%%%.tiff");

  // Fetch the response from the API
  TSR_LOG_TRACE("fetching raster chunk");
  try {
    API::APICaller caller;

    TSR_LOG_TRACE("calling API");
    caller.FetchDataFromAPI(chunkURL, filepath.string());

    TSR_LOG_TRACE("parsing response");
  } catch (std::exception &e) {
    TSR_LOG_TRACE("{}", e.what());
    TSR_LOG_ERROR("Calling API failed");
    throw e;
  }

  TSR_LOG_TRACE("parsing raster chunk");
  GDALDatasetH dataset;
  try {
    IO::LoadGdalDatasetFromFile(filepath.string(), dataset);
  } catch (std::exception &e) {
    TSR_LOG_ERROR("failed to open GDAL dataset");
    TSR_LOG_TRACE("{}", e.what());
    throw e;
  }

  // Warp dataset
  TSR_LOG_TRACE("warping to UTM");
  GDALDatasetH warpedDS;

  TSR_LOG_TRACE("creating warping temporary filename");
  boost::filesystem::path warpedFilepath =
      boost::filesystem::unique_path(dir / "tempfile-%%%%%.tmp");

  try {
    warpedDS = API::WarpRasterDatasetToUtm(dataset, warpedFilepath.string());
  } catch (std::exception &e) {
    TSR_LOG_ERROR("failed to warp dataset");
    TSR_LOG_TRACE("{}", e.what());
    GDALReleaseDataset(dataset);
    boost::filesystem::remove(filepath);
    throw e;
  }

  TSR_LOG_TRACE("returning dataset");

  // Delete the dataset
  GDALReleaseDataset(dataset);
  boost::filesystem::remove(filepath);
  return {warpedDS, warpedFilepath.string()};
}

bool ChunkManager::IsAvailableInCache(const std::string &feature_id,
                                      const ChunkInfo &chunk) {
  // check if caching is disabled
  if (!g_cache_enabled) {
    return false;
  }

  return IO::IsChunkCached(feature_id, chunk);
}

} // namespace tsr