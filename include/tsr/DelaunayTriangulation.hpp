#pragma once

#include "tsr/MeshBoundary.hpp"
#include "tsr/Point3.hpp"
#include "tsr/SurfaceMesh.hpp"
#include "tsr/Tin.hpp"
#include <utility>

namespace tsr {

#define DEFAULT_COSINE_MAX_ANGLE_REGIONS 0.60
#define DEFAULT_MAX_DISTANCE_REGIONS 5.0
#define DEFAULT_COSINE_MAX_ANGLE_CORNERS 0.9
#define DEFAULT_MAX_DISTANCE_CORNERS 3.0

Tin InitializeTinFromBoundary(
    MeshBoundary boundary, std::string api_key,
    std::string url_format = "https://portal.opentopography.org/API/"
                             "globaldem?demtype=COP30&south={}&west={}&north={}"
                             "&east={}&outputFormat=GeoTiff&API_Key={}");

void MergeTinPointsInBoundary(MeshBoundary &boundary, const Tin &srcTIN,
                              Tin &dstTIN);

Tin CreateTinFromPoints(std::vector<Point3> &points, Point3 source_point,
                        Point3 target_point, double radii);

Tin CreateTinFromPoints(const std::vector<Point3> &points);

void ConvertSurfaceMeshToTin(SurfaceMesh const &source, Tin &target);

void ConvertTinToSurfaceMesh(Tin const &source, SurfaceMesh &target);

void SimplifyTin(Tin const &source_mesh, Tin &target_mesh,
                 float cosine_max_angle_regions, float max_distance_regions,
                 float cosine_max_angle_corners, float max_distance_corners);

void SimplifyTin(Tin const &source_mesh, Tin &target_mesh);

std::set<std::pair<Point3, Point3>>
AddContourConstraint(Tin &tin, std::vector<Point2> contour,
                     double max_segment_length);

} // namespace tsr