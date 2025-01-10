#pragma once

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_2.h>
#include <CGAL/Default.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Triangulation_data_structure_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>

namespace tsr {

typedef CGAL::Exact_predicates_inexact_constructions_kernel TIN_K;
typedef CGAL::Exact_predicates_tag TIN_It;
typedef CGAL::Projection_traits_xy_3<TIN_K> TIN_Pt;

// Define the Delaunay triangulation
typedef CGAL::Constrained_Delaunay_triangulation_2<TIN_Pt, CGAL::Default,
                                                   TIN_It>
    Tin;

// Define commonly used features of the mesh
typedef Tin::Vertex_handle Vertex_handle;
typedef Tin::Face_handle Face_handle;

} // namespace tsr