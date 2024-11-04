
// #include <SageIsAWank.h>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

#include "tsr/Point_2.hpp"

#include <opencv2/core/mat.hpp>

#include <memory>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel Kernel;
typedef CGAL::Point_3<Kernel> Point_3;

namespace tsr::IO {

std::unique_ptr<std::vector<std::vector<Point_2>>>
extract_feature_contours(cv::Mat &image, double simplification_factor,
                         double uloffset_x, double uloffset_y,
                         double cellsize_x, double cellsize_y);

}