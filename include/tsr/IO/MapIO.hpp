#include "tsr/Point3.hpp"
#include "tsr/Tin.hpp"
#include <gdal/gdal_priv.h>
#include <memory>
#include <string>

namespace tsr::IO {

GDALDatasetH ParseGdalDataset(std::string data);

/**
 * @brief Parses a file for use with GDAL mapping data tools.
 *
 * @param filepath Map data file to load
 * @return std::unique_ptr<GDALDatasetH> GDAL dataset
 */
void LoadGdalDatasetFromFile(std::string filepath, GDALDatasetH &dataset);
void LoadVectorGdalDatasetFromFile(std::string filepath,
                                        GDALDatasetH &dataset);

std::vector<std::vector<Point3>>
ExtractGdalDatasetContours(GDALDataset *dataset, int layerID);

void WriteGdalDatasetToFile(std::string filename, const GDALDatasetH &hDataset);

std::vector<Point3> ExtractGdalDatasetPoints(GDALDatasetH hDataset,
                                                 int layerID);

} // namespace tsr::IO