#include <boost/container_hash/extensions.hpp>
#include <filesystem>
#include <gdal.h>
#include <gdal/gdal_priv.h>
#include <string>

#include "tsr/Delaunay_3.hpp"
#include "tsr/IO/ChunkCache.hpp"
#include "tsr/Point_2.hpp"
#include <boost/functional/hash.hpp>

#include "tsr/ChunkInfo.hpp"
#include "tsr/IO/MapIO.hpp"
#include "tsr/IO/MeshIO.hpp"
#include "tsr/logging.hpp"

#define CACHE_DIR "./tsrCache"

namespace tsr::IO {

std::string generateChunkID(const ChunkInfo &chunk) {

  boost::hash<ChunkInfo> hasher;
  auto positionIDHash = hasher(chunk);

  return fmt::format("chunk_{}", positionIDHash);
}

bool isChunkCached(const std::string featureID, const ChunkInfo &chunk) {

  std::string filepath = getChunkFilepath(featureID, chunk);

  return std::filesystem::exists(filepath);
}

void cacheChunk(const std::string featureID, const ChunkInfo &chunk,
                const Delaunay_3 &cdt) {

  std::string filepath = getChunkFilepath(featureID, chunk);

  TSR_LOG_TRACE("caching to: {}", filepath.c_str());
  IO::write_CDT_to_file(filepath, cdt);
}

void cacheChunk(const std::string featureID, const ChunkInfo &chunk,
                const GDALDatasetH &dataset) {

  TSR_LOG_TRACE("calculating dir");
  std::filesystem::path dir = std::filesystem::path(CACHE_DIR) / featureID;

  if (!std::filesystem::exists(dir)) {
    if (!std::filesystem::create_directories(dir)) {
      TSR_LOG_ERROR("failed to create cache directory");
      return;
    }
  }

  auto filepath = getChunkFilepath(featureID, chunk);
  TSR_LOG_TRACE("caching to: {}", filepath.c_str());

  IO::writeGDALDatasetToFile(filepath, dataset);
}

std::string getChunkFilepath(const std::string featureID,
                             const ChunkInfo &chunk) {
  std::filesystem::path dir = std::filesystem::path(CACHE_DIR) / featureID;

  if (!std::filesystem::exists(dir)) {
    if (!std::filesystem::create_directories(dir)) {
      TSR_LOG_WARN("failed to create cache directory");
    }
  }

  std::string filename = generateChunkID(chunk);
  return std::filesystem::path(dir) / filename;
}

void cacheChunk(const std::string featureID, const ChunkInfo &chunk,
                const std::vector<std::vector<Point_2>> &contours) {

  auto filepath = getChunkFilepath(featureID, chunk);

  TSR_LOG_TRACE("caching contours to: {}", filepath.c_str());

  IO::write_vector_contours_to_file(filepath, contours);
}

template <>
void getChunkFromCache<Delaunay_3>(const std::string featureID,
                                   const ChunkInfo &chunk, Delaunay_3 &dtm) {

  auto filepath = getChunkFilepath(featureID, chunk);

  dtm = loadCDTFromFile(filepath);
}

template <>
void getChunkFromCache<GDALDatasetH>(const std::string featureID,
                                     const ChunkInfo &chunk,
                                     GDALDatasetH &dataset) {

  auto filepath = getChunkFilepath(featureID, chunk);

  load_gdal_dataset_from_file(filepath, dataset);
}

template <>
void getChunkFromCache<std::vector<std::vector<Point_2>>>(
    const std::string featureID, const ChunkInfo &chunk,
    std::vector<std::vector<Point_2>> &contours) {

  auto filepath = getChunkFilepath(featureID, chunk);
  load_vector_contours_from_file(filepath, contours);
}

void deleteCachedChunk(const std::string featureID, const ChunkInfo &chunk) {
  auto filepath = getChunkFilepath(featureID, chunk);
  if (std::filesystem::exists(filepath)) {
    std::filesystem::remove(filepath);
  }
}

} // namespace tsr::IO