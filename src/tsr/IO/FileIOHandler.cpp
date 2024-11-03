#include "tsr/IO/FileIOHandler.hpp"
#include "tsr/logging.hpp"

#include <gdal/gdal.h>
#include <stdexcept>
#include <string>
#include <memory>
#include <filesystem>

#include <opencv2/imgcodecs.hpp>

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

std::unique_ptr<std::vector<Point_3>> load_dem_from_file(std::string filepath) {

  // TODO: error checking

  TSR_LOG_TRACE("loading points from file");

  // Read points from file
  std::ifstream ifile(filepath, std::ios_base::binary);
  auto points = std::make_unique<std::vector<Point_3>>();
  
  double x, y, z;
  while (ifile >> x >> y >> z) {
    points->push_back(Point_3(round(x), round(y), round(z)));
  }

  return points;
}

std::unique_ptr<cv::Mat> load_image_from_file(std::string filepath) {
    auto img = std::make_unique<cv::Mat>(cv::imread(filepath));

    // Check the image was loaded correctly
    if (!img || img->empty()) {
        TSR_LOG_ERROR("image failed to load ({})", filepath);
        throw std::ios_base::failure("loading image failed: " + filepath);
    }

    return img;
}

std::unique_ptr<GDALDatasetH> load_gdal_dataset_from_file(std::string filepath) {
    auto src_ds = std::make_unique<GDALDatasetH>(GDALOpen(filepath.c_str(), GA_ReadOnly));

    if (*src_ds == NULL){
      TSR_LOG_ERROR("Failed to open GDAL dataset ({})", filepath);
      throw std::runtime_error("Failed to open GDAL dataset: " + filepath);
    }

    return src_ds;
}

void write_mesh_to_obj(std::string filepath, Surface_mesh mesh) {
  std::ofstream ofile(filepath, std::ios_base::binary);
  CGAL::IO::set_binary_mode(ofile);

  CGAL::IO::write_OBJ(ofile, mesh);
  ofile.close();
};

};
