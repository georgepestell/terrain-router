#pragma once

#include "tsr/ChunkInfo.hpp"
#include "tsr/DataFile.hpp"
#include "tsr/Point2.hpp"

#include <gdal/gdal.h>
#include <string>

namespace tsr::API {

GDALDatasetH parseGDALDatasetFromString(std::string data, std::string filepath);

GDALDatasetH warpVectorDatasetToUTM(GDALDatasetH hDataset,
                                    std::string filepath);
GDALDatasetH warpRasterDatasetToUTM(GDALDatasetH hDataset,
                                    std::string filepath);
GDALDatasetH rasterizeDataset(const GDALDatasetH &source_dataset,
                              std::string filepath, const ChunkInfo &chunk,
                              double pixel_resolution);

std::vector<std::vector<Point2>>
extract_feature_contours(const std::string &filepath,
                         double adf_geotransform[6],
                         double simplification_factor);

} // namespace tsr::API