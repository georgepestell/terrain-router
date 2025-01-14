#pragma once

#include "tsr/ChunkInfo.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Tin.hpp"
#include <gdal/gdal_priv.h>
#include <string>

namespace tsr::IO {

std::string GenerateChunkId(const ChunkInfo &chunk);

std::string GetChunkFilepath(const std::string feature_id,
                             const ChunkInfo &chunk);

void CacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const Tin &tin);

void CacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const GDALDatasetH &dataset);
void CacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const std::vector<std::vector<Point2>> &contours);

bool IsChunkCached(const std::string feature_id, const ChunkInfo &chunk);

template <typename DataType>
void GetChunkFromCache(const std::string feature_id, const ChunkInfo &chunk,
                       DataType &data);

void DeleteChunkFromCache(const std::string feature_id, const ChunkInfo &chunk);

} // namespace tsr::IO