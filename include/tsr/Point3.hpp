#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_3.h>
#include <CGAL/Segment_3.h>

#include <string>

namespace tsr {
typedef CGAL::Point_3<CGAL::Exact_predicates_inexact_constructions_kernel>
    Point3;

std::string toString(const Point3 &p);

} // namespace tsr