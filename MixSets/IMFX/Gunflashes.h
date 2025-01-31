/*
Credits for DK22Pac - IMFX
If you consider fixing something here, you should also consider fixing there: https://github.com/DK22Pac/imfx
*/

#pragma once
#include "plugin.h"
#include "game_sa\Fx_c.h"
#include "game_sa\CPed.h"
#include "game_sa\CTaskSimpleUseGun.h"

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

	static unsigned int matrixCounter;
	static plugin::PedExtendedData<PedExtension> pedExt;
	static bool bLeftHand;
	static bool bVehicleGunflash;

	// Setup and configuration
	static void Setup(bool sampFix);
	static void AddDefaultWeaponData();
	static void UpdateWeaponData(unsigned int weaponID, const std::string& particle, bool rotate, bool smoke);

	// Frame and effects processing
	static void ProcessPerFrame();
	static void CreateGunflashEffectsForPed(CPed* ped);

	// Gunflash handling
	static bool __fastcall MyProcessUseGunTask(CTaskSimpleUseGun* task, int, CPed* ped);
	static bool __fastcall MyProcessUseGunTaskSAMP(CTaskSimpleUseGun* task, int, CPed* ped);
	static void __fastcall DoDriveByGunflash(CPed* driver, int, int, bool leftHand);
	static void __fastcall MyTriggerGunflash(Fx_c* fx, int, CEntity* owner, CVector& origin, CVector& target, bool doGunflash);

	//Underflash Handling
	static void DrawUnderflash(CPed* ped, RwV3d& newOffset);
	static void DrawUnderflash(CPed* ped);

	// Settings configuration
	static void SetPistolFixOffset(const float newValue);
	static void SetUnderFlashLightRComponent(const int newValue);
	static void SetUnderFlashLightGComponent(const int newValue);
	static void SetUnderFlashLightBComponent(const int newValue);
	static void SetUnderflashLightRange(const float newValue);
	static void SetUnderflashShadowID(const int newValue);
	static void SetUnderflashShadowIntensity(const int newValue);
	static void SetUnderflashShadowRadius(const float newValue);
	static void SetUnderFlashShadowAngle(const float newValue);
	static void SetUnderflashOffsetX(const float newValue);
	static void SetUnderflashOffsetY(const float newValue);
	static void SetUnderflashOffsetZ(const float newValue);
	static void SetSurfingOffsetFactor(const RwReal newValue);
	static void SetLocalParticleFix(const bool newValue);
	static void SetGunflashLowerLight(const bool newValue);
	static void SetSurfingTimeMult(const float value);
	static void SetFpsFixTimeMult(const float newValue);
	static void SetFpsFixComputing(const bool newValue);

	//Utility
	static eTaskType GetPedActiveTask(CPed* ped);

	//Temporary, will be deleted
	static void ProcessGunflashLogicWithoutLocalParticles(
		CPed* ped,
		bool isInVehicle,
		bool isUsingJetpack,
		bool driverDriveby,
		bool isInBike,
		bool isInMoped,
		bool isDualWeilding,
		float& particleTimeMult,
		RwV3d& gunflashOffset,
		RwV3d& underflashOffset,
		int& boneIDToAttachTo,
		bool& attachedToBone,
		float& additionalOffsetX,
		float& additionalOffsetY,
		float& additionalOffsetZ,
		int handIndex,
		bool isLeftHand,
		float surfingOffsetFactor,
		float surfingTimeMult,
		float fpsFixTimeMult,
		float pistolFixOffset);
};
