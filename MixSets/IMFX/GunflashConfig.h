#pragma once
#include <string>

class GunflashConfig {
public:
    struct Color { int r, g, b; };

    // ctor: initializes all values to their defaults
    GunflashConfig();

    // getters
    const Color& getUnderflashColor() const;
    const float& getUnderflashLightRange() const;
    const int& getUnderflashShadowID() const;
    const int& getUnderflashShadowIntensity() const;
    const float& getUnderflashShadowRadius() const;
    const float& getUnderflashShadowAngle() const;
    const float& getUnderflashOffsetX() const;
    const float& getUnderflashOffsetY() const;
    const float& getUnderflashOffsetZ() const;
    const float& getSurfingOffsetFactor() const;
    const float& getSurfingTimeMult() const;
    const float& getPistolFixOffset() const;
    const float& getFpsFixTimeMult() const;
    const bool& computeFpsFix() const;
    const bool& isLocalParticleFixEnabled() const;
    const bool& isGunflashLowerLightEnabled() const;
    const std::string& getDefaultGunflashSmokeParticleName();
    const std::string& getDefaultGunflashParticleName();

    // setters
    void setUnderflashColor(int r, int g, int b);
    void setUnderflashLightRange(float range);
    void setUnderflashShadow(int id, int intensity, float radius, float angle);
    void setUnderflashOffset(float x, float y, float z);
    void setSurfingOffsetFactor(float factor);
    void setSurfingTimeMult(float mult);
    void setPistolFixOffset(float offset);
    void setFpsFixTimeMult(float mult);
    void setComputeFpsFix(bool flag);
    void setLocalParticleFix(bool flag);
    void setGunflashLowerLight(bool flag);
    void setDefaultFxNames(const std::string& flash, const std::string& smoke);

private:
    Color      underflashColor_;
    float      underflashLightRange_;
    int        underflashShadowID_;
    int        underflashShadowIntensity_;
    float      underflashShadowRadius_;
    float      underflashShadowAngle_;
    float      underflashOffsetX_;
    float      underflashOffsetY_;
    float      underflashOffsetZ_;
    float      surfingOffsetFactor_;
    float      surfingTimeMult_;
    float      pistolFixOffset_;
    float      fpsFixTimeMult_;
    bool       computeFpsFix_;
    bool       localParticleFix_;
    bool       gunflashLowerLight_;
    std::string defaultGunflashFx_;
    std::string defaultGunSmokeFx_;
};