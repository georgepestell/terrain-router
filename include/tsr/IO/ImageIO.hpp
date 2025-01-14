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
cv::Mat LoadImageFromFile(std::string filepath);

cv::Mat ConvertGrayscaleToRgb(cv::Mat &image);

std::vector<std::vector<Point2>>
ExtractFeatureContours(cv::Mat &image, double simplification_factor,
                       double adfGeoTransform[6]);

} // namespace tsr::IO