#include "tsr/IO/ImageIO.hpp"

#include <cmath>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tsr/Point_2.hpp"
#include "tsr/logging.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace tsr::IO {

cv::Mat load_image_from_file(std::string filepath) {
  auto img = cv::imread(filepath, cv::IMREAD_GRAYSCALE);

  if (img.empty()) {
    TSR_LOG_ERROR("failed to open image");
    throw std::runtime_error("failed to open image");
  }

  return img;
}

cv::Mat convert_grayscale_image_to_rgb(cv::Mat &image) {
  cv::Mat rgbImage;
  cv::cvtColor(image, rgbImage, cv::COLOR_GRAY2RGB);
  return rgbImage;
}

std::unique_ptr<std::vector<std::vector<Point_2>>>
extract_feature_contours(cv::Mat &image, double simplification_factor,
                         double uloffset_x, double uloffset_y,
                         double cellsize_x, double cellsize_y) {

  cv::cvtColor(image, image, cv::COLOR_BGR2HSV);

  cv::Mat edges;
  cv::Canny(image, edges, 100, 200);

  // Process the detected edges (find contours)
  std::vector<std::vector<cv::Point>> cv_contours;
  cv::findContours(edges, cv_contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_SIMPLE);

  // Convert contours to CGAL contours
  auto contours = std::make_unique<std::vector<std::vector<Point_2>>>();

  for (const auto &cv_contour : cv_contours) {
    std::vector<cv::Point> new_contour;
    if (simplification_factor > 0) {
      double epsilon = simplification_factor * cv::arcLength(cv_contour, true);
      cv::approxPolyDP(cv_contour, new_contour, epsilon, true);
    }

    // Skip contour if it doesn't contain an edge
    if (new_contour.size() <= 1) {
      continue;
    }

    // Convert contour to list of points
    std::vector<Point_2> contour;
    for (const auto &point : new_contour) {
      // Convert OpenCV points to CGAL points
      Point_2 cgal_point(round(point.x * cellsize_x + uloffset_x),
                         round(uloffset_y - point.y * cellsize_y));
      contour.push_back(cgal_point);
    }

    contours->push_back(contour);
  }

  return contours;
}

} // namespace tsr::IO