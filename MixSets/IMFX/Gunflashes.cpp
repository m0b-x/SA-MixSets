/*
Credits for DK22Pac - IMFX
If you consider fixing something here, you should also consider fixing there: https://github.com/DK22Pac/imfx
*/
#include "Gunflashes.h"
#include "imfx.h"
#include "plugin.h"
#include "game_sa\FxManager_c.h"
#include "game_sa\common.h"
#include "game_sa\CWeaponInfo.h"
#include "game_sa\CGeneral.h"
#include "game_sa\CCamera.h"
#include "game_sa\CTimer.h"
#include <fstream>
#include <string>
/*
#include <Windows.h>
#include <shlobj.h>
*/
#include <map>
#include <CAnimManager.h>
#include <CAnimBlendAssociation.h>
#include <CAnimBlendHierarchy.h>
#include <CAnimBlock.h>
#include <CAnimBlendAssocGroup.h>

#include "..\MixSets.h"

using namespace plugin;

unsigned int Gunflashes::matrixCounter = 0;
PedExtendedData<Gunflashes::PedExtension> Gunflashes::pedExt;
bool Gunflashes::bLeftHand = false;
bool Gunflashes::bVehicleGunflash = false;

RwReal onfootOffsetFactor = 2.0f;
RwReal carDriverOffsetFactor = 2.0f;
RwReal carPassengerOffsetFactor = 1.15f;
RwReal bikePassengerOffsetFactor = 1.15f;
RwReal bikeDriverOffsetFactor = 2.0f;

float inVehicleTimeMult = 1.0f; //was 1.35f
float dualWeildingTimeMult = 1.0f;//was 1.25f
float singleWeaponWeildingTimeMult = 1.0f;//was 1.15f
float surfingSpeed = 0.1f;

RwReal mopedFixOffset = 2.5f;

const int BikeAppereance = 2;

enum Bikes : unsigned int {
	// BIKEV animation group
	PIZZABOY = 448,
	FAGGIO = 462,
	// BIKEH animation group
	FREEWAY = 463,
	// BIKED animation group
	SANCHEZ = 468,
	// QUAD animation group
	QUAD = 471,
	// BMX animation group
	BMX = 481,
	// MTB animation group
	BIKE = 509,
	MBIKE = 510,
	// BIKES animation group
	FCR = 521,
	NRG = 522,
	PCJ = 461,
	BF = 581,
	HPV = 523,
	// WAYFARER animation group
	WAYFARER = 586
};

enum AnimationHashKeys {
	ANIM_HASH_PASSENGER_DBFRONT_BIKE = 826417989,
	ANIM_HASH_PASSENGER_DBLEFT_BIKE = 1918643658,
	ANIM_HASH_PASSENGER_DBRIGHT_BIKE = 1692705712,
	ANIM_HASH_PASSENGER_DBBACK_BIKE = 910920601,

	ANIM_HASH_DBRIGHT_CAR = 2872216017,
	ANIM_HASH_DBLEFT_CAR = 1362998450,
	ANIM_HASH_DBLEFT2_CAR = 115705905,
	ANIM_HASH_DBRIGHT2_CAR = 4243240274,

	ANIM_HASH_TOP_DRIVEBY_RIGHT_SHOOTING_LEFT = 2338707153,
	ANIM_HASH_TOP_DRIVEBY_LEFT_SHOOTING_RIGHT = 2648529067,
	ANIM_HASH_RIGHT_DRIVEBY_SHOOTING_RIGHT = 2664255580,
	ANIM_HASH_LEFT_DRIVEBY_SHOOTING_FRONT = 3887475062,
	ANIM_HASH_LEFT_DRIVEBY_SHOOTING_BACK = 3770646954,
	ANIM_HASH_LEFT_DRIVEBY_SHOOTING_LEFT = 2289425958,
	ANIM_HASH_LEFT_DRIVEBY_SHOOTING_FRONT_2 = 3495415525,
	ANIM_HASH_LEFT_DRIVEBY_SHOOTING_BACK_2 = 3613287993,

	ANIM_HASH_GUNMOVE_BACKWARD = 205664729
};

void Gunflashes::SetBikePassengerOffsetFactor(const RwReal newValue) {
	bikePassengerOffsetFactor = newValue;
}

void Gunflashes::SetBikeDriverOffsetFactor(const RwReal newValue) {
	bikeDriverOffsetFactor = newValue;
}

void Gunflashes::SetCarDriverOffsetFactor(const RwReal newValue) {
	carDriverOffsetFactor = newValue;
}

void Gunflashes::SetCarPassengerOffsetFactor(const RwReal newValue) {
	carPassengerOffsetFactor = newValue;
}

void Gunflashes::SetOnFootOffsetFactor(const RwReal newValue) {
	onfootOffsetFactor = newValue;
}

void Gunflashes::SetInVehicleTimeMult(const float newValue) {
	inVehicleTimeMult = newValue;
}

void Gunflashes::SetDualWeildingTimeMult(const float newValue) {
	dualWeildingTimeMult = newValue;
}

void Gunflashes::SetSingleWeaponWeildingTimeMult(const float newValue) {
	singleWeaponWeildingTimeMult = newValue;
}

void Gunflashes::SetSurfingSpeed(const float newValue) {
	surfingSpeed = newValue;
}

void Gunflashes::SetMopedFixOffset(const RwReal newValue) {
	mopedFixOffset = newValue;
}

struct WeaponData
{
	char* particleName = "gunflash";
	bool rotate = true;
	bool smoke = true;
};
std::unordered_map<unsigned int, WeaponData> weaponMap;

void Gunflashes::AddDefaultWeaponData()
{
	for (unsigned int weaponID = WEAPON_PISTOL; weaponID <= WEAPON_SNIPERRIFLE; weaponID++)
	{
		WeaponData weapon;
		weapon.particleName = "gunflash";
		weapon.smoke = true;
		weapon.rotate = true;

		weaponMap[weaponID] = weapon;
	}
	//minigun exception
	unsigned int minigunID = 38;

	WeaponData weapon;
	weapon.particleName = "gunflash";
	weapon.smoke = true;
	weapon.rotate = true;

	weaponMap[minigunID] = weapon;
}

void Gunflashes::AddOrUpdateWeaponData(unsigned int weaponID, const std::string particle, bool rotate, bool smoke)
{
	WeaponData weapon;

	char* particleCharPointer = _strdup(particle.c_str());

	weapon.particleName = particleCharPointer;
	weapon.smoke = smoke;
	weapon.rotate = rotate;

	weaponMap[weaponID] = weapon;
}

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
}

Gunflashes::PedExtension::~PedExtension() {
	if (pMats[0] != nullptr) delete pMats[0];
	if (pMats[1] != nullptr) delete pMats[1];
}

void Gunflashes::Setup(bool experimental)
{
	if (experimental)
	{
		patch::Nop(0x73306D, 9); // Remove default gunflashes
		patch::Nop(0x7330FF, 9); // Remove default gunflashes
		patch::SetUShort(0x5DF425, 0xE990); // Remove default gunflashes
		patch::SetUChar(0x741353, 0); // Add gunflash for cuntgun

		patch::RedirectCall(0x742299, DoDriveByGunflash);
		patch::RedirectJump(0x4A0DE0, MyTriggerGunflash);
		patch::SetPointer(0x86D744, MyProcessUseGunTask);
		AddDefaultWeaponData();
		//ReadSettings();
	}
	else
	{
		/*changed by m0b : deactivating mixsets' system
		patch::Nop(0x73306D, 9); // Remove default gunflashes
		patch::Nop(0x7330FF, 9); // Remove default gunflashes
		patch::SetUShort(0x5DF425, 0xE990); // Remove default gunflashes
		*/
		patch::SetUChar(0x741353, 0); // Add gunflash for cuntgun

		patch::RedirectCall(0x742299, DoDriveByGunflash);
		//patch::RedirectJump(0x4A0DE0, MyTriggerGunflash);
		patch::SetPointer(0x86D744, MyProcessUseGunTask);
		AddDefaultWeaponData();
		//ReadSettings();
	}
}

void Gunflashes::ProcessPerFrame() {
	Gunflashes::matrixCounter = 0;
	for (int i = 0; i < CPools::ms_pPedPool->m_nSize; i++) {
		CPed* ped = CPools::ms_pPedPool->GetAt(i);
		if (ped)
			pedExt.Get(ped).Reset();
	}
}

bool __fastcall Gunflashes::MyProcessUseGunTask(CTaskSimpleUseGun* task, int, CPed* ped) {
	if (task->m_pWeaponInfo == CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, ped->GetWeaponSkill())) {
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
	MyTriggerGunflash(&g_fx, 0, driver, CVector(0.0f, 0.0f, 0.0f), CVector(0.0f, 0.0f, 0.0f), true);
}

void __fastcall Gunflashes::MyTriggerGunflash(Fx_c* fx, int, CEntity* entity, CVector& origin, CVector& target, bool doGunflash) {
	if (entity && entity->m_nType == ENTITY_TYPE_PED) {
		CPed* owner = reinterpret_cast<CPed*>(entity);
		pedExt.Get(owner).bLeftHandGunflashThisFrame = bLeftHand;
		pedExt.Get(owner).bRightHandGunflashThisFrame = !bLeftHand;
		pedExt.Get(owner).bInVehicle = bVehicleGunflash;
	}
	else {
		if (DistanceBetweenPoints(target, origin) > 0.0f) {
			RwMatrix fxMat;
			fx->CreateMatFromVec(&fxMat, &origin, &target);
			RwV3d offset = { 0.0f, 0.0f, 0.0f };
			FxSystem_c* gunflashFx = g_fxMan.CreateFxSystem("gunflash", &offset, &fxMat, false);
			if (MixSets::G_GunflashEmissionMult > -1.0f) gunflashFx->SetRateMult(MixSets::G_GunflashEmissionMult);
			if (gunflashFx) {
				gunflashFx->CopyParentMatrix();
				gunflashFx->PlayAndKill();
			}
			FxSystem_c* smokeFx = g_fxMan.CreateFxSystem("gunsmoke", &offset, &fxMat, false);
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

static void GetBikePassengerDrivebyAnimations(unsigned short id, unsigned int& front, unsigned int& left, unsigned int& right, unsigned int& back)
{
	front = 826417989;
	left = 1918643658;
	right = 1692705712;
	back = 910920601;
}

static bool IsPedDrivebyingInVehicle(CPed* ped, bool& driverDriveby)
{
	if (!ped->m_pVehicle || !ped->m_pIntelligence)
	{
		return false;
	}

	CTask* activeTask = ped->m_pIntelligence->m_TaskMgr.GetSimplestActiveTask();

	if (activeTask)
	{
		eTaskType taskType = activeTask->GetId();
		if (taskType == TASK_SIMPLE_CAR_DRIVE)
		{
			driverDriveby = true;
			return true;
		}
		else if (taskType == TASK_SIMPLE_GANG_DRIVEBY)
		{
			return true;
		}
		return false;
	}
	return false;
}

static bool IsPedInBike(CPed* ped)
{
	return (ped->m_pVehicle->GetVehicleAppearance() == BikeAppereance || ped->m_pVehicle->m_nModelIndex == QUAD) ? true : false;
}

static bool IsPedInMoped(CPed* ped)
{
	return (ped->m_pVehicle->m_nModelIndex == FAGGIO || ped->m_pVehicle->m_nModelIndex == PIZZABOY);
}

static void GetBikeDriverDrivebyAnimationsByAnimGroup(unsigned short animGroup, unsigned int& front, unsigned int& left, unsigned int& right)
{
	switch (animGroup)
	{
	case 2:
	{
		front = 1762059363;
		left = 3444633283;
		right = 3689439417;
		break;
	}
	case 3:
	{
		front = 407980065;
		left = 1442027987;
		right = 1128994729;
		break;
	}
	case 4:
	{
		front = 3838945260;
		left = 2967119665;
		right = 2791450955;
		break;
	}
	case 5:
	{
		front = 4006294827;
		left = 3050367664;
		right = 2741527754;
		break;
	}
	case 6:
	{
		front = 760534612;
		left = 1974016510;
		right = 1662097284;
		break;
	}
	case 7:
	{
		front = 2659948553;
		left = 3673166122;
		right = 3427311440;
		break;
	}
	case 8:
	{
		front = 1678968350;
		left = 3368341863;
		right = 3732692765;
		break;
	}
	case 9:
	{
		front = 4227475174;
		left = 802096627;
		right = 964135817;
		break;
	}
	case 10:
	{
		front = 3751445549;
		left = 1552031284;
		right = 1245352014;
		break;
	}
	default:
	{
		front = 0;  // Default front animation ID
		left = 0;   // Default left animation ID
		right = 0;  // Default right animation ID
		break;
	}
	}
}

static void ChangeOffsetForDriverBikeDriveBy(CPed* ped, RwV3d& offset, RwReal reversingFactor)
{
	unsigned int carAnimGroup = ped->m_pVehicle->GetRideAnimData()->m_nAnimGroup;
	unsigned int dFrontAnim, dLeftAnim, dRightAnim;
	GetBikeDriverDrivebyAnimationsByAnimGroup(carAnimGroup, dFrontAnim, dLeftAnim, dRightAnim);

	if (ped->m_pRwClump)
	{
		unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);

		if (totalAnims > 0)
		{
			const RwReal posDeltaDriver = ped->m_pVehicle->m_fMovingSpeed * bikeDriverOffsetFactor * reversingFactor;
			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
			while (association)
			{
				if (association->m_pHierarchy->m_hashKey == dFrontAnim)
				{
					offset.x += posDeltaDriver;
					return;
				}
				else if (association->m_pHierarchy->m_hashKey == dLeftAnim)
				{
					offset.z -= posDeltaDriver;
					return;
				}
				else if (association->m_pHierarchy->m_hashKey == dRightAnim)
				{
					offset.z += posDeltaDriver;
					return;
				}
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
}

static void ChangeOffsetForDriverMopedDriveBy(CPed* ped, RwV3d& offset, RwReal reversingFactor)
{
	unsigned int carAnimGroup = ped->m_pVehicle->GetRideAnimData()->m_nAnimGroup;
	unsigned int dFrontAnim, dLeftAnim, dRightAnim;
	GetBikeDriverDrivebyAnimationsByAnimGroup(carAnimGroup, dFrontAnim, dLeftAnim, dRightAnim);

	if (ped->m_pRwClump)
	{
		unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);

		if (totalAnims > 0)
		{
			const RwReal posDeltaDriver = ped->m_pVehicle->m_fMovingSpeed * bikeDriverOffsetFactor * reversingFactor;
			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
			while (association)
			{
				if (association->m_pHierarchy->m_hashKey == dFrontAnim)
				{
					offset.x += posDeltaDriver;
					return;
				}
				else if (association->m_pHierarchy->m_hashKey == dLeftAnim)
				{
					offset.y -= posDeltaDriver;
					//moped upward/forward pos correction
					offset.x -= posDeltaDriver / mopedFixOffset;
					offset.z -= posDeltaDriver / mopedFixOffset;
					return;
				}
				else if (association->m_pHierarchy->m_hashKey == dRightAnim)
				{
					offset.y += posDeltaDriver;
					return;
				}
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
}
static void ChangeOffsetForPassengerBikeDriveBy(CPed* ped, RwV3d& offset, RwReal reversingFactor)
{
	unsigned int carModel = ped->m_pVehicle->m_nModelIndex;

	if (ped->m_pRwClump)
	{
		unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);

		if (totalAnims > 0)
		{
			const RwReal posDeltaPassenger = ped->m_pVehicle->m_fMovingSpeed * bikePassengerOffsetFactor * reversingFactor;
			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
			while (association)
			{
				const unsigned int animHashKey = association->m_pHierarchy->m_hashKey;

				switch (animHashKey)
				{
				case ANIM_HASH_PASSENGER_DBFRONT_BIKE:
				{
					offset.x += posDeltaPassenger;
					return;
				}
				case ANIM_HASH_PASSENGER_DBLEFT_BIKE:
				{
					offset.y -= posDeltaPassenger;
					return;
				}
				case ANIM_HASH_PASSENGER_DBRIGHT_BIKE:
				{
					offset.z += posDeltaPassenger;
					return;
				}
				case ANIM_HASH_PASSENGER_DBBACK_BIKE:
				{
					offset.x -= posDeltaPassenger;
					return;
				}
				}
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
}

static void ChangeOffsetForCarDriverDriveBy(CPed* ped, RwV3d& offset, RwReal& reversingFactor)
{
	if (ped->m_pRwClump)
	{
		unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
		if (totalAnims > 0)
		{
			const RwReal posDeltaDriver = ped->m_pVehicle->m_fMovingSpeed * carDriverOffsetFactor * reversingFactor;
			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
			while (association)
			{
				const unsigned int animHashKey = association->m_pHierarchy->m_hashKey;

				switch (animHashKey)
				{
					// DRIVER DRIVEBY
				case ANIM_HASH_DBRIGHT_CAR:
				{
					offset.z += posDeltaDriver;
					return;
				}
				case ANIM_HASH_DBLEFT_CAR:
				{
					offset.z -= posDeltaDriver;
					return;
				}
				// DRIVER DRIVEBY
				case ANIM_HASH_DBRIGHT2_CAR:
				{
					offset.z += posDeltaDriver;
					return;
				}
				case ANIM_HASH_DBLEFT2_CAR:
				{
					offset.z -= posDeltaDriver;
					return;
				}
				}
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
}

static void ChangeOffsetForCarPassengerDriveBy(CPed* ped, RwV3d& offset, RwReal& reversingFactor)
{
	if (ped->m_pRwClump)
	{
		unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
		if (totalAnims > 0)
		{
			const RwReal posDeltaPassenger = ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor * reversingFactor;
			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
			while (association)
			{
				const unsigned int animHashKey = association->m_pHierarchy->m_hashKey;

				switch (animHashKey)
				{
				// PASSENGER TOP RIGHT LEFT DRIVEBY
				case ANIM_HASH_TOP_DRIVEBY_RIGHT_SHOOTING_LEFT:
				{
					offset.z -= posDeltaPassenger;
					return;
				}
				case ANIM_HASH_TOP_DRIVEBY_LEFT_SHOOTING_RIGHT:
				{
					offset.z += posDeltaPassenger;
					return;
				}

				// PASSENGER RIGHT SEATS DRIVEBY
				case ANIM_HASH_RIGHT_DRIVEBY_SHOOTING_RIGHT:
				{
					offset.z += posDeltaPassenger;
					return;
				}
				case ANIM_HASH_LEFT_DRIVEBY_SHOOTING_FRONT:
				{
					offset.x += posDeltaPassenger;
					return;
				}
				case ANIM_HASH_LEFT_DRIVEBY_SHOOTING_BACK:
				{
					offset.x -= posDeltaPassenger;
					return;
				}

				// PASSENGER LEFT SEATS DRIVEBY
				case ANIM_HASH_LEFT_DRIVEBY_SHOOTING_LEFT:
				{
					offset.z -= posDeltaPassenger;
					return;
				}
				case ANIM_HASH_LEFT_DRIVEBY_SHOOTING_FRONT_2:
				{
					offset.x += posDeltaPassenger;
					return;
				}
				case ANIM_HASH_LEFT_DRIVEBY_SHOOTING_BACK_2:
				{
					offset.x -= posDeltaPassenger;
					return;
				}
				}
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
}

static bool IsPedWalkingBackWhileShooting(CPed* ped)
{
	if (ped->m_pRwClump)
	{
		unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
		if (totalAnims > 0)
		{
			CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
			while (association)
			{
				if (association->m_pHierarchy->m_hashKey == ANIM_HASH_GUNMOVE_BACKWARD)
					return true;
				association = RpAnimBlendGetNextAssociation(association);
			}
		}
	}
	return false;
}

static void ChangeOnFootOffsetForTwoHandedWeapons(CPed* ped, RwMatrix* mat)
{
	const RwReal reverseFactor = IsPedWalkingBackWhileShooting(ped) ? -1.0f : 1.0f;
	const RwReal onFootOffset = onfootOffsetFactor * reverseFactor;

	mat->pos.x += ped->m_vecMoveSpeed.x * onFootOffset;
	mat->pos.y += ped->m_vecMoveSpeed.y * onFootOffset;
	mat->pos.z += ped->m_vecMoveSpeed.z * onFootOffset;
}

static bool CanWeaponBeDualWeilded(const eWeaponType weaponType)
{
	switch (weaponType)
	{
	case WEAPON_PISTOL:
	{
		return true;
	}
	case WEAPON_SAWNOFF:
	{
		return true;
	}
	case WEAPON_MICRO_UZI:
	{
		return true;
	}
	case WEAPON_TEC9:
	{
		return true;
	}
	default:
	{
		return false;
	}
	}
}

void Gunflashes::CreateGunflashEffectsForPed(CPed* ped) {
	bool handThisFrame[2];
	handThisFrame[0] = pedExt.Get(ped).bLeftHandGunflashThisFrame;
	handThisFrame[1] = pedExt.Get(ped).bRightHandGunflashThisFrame;

	bool driverDriveby = false;
	const bool isInVehicle = IsPedDrivebyingInVehicle(ped, driverDriveby);

	//Is ped surfing on a vehicle? => Don't make the gun effect
	if (!isInVehicle && ped->m_fMovingSpeed > surfingSpeed)
		return;

	const bool needsCustomMat = CanWeaponBeDualWeilded(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType);
	const bool isInBike = isInVehicle ? IsPedInBike(ped) : false;
	const bool isInMoped = isInBike ? IsPedInMoped(ped) : false;
	const bool weapSkill = ped->m_nWeaponSkill > (char)2;
	const bool dualWeildedWeapon = needsCustomMat && weapSkill;

	for (int i = 0; i < 2; i++) {
		if (handThisFrame[i])
		{
			if (pedExt.Get(ped).pMats[i] == nullptr) pedExt.Get(ped).pMats[i] = new RwMatrix();
			RwMatrix* mat = pedExt.Get(ped).pMats[i];
			if (!mat) break;

			bool leftHand = i == 0;
			if (ped->m_pRwObject && ped->m_pRwObject->type == rpCLUMP) {
				bool rotate = true;
				bool smoke = true;
				char* fxName = "gunflash";

				unsigned int mapKey = ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType;

				auto it = weaponMap.find(mapKey);
				if (it != weaponMap.end())
				{
					WeaponData weaponData = weaponMap[mapKey];
					fxName = weaponData.particleName;
					rotate = weaponData.rotate;
					smoke = weaponData.smoke;
				}

				char dualWeildingSKill = ped->GetWeaponSkill(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType);
				CWeaponInfo* weapInfo = CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, dualWeildingSKill);
				RwV3d offset = weapInfo->m_vecFireOffset.ToRwV3d();
				if (leftHand)
				{
					offset.z *= -1.0f;
				}

				static RwV3d axis_y = { 0.0f, 1.0f, 0.0f };
				static RwV3d axis_z = { 0.0f, 0.0f, 1.0f };

				RpHAnimHierarchy* hierarchy = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
				RwMatrix* boneMat = &RpHAnimHierarchyGetMatrixArray(hierarchy)[RpHAnimIDGetIndex(hierarchy, 24 + (10 * leftHand))];
				memcpy(mat, boneMat, sizeof(RwMatrix));
				RwMatrixUpdate(mat);

				if (!isInVehicle)
				{
					//Fix offset while moving for certain weapons
					if (ped->m_fMovingSpeed > 0.0f && needsCustomMat)
					{
						ChangeOnFootOffsetForTwoHandedWeapons(ped, mat);
					}
				}
				else if (!leftHand)
				{
					if (ped->m_pVehicle->m_fMovingSpeed > 0.0f)
					{
						RwReal reversingFactor = (ped->m_pVehicle->m_nCurrentGear == 0) ? -1.0f : 1.0f;
						if (isInBike)
						{
							if (driverDriveby)
							{
								if (!isInMoped)
									ChangeOffsetForDriverBikeDriveBy(ped, offset, reversingFactor);
								else
									ChangeOffsetForDriverMopedDriveBy(ped, offset, reversingFactor);
							}
							else
							{
								ChangeOffsetForPassengerBikeDriveBy(ped, offset, reversingFactor);
							}
						}
						else
						{
							if (driverDriveby)
							{
								ChangeOffsetForCarDriverDriveBy(ped, offset, reversingFactor);
							}
							else
							{
								ChangeOffsetForCarPassengerDriveBy(ped, offset, reversingFactor);
							}
						}
					}
				}

				FxSystem_c* gunflashFx = g_fxMan.CreateFxSystem(fxName, &offset, mat, true);
				//if (MixSets::G_GunflashEmissionMult > -1.0f) gunflashFx->SetRateMult(MixSets::G_GunflashEmissionMult);
				if (gunflashFx)
				{
					if (isInVehicle || !needsCustomMat || (!leftHand && dualWeildedWeapon))
					{
						gunflashFx->m_pParentMatrix = boneMat;
						if (isInVehicle)
							gunflashFx->SetTimeMult(inVehicleTimeMult);
					}
					if (needsCustomMat)
					{
						if (dualWeildingSKill)
						{
							gunflashFx->SetTimeMult(dualWeildingTimeMult);
						}
						else
						{
							gunflashFx->SetTimeMult(singleWeaponWeildingTimeMult);
						}
					}
					RwMatrixRotate(&gunflashFx->m_localMatrix, &axis_z, -90.0f, rwCOMBINEPRECONCAT);
					if (rotate)
					{
						RwMatrixRotate(&gunflashFx->m_localMatrix, &axis_y, CGeneral::GetRandomNumberInRange(0.0f, 360.0f), rwCOMBINEPRECONCAT);
					}
					gunflashFx->PlayAndKill();
				}
				if (smoke)
				{
					if (!ped->m_pVehicle || ped->m_pVehicle->m_vecMoveSpeed.Magnitude() < 0.15f)
					{
						FxSystem_c* smokeFx = g_fxMan.CreateFxSystem("gunsmoke", &offset, mat, true);
						if (smokeFx) {
							RwMatrixRotate(&smokeFx->m_localMatrix, &axis_z, -90.0f, rwCOMBINEPRECONCAT);
							smokeFx->PlayAndKill();
						}
					}
				}
			}
		}
	}
	pedExt.Get(ped).Reset();
}