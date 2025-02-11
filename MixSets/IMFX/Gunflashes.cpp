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

using namespace plugin;

// Constants
const bool isLeftHand[2] = { true, false };
const ePedBones pedHands[2] = { BONE_LEFTWRIST, BONE_RIGHTWRIST };

const int BikeAppereance = 2;
const int weaponArraySize = WEAPON_MINIGUN + 1;

// Underflash properties
int UnderflashRComponent = 70;
int UnderflashGComponent = 55;
int UnderflashBComponent = 22;

float UnderflashLightRange = 3.5f;

int UnderflashShadowID = 3;
int UnderflashShadowIntensity = 1;
float UnderflashShadowRadius = 2.5f;
float UnderflashShadowAngle = 0.0f;

RwReal UnderflashPlayerOffsetX = 0.0f;
RwReal UnderflashPlayerOffsetY = 1.3f;
RwReal UnderflashPlayerOffsetZ = 1.0f;

// Gunflashes
PedExtendedData<Gunflashes::PedExtension> Gunflashes::pedExt;
bool Gunflashes::bLeftHand = false;
bool Gunflashes::bVehicleGunflash = false;
float fpsFixTimeMult = 1.0f;
bool computeFpsFix = true;

// Offsets and factors
RwReal surfingOffsetFactor = 0.0f;

float pistolFixOffset = 0.005f;

bool gunflashLowerLight = false;
bool localParticleFix = false;

// Time multipliers
float surfingTimeMult = 1.0f;

std::string defaultGunflashName = "gunflash";
std::string defaultGunSmokeName = "gunsmoke";

// Weapon data structure
struct WeaponData {
	std::string particleName;

	bool rotate = true;
	bool smoke = true;

	WeaponData(
		const std::string& pName = "gunflash",
		bool r = true, bool s = true
	) : particleName(pName), rotate(r), smoke(s) {
	}
};
WeaponData weaponArray[weaponArraySize];


// PedExtension implementation
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


void Gunflashes::SetPistolFixOffset(const float newValue) {
	pistolFixOffset = newValue;
}

void Gunflashes::SetUnderFlashLightRComponent(const int newValue) {
	UnderflashRComponent = newValue;
}

void Gunflashes::SetUnderFlashLightGComponent(const int newValue) {
	UnderflashGComponent = newValue;
}

void Gunflashes::SetUnderFlashLightBComponent(const int newValue) {
	UnderflashBComponent = newValue;
}

void Gunflashes::SetUnderflashLightRange(const float newValue) {
	UnderflashLightRange = newValue;
}

void Gunflashes::SetUnderflashShadowID(const int newValue) {
	UnderflashShadowID = newValue;
}

void Gunflashes::SetUnderflashShadowIntensity(const int newValue) {
	UnderflashShadowIntensity = newValue;
}

void Gunflashes::SetUnderflashShadowRadius(const float newValue) {
	UnderflashShadowRadius = newValue;
}

void Gunflashes::SetUnderFlashShadowAngle(const float newValue) {
	UnderflashShadowAngle = newValue;
}

void Gunflashes::SetUnderflashOffsetX(const float newValue) {
	UnderflashPlayerOffsetX = newValue;
}

void Gunflashes::SetUnderflashOffsetY(const float newValue) {
	UnderflashPlayerOffsetY = newValue;
}

void Gunflashes::SetUnderflashOffsetZ(const float newValue) {
	UnderflashPlayerOffsetZ = newValue;
}

void Gunflashes::SetSurfingOffsetFactor(const RwReal newValue) {
	surfingOffsetFactor = newValue;
}

void Gunflashes::SetLocalParticleFix(const bool newValue) {
	localParticleFix = newValue;
}

void Gunflashes::SetGunflashLowerLight(const bool newValue) {
	gunflashLowerLight = newValue;
}

void Gunflashes::SetFpsFixTimeMult(const float newValue) {
	fpsFixTimeMult = newValue;
}
void Gunflashes::SetFpsFixComputing(const bool newValue)
{
	computeFpsFix = newValue;
}

void Gunflashes::SetSurfingTimeMult(const float newValue) {
	surfingTimeMult = newValue;
}

void Gunflashes::AddDefaultWeaponData()
{
	for (int weaponID = WEAPON_PISTOL; weaponID <= WEAPON_SNIPERRIFLE; ++weaponID) {
		weaponArray[weaponID] = WeaponData(); // Use default constructor
	}
	weaponArray[WEAPON_MINIGUN] = WeaponData();
}


void Gunflashes::UpdateWeaponData(unsigned int weaponID, const std::string& particle, bool rotate, bool smoke)
{
	if (weaponID < weaponArraySize)
	{
		weaponArray[weaponID] = WeaponData(particle, rotate, smoke);
	}
}


void Gunflashes::Setup(bool sampFix)
{
	patch::Nop(0x73306D, 9); // Remove default gunflashes
	patch::Nop(0x7330FF, 9); // Remove default gunflashes
	patch::SetUShort(0x5DF425, 0xE990); // Remove default gunflashes
	patch::SetUChar(0x741353, 0); // Add gunflash for cuntgun

	patch::RedirectCall(0x742299, DoDriveByGunflash);
	patch::RedirectJump(0x4A0DE0, MyTriggerGunflash);

	patch::SetPointer(0x86D744, (sampFix) ? MyProcessUseGunTaskSAMP : MyProcessUseGunTask);

	AddDefaultWeaponData();
	//ReadSettings();
}

void Gunflashes::ProcessPerFrame() {
	if (computeFpsFix)
	{
		fpsFixTimeMult = CTimer::game_FPS / 30.0f;
		surfingOffsetFactor = -CTimer::game_FPS / 60.0f + 2;
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
			RwMatrix fxMat;
			fx->CreateMatFromVec(&fxMat, &origin, &target);
			RwV3d offset = { 0.0f, 0.0f, 0.0f };
			FxSystem_c* gunflashFx = g_fxMan.CreateFxSystem(defaultGunflashName.data(), &offset, &fxMat, false);
			if (MixSets::G_GunflashEmissionMult > -1.0f) gunflashFx->SetRateMult(MixSets::G_GunflashEmissionMult);
			if (gunflashFx) {
				gunflashFx->CopyParentMatrix();
				gunflashFx->PlayAndKill();
			}
			FxSystem_c* smokeFx = g_fxMan.CreateFxSystem(defaultGunSmokeName.data(), &offset, &fxMat, false);
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
	/*Source: DK22Pac - GTA IV Lights
	04C4: store_coords_to 10@ 11@ 12@ from_actor 3@ with_offset PED_OFFSET_X PED_OFFSET_Y PED_OFFSET_Z
	09E5: create_flash_light_at 10@ 11@ 12@ SHOT_LIGHT_R SHOT_LIGHT_G SHOT_LIGHT_B SHOT_FLASH_LIGHT_RADIUS
	016F: particle 3 rot 0.0 size SHOT_LIGHT_SIZE SHOT_LIGHT_INTENSITY SHOT_LIGHT_R SHOT_LIGHT_G SHOT_LIGHT_B at 10@ 11@ 12@
	*/
	float x = 0.0f, y = 0.0f, z = 0.0f;
	Command<Commands::GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS>(ped, newOffset.x, newOffset.y, newOffset.z, &x, &y, &z);
	Command<Commands::DRAW_LIGHT_WITH_RANGE>(x, y, z, UnderflashRComponent, UnderflashGComponent, UnderflashBComponent, UnderflashLightRange);
	Command<Commands::DRAW_SHADOW>(UnderflashShadowID, x, y, z, UnderflashShadowAngle, UnderflashShadowRadius, UnderflashShadowIntensity, UnderflashRComponent, UnderflashGComponent, UnderflashBComponent);
}
void Gunflashes::DrawUnderflash(CPed* ped)
{
	/*Source: DK22Pac - GTA IV Lights
	04C4: store_coords_to 10@ 11@ 12@ from_actor 3@ with_offset PED_OFFSET_X PED_OFFSET_Y PED_OFFSET_Z
	09E5: create_flash_light_at 10@ 11@ 12@ SHOT_LIGHT_R SHOT_LIGHT_G SHOT_LIGHT_B SHOT_FLASH_LIGHT_RADIUS
	016F: particle 3 rot 0.0 size SHOT_LIGHT_SIZE SHOT_LIGHT_INTENSITY SHOT_LIGHT_R SHOT_LIGHT_G SHOT_LIGHT_B at 10@ 11@ 12@
	*/
	float x = 0.0f, y = 0.0f, z = 0.0f;

	Command<Commands::GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS>(ped, UnderflashPlayerOffsetX, UnderflashPlayerOffsetY, UnderflashPlayerOffsetZ, &x, &y, &z);
	Command<Commands::DRAW_LIGHT_WITH_RANGE>(x, y, z, UnderflashRComponent, UnderflashGComponent, UnderflashBComponent, UnderflashLightRange);
	Command<Commands::DRAW_SHADOW>(UnderflashShadowID, x, y, z, UnderflashShadowAngle, UnderflashShadowRadius, UnderflashShadowIntensity, UnderflashRComponent, UnderflashGComponent, UnderflashBComponent);
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

				WeaponData weapon = (arrayIndex < weaponArraySize) ? weaponArray[arrayIndex] : WeaponData();

				std::string& fxName = weapon.particleName;
				bool rotate = weapon.rotate;
				bool smoke = weapon.smoke;

				bool attachedToBone = true;

				const auto pedWeaponSkill = ped->GetWeaponSkill(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType);
				CWeaponInfo* weapInfo = CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, pedWeaponSkill);
				const auto isDualWeilding = weapInfo->m_nFlags.bTwinPistol;

				if (!isDualWeilding && ped->m_pedIK.bUseArm) attachedToBone = false;

				RwV3d gunflashOffset = weapInfo->m_vecFireOffset.ToRwV3d();
				RwV3d underflashOffset = { UnderflashPlayerOffsetX, UnderflashPlayerOffsetY, UnderflashPlayerOffsetZ };

				float additionalOffsetX = 0.0f, additionalOffsetY = 0.0f, additionalOffsetZ = 0.0f;

				if (isLeftHand[i])
				{
					gunflashOffset.z *= -1.0f;
				}

				static RwV3d axis_x = { 1.0f, 0.0f, 0.0f };
				static RwV3d axis_y = { 0.0f, 1.0f, 0.0f };
				static RwV3d axis_z = { 0.0f, 0.0f, 1.0f };

				int boneIDToAttachTo = pedHands[i];


				if (localParticleFix)
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

				FxSystem_c* gunflashFx = g_fxMan.CreateFxSystem(fxName.data(), &gunflashOffset, mat, true);

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

					if (localParticleFix) gunflashFx->SetLocalParticles(true);

					gunflashFx->PlayAndKill();

				}
				if (gunflashLowerLight)
				{
					if (isInVehicle)
					{
						DrawUnderflash(ped, underflashOffset);
					}
					else
					{
						DrawUnderflash(ped);
					}
				}
				if (smoke)
				{
					if (!ped->m_pVehicle || ped->m_pVehicle->m_vecMoveSpeed.Magnitude() < 0.15f)
					{

						FxSystem_c* smokeFx = g_fxMan.CreateFxSystem(defaultGunSmokeName.data(), &gunflashOffset, mat, true);
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