// GunflashConfig.cpp
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
const GunflashConfig::Color& GunflashConfig::getUnderflashColor() const noexcept {
    return underflashColor_;
}
float GunflashConfig::getUnderflashLightRange() const noexcept {
    return underflashLightRange_;
}
int   GunflashConfig::getUnderflashShadowID() const noexcept {
    return underflashShadowID_;
}
int   GunflashConfig::getUnderflashShadowIntensity() const noexcept {
    return underflashShadowIntensity_;
}
float GunflashConfig::getUnderflashShadowRadius() const noexcept {
    return underflashShadowRadius_;
}
float GunflashConfig::getUnderflashShadowAngle() const noexcept {
    return underflashShadowAngle_;
}
float GunflashConfig::getUnderflashOffsetX() const noexcept {
    return underflashOffsetX_;
}
float GunflashConfig::getUnderflashOffsetY() const noexcept {
    return underflashOffsetY_;
}
float GunflashConfig::getUnderflashOffsetZ() const noexcept {
    return underflashOffsetZ_;
}
float GunflashConfig::getSurfingOffsetFactor() const noexcept {
    return surfingOffsetFactor_;
}
float GunflashConfig::getSurfingTimeMult() const noexcept {
    return surfingTimeMult_;
}
float GunflashConfig::getPistolFixOffset() const noexcept {
    return pistolFixOffset_;
}
float GunflashConfig::getFpsFixTimeMult() const noexcept {
    return fpsFixTimeMult_;
}
bool  GunflashConfig::computeFpsFix() const noexcept {
    return computeFpsFix_;
}
bool  GunflashConfig::isLocalParticleFixEnabled() const noexcept {
    return localParticleFix_;
}
bool  GunflashConfig::isGunflashLowerLightEnabled() const noexcept {
    return gunflashLowerLight_;
}

// FX-name getters now return const char*
const char* GunflashConfig::getDefaultGunflashParticleName() const noexcept {
    return defaultGunflashFx_;
}
const char* GunflashConfig::getDefaultGunflashSmokeParticleName() const noexcept {
    return defaultGunSmokeFx_;
}

// setters
void GunflashConfig::setUnderflashColor(int r, int g, int b) noexcept {
    underflashColor_ = { r, g, b };
}
void GunflashConfig::setUnderflashLightRange(float range) noexcept {
    underflashLightRange_ = range;
}
void GunflashConfig::setUnderflashShadow(int id, int intensity, float radius, float angle) noexcept {
    underflashShadowID_ = id;
    underflashShadowIntensity_ = intensity;
    underflashShadowRadius_ = radius;
    underflashShadowAngle_ = angle;
}
void GunflashConfig::setUnderflashOffset(float x, float y, float z) noexcept {
    underflashOffsetX_ = x;
    underflashOffsetY_ = y;
    underflashOffsetZ_ = z;
}
void GunflashConfig::setSurfingOffsetFactor(float f) noexcept {
    surfingOffsetFactor_ = f;
}
void GunflashConfig::setSurfingTimeMult(float t) noexcept {
    surfingTimeMult_ = t;
}
void GunflashConfig::setPistolFixOffset(float f) noexcept {
    pistolFixOffset_ = f;
}
void GunflashConfig::setFpsFixTimeMult(float t) noexcept {
    fpsFixTimeMult_ = t;
}
void GunflashConfig::setComputeFpsFix(bool b) noexcept {
    computeFpsFix_ = b;
}
void GunflashConfig::setLocalParticleFix(bool b) noexcept {
    localParticleFix_ = b;
}
void GunflashConfig::setGunflashLowerLight(bool b) noexcept {
    gunflashLowerLight_ = b;
}

// now takes C‑strings only—no allocations
void GunflashConfig::setDefaultFxNames(const char* flash, const char* smoke) noexcept {
    defaultGunflashFx_ = flash;
    defaultGunSmokeFx_ = smoke;
}
