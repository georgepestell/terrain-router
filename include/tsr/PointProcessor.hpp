#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"

#include <CGAL/hierarchy_simplify_point_set.h>
#include <CGAL/tags.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>

#include <CGAL/grid_simplify_point_set.h>
#include <CGAL/jet_smooth_point_set.h>

namespace tsr {

Point3 UTM_point_to_WGS84(Point3 pointUTM, short zone,
                          bool isNorthernHemisphere);
Point2 UTM_point_to_WGS84(Point2 pointUTM, short zone,
                          bool isNorthernHemisphere);

Point3 WGS84_point_to_UTM(Point3 pointWGS84);
Point2 WGS84_point_to_UTM(Point2 pointWGS84);

void jet_smooth_points(std::vector<Point3> &points);
void simplify_points(std::vector<Point3> &points);

void filter_points_domain(std::vector<Point3> &points, Point3 &source_point,
                          Point3 &target_point, double radii_multiplier);

double calculate_xy_distance(const Point3 &p1, const Point3 &p2);
double calculate_xy_angle(const Point3 &p1, const Point3 &p2);

} // namespace tsr