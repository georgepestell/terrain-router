#include "TerrainParser.hpp"

using namespace std;
using namespace GEOM_FADE25D;

namespace terrainParserLib {

void parseFile(string filepath) {

  /**
   * @brief For each cell/DEPTH in the ASCIIGrid file, we take a measurement,
   * and use that as the centrepoint of the triangle.
   *
   */
  cout << filepath << endl;

  // Triangulate

  Fade_2D dt;
  dt.setFastMode(true);
  dt.setNumCPU(0);

  CloudPrepare cloudPrep;

  /** DEBUG BEGIN
   * - Testing fade25d 2.5D Delaunay Triangulation Library
   */

  vector<Point2> vPointsOut;

  ifstream file(filepath);

  if (!file.is_open()) {
    cerr << "Error: Couldn't open file: " << filepath.c_str() << endl;
    return;
  }

  // Get the first 5 info lines
  string configLines[5];
  for (int i = 0; i < 5; i++) {
    getline(file, configLines[i]);
  }

  // Get the cell size as double
  const char *end =
      strchr(configLines[4].c_str(), 0); // find the terminating null
  double cellSize;
  from_chars(configLines[4].c_str() + 13, end, cellSize);
  cellSize *= 100000; // Convert to metres
  cerr << "cellsize: " << cellSize << endl;

  // Fetch the start

  // Parse each row
  string line;
  int row = 0;
  vector<Point2> vInputPoints;
  while (getline(file, line)) {
    std::stringstream ss(line);

    string valueString;
    int col = 0;
    while (getline(ss, valueString, ' ')) {

      // Parse column value to float
      const char *end = strchr(valueString.c_str(), 0);

      // Save memory space by ignoring floating-point precision
      short shortValue;
      from_chars_result result =
          from_chars(valueString.c_str(), end, shortValue);

      if (result.ec != errc{}) { // || strcmp(result.ptr, end) != 0)
        cerr << "result.ptr:" << result.ptr << ", end:" << end << endl;
        ;
        cerr << "Error: Couldn't convert value to short: " << valueString
             << endl;
        cerr << "row: " << row << "col: " << col << endl;
        return;
      }

      Point2 point(row * cellSize, col * cellSize, shortValue);

      vInputPoints.push_back(point);

      col++;
    }
    cout << "row[" << row << "] = " << col << endl;
    ;
    row++;
    line.clear();
  };

  file.close();

  cloudPrep.add(vInputPoints);

  double maxDiffZ(2.0);         // z-tolerance per point group
  SumStrategy sms(SMS_AVERAGE); // or SMS_MEDIAN, SMS_MINIMUM, SMS_MAXIMUM
  ConvexHullStrategy chs(CHS_MAXHULL); // or CHS_NONE, CHS_MINHULL
  cloudPrep.adaptiveSimplify(maxDiffZ, sms, chs, false); // bDryRun=false

  dt.insert(&cloudPrep, true);

  Zone2 *pGlobalZone(dt.createZone(NULL, ZL_GLOBAL));
  pGlobalZone->smoothing(2);

  FadeExport fadeExport;
  bool bCustomIndices(true);
  bool bClear(true);
  dt.exportTriangulation(fadeExport, bCustomIndices, bClear);

  // fadeExport.print();
  fadeExport.writeObj("triangulation.obj");

  /** DEBUG END */
}

} // namespace terrainParserLib

int main(int argc, char *args[]) {

  // Pass absolute path in directly, or fetch the PWD
  if (argc == 2 && args[1][0] == '/') {
    // Absolute path
    terrainParserLib::parseFile((string)args[1]);
  } else {

    // Relative path

    // Use default if no argument given
    string relative_filepath = (argc == 1) ? DEFAULT_TERRAIN_FILE : args[1];

    filesystem::path fp = filesystem::path(filesystem::current_path());

    fp.append(relative_filepath);

    terrainParserLib::parseFile(fp);
  }

  return 0;
}
