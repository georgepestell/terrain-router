#define OPENTOP_KEY "0f789809fed28dc634c8d75695d0cc5c"

#include <exception>
#include <gtest/gtest.h>

#include "tsr/API/Chunking.hpp"
#include "tsr/API/GDALHandler.hpp"
#include "tsr/IO/MapIO.hpp"

#include "tsr/Logging.hpp"

#include <gdal/gdal.h>

using namespace tsr;

TEST(GDALHandlerTests, TestWarping) {

  double minLat, minLng, maxLat, maxLng;

  API::getChunkBoundaries(56.338445, -2.798822, minLat, minLng, maxLat, maxLng);

  GDALDatasetH dataset = API::getChunk(minLat, minLng, maxLat, maxLng);

  TSR_LOG_TRACE("Warping to UTM");
  GDALDatasetH warpedDataset;
  try {
    warpedDataset = API::warpWGS84DatasetToUTM(dataset);
  } catch (std::exception e) {
    GDALReleaseDataset(dataset);
    FAIL();
  }

  GDALDataset *d = (GDALDataset *)warpedDataset;

  TSR_LOG_TRACE("Successfully warped");
  TSR_LOG_INFO("x size: {}", d->GetRasterXSize());

  GDALReleaseDataset(dataset);
  GDALReleaseDataset(warpedDataset);
}
