#include "tsr/Point_2.hpp"
#include "tsr/Point_3.hpp"

#include <CGAL/hierarchy_simplify_point_set.h>
#include <CGAL/tags.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>

#include <CGAL/grid_simplify_point_set.h>
#include <CGAL/jet_smooth_point_set.h>

namespace tsr {

Point_3 UTM_point_to_WGS84(Point_3 pointUTM, short zone,
                           bool isNorthernHemisphere);
Point_2 UTM_point_to_WGS84(Point_2 pointUTM, short zone,
                           bool isNorthernHemisphere);

Point_3 WGS84_point_to_UTM(Point_3 pointWGS84);
Point_2 WGS84_point_to_UTM(Point_2 pointWGS84);

void jet_smooth_points(std::vector<Point_3> &points);
void simplify_points(std::vector<Point_3> &points);

void filter_points_domain(std::vector<Point_3> &points, Point_3 &source_point,
                          Point_3 &target_point, double radii_multiplier);

double calculate_xy_distance(const Point_3 &p1, const Point_3 &p2);
double calculate_xy_angle(const Point_3 &p1, const Point_3 &p2);

} // namespace tsr