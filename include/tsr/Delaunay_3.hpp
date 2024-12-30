#pragma once

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_2.h>
#include <CGAL/Default.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Delaunay_mesh_face_base_2.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/Triangulation_data_structure_2.h>

namespace tsr {

typedef CGAL::Exact_predicates_inexact_constructions_kernel D3_K;
typedef CGAL::Exact_predicates_tag D3_It;
typedef CGAL::Projection_traits_xy_3<D3_K> D3_Pt;

// Define the Delaunay triangulation
typedef CGAL::Constrained_Delaunay_triangulation_2<D3_Pt, CGAL::Default, D3_It>
    Delaunay_3;

// Define commonly used features of the mesh
typedef Delaunay_3::Vertex_handle Vertex_handle;
typedef Delaunay_3::Face_handle Face_handle;
} // namespace tsr