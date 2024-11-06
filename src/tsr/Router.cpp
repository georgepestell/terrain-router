#include "tsr/Router.hpp"
#include "tsr/Delaunay_3.hpp"

#include <limits>
#include <memory>
#include <vector>

namespace tsr {

Router::Router(Delaunay_3 &dtm) {
  this->dtm = std::make_unique<Delaunay_3>(dtm);

  // Calculate the constant weights for each face
  for (auto face = this->dtm->finite_faces_begin();
       face != this->dtm->finite_faces_end(); face++) {
    this->constant_weights[face] = this->calculate_constant_face_cost(face);
  }
}

std::vector<Point_3> Router::calculate_route(Vertex_handle &start,
                                             Vertex_handle &end) {

  // Initialize the route with the start point
  std::vector<Point_3> route;
  route.push_back(start->point());

  Vertex_handle current_vertex = start;

  while (current_vertex != end) {
    // TODO: Implement interpolation
    if (false) {

    } else {
      //// Direct route

      // Get the direct cost to travel to each vertex of adjacent faces
      Vertex_handle bestVertex = nullptr;
      double minCost = std::numeric_limits<double>::infinity();
      for (auto face_circulator = current_vertex->incident_faces();
           !this->dtm->is_infinite(face_circulator); face_circulator++) {

        Face_handle face = face_circulator;

        for (int i = 0; i < 3; i++) {
          Vertex_handle v = face->vertex(i);
          if (v != current_vertex && !this->nodes[v].visited) {
            double tmp_cost =
                this->calculate_trivial_cost(face, v, current_vertex);
            if (tmp_cost < minCost) {
              bestVertex = v;
              minCost = tmp_cost;
            }
          }
        }
      }

      // We have the best vertex to travel to
      route.push_back(bestVertex->point());
      current_vertex = bestVertex;
      this->nodes[current_vertex].visited = true;
    }
  }

  return route;
}

} // namespace tsr