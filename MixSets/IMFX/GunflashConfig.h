// GunflashConfig.h
#pragma once

class GunflashConfig {
public:
    struct Color { int r, g, b; };

    // constructor
    GunflashConfig();

    // getters
    const Color&  getUnderflashColor()         const noexcept;
    float         getUnderflashLightRange()    const noexcept;
    int           getUnderflashShadowID()      const noexcept;
    int           getUnderflashShadowIntensity() const noexcept;
    float         getUnderflashShadowRadius()  const noexcept;
    float         getUnderflashShadowAngle()   const noexcept;
    float         getUnderflashOffsetX()       const noexcept;
    float         getUnderflashOffsetY()       const noexcept;
    float         getUnderflashOffsetZ()       const noexcept;
    float         getSurfingOffsetFactor()     const noexcept;
    float         getSurfingTimeMult()         const noexcept;
    float         getPistolFixOffset()         const noexcept;
    float         getFpsFixTimeMult()          const noexcept;
    bool          computeFpsFix()              const noexcept;
    bool          isLocalParticleFixEnabled()  const noexcept;
    bool          isGunflashLowerLightEnabled() const noexcept;

    // FX name getters now return raw C strings
    const char* getDefaultGunflashParticleName() const noexcept;
    const char* getDefaultGunflashSmokeParticleName() const noexcept;

    // setters
    void setUnderflashColor(int r, int g, int b) noexcept;
    void setUnderflashLightRange(float range) noexcept;
    void setUnderflashShadow(int id, int intensity, float radius, float angle) noexcept;
    void setUnderflashOffset(float x, float y, float z) noexcept;
    void setSurfingOffsetFactor(float factor) noexcept;
    void setSurfingTimeMult(float mult) noexcept;
    void setPistolFixOffset(float offset) noexcept;
    void setFpsFixTimeMult(float mult) noexcept;
    void setComputeFpsFix(bool flag) noexcept;
    void setLocalParticleFix(bool flag) noexcept;
    void setGunflashLowerLight(bool flag) noexcept;

    // now takes C‑strings (no allocation)
    void setDefaultFxNames(const char* flash, const char* smoke) noexcept;

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

    // C‑string pointers only
    const char* defaultGunflashFx_;
    const char* defaultGunSmokeFx_;
};