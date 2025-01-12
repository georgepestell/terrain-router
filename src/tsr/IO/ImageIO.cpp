#include "tsr/IO/ImageIO.hpp"

#include <cmath>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "tsr/Logging.hpp"
#include "tsr/Point2.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace tsr::IO {

cv::Mat LoadImageFromFile(std::string filepath) {
  auto img = cv::imread(filepath, cv::IMREAD_GRAYSCALE);

  if (img.empty()) {
    TSR_LOG_ERROR("failed to open image");
    throw std::runtime_error("failed to open image");
  }

  return img;
}

cv::Mat ConvertGrayscaleToRgb(cv::Mat &image) {
  cv::Mat rgbImage;
  cv::cvtColor(image, rgbImage, cv::COLOR_GRAY2RGB);
  return rgbImage;
}

std::vector<std::vector<Point2>>
ExtractFeatureContours(cv::Mat &image, double simplification_factor,
                         double uloffset_x, double uloffset_y,
                         double cellsize_x, double cellsize_y) {

  // Convert images to RGB
  unsigned int channels = image.channels();
  if (channels == 1) {
    cv::cvtColor(image, image, cv::COLOR_GRAY2RGB);
  } else if (channels == 4) {
    cv::cvtColor(image, image, cv::COLOR_BGRA2RGB);
  }

  // Convert RGB to HSV for better canny edge detection
  cv::cvtColor(image, image, cv::COLOR_RGB2HSV);

  cv::Mat edges;
  cv::Canny(image, edges, 100, 200);

  // Process the detected edges (find contours)
  std::vector<std::vector<cv::Point>> cv_contours;
  cv::findContours(edges, cv_contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_SIMPLE);

  // Convert contours to CGAL contours
  std::vector<std::vector<Point2>> contours;

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
    std::vector<Point2> contour;
    for (const auto &point : new_contour) {
      // Convert OpenCV points to CGAL points
      Point2 cgal_point(round(point.x * cellsize_x + uloffset_x),
                        round(uloffset_y - point.y * cellsize_y));
      contour.push_back(cgal_point);
    }

    contours.push_back(contour);
  }

  return contours;
}

} // namespace tsr::IO