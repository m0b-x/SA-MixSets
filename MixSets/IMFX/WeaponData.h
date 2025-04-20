#pragma once

struct WeaponData {
    const char* particleName;
    bool        rotate;
    bool        smoke;
    bool        underFlash;

    constexpr WeaponData(
        const char* pName = "gunflash",
        bool         r = true,
        bool         s = true,
        bool         u = true
    ) noexcept
        : particleName(pName)
        , rotate(r)
        , smoke(s)
        , underFlash(u)
    {
    }
};
