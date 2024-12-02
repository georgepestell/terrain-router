#pragma once

#include "tsr/Delaunay_3.hpp"
#include "tsr/Mesh.hpp"
#include "tsr/Point_2.hpp"
#include "tsr/Point_3.hpp"
#include <memory>

namespace tsr {

#define DEFAULT_COSINE_MAX_ANGLE_REGIONS 0.60
#define DEFAULT_MAX_DISTANCE_REGIONS 5.0
#define DEFAULT_COSINE_MAX_ANGLE_CORNERS 0.9
#define DEFAULT_MAX_DISTANCE_CORNERS 3.0

Delaunay_3 create_tin_from_points(std::vector<Point_3> &points,
                                  Point_3 source_point, Point_3 target_point,
                                  double radii);

Delaunay_3 create_tin_from_points(std::vector<Point_3> &points);

void convert_surface_mesh_to_tin(Mesh const &source, Delaunay_3 &target);

void convert_tin_to_surface_mesh(Delaunay_3 const &source, Mesh &target);

void simplify_tin(Delaunay_3 const &source_mesh, Delaunay_3 &target_mesh,
                  float cosine_max_angle_regions, float max_distance_regions,
                  float cosine_max_angle_corners, float max_distance_corners);

void simplify_tin(Delaunay_3 const &source_mesh, Delaunay_3 &target_mesh);

void add_contour_constraint(Delaunay_3 &dtm, std::vector<Point_2> contour,
                            double max_segment_length);

} // namespace tsr