#include <exception>
#include <gtest/gtest.h>

#include "tsr/API/Chunking.hpp"
#include "tsr/API/GDALHandler.hpp"
#include "tsr/IO/MapIO.hpp"

#include "tsr/logging.hpp"

#include <gdal/gdal.h>

#define OPENTOP_KEY "0f789809fed28dc634c8d75695d0cc5c"

using namespace tsr;

TEST(GDALHandlerTests, TestWarping) {

  GDALDatasetH dataset = API::getChunk(56.338445, -2.798822, OPENTOP_KEY);

  TSR_LOG_TRACE("Warping to UTM");
  GDALDatasetH warpedDataset;
  try {
    warpedDataset = API::warpWGS84DatasetToUTM(dataset);
  } catch (std::exception e) {
    GDALReleaseDataset(dataset);
    FAIL();
  }

  TSR_LOG_TRACE("Successfully warped");

  TSR_LOG_INFO("x size: {}", GDALGetRasterBandXSize(warpedDataset));

  GDALReleaseDataset(dataset);
  GDALReleaseDataset(warpedDataset);
}
