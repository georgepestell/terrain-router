#include "tsr/MeshUtils.hpp"
#include "tsr/Point_3.hpp"

namespace tsr {

double interpolate_z(const Point_3 &p1, const Point_3 &p2, const Point_3 &p3,
                     const double x, const double y) {
  // Compute two edge vectors

  typedef CGAL::Vector_3<CGAL::Exact_predicates_inexact_constructions_kernel>
      Vector_3;

  Vector_3 v1 = p2 - p1;
  Vector_3 v2 = p3 - p1;

  Vector_3 normal = CGAL::cross_product(v1, v2);

  // The plane equation is A * x + B * y + C * z + D = 0
  // Where (A, B, C) is the normal vector
  double A = normal[0];
  double B = normal[1];
  double C = normal[2];

  // Compute D using one of the triangle vertices (e.g., p1)
  double D = -(A * p1[0] + B * p1[1] + C * p1[2]);

  // Now we can solve for z: z = (-A * x - B * y - D) / C
  return (-A * x - B * y - D) / C;
}

} // namespace tsr