#pragma once

#include <CGAL/Surface_mesh.h>
#include <CGAL/Point_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <iostream>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;

typedef CGAL::Point_3<Kernel> Point_3;
typedef CGAL::Surface_mesh<Point_3> Mesh;

