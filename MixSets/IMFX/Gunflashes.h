/*
Credits for DK22Pac - IMFX
If you consider fixing something here, you should also consider fixing there: https://github.com/DK22Pac/imfx
*/
#pragma once
#include "plugin.h"
#include "game_sa\Fx_c.h"
#include "game_sa\CPed.h"
#include "game_sa\CTaskSimpleUseGun.h"
#include "game_sa\FxPrimBP_c.h"
#include <vector>

/*struct GunflashInfo {
    unsigned int weapId;
    char fxName[64];
    bool rotate;
    bool smoke;
};*/

class Gunflashes {
public:
    class PedExtension {
    public:
        bool bLeftHandGunflashThisFrame;
        bool bRightHandGunflashThisFrame;
        bool bInVehicle;
        RwMatrix* pMats[2];

        PedExtension(CPed*);
        ~PedExtension();

        void Reset();
    };

    //static RwMatrix matrixAry[20];
    static unsigned int matrixCounter;
    static plugin::PedExtendedData<PedExtension> pedExt;
    //static std::vector<GunflashInfo> gunflashInfos;
    static bool bLeftHand;
    static bool bVehicleGunflash;

    static void Setup();
    //static void ReadSettings();
    static void __fastcall MyTriggerGunflash(Fx_c* fx, int, CEntity* owner, CVector& origin, CVector& target, bool doGunflash);
    static void __fastcall DoDriveByGunflash(CPed* driver, int, int, bool leftHand);
    static bool __fastcall MyProcessUseGunTask(CTaskSimpleUseGun* task, int, CPed* ped);
    static void ProcessPerFrame();
    static void CreateGunflashEffectsForPed(CPed* ped);

    static void AddDefaultWeaponData();
    static void UpdateWeaponData(unsigned int weaponID, const std::string particle, bool rotate = true, bool smoke = true);

    static void SetBikePassengerOffsetFactor(const RwReal newValue);
    static void SetBikeDriverOffsetFactor(const RwReal newValue);
    static void SetCarDriverOffsetFactor(const RwReal newValue);
    static void SetCarPassengerOffsetFactor(const RwReal newValue);
    static void SetOnFootOffsetFactor(const RwReal newValue);
    static void SetSurfingOffsetFactor(const RwReal newValue);
    static void SetOnFootReverseFactor(const RwReal newValue);

    static void SetGunflashLowerLight(const bool newValue);
    static void SetSurfingTimeMult(const float value);
    static void SetJetpackTimeMult(const float value);
    static void SetInVehicleTimeMult(const float value);
    static void SetDualWeildingTimeMult(const float value);
    static void SetSingleWeaponWeildingTimeMult(const float value);
    static void SetSurfingSpeed(const float value);

    static void SetMopedFixOffset(const float newValue);
};