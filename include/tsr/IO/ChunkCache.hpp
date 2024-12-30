#pragma once

#include "tsr/ChunkInfo.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/Point_2.hpp"
#include <gdal/gdal_priv.h>
#include <string>

namespace tsr::IO {

std::string generateChunkID(const ChunkInfo &chunk);

std::string getChunkFilepath(const std::string featureID,
                             const ChunkInfo &chunk);

void cacheChunk(const std::string featureID, const ChunkInfo &chunk,
                const Delaunay_3 &cdt);

void cacheChunk(const std::string featureID, const ChunkInfo &chunk,
                const GDALDatasetH &dataset);
void cacheChunk(const std::string featureID, const ChunkInfo &chunk,
                const std::vector<std::vector<Point_2>> &contours);

bool isChunkCached(const std::string featureID, const ChunkInfo &chunk);

template <typename DataType>
void getChunkFromCache(const std::string featureID, const ChunkInfo &chunk,
                       DataType &data);

void deleteCachedChunk(const std::string featureID, const ChunkInfo &chunk);

} // namespace tsr::IO