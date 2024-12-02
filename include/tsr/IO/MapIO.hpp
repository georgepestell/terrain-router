#include <gdal/gdal_priv.h>
#include <memory>
#include <string>

namespace tsr::IO {

std::unique_ptr<GDALDatasetH> parse_GDAL_dataset(std::string data);

/**
 * @brief Parses a file for use with GDAL mapping data tools.
 *
 * @param filepath Map data file to load
 * @return std::unique_ptr<GDALDatasetH> GDAL dataset
 */
std::unique_ptr<GDALDatasetH> load_gdal_dataset_from_file(std::string filepath);

} // namespace tsr::IO