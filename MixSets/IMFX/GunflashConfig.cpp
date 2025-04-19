#include "GunflashConfig.h"

// initialize all defaults
GunflashConfig::GunflashConfig()
    : underflashColor_{ 70, 55, 22 }
    , underflashLightRange_{ 3.5f }
    , underflashShadowID_{ 3 }
    , underflashShadowIntensity_{ 1 }
    , underflashShadowRadius_{ 2.5f }
    , underflashShadowAngle_{ 0.0f }
    , underflashOffsetX_{ 0.0f }
    , underflashOffsetY_{ 1.3f }
    , underflashOffsetZ_{ 1.0f }
    , surfingOffsetFactor_{ 0.0f }
    , surfingTimeMult_{ 1.0f }
    , pistolFixOffset_{ 0.005f }
    , fpsFixTimeMult_{ 1.0f }
    , computeFpsFix_{ true }
    , localParticleFix_{ false }
    , gunflashLowerLight_{ false }
    , defaultGunflashFx_{ "gunflash" }
    , defaultGunSmokeFx_{ "gunsmoke" }
{
}

// getters
const GunflashConfig::Color& GunflashConfig::getUnderflashColor() const { return underflashColor_; }
const float& GunflashConfig::getUnderflashLightRange() const { return underflashLightRange_; }
const int& GunflashConfig::getUnderflashShadowID() const { return underflashShadowID_; }
const int& GunflashConfig::getUnderflashShadowIntensity() const { return underflashShadowIntensity_; }
const float& GunflashConfig::getUnderflashShadowRadius() const { return underflashShadowRadius_; }
const float& GunflashConfig::getUnderflashShadowAngle() const { return underflashShadowAngle_; }
const float& GunflashConfig::getUnderflashOffsetX() const { return underflashOffsetX_; }
const float& GunflashConfig::getUnderflashOffsetY() const { return underflashOffsetY_; }
const float& GunflashConfig::getUnderflashOffsetZ() const { return underflashOffsetZ_; }
const float& GunflashConfig::getSurfingOffsetFactor() const { return surfingOffsetFactor_; }
const float& GunflashConfig::getSurfingTimeMult() const { return surfingTimeMult_; }
const float& GunflashConfig::getPistolFixOffset() const { return pistolFixOffset_; }
const float& GunflashConfig::getFpsFixTimeMult() const { return fpsFixTimeMult_; }
const bool& GunflashConfig::computeFpsFix() const { return computeFpsFix_; }
const bool& GunflashConfig::isLocalParticleFixEnabled() const { return localParticleFix_; }
const bool& GunflashConfig::isGunflashLowerLightEnabled() const { return gunflashLowerLight_; }
const std::string& GunflashConfig::getDefaultGunflashParticleName() { return defaultGunflashFx_; }
const std::string& GunflashConfig::getDefaultGunflashSmokeParticleName() { return defaultGunSmokeFx_; }

// setters
void GunflashConfig::setUnderflashColor(int r, int g, int b) {
    underflashColor_ = { r, g, b };
}
void GunflashConfig::setUnderflashLightRange(float range) {
    underflashLightRange_ = range;
}
void GunflashConfig::setUnderflashShadow(int id, int intensity, float radius, float angle) {
    underflashShadowID_ = id;
    underflashShadowIntensity_ = intensity;
    underflashShadowRadius_ = radius;
    underflashShadowAngle_ = angle;
}
void GunflashConfig::setUnderflashOffset(float x, float y, float z) {
    underflashOffsetX_ = x;
    underflashOffsetY_ = y;
    underflashOffsetZ_ = z;
}
void GunflashConfig::setSurfingOffsetFactor(float f) {
    surfingOffsetFactor_ = f;
}
void GunflashConfig::setSurfingTimeMult(float t) {
    surfingTimeMult_ = t;
}
void GunflashConfig::setPistolFixOffset(float f) {
    pistolFixOffset_ = f;
}
void GunflashConfig::setFpsFixTimeMult(float t) {
    fpsFixTimeMult_ = t;
}
void GunflashConfig::setComputeFpsFix(bool b) {
    computeFpsFix_ = b;
}
void GunflashConfig::setLocalParticleFix(bool b) {
    localParticleFix_ = b;
}
void GunflashConfig::setGunflashLowerLight(bool b) {
    gunflashLowerLight_ = b;
}
void GunflashConfig::setDefaultFxNames(const std::string& flash, const std::string& smoke) {
    defaultGunflashFx_ = flash;
    defaultGunSmokeFx_ = smoke;
}