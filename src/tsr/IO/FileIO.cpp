#include "tsr/IO/FileIO.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace tsr::IO {

std::string path_to_absolute(std::string filename) {
  std::filesystem::path path(filename);

  // Add pwd to relative path
  if (path.is_relative()) {
    // Get the current working directory
    path = std::filesystem::current_path() / path;
  }

  return path;
}

void write_data_to_file(std::string filepath, std::string data) {

  std::ofstream ofile(filepath);
  ofile << data;

  ofile.close();
}

}; // namespace tsr::IO
