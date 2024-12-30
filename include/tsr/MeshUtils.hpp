#include "tsr/Mesh.hpp"

#include "tsr/Delaunay_3.hpp"
#include "tsr/MeshBoundary.hpp"

namespace tsr {

double interpolate_z(const Point_3 &p1, const Point_3 &p2, const Point_3 &p3,
                     const double x, const double y);

Delaunay_3 initializeMesh(MeshBoundary boundary, std::string api_key);

} // namespace tsr