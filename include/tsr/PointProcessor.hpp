#include "tsr/Point2.hpp"
#include "tsr/Point3.hpp"

#include <CGAL/hierarchy_simplify_point_set.h>
#include <CGAL/tags.h>
#include <CGAL/wlop_simplify_and_regularize_point_set.h>

#include <CGAL/grid_simplify_point_set.h>
#include <CGAL/jet_smooth_point_set.h>

namespace tsr {

Point3 TranslateUtmPointToWgs84(Point3 pointUTM, short zone,
                          bool isNorthernHemisphere);
Point2 TranslateUtmPointToWgs84(Point2 pointUTM, short zone,
                          bool isNorthernHemisphere);

Point3 TranslateWgs84PointToUtm(Point3 pointWGS84);
Point2 TranslateWgs84PointToUtm(Point2 pointWGS84);

void JetSmoothPoints(std::vector<Point3> &points);
void SimplifyPoints(std::vector<Point3> &points);

double CalculateXYDistance(const Point3 &p1, const Point3 &p2);
double CalculateXYAngle(const Point3 &p1, const Point3 &p2);

} // namespace tsr