#pragma once

#include <gdal/gdal_priv.h>

#include <string>

namespace tsr::IO {

/**
 * @brief Converts relative filepaths to an absolute form.
 *
 * @param filename Path to convert to absolute form.
 * @return std::string The absolute path version of filename
 */
std::string path_to_absolute(std::string filename);

void write_data_to_file(std::string filepath, std::string data);

} // namespace tsr::IO