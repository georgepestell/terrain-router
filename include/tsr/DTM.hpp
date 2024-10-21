#pragma once

#include <CGAL/Projection_traits_xy_3.h>
#define CGAL_PMP_USE_CERES_SOLVER

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_2.h>
#include <CGAL/Point_3.h>
#include <CGAL/Point_set_3.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Delaunay_triangulation_2.h>

#include <unordered_map>

using namespace std;

namespace tsr {

#define DEFAULT_COSINE_MAX_ANGLE_REGIONS 0.60
#define DEFAULT_MAX_DISTANCE_REGIONS 5.0
#define DEFAULT_COSINE_MAX_ANGLE_CORNERS 0.9
#define DEFAULT_MAX_DISTANCE_CORNERS 3.0

using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point_3 = CGAL::Point_3<Kernel>;
using Point_set_3 = CGAL::Point_set_3<Point_3>;
using Projection_traits = CGAL::Projection_traits_xy_3<Kernel>;
using Delaunay_3D = CGAL::Delaunay_triangulation_2<Projection_traits>;
using Delaunay_2D = CGAL::Delaunay_triangulation_2<Kernel>;
using Point_2 = CGAL::Point_2<Kernel>;
using Surface_mesh = CGAL::Surface_mesh<Point_3>;

class DTM {

private:
  unique_ptr<Delaunay_3D> topology_mesh = make_unique<Delaunay_3D>();

  unordered_map<string, unique_ptr<Delaunay_2D>> binary_features;

public:
  inline DTM(Point_set_3 points) { initialize_dtm(points); }

  void initialize_dtm(Point_set_3 topology_points);

  void simplify_3d_feature(Delaunay_3D const &source_mesh,
                           Delaunay_3D &target_mesh,
                           float cosine_max_angle_regions,
                           float max_distance_regions,
                           float cosine_max_angle_corners,
                           float max_distance_corners);
  void simplify_3d_feature(Delaunay_3D const &source_mesh,
                           Delaunay_3D &target_mesh);

  Delaunay_3D &get_topology() const;

  Delaunay_2D &get_binary_feature() const;
};

void convert_surface_mesh_to_delaunay(Surface_mesh const &source,
                                      Delaunay_3D &target);

unique_ptr<Delaunay_2D> create_binary_feature(const char *layer_id,
                                              vector<Point_2> &points);

} // namespace tsr