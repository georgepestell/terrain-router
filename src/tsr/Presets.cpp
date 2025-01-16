/**
 * @file Presets.cpp
 * @author George Pestell
 * @brief Supplies FeatureManager model presets
 * @version 0.1
 * @date 2025-01-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "tsr/FeatureManager.hpp"
#include "tsr/Logging.hpp"
#include "tsr/MeshBoundary.hpp"
#include "tsr/Tin.hpp"

#include "tsr/Features/BoolWaterFeature.hpp"
#include "tsr/Features/CEHTerrainFeature.hpp"
#include "tsr/Features/ConditionalFeature.hpp"
#include "tsr/Features/ConstantFeature.hpp"
#include "tsr/Features/DistanceFeature.hpp"
#include "tsr/Features/GradientFeature.hpp"
#include "tsr/Features/GradientSpeedFeature.hpp"
#include "tsr/Features/InverseFeature.hpp"
#include "tsr/Features/MultiplierFeature.hpp"
#include "tsr/Features/PathFeature.hpp"

#include <memory>

namespace tsr {
FeatureManager SetupTimePreset(Tin &tin, const MeshBoundary &boundary) {

  // Feature Manager Configuration
  TSR_LOG_TRACE("Setting up feature manager");
  FeatureManager fm;

  TSR_LOG_DEBUG("Constructors");

  auto gradientFeature = std::make_shared<GradientFeature>("gradient");
  auto distance = std::make_shared<DistanceFeature>("distance");
  auto gradientSpeedInfluence =
      std::make_shared<GradientSpeedFeature>("gradient_speed");
  gradientSpeedInfluence->AddDependency(gradientFeature);

  auto terrainFeature =
      std::make_shared<CEHTerrainFeature>("terrain_type", 0.1);

  auto waterFeature = std::make_shared<BoolWaterFeature>("water", 0.1);

  auto pathFeature = std::make_shared<PathFeature>("paths", 0.05);

  auto waterSpeedInfluence =
      std::make_shared<InverseFeature<bool, bool>>("water_speed");
  waterSpeedInfluence->AddDependency(waterFeature);

  auto speedFeature = std::make_shared<MultiplierFeature>("speed");
  speedFeature->AddDependency(waterSpeedInfluence, MultiplierFeature::BOOL);
  speedFeature->AddDependency(terrainFeature, MultiplierFeature::DOUBLE);
  speedFeature->AddDependency(gradientSpeedInfluence,
                              MultiplierFeature::DOUBLE);

  auto pathSpeed = std::make_shared<MultiplierFeature>("path_speed");
  pathSpeed->AddDependency(gradientSpeedInfluence, MultiplierFeature::DOUBLE);

  auto speedWithPathFeature =
      std::make_shared<ConditionalFeature<double>>("speed_with_path");
  speedWithPathFeature->AddDependency(pathFeature);
  speedWithPathFeature->AddDependency(pathSpeed);
  speedWithPathFeature->AddDependency(speedFeature);

  auto inverseSpeedFeature =
      std::make_shared<InverseFeature<double, double>>("inverse_speed");
  inverseSpeedFeature->AddDependency(speedWithPathFeature);

  auto timeFeature = std::make_shared<MultiplierFeature>("time");
  timeFeature->AddDependency(distance, MultiplierFeature::DOUBLE);
  timeFeature->AddDependency(inverseSpeedFeature, MultiplierFeature::DOUBLE);

  fm.SetOutputFeature(timeFeature);

  TSR_LOG_TRACE("initializing features");

  TSR_LOG_DEBUG("Terrain");
  terrainFeature->Initialize(tin, boundary);
  TSR_LOG_DEBUG("Water");
  waterFeature->Initialize(tin, boundary);
  TSR_LOG_DEBUG("Paths");
  pathFeature->Initialize(tin, boundary);

  TSR_LOG_DEBUG("Tagging");
  terrainFeature->Tag(tin);
  waterFeature->Tag(tin);
  pathFeature->Tag(tin);

  //   /// DEBUG: Write watermap to KML
  waterFeature->WriteWaterToKml();
  pathFeature->WritePathsToKml();

  return fm;
}

FeatureManager SetupTimeWithSwimmingPreset(Tin &tin,
                                           const MeshBoundary &boundary) {

  // Feature Manager Configuration
  TSR_LOG_TRACE("Setting up feature manager");
  FeatureManager fm;

  TSR_LOG_DEBUG("Constructors");

  auto gradientFeature = std::make_shared<GradientFeature>("gradient");
  auto distance = std::make_shared<DistanceFeature>("distance");
  auto gradientSpeedInfluence =
      std::make_shared<GradientSpeedFeature>("gradient_speed");
  gradientSpeedInfluence->AddDependency(gradientFeature);

  auto terrainFeature =
      std::make_shared<CEHTerrainFeature>("terrain_type", 0.1);

  auto waterFeature = std::make_shared<BoolWaterFeature>("water", 0.1);

  auto pathFeature = std::make_shared<PathFeature>("paths", 0.05);

  auto swimSpeed = std::make_shared<ConstantFeature<double>>("swimSpeed", 0.89);
  auto noImpact = std::make_shared<ConstantFeature<double>>("swimSpeed", 1);
  auto waterSpeedInfluence =
      std::make_shared<ConditionalFeature<double>>("water_speed");
  // If water
  waterSpeedInfluence->AddDependency(waterFeature);
  // Then swimspeed
  waterSpeedInfluence->AddDependency(swimSpeed);
  // Else, don't impact speed
  waterSpeedInfluence->AddDependency(noImpact);

  auto speedFeature = std::make_shared<MultiplierFeature>("speed");
  speedFeature->AddDependency(waterSpeedInfluence, MultiplierFeature::DOUBLE);
  speedFeature->AddDependency(terrainFeature, MultiplierFeature::DOUBLE);
  speedFeature->AddDependency(gradientSpeedInfluence,
                              MultiplierFeature::DOUBLE);

  auto pathSpeed = std::make_shared<MultiplierFeature>("path_speed");
  pathSpeed->AddDependency(gradientSpeedInfluence, MultiplierFeature::DOUBLE);

  auto speedWithPathFeature =
      std::make_shared<ConditionalFeature<double>>("speed_with_path");
  speedWithPathFeature->AddDependency(pathFeature);
  speedWithPathFeature->AddDependency(pathSpeed);
  speedWithPathFeature->AddDependency(speedFeature);

  auto inverseSpeedFeature =
      std::make_shared<InverseFeature<double, double>>("inverse_speed");
  inverseSpeedFeature->AddDependency(speedWithPathFeature);

  auto timeFeature = std::make_shared<MultiplierFeature>("time");
  timeFeature->AddDependency(distance, MultiplierFeature::DOUBLE);
  timeFeature->AddDependency(inverseSpeedFeature, MultiplierFeature::DOUBLE);

  fm.SetOutputFeature(timeFeature);

  TSR_LOG_TRACE("initializing features");

  TSR_LOG_DEBUG("Terrain");
  terrainFeature->Initialize(tin, boundary);
  TSR_LOG_DEBUG("Water");
  waterFeature->Initialize(tin, boundary);
  TSR_LOG_DEBUG("Paths");
  pathFeature->Initialize(tin, boundary);

  TSR_LOG_DEBUG("Tagging");
  terrainFeature->Tag(tin);
  waterFeature->Tag(tin);
  pathFeature->Tag(tin);

  //   /// DEBUG: Write watermap to KML
  waterFeature->WriteWaterToKml();
  pathFeature->WritePathsToKml();

  return fm;
}
FeatureManager SetupTimeRestrictSwimmingPreset(Tin &tin,
                                               const MeshBoundary &boundary) {

  // Feature Manager Configuration
  TSR_LOG_TRACE("Setting up feature manager");
  FeatureManager fm;

  TSR_LOG_DEBUG("Constructors");

  auto gradientFeature = std::make_shared<GradientFeature>("gradient");
  auto distance = std::make_shared<DistanceFeature>("distance");
  auto gradientSpeedInfluence =
      std::make_shared<GradientSpeedFeature>("gradient_speed");
  gradientSpeedInfluence->AddDependency(gradientFeature);

  auto terrainFeature =
      std::make_shared<CEHTerrainFeature>("terrain_type", 0.1);

  auto waterFeature = std::make_shared<BoolWaterFeature>("water", 0.1);

  auto pathFeature = std::make_shared<PathFeature>("paths", 0.05);

  auto swimSpeed =
      std::make_shared<ConstantFeature<double>>("swimSpeed", 0.001);
  auto noImpact = std::make_shared<ConstantFeature<double>>("swimSpeed", 1);
  auto waterSpeedInfluence =
      std::make_shared<ConditionalFeature<double>>("water_speed");
  // If water
  waterSpeedInfluence->AddDependency(waterFeature);
  // Then swimspeed
  waterSpeedInfluence->AddDependency(swimSpeed);
  // Else, don't impact speed
  waterSpeedInfluence->AddDependency(noImpact);

  auto speedFeature = std::make_shared<MultiplierFeature>("speed");
  speedFeature->AddDependency(waterSpeedInfluence, MultiplierFeature::DOUBLE);
  speedFeature->AddDependency(terrainFeature, MultiplierFeature::DOUBLE);
  speedFeature->AddDependency(gradientSpeedInfluence,
                              MultiplierFeature::DOUBLE);

  auto pathSpeed = std::make_shared<MultiplierFeature>("path_speed");
  pathSpeed->AddDependency(gradientSpeedInfluence, MultiplierFeature::DOUBLE);

  auto speedWithPathFeature =
      std::make_shared<ConditionalFeature<double>>("speed_with_path");
  speedWithPathFeature->AddDependency(pathFeature);
  speedWithPathFeature->AddDependency(pathSpeed);
  speedWithPathFeature->AddDependency(speedFeature);

  auto inverseSpeedFeature =
      std::make_shared<InverseFeature<double, double>>("inverse_speed");
  inverseSpeedFeature->AddDependency(speedWithPathFeature);

  auto timeFeature = std::make_shared<MultiplierFeature>("time");
  timeFeature->AddDependency(distance, MultiplierFeature::DOUBLE);
  timeFeature->AddDependency(inverseSpeedFeature, MultiplierFeature::DOUBLE);

  fm.SetOutputFeature(timeFeature);

  TSR_LOG_TRACE("initializing features");

  TSR_LOG_DEBUG("Terrain");
  terrainFeature->Initialize(tin, boundary);
  TSR_LOG_DEBUG("Water");
  waterFeature->Initialize(tin, boundary);
  TSR_LOG_DEBUG("Paths");
  pathFeature->Initialize(tin, boundary);

  TSR_LOG_DEBUG("Tagging");
  terrainFeature->Tag(tin);
  waterFeature->Tag(tin);
  pathFeature->Tag(tin);

  //   /// DEBUG: Write watermap to KML
  waterFeature->WriteWaterToKml();
  pathFeature->WritePathsToKml();

  return fm;
}
} // namespace tsr