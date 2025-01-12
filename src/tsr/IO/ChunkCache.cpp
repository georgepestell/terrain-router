#include <boost/container_hash/extensions.hpp>
#include <filesystem>
#include <gdal.h>
#include <gdal/gdal_priv.h>
#include <string>

#include "tsr/IO/ChunkCache.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Tin.hpp"
#include <boost/functional/hash.hpp>

#include "tsr/ChunkInfo.hpp"
#include "tsr/IO/MapIO.hpp"
#include "tsr/IO/MeshIO.hpp"
#include "tsr/Logging.hpp"

#define CACHE_DIR "./tsrCache"

namespace tsr::IO {

std::string GenerateChunkId(const ChunkInfo &chunk) {

  boost::hash<ChunkInfo> hasher;
  auto positionIDHash = hasher(chunk);

  return fmt::format("chunk_{}", positionIDHash);
}

bool IsChunkCached(const std::string feature_id, const ChunkInfo &chunk) {

  std::string filepath = GetChunkFilepath(feature_id, chunk);

  return std::filesystem::exists(filepath);
}

void CacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const Tin &tin) {

  std::string filepath = GetChunkFilepath(feature_id, chunk);

  TSR_LOG_TRACE("caching to: {}", filepath.c_str());
  IO::WriteTinToFile(filepath, tin);
}

void CacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const GDALDatasetH &dataset) {

  TSR_LOG_TRACE("calculating dir");
  std::filesystem::path dir = std::filesystem::path(CACHE_DIR) / feature_id;

  if (!std::filesystem::exists(dir)) {
    if (!std::filesystem::create_directories(dir)) {
      TSR_LOG_ERROR("failed to create cache directory");
      return;
    }
  }

  auto filepath = GetChunkFilepath(feature_id, chunk);
  TSR_LOG_TRACE("caching to: {}", filepath.c_str());

  IO::WriteGdalDatasetToFile(filepath, dataset);
}

std::string GetChunkFilepath(const std::string feature_id,
                             const ChunkInfo &chunk) {
  std::filesystem::path dir = std::filesystem::path(CACHE_DIR) / feature_id;

  if (!std::filesystem::exists(dir)) {
    if (!std::filesystem::create_directories(dir)) {
      TSR_LOG_WARN("failed to create cache directory");
    }
  }

  std::string filename = GenerateChunkId(chunk);
  return std::filesystem::path(dir) / filename;
}

void CacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const std::vector<std::vector<Point2>> &contours) {

  auto filepath = GetChunkFilepath(feature_id, chunk);

  TSR_LOG_TRACE("caching contours to: {}", filepath.c_str());

  IO::WriteContoursToFile(filepath, contours);
}

template <>
void GetChunkFromCache<Tin>(const std::string feature_id, const ChunkInfo &chunk,
                            Tin &dtm) {

  auto filepath = GetChunkFilepath(feature_id, chunk);

  dtm = loadCDTFromFile(filepath);
}

template <>
void GetChunkFromCache<GDALDatasetH>(const std::string feature_id,
                                     const ChunkInfo &chunk,
                                     GDALDatasetH &dataset) {

  auto filepath = GetChunkFilepath(feature_id, chunk);

  LoadGdalDatasetFromFile(filepath, dataset);
}

template <>
void GetChunkFromCache<std::vector<std::vector<Point2>>>(
    const std::string feature_id, const ChunkInfo &chunk,
    std::vector<std::vector<Point2>> &contours) {

  auto filepath = GetChunkFilepath(feature_id, chunk);
  LoadContoursFromFile(filepath, contours);
}

void DeleteChunkFromCache(const std::string feature_id, const ChunkInfo &chunk) {
  auto filepath = GetChunkFilepath(feature_id, chunk);
  if (std::filesystem::exists(filepath)) {
    std::filesystem::remove(filepath);
  }
}

} // namespace tsr::IO