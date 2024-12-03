#include "tsr/IO/KMLWriter.hpp"
#include "tsr/Delaunay_3.hpp"
#include "tsr/PointProcessor.hpp"
#include "tsr/Point_3.hpp"
#include <boost/dynamic_bitset/dynamic_bitset.hpp>

namespace tsr::IO {

std::string generateKML(std::vector<Face_handle> &faces) {
  std::string kml;

  kml += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  kml += "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n";
  kml += "<Document>\n";
  kml += "  <name>3D Triangles</name>\n";
  // Define the blue style
  kml += "  <Style id=\"blueStyle\">\n";
  kml += "    <LineStyle>\n";
  kml += "      <color>ff0000ff</color> <!-- Blue -->\n";
  kml += "      <width>2</width>\n";
  kml += "    </LineStyle>\n";
  kml += "    <PolyStyle>\n";
  kml += "      <color>7f0000ff</color> <!-- Blue with transparency -->\n";
  kml += "      <fill>1</fill>\n";
  kml += "      <outline>1</outline>\n";
  kml += "    </PolyStyle>\n";
  kml += "  </Style>\n";

  for (uint f = 0; f < faces.size(); f++) {

    Face_handle face = faces[f];
    Point_3 p1_UTM = face->vertex(0)->point();
    Point_3 p2_UTM = face->vertex(1)->point();
    Point_3 p3_UTM = face->vertex(2)->point();

    auto p1 = UTM_point_to_WGS84(p1_UTM, 30, true);
    auto p2 = UTM_point_to_WGS84(p2_UTM, 30, true);
    auto p3 = UTM_point_to_WGS84(p3_UTM, 30, true);

    kml += "<Placemark>\n";
    kml += "  <name>Triangle " + std::to_string(f) + "</name>\n";
    kml += "  <styleUrl>#blueStyle</styleUrl>\n";
    kml += "  <Polygon>\n";
    kml += "    <altitudeMode>absolute</altitudeMode>\n"; // Ensure the altitude
                                                          // is interpreted as
                                                          // absolute height
                                                          // above sea level
    kml += "    <outerBoundaryIs>\n";
    kml += "      <LinearRing>\n";
    kml += "        <coordinates>\n";

    kml += "          " + std::to_string(p1.y()) + "," +
           std::to_string(p1.x()) + "," + std::to_string((p1.z() + 1)) + "\n";
    kml += "          " + std::to_string(p2.y()) + "," +
           std::to_string(p2.x()) + "," + std::to_string((p2.z() + 1)) + "\n";
    kml += "          " + std::to_string(p3.y()) + "," +
           std::to_string(p3.x()) + "," + std::to_string((p3.z() + 1)) + "\n";
    kml += "          " + std::to_string(p1.y()) + "," +
           std::to_string(p1.x()) + "," + std::to_string((p1.z() + 1)) + "\n";

    kml += "        </coordinates>\n";
    kml += "      </LinearRing>\n";
    kml += "    </outerBoundaryIs>\n";
    kml += "  </Polygon>\n";
    kml += "</Placemark>\n";
  }

  kml += "</Document>\n";
  kml += "</kml>\n";

  return kml;
}

} // namespace tsr::IO