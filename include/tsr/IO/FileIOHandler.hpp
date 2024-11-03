#pragma once

#include <opencv2/core/mat.hpp>
#include <gdal/gdal.h>

#include "tsr/Point_3.hpp"
#include "tsr/Surface_mesh.hpp"

#include <vector>
#include <memory>
#include <string>

namespace tsr::IO {

    /**
     * @brief Converts relative filepaths to an absolute form.
     * 
     * @param filename Path to convert to absolute form.
     * @return std::string The absolute path version of filename
     */
    std::string path_to_absolute(std::string filename);

    std::unique_ptr<std::vector<Point_3>> load_dem_from_file(std::string filepath);

    /**
     * @brief Loads image as an OpenCV Matrix
     * 
     * @param filepath Image file to load
     * @return std::unique_ptr<cv::Mat> Image as OpenCV matrix
     */
    std::unique_ptr<cv::Mat> load_image_from_file(std::string filepath);
    
    /**
     * @brief Parses a file for use with GDAL mapping data tools.
     * 
     * @param filepath Map data file to load
     * @return std::unique_ptr<GDALDatasetH> GDAL dataset
     */
    std::unique_ptr<GDALDatasetH> load_gdal_dataset_from_file(std::string filepath);

    /**
     * @brief Writes a Surface mesh object to an OBJ file. Useful for blender visualization of meshes.
     * 
     * @param filepath Output OBJ file path. Will be overriden if already exists.
     * @param mesh Surface mesh to write to the file
     */
    void write_mesh_to_obj(std::string filepath, Surface_mesh mesh);

}