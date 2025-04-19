/*
Credits for DK22Pac - IMFX
If you consider fixing something here, you should also consider fixing there: https://github.com/DK22Pac/imfx
*/

#include "Gunflashes.h"
#include "game_sa\FxManager_c.h"
#include "game_sa\common.h"
#include "game_sa\CWeaponInfo.h"
#include "game_sa\CGeneral.h"
#include "game_sa\CTimer.h"

#include "../MixSets.h"
#include <string>

/*
#include <Windows.h>
#include <shlobj.h>
*/

/*
#include <CAnimManager.h>
#include <CAnimBlendAssociation.h>
#include <CAnimBlendHierarchy.h>
#include <CAnimBlock.h>
#include <CAnimBlendAssocGroup.h>
*/
#include "extensions\ScriptCommands.h"
#include "ePedBones.h"
#include "IMFX\GunflashConfig.h"
#include <IMFX\WeaponData.h>

using namespace plugin;

// Constants
const bool isLeftHand[2] = { true, false };
const ePedBones pedHands[2] = { BONE_LEFTWRIST, BONE_RIGHTWRIST };

const int BikeAppereance = 2;
const int weaponArraySize = WEAPON_MINIGUN + 1;

// Gunflashes
bool Gunflashes::bLeftHand = false;
bool Gunflashes::bVehicleGunflash = false;
static GunflashConfig g_gunflashConfig;


// PedExtension implementation & data
Gunflashes::PedExtension::PedExtension(CPed*) {
	bLeftHandGunflashThisFrame = false;
	bRightHandGunflashThisFrame = false;
	bInVehicle = false;
	pMats[0] = nullptr;
	pMats[1] = nullptr;
	Reset();
}

void Gunflashes::PedExtension::Reset() {
	bLeftHandGunflashThisFrame = false;
	bRightHandGunflashThisFrame = false;
	bInVehicle = false;
	pMats[0] = nullptr;
	pMats[1] = nullptr;
}

Gunflashes::PedExtension::~PedExtension() {
	if (pMats[0] != nullptr) delete pMats[0];
	if (pMats[1] != nullptr) delete pMats[1];
}
PedExtendedData<Gunflashes::PedExtension> Gunflashes::pedExt;

// Weapon data structure
WeaponData weaponArray[weaponArraySize];
inline static WeaponData const defaultWeaponDataObj;

// Config Setters
void Gunflashes::SetLocalParticleFix(bool value) { g_gunflashConfig.setLocalParticleFix(value); }
void Gunflashes::SetGunflashLowerLight(bool value) { g_gunflashConfig.setGunflashLowerLight(value); }
void Gunflashes::SetUnderFlashLightRComponent(int r) { g_gunflashConfig.setUnderflashColor(r, g_gunflashConfig.getUnderflashColor().g, g_gunflashConfig.getUnderflashColor().b); }
void Gunflashes::SetUnderFlashLightGComponent(int g) { g_gunflashConfig.setUnderflashColor(g_gunflashConfig.getUnderflashColor().r, g, g_gunflashConfig.getUnderflashColor().b); }
void Gunflashes::SetUnderFlashLightBComponent(int b) { g_gunflashConfig.setUnderflashColor(g_gunflashConfig.getUnderflashColor().r, g_gunflashConfig.getUnderflashColor().g, b); }
void Gunflashes::SetUnderflashLightRange(float range) { g_gunflashConfig.setUnderflashLightRange(range); }
void Gunflashes::SetUnderflashShadowID(int id) { g_gunflashConfig.setUnderflashShadow(id, g_gunflashConfig.getUnderflashShadowIntensity(), g_gunflashConfig.getUnderflashShadowRadius(), g_gunflashConfig.getUnderflashShadowAngle()); }
void Gunflashes::SetUnderflashShadowIntensity(int intensity) { g_gunflashConfig.setUnderflashShadow(g_gunflashConfig.getUnderflashShadowID(), intensity, g_gunflashConfig.getUnderflashShadowRadius(), g_gunflashConfig.getUnderflashShadowAngle()); }
void Gunflashes::SetUnderflashShadowRadius(float radius) { g_gunflashConfig.setUnderflashShadow(g_gunflashConfig.getUnderflashShadowID(), g_gunflashConfig.getUnderflashShadowIntensity(), radius, g_gunflashConfig.getUnderflashShadowAngle()); }
void Gunflashes::SetUnderFlashShadowAngle(float angle) { g_gunflashConfig.setUnderflashShadow(g_gunflashConfig.getUnderflashShadowID(), g_gunflashConfig.getUnderflashShadowIntensity(), g_gunflashConfig.getUnderflashShadowRadius(), angle); }
void Gunflashes::SetUnderflashOffsetX(float x) { g_gunflashConfig.setUnderflashOffset(x, g_gunflashConfig.getUnderflashOffsetY(), g_gunflashConfig.getUnderflashOffsetZ()); }
void Gunflashes::SetUnderflashOffsetY(float y) { g_gunflashConfig.setUnderflashOffset(g_gunflashConfig.getUnderflashOffsetX(), y, g_gunflashConfig.getUnderflashOffsetZ()); }
void Gunflashes::SetUnderflashOffsetZ(float z) { g_gunflashConfig.setUnderflashOffset(g_gunflashConfig.getUnderflashOffsetX(), g_gunflashConfig.getUnderflashOffsetY(), z); }
void Gunflashes::SetPistolFixOffset(float f) { g_gunflashConfig.setPistolFixOffset(f); }
void Gunflashes::SetSurfingOffsetFactor(float f) { g_gunflashConfig.setSurfingOffsetFactor(f); }
void Gunflashes::SetFpsFixTimeMult(float f) { g_gunflashConfig.setFpsFixTimeMult(f); }
void Gunflashes::SetFpsFixComputing(bool b) { g_gunflashConfig.setComputeFpsFix(b); }
void Gunflashes::SetSurfingTimeMult(float f) { g_gunflashConfig.setSurfingTimeMult(f); }

// Enums for Bikes
enum Bikes : unsigned int {
	PIZZABOY = 448,
	FAGGIO = 462,
	FREEWAY = 463,
	SANCHEZ = 468,
	QUAD = 471,
	BMX = 481,
	BIKE = 509,
	MBIKE = 510,
	FCR = 521,
	NRG = 522,
	PCJ = 461,
	BF = 581,
	HPV = 523,
	WAYFARER = 586
};

static constexpr unsigned int BIKE_DRIVE_DB_ANIM_IDS[][3] = {
	{0, 0, 0},
	{0, 0, 0},
	{ANIM_BIKES_BIKES_DRIVEBYFT, ANIM_BIKES_BIKES_DRIVEBYLHS, ANIM_BIKES_BIKES_DRIVEBYRHS}, // Group 2
	{ANIM_BIKEV_BIKEV_DRIVEBYFT, ANIM_BIKEV_BIKEV_DRIVEBYLHS, ANIM_BIKEV_BIKEV_DRIVEBYRHS}, // Group 3
	{ANIM_BIKEH_BIKEH_DRIVEBYFT, ANIM_BIKEH_BIKEH_DRIVEBYLHS, ANIM_BIKEH_BIKEH_DRIVEBYRHS}, // Group 4
	{ANIM_BIKED_BIKED_DRIVEBYRHS, ANIM_BIKED_BIKED_DRIVEBYLHS, ANIM_BIKED_BIKED_DRIVEBYFT}, // Group 5
	{ANIM_WAYFARER_WF_DRIVEBYFT, ANIM_WAYFARER_WF_DRIVEBYLHS, ANIM_WAYFARER_WF_DRIVEBYRHS}, // Group 6
	{ANIM_BMX_BMX_DRIVEBYFT, ANIM_BMX_BMX_DRIVEBY_LHS, ANIM_BMX_BMX_DRIVEBY_RHS},           // Group 7
	{ANIM_MTB_MTB_DRIVEBYFT, ANIM_MTB_MTB_DRIVEBY_LHS, ANIM_MTB_MTB_DRIVEBY_RHS},           // Group 8
	{ANIM_CHOPPA_CHOPPA_DRIVEBYFT, ANIM_CHOPPA_CHOPPA_DRIVEBY_LHS, ANIM_CHOPPA_CHOPPA_DRIVEBY_RHS}, // Group 9
	{ANIM_QUAD_QUAD_DRIVEBY_FT, ANIM_QUAD_QUAD_DRIVEBY_LHS, ANIM_QUAD_QUAD_DRIVEBY_RHS}     // Group 10
};

enum DriveByAnimIndex {
	DRIVEBY_FRONT = 0,
	DRIVEBY_LEFT = 1,
	DRIVEBY_RIGHT = 2
};

void Gunflashes::AddDefaultWeaponData()
{
	for (int weaponID = WEAPON_PISTOL; weaponID <= WEAPON_SNIPERRIFLE; ++weaponID) {
		weaponArray[weaponID] = WeaponData();
	}
	weaponArray[WEAPON_MINIGUN] = WeaponData();
}


void Gunflashes::UpdateWeaponData(unsigned int weaponID, const std::string& particle, bool rotate, bool smoke, bool underFlash)
{
	if (weaponID < weaponArraySize)
	{
		weaponArray[weaponID] = WeaponData(particle, rotate, smoke, underFlash);
	}
}


void Gunflashes::Setup(bool sampFix)
{
	patch::SetUChar(0x741353, 0); // Add gunflash for cuntgun

	patch::Nop(0x73306D, 9); // Remove default gunflashes (brightHand = 0)
	patch::Nop(0x7330FF, 9); // Remove default gunflashes (bRightHand = 1)
	patch::SetUShort(0x5DF425, 0xE990); // Remove default gunflashes 

	patch::RedirectCall(0x742299, DoDriveByGunflash);
	patch::RedirectJump(0x4A0DE0, MyTriggerGunflash);

	patch::SetPointer(0x86D744, (sampFix) ? MyProcessUseGunTaskSAMP : MyProcessUseGunTask);

	AddDefaultWeaponData();
	//ReadSettings();
}

void Gunflashes::ProcessPerFrame() {
	auto& config = g_gunflashConfig;

	if (config.computeFpsFix())
	{
		const float fps = CTimer::game_FPS;
		config.setFpsFixTimeMult(fps / 30.0f);
		config.setSurfingOffsetFactor(-fps / 60.0f + 2.0f);
	}

	for (int i = 0; i < CPools::ms_pPedPool->m_nSize; i++) {
		CPed* ped = CPools::ms_pPedPool->GetAt(i);
		if (ped)
			pedExt.Get(ped).Reset();
	}
}

bool __fastcall Gunflashes::MyProcessUseGunTaskSAMP(CTaskSimpleUseGun* task, int, CPed* ped)
{
	//disabled for sa-mp
	//if (task->m_pWeaponInfo == CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, ped->GetWeaponSkill()))
	{
		if (task->bRightHand) {
			bLeftHand = false;
			CallMethod<0x61EB10>(task, ped, false);
		}
		if (task->bLefttHand) {
			bLeftHand = true;
			CallMethod<0x61EB10>(task, ped, true);
			bLeftHand = false;
		}
		//*reinterpret_cast<unsigned char *>(&task->m_nFlags) = 0;
		task->bRightHand = false;
		task->bLefttHand = false;
	}
	return 0;
}

bool __fastcall Gunflashes::MyProcessUseGunTask(CTaskSimpleUseGun* task, int, CPed* ped)
{
	if (task->m_pWeaponInfo == CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, ped->GetWeaponSkill()))
	{
		if (task->bRightHand) {
			bLeftHand = false;

			CallMethod<0x61EB10>(task, ped, false);
		}
		if (task->bLefttHand) {
			bLeftHand = true;
			CallMethod<0x61EB10>(task, ped, true);
			bLeftHand = false;
		}
		//*reinterpret_cast<unsigned char *>(&task->m_nFlags) = 0;
		task->bRightHand = false;
		task->bLefttHand = false;
	}
	return 0;
}

void __fastcall Gunflashes::DoDriveByGunflash(CPed* driver, int, int, bool leftHand) {
	bLeftHand = leftHand;
	bVehicleGunflash = true;
	CVector origin(0.0f, 0.0f, 0.0f);
	CVector target(0.0f, 0.0f, 0.0f);
	MyTriggerGunflash(&g_fx, 0, driver, origin, target, true);
}

void __fastcall Gunflashes::MyTriggerGunflash(Fx_c* fx, int, CEntity* entity, CVector& origin, CVector& target, bool doGunflash) {
	if (entity && entity->m_nType == ENTITY_TYPE_PED) {
		CPed* owner = reinterpret_cast<CPed*>(entity);

		auto& pedExtData = pedExt.Get(owner);
		pedExtData.bLeftHandGunflashThisFrame = bLeftHand;
		pedExtData.bRightHandGunflashThisFrame = !bLeftHand;
		pedExtData.bInVehicle = bVehicleGunflash;
	}
	else {

		if (DistanceBetweenPoints(target, origin) > 0.0f) {
			const std::string& flashName = g_gunflashConfig.getDefaultGunflashParticleName();
			const std::string& smokeName = g_gunflashConfig.getDefaultGunflashSmokeParticleName();
			RwMatrix fxMat;
			fx->CreateMatFromVec(&fxMat, &origin, &target);
			RwV3d offset = { 0.0f, 0.0f, 0.0f };
			FxSystem_c* gunflashFx = g_fxMan.CreateFxSystem(const_cast<char*>(flashName.c_str()), &offset, &fxMat, false);
			if (MixSets::G_GunflashEmissionMult > -1.0f) gunflashFx->SetRateMult(MixSets::G_GunflashEmissionMult);
			if (gunflashFx) {
				gunflashFx->CopyParentMatrix();
				gunflashFx->PlayAndKill();
			}
			FxSystem_c* smokeFx = g_fxMan.CreateFxSystem(const_cast<char*>(smokeName.c_str()), &offset, &fxMat, false);
			if (smokeFx) {
				smokeFx->CopyParentMatrix();
				smokeFx->PlayAndKill();
			}
		}
	}
	bLeftHand = false;
	bVehicleGunflash = false;
}

/* Functions that are useful for debugging
template<typename T>
static void WriteToDesktopFile(const T& data, const std::string& dataName = "")
{
	TCHAR desktopPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath)))
	{
		std::string filePath = std::string(desktopPath) + "\\debugFile.txt";

		std::ofstream outputFile(filePath.c_str(), std::ios::app);
		if (outputFile.is_open())
		{
			outputFile << dataName << data << "\n";

			outputFile.close();
		}
	}
}

static void DebugPedAnim(CPed* ped)
{
	if (ped->m_pRwClump)
	{
		unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
		if (totalAnims > 0)
		{
			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
			while (association)
			{
				WriteToDesktopFile(association->m_pHierarchy->m_hashKey, "Anim Hash ID: ");
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
}
*/
static bool IsPedInBike(CPed* ped)
{
	return IsVehiclePointerValid(ped->m_pVehicle) &&
		(ped->m_pVehicle->GetVehicleAppearance() == BikeAppereance ||
			ped->m_pVehicle->m_nModelIndex == QUAD);
}

static bool IsPedInMoped(CPed* ped)
{
	return IsVehiclePointerValid(ped->m_pVehicle) &&
		(ped->m_pVehicle->m_nModelIndex == FAGGIO ||
			ped->m_pVehicle->m_nModelIndex == PIZZABOY);
}

static void ChangeOffsetForDriverBikeDriveBy(CPed* ped, RwV3d& gunflashOffset, RwV3d& underflashOFfset, RwReal reversingFactor)
{
	if (ped->m_pRwClump)
	{
		const auto totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);

		if (totalAnims > 0)
		{
			const auto bikeAnimGroup = ped->m_pVehicle->GetRideAnimData()->m_nAnimGroup;

			const auto
				dFrontAnim = BIKE_DRIVE_DB_ANIM_IDS[bikeAnimGroup][DRIVEBY_FRONT],
				dLeftAnim = BIKE_DRIVE_DB_ANIM_IDS[bikeAnimGroup][DRIVEBY_LEFT],
				dRightAnim = BIKE_DRIVE_DB_ANIM_IDS[bikeAnimGroup][DRIVEBY_RIGHT];

			RwReal posDeltaDriver = ped->m_pVehicle->m_fMovingSpeed * reversingFactor;
			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);

			while (association)
			{
				const auto animID = association->m_nAnimId;

				if (animID == dFrontAnim)
				{
					gunflashOffset.x += posDeltaDriver;
					return;
				}
				else if (animID == dLeftAnim)
				{
					std::swap(underflashOFfset.x, underflashOFfset.y);
					underflashOFfset.x *= -1.0f;

					gunflashOffset.z -= posDeltaDriver;
					return;
				}
				else if (animID == dRightAnim)
				{
					std::swap(underflashOFfset.x, underflashOFfset.y);

					gunflashOffset.z += posDeltaDriver;
					return;
				}
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
}

static void ChangeOffsetForCarDriverDriveBy(CPed* ped, RwV3d& gunflashOffset, RwV3d& underflashOFfset, RwReal& reversingFactor)
{
	if (ped->m_pRwClump)
	{
		unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
		if (totalAnims > 0)
		{
			const RwReal posDeltaDriver = ped->m_pVehicle->m_fMovingSpeed * reversingFactor;

			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
			while (association)
			{
				const unsigned int animID = association->m_nAnimId;

				switch (animID)
				{
					// DRIVER DRIVEBY
				case ANIM_DEFAULT_DRIVEBY_R:
				{
					gunflashOffset.z += posDeltaDriver;
					return;
				}
				case ANIM_DEFAULT_DRIVEBY_L:
				{
					gunflashOffset.z -= posDeltaDriver;
					return;
				}
				// DRIVER DRIVEBY
				case ANIM_DEFAULT_DRIVEBYL_R:
				{
					gunflashOffset.z += posDeltaDriver;
					return;
				}
				case ANIM_DEFAULT_DRIVEBYL_L:
				{
					gunflashOffset.z -= posDeltaDriver;
					return;
				}
				}
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
}

void Gunflashes::DrawUnderflash(CPed* ped, RwV3d& newOffset)
{
	float x = 0.0f, y = 0.0f, z = 0.0f;
	Command<Commands::GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS>(
		ped, newOffset.x, newOffset.y, newOffset.z, &x, &y, &z);

	const auto& config = g_gunflashConfig;
	const auto& color = config.getUnderflashColor();
	const float& range = config.getUnderflashLightRange();
	const int& shadowID = config.getUnderflashShadowID();
	const int& shadowIntensity = config.getUnderflashShadowIntensity();
	const float& shadowRadius = config.getUnderflashShadowRadius();
	const float& shadowAngle = config.getUnderflashShadowAngle();

	Command<Commands::DRAW_LIGHT_WITH_RANGE>(x, y, z, color.r, color.g, color.b, range);
	Command<Commands::DRAW_SHADOW>(
		shadowID, x, y, z, shadowAngle, shadowRadius, shadowIntensity,
		color.r, color.g, color.b);
}


eTaskType Gunflashes::GetPedActiveTask(CPed* ped)
{
	CTask* activeTask = ped->m_pIntelligence->m_TaskMgr.GetSimplestActiveTask();
	if (activeTask)
	{
		return activeTask->GetId();
	}
	else
	{
		return TASK_NONE;
	}
}

void Gunflashes::CreateGunflashEffectsForPed(CPed* ped) {
	auto& pedExtData = pedExt.Get(ped);
	bool handThisFrame[2] = { pedExtData.bLeftHandGunflashThisFrame , pedExtData.bRightHandGunflashThisFrame };


	for (int i = 0; i < 2; i++) {
		if (handThisFrame[i])
		{
			if (pedExtData.pMats[i] == nullptr) pedExtData.pMats[i] = new RwMatrix();
			RwMatrix* mat = pedExtData.pMats[i];
			if (!mat) break;

			if (ped->m_pRwObject && ped->m_pRwObject->type == rpCLUMP) {

				const eTaskType task = GetPedActiveTask(ped);

				const unsigned int arrayIndex = ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType;

				const bool driverDriveby = (task == TASK_SIMPLE_CAR_DRIVE);
				const bool isUsingJetpack = (task == TASK_SIMPLE_JETPACK);

				const bool isInVehicle = IsVehiclePointerValid(ped->m_pVehicle) && (driverDriveby || (task == TASK_SIMPLE_GANG_DRIVEBY));
				const bool isInBike = IsPedInBike(ped);
				const bool isInMoped = IsPedInMoped(ped);

				// Initial particle time multiplier
				float particleTimeMult = 1.0f;

				const WeaponData& weapon = (arrayIndex < weaponArraySize) ? weaponArray[arrayIndex] : defaultWeaponDataObj;

				const std::string& fxName = weapon.particleName;
				const bool rotate = weapon.rotate;
				const bool smoke = weapon.smoke;
				const bool underFlash = weapon.underFlash;

				bool attachedToBone = true;

				const auto pedWeaponSkill = ped->GetWeaponSkill(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType);
				const CWeaponInfo* weapInfo = CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, pedWeaponSkill);
				const auto isDualWeilding = weapInfo->m_nFlags.bTwinPistol;

				if (!isDualWeilding && ped->m_pedIK.bUseArm) attachedToBone = false;

				RwV3d gunflashOffset = weapInfo->m_vecFireOffset.ToRwV3d();
				RwV3d underflashOffset = { g_gunflashConfig.getUnderflashOffsetX(), g_gunflashConfig.getUnderflashOffsetY(), g_gunflashConfig.getUnderflashOffsetZ() };

				float additionalOffsetX = 0.0f, additionalOffsetY = 0.0f, additionalOffsetZ = 0.0f;

				if (isLeftHand[i])
				{
					gunflashOffset.z *= -1.0f;
				}

				static RwV3d axis_x = { 1.0f, 0.0f, 0.0f };
				static RwV3d axis_y = { 0.0f, 1.0f, 0.0f };
				static RwV3d axis_z = { 0.0f, 0.0f, 1.0f };

				int boneIDToAttachTo = pedHands[i];


				if (g_gunflashConfig.isLocalParticleFixEnabled())
				{
					attachedToBone = true;
					particleTimeMult = 1.0f;
					if (isInVehicle)
					{
						RwReal reversingfactor = 0.0f;
						RwV3d emptyOffset;

						if (isInBike)
						{
							ChangeOffsetForDriverBikeDriveBy(ped, emptyOffset, underflashOffset, reversingfactor);
						}
						else
						{
							ChangeOffsetForCarDriverDriveBy(ped, emptyOffset, underflashOffset, reversingfactor);
						}
					}
				}
				//TODO: REMOVE THIS AFTER ROTATION ISSUE IS FIXED
				else
				{
					const float& surfingOffsetFactor = g_gunflashConfig.getSurfingOffsetFactor();
					const float& surfingTimeMult = g_gunflashConfig.getSurfingTimeMult();
					const float& fpsFixTimeMult = g_gunflashConfig.getFpsFixTimeMult();
					const float& pistolFixOffset = g_gunflashConfig.getPistolFixOffset();

					ProcessGunflashLogicWithoutLocalParticles(
						ped,
						isInVehicle,
						isUsingJetpack,
						driverDriveby,
						isInBike,
						isInMoped,
						isDualWeilding,
						particleTimeMult,
						gunflashOffset,
						underflashOffset,
						boneIDToAttachTo,
						attachedToBone,
						additionalOffsetX,
						additionalOffsetY,
						additionalOffsetZ,
						i,
						isLeftHand[i],
						surfingOffsetFactor,
						surfingTimeMult,
						fpsFixTimeMult,
						pistolFixOffset
					);
				}

				mat->pos.x += additionalOffsetX;
				mat->pos.y += additionalOffsetY;
				mat->pos.z += additionalOffsetZ;

				RpHAnimHierarchy* hierarchy = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
				RwMatrix* boneMat = &RpHAnimHierarchyGetMatrixArray(hierarchy)[RpHAnimIDGetIndex(hierarchy, boneIDToAttachTo)];
				memcpy(mat, boneMat, sizeof(RwMatrix));
				RwMatrixUpdate(mat);

				// In order to avoid string copying, static_cast is used (fxName is not modified in the function)
				FxSystem_c* gunflashFx = g_fxMan.CreateFxSystem(const_cast<char*>(fxName.c_str()), &gunflashOffset, mat, true);

				if (MixSets::G_GunflashEmissionMult > -1.0f) gunflashFx->SetRateMult(MixSets::G_GunflashEmissionMult);

				if (gunflashFx)
				{
					gunflashFx->SetTimeMult(particleTimeMult);

					if (attachedToBone)
					{
						gunflashFx->m_pParentMatrix = boneMat;
					}

					RwMatrixRotate(&gunflashFx->m_localMatrix, &axis_z, -90.0f, rwCOMBINEPRECONCAT);
					if (rotate)
					{
						RwMatrixRotate(&gunflashFx->m_localMatrix, &axis_y, CGeneral::GetRandomNumberInRange(0.0f, 360.0f), rwCOMBINEPRECONCAT);
					}

					if (g_gunflashConfig.isLocalParticleFixEnabled()) gunflashFx->SetLocalParticles(true);

					gunflashFx->PlayAndKill();

				}
				if (g_gunflashConfig.isGunflashLowerLightEnabled() && underFlash)
				{
					DrawUnderflash(ped, underflashOffset);
				}
				if (smoke)
				{
					if (!ped->m_pVehicle || ped->m_pVehicle->m_vecMoveSpeed.Magnitude() < 0.15f)
					{
						const std::string& smokeName = g_gunflashConfig.getDefaultGunflashSmokeParticleName();
						FxSystem_c* smokeFx = g_fxMan.CreateFxSystem(const_cast<char*>(smokeName.c_str()), &gunflashOffset, mat, true);
						if (smokeFx) {
							RwMatrixRotate(&smokeFx->m_localMatrix, &axis_z, -90.0f, rwCOMBINEPRECONCAT);
							smokeFx->PlayAndKill();
						}
					}
				}
			}
		}
	}
	pedExtData.Reset();
}

void Gunflashes::ProcessGunflashLogicWithoutLocalParticles(
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
	float pistolFixOffset) {

	const auto msMagnitude = ped->m_vecMoveSpeed.Magnitude();
	const auto animMsMagnitude = ped->m_vecAnimMovingShift.Magnitude();
	const bool surfing = !isInVehicle && (msMagnitude > animMsMagnitude) && !isUsingJetpack;
	const auto* playerData = ped->m_pPlayerData;

	if (!isInVehicle) {

		if (surfing) {
			if (playerData) {
				if (!playerData->m_bFreeAiming) {
					attachedToBone = false;
					additionalOffsetX += ped->m_vecMoveSpeed.x * surfingOffsetFactor;
					additionalOffsetY += ped->m_vecMoveSpeed.y * surfingOffsetFactor;
					additionalOffsetZ += ped->m_vecMoveSpeed.z * surfingOffsetFactor;
					particleTimeMult = surfingTimeMult;
				}
				else {
					attachedToBone = true;
					particleTimeMult = fpsFixTimeMult;
				}
			}
			else {
				attachedToBone = false;
				particleTimeMult = fpsFixTimeMult;
			}
		}
		else if (ped->m_pedIK.bUseArm) {
			if (playerData) {
				if (!playerData->m_bFreeAiming) {
					if (isDualWeilding) {
						boneIDToAttachTo = BONE_RIGHTWRIST;
						if (isLeftHand) {
							gunflashOffset.z *= -1.0f;
						}
						else if (ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType == WEAPON_PISTOL) {
							gunflashOffset.z += pistolFixOffset;
						}
					}
					else {
						attachedToBone = false;
						particleTimeMult = fpsFixTimeMult;
						gunflashOffset.x += ped->m_vecAnimMovingShiftLocal.y;
						gunflashOffset.y -= ped->m_vecAnimMovingShiftLocal.x;
					}
				}
				else {
					gunflashOffset.x += ped->m_vecAnimMovingShiftLocal.y * surfingOffsetFactor;
					gunflashOffset.y -= ped->m_vecAnimMovingShiftLocal.x * surfingOffsetFactor;
					attachedToBone = true;
				}
			}
			else {
				attachedToBone = false;
			}
		}
		else {
			gunflashOffset.x += ped->m_vecAnimMovingShiftLocal.y;
			gunflashOffset.y -= ped->m_vecAnimMovingShiftLocal.x;
		}
	}
	else {
		particleTimeMult = fpsFixTimeMult;
		RwReal reversingFactor = (ped->m_pVehicle->m_nCurrentGear == 0) ? -1.0f : 1.0f;

		if (driverDriveby) {
			if (isInBike) {
				if (isInMoped) {
					attachedToBone = false;
					particleTimeMult = surfingTimeMult;
					additionalOffsetX = ped->m_pVehicle->m_vecMoveSpeed.x * surfingOffsetFactor;
					additionalOffsetY = ped->m_pVehicle->m_vecMoveSpeed.y * surfingOffsetFactor;
					additionalOffsetZ = ped->m_pVehicle->m_vecMoveSpeed.z * surfingOffsetFactor;
				}
				else {
					ChangeOffsetForDriverBikeDriveBy(ped, gunflashOffset, underflashOffset, reversingFactor);
				}
			}
			else {
				ChangeOffsetForCarDriverDriveBy(ped, gunflashOffset, underflashOffset, reversingFactor);
			}
		}
	}
}