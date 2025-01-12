#include "tsr/IO/FileIO.hpp"

#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <string>

namespace tsr::IO {

std::string PathToAbsolute(std::string filename) {
  boost::filesystem::path path(filename);

  // Add pwd to relative path
  if (path.is_relative()) {
    // Get the current working directory
    path = boost::filesystem::current_path() / path;
  }

  return path.string();
}

void deleteFile(std::string filepath) {
  if (boost::filesystem::exists(filepath)) {
    boost::filesystem::remove(filepath);
  }
}

void WriteDataToFile(std::string filepath, std::string data) {

  std::ofstream ofile(filepath);
  ofile << data;

  ofile.close();
}

}; // namespace tsr::IO
