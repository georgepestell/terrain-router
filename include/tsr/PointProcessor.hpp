#include "tsr/Point_3.hpp"

namespace tsr {

Point_3 UTM_point_to_WGS84(Point_3 pointUTM, short zone,
                           bool isNorthernHemisphere);

Point_3 WGS84_point_to_UTM(Point_3 pointWGS84);

}