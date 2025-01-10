#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include "tsr/Point2.hpp"

/**
 * Ignore _Atomic warning from opencv2.
 * The c++20 standard shouldn't cause issues with this
 */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc11-extensions"
#include <opencv2/core/mat.hpp>
#pragma clang diagnostic pop

#include <vector>

namespace tsr::IO {

/**
 * @brief Loads image as an OpenCV Matrix
 *
 * @param filepath Image file to load
 * @return std::unique_ptr<cv::Mat> Image as OpenCV matrix
 */
cv::Mat load_image_from_file(std::string filepath);

cv::Mat convert_grayscale_image_to_rgb(cv::Mat &image);

std::vector<std::vector<Point2>>
extract_feature_contours(cv::Mat &image, double simplification_factor,
                         double uloffset_x, double uloffset_y,
                         double cellsize_x, double cellsize_y);

} // namespace tsr::IO