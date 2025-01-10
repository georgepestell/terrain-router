#pragma once

#include "tsr/ChunkInfo.hpp"
#include "tsr/Point2.hpp"
#include "tsr/Tin.hpp"
#include <gdal/gdal_priv.h>
#include <string>

namespace tsr::IO {

std::string generateChunkID(const ChunkInfo &chunk);

std::string getChunkFilepath(const std::string feature_id,
                             const ChunkInfo &chunk);

void cacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const Tin &tin);

void cacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const GDALDatasetH &dataset);
void cacheChunk(const std::string feature_id, const ChunkInfo &chunk,
                const std::vector<std::vector<Point2>> &contours);

bool isChunkCached(const std::string feature_id, const ChunkInfo &chunk);

template <typename DataType>
void getChunkFromCache(const std::string feature_id, const ChunkInfo &chunk,
                       DataType &data);

void deleteCachedChunk(const std::string feature_id, const ChunkInfo &chunk);

} // namespace tsr::IO