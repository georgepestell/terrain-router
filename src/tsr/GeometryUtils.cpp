#include "tsr/GeometryUtils.hpp"
#include "tsr/Point3.hpp"

#include "tsr/Vector3.hpp"
#include <CGAL/Kernel/global_functions_3.h>

namespace tsr {

#define TIN_ID "tin"
#define DEM_URL_FORMAT                                                         \
  "https://portal.opentopography.org/API/"                                     \
  "globaldem?demtype=COP30&south={}&west={}&north="                            \
  "{}&east={}&outputFormat=GeoTiff&API_Key={}"

double InterpolateZ(const Point3 &p1, const Point3 &p2, const Point3 &p3,
                     const double x, const double y) {
  // Compute two edge vectors

  Vector3 v1 = p2 - p1;
  Vector3 v2 = p3 - p1;

  Vector3 normal = CGAL::cross_product(v1, v2);

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