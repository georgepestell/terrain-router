#pragma once

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Projection_traits_xy_3.h>

namespace tsr {

// Define the Delaunay triangulation
typedef CGAL::Constrained_Delaunay_triangulation_2<
    CGAL::Projection_traits_xy_3<
        CGAL::Exact_predicates_inexact_constructions_kernel>,
    CGAL::Default, CGAL::Exact_predicates_tag>
    Delaunay_3;

// Define commonly used features of the mesh
typedef Delaunay_3::Vertex_handle Vertex_handle;
typedef Delaunay_3::Face_handle Face_handle;
} // namespace tsr