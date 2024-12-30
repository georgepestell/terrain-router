#include "tsr/Delaunay_3.hpp"
#include "tsr/Point_3.hpp"
#include <gdal/gdal_priv.h>
#include <memory>
#include <string>

namespace tsr::IO {

GDALDatasetH parse_GDAL_dataset(std::string data);

/**
 * @brief Parses a file for use with GDAL mapping data tools.
 *
 * @param filepath Map data file to load
 * @return std::unique_ptr<GDALDatasetH> GDAL dataset
 */
void load_gdal_dataset_from_file(std::string filepath, GDALDatasetH &dataset);
void load_vector_gdal_dataset_from_file(std::string filepath,
                                        GDALDatasetH &dataset);

std::vector<std::vector<Point_3>>
extractContoursFromGDALDataset(GDALDataset *dataset, int layerID);

void writeGDALDatasetToFile(std::string filename, const GDALDatasetH &hDataset);

std::vector<Point_3> extractPointsFromGDALDataset(GDALDatasetH hDataset,
                                                  int layerID);

} // namespace tsr::IO