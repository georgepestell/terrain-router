#pragma once

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_2.h>
#include <CGAL/Projection_traits_xy_3.h>

namespace tsr {
typedef CGAL::Constrained_Delaunay_triangulation_2<CGAL::Projection_traits_xy_3<CGAL::Exact_predicates_inexact_constructions_kernel>, CGAL::Default, CGAL::Exact_predicates_tag> Delaunay_3;
}