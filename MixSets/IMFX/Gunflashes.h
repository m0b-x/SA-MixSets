/*
Credits for DK22Pac - IMFX
If you consider fixing something here, you should also consider fixing there: https://github.com/DK22Pac/imfx
*/

#pragma once
#include "plugin.h"
#include "game_sa\Fx_c.h"
#include "game_sa\CPed.h"
#include "game_sa\CTaskSimpleUseGun.h"
#include "GunflashConfig.h"

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
	static void UpdateWeaponData(unsigned int weaponID, const char* particle, bool rotate, bool smoke, bool underFlash);

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

	//Temporary, will be deleted
	static void ProcessGunflashLogicWithoutLocalParticles(
		CPed* ped,
		bool isInVehicle,
		bool isUsingJetpack,
		bool driverDriveby,
		CTask* task,
		eTaskType taskType,
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
		bool isLeftHand);

	// Config Setters
	static void SetLocalParticleFix(bool value);
	static void SetGunflashLowerLight(bool value);
	static void SetUnderFlashLightRComponent(int r);
	static void SetUnderFlashLightGComponent(int g);
	static void SetUnderFlashLightBComponent(int b);
	static void SetUnderflashLightRange(float range);
	static void SetUnderflashShadowID(int id);
	static void SetUnderflashShadowIntensity(int intensity);
	static void SetUnderflashShadowRadius(float radius);
	static void SetUnderFlashShadowAngle(float angle);
	static void SetUnderflashOffsetX(float x);
	static void SetUnderflashOffsetY(float y);
	static void SetUnderflashOffsetZ(float z);
	static void SetPistolFixOffset(float f);
	static void SetSurfingOffsetFactor(float f);
	static void SetFpsFixTimeMult(float f);
	static void SetFpsFixComputing(bool b);
	static void SetSurfingTimeMult(float f);

};
