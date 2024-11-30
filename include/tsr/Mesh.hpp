#pragma once

#include <CGAL/Surface_mesh.h>

#include "tsr/Point_3.hpp"

namespace tsr {
typedef CGAL::Surface_mesh<Point_3> Mesh;

double interpolate_z(const Point_3 &p1, const Point_3 &p2, const Point_3 &p3,
                     const double x, const double y);

} // namespace tsr
