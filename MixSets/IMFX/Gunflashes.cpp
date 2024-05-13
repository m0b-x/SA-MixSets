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
#include <CAnimManager.h>
#include <CAnimBlendAssociation.h>
#include <CAnimBlendHierarchy.h>
#include <CAnimBlock.h>
#include <CAnimBlendAssocGroup.h>

#include "..\MixSets.h"
#include "extensions\ScriptCommands.h"

using namespace plugin;

const bool isLeftHand[2] = { true, false };

const int GUNFLASH_LIGHT_R = 70;
const int GUNFLASH_LIGHT_G = 55;
const int GUNFLASH_LIGHT_B = 22;

const float GUNFLASH_LIGHT_RANGE = 3.5f;

const float GUNFLASH_PLAYER_OFFSET_X = 0.0f;
const float GUNFLASH_PLAYER_OFFSET_Y = 1.3f;
const float GUNFLASH_PLAYER_OFFSET_Z = 1.0f;

const int GUNFLASH_SHADOW_ID = 3;
const int GUNFLASH_SHADOW_INTENSITY = 1;
const float GUNFLASH_SHADOW_RADIUS = 2.0f;
const float GUNFLASH_SHADOW_ANGLE = 0.0f;

unsigned int Gunflashes::matrixCounter = 0;
PedExtendedData<Gunflashes::PedExtension> Gunflashes::pedExt;
bool Gunflashes::bLeftHand = false;
bool Gunflashes::bVehicleGunflash = false;

RwReal onfootOffsetFactor = 0.0f;
RwReal onfootReverseFactor = 0.0f;
RwReal surfingOfsetFactor = 0.0f;
RwReal carDriverOffsetFactor = 0.0f;
RwReal carPassengerOffsetFactor = 0.0f;
RwReal bikePassengerOffsetFactor = 0.0f;
RwReal bikeDriverOffsetFactor = 0.0f;

bool gunflashLowerLight = false;

float inVehicleTimeMult = 1.0f; 
float surfingTimeMult = 1.0f;
float dualWeildingTimeMult = 1.0f;
float singleWeaponWeildingTimeMult = 1.0f;
float jetpackTimeMult = 1.0f;

float surfingSpeed = 0.2f;

RwReal mopedFixOffset = 2.5f;
RwReal carXFixOffset = 3.0f;

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

void Gunflashes::SetSurfingOffsetFactor(const RwReal newValue) {
	surfingOfsetFactor = newValue;
}


void Gunflashes::SetOnFootReverseFactor(const RwReal newValue) {
	onfootReverseFactor = newValue;
}

void Gunflashes::SetGunflashLowerLight(const bool newValue) {
	gunflashLowerLight = newValue;
}

void Gunflashes::SetJetpackTimeMult(const float newValue) {
	jetpackTimeMult = newValue;
}

void Gunflashes::SetSurfingTimeMult(const float newValue) {
	surfingTimeMult = newValue;
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

const int weaponArraySize = WEAPON_MINIGUN + 1;
struct WeaponData
{
	char* particleName = "gunflash";
	bool rotate = true;
	bool smoke = true;
};
WeaponData weaponArray[weaponArraySize];

void Gunflashes::AddDefaultWeaponData()
{
	for (unsigned int weaponID = WEAPON_PISTOL; weaponID <= WEAPON_SNIPERRIFLE; weaponID++)
	{
		WeaponData weapon;
		weapon.particleName = "gunflash";
		weapon.smoke = true;
		weapon.rotate = true;

		weaponArray[weaponID] = weapon;
	}

	//minigun exception
	unsigned int minigunID = 38;

	WeaponData weapon;
	weapon.particleName = "gunflash";
	weapon.smoke = true;
	weapon.rotate = true;

	weaponArray[minigunID] = weapon;
}

void Gunflashes::UpdateWeaponData(unsigned int weaponID, const std::string particle, bool rotate, bool smoke)
{
	if (weaponID < weaponArraySize)
	{
		WeaponData weapon;

		char* particleCharPointer = _strdup(particle.c_str());

		weapon.particleName = particleCharPointer;
		weapon.smoke = smoke;
		weapon.rotate = rotate;

		weaponArray[weaponID] = weapon;
	}
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

void Gunflashes::Setup()
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
			const RwReal posMopedFix = posDeltaDriver / mopedFixOffset;

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
					offset.x -= posMopedFix;
					offset.z -= posMopedFix;
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
			const RwReal posCarFix = posDeltaDriver / carXFixOffset;

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
					//Offset Fix
					offset.x -= posCarFix;
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


static void ChangeOnFootOffsetForCustomMatWeapons(CPed* ped, RwMatrix* mat)
{
	//ped moving backwards
	const RwReal reverseFactor = (ped->m_vecAnimMovingShiftLocal.y < 0.0f) ? onfootReverseFactor : 1.0f;
	const RwReal onFootOffset = onfootOffsetFactor * reverseFactor;

	mat->pos.x += ped->m_vecMoveSpeed.x * onFootOffset;
	mat->pos.y += ped->m_vecMoveSpeed.y * onFootOffset;
	mat->pos.z += ped->m_vecMoveSpeed.z * onFootOffset;
}

static void ChangeOnFootOffsetWhenSurfing(CPed* ped, RwMatrix* mat)
{
	mat->pos.x += ped->m_vecMoveSpeed.x * surfingOfsetFactor;
	mat->pos.y += ped->m_vecMoveSpeed.y * surfingOfsetFactor;
	mat->pos.z += ped->m_vecMoveSpeed.z * surfingOfsetFactor;
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
static void DrawGunflashLowerLight(CPed* ped)
{
	/*Source: DK22Pac - GTA IV Lights
	04C4: store_coords_to 10@ 11@ 12@ from_actor 3@ with_offset PED_OFFSET_X PED_OFFSET_Y PED_OFFSET_Z
	09E5: create_flash_light_at 10@ 11@ 12@ SHOT_LIGHT_R SHOT_LIGHT_G SHOT_LIGHT_B SHOT_FLASH_LIGHT_RADIUS
	016F: particle 3 rot 0.0 size SHOT_LIGHT_SIZE SHOT_LIGHT_INTENSITY SHOT_LIGHT_R SHOT_LIGHT_G SHOT_LIGHT_B at 10@ 11@ 12@
	*/
	float x = 0.0f, y = 0.0f, z = 0.0f;
	Command<Commands::GET_OFFSET_FROM_CHAR_IN_WORLD_COORDS>(ped, GUNFLASH_PLAYER_OFFSET_X, GUNFLASH_PLAYER_OFFSET_Y, GUNFLASH_PLAYER_OFFSET_Z, &x, &y, &z);
	Command<Commands::DRAW_LIGHT_WITH_RANGE>(x, y, z, GUNFLASH_LIGHT_R, GUNFLASH_LIGHT_G, GUNFLASH_LIGHT_B, GUNFLASH_LIGHT_RANGE);
	Command<Commands::DRAW_SHADOW>(GUNFLASH_SHADOW_ID, x, y, z, GUNFLASH_SHADOW_ANGLE, GUNFLASH_SHADOW_RADIUS, GUNFLASH_SHADOW_INTENSITY, GUNFLASH_LIGHT_R, GUNFLASH_LIGHT_G, GUNFLASH_LIGHT_B);
}

static eTaskType GetPedActiveTask(CPed* ped)
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

	bool handThisFrame[2];

	handThisFrame[0] = pedExt.Get(ped).bLeftHandGunflashThisFrame;
	handThisFrame[1] = pedExt.Get(ped).bRightHandGunflashThisFrame;

	for (int i = 0; i < 2; i++) {
		if (handThisFrame[i])
		{
			if (pedExt.Get(ped).pMats[i] == nullptr) pedExt.Get(ped).pMats[i] = new RwMatrix();
			RwMatrix* mat = pedExt.Get(ped).pMats[i];
			if (!mat) break;

			eTaskType task = GetPedActiveTask(ped);

			const bool driverDriveby = (task == TASK_SIMPLE_CAR_DRIVE);
			const bool isInVehicle = (driverDriveby || (task == TASK_SIMPLE_GANG_DRIVEBY)) && (IsVehiclePointerValid(ped->m_pVehicle));
			
			const bool isUsingJetpack = (task == TASK_SIMPLE_JETPACK);

			const bool surfing = (!isInVehicle && ped->m_fMovingSpeed > surfingSpeed && !isUsingJetpack);

			bool needsCustomMat = CanWeaponBeDualWeilded(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType);
			
			const bool isInBike = isInVehicle ? IsPedInBike(ped) : false;
			const bool isInMoped = isInBike ? IsPedInMoped(ped) : false;

			const bool weapSkill = ped->m_nWeaponSkill > (char)2;
			const bool dualWeildedWeapon = needsCustomMat && weapSkill;

			const unsigned int arrayIndex = ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType;

			if (ped->m_pRwObject && ped->m_pRwObject->type == rpCLUMP) {

				//initial particle time multiplier
				float particleTimeMult = 1.0f;

				bool rotate = true;
				bool smoke = true;
				char* fxName = "gunflash";

				//Check if the player's weapons matches the array
				if (arrayIndex < weaponArraySize)
				{
					fxName = weaponArray[arrayIndex].particleName;
					rotate = weaponArray[arrayIndex].rotate;
					smoke = weaponArray[arrayIndex].smoke;
				}

				char dualWeildingSKill = ped->GetWeaponSkill(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType);
				CWeaponInfo* weapInfo = CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, dualWeildingSKill);
				RwV3d offset = weapInfo->m_vecFireOffset.ToRwV3d();

				if (isLeftHand[i])
				{
					offset.z *= -1.0f;
				}

				static RwV3d axis_y = { 0.0f, 1.0f, 0.0f };
				static RwV3d axis_z = { 0.0f, 0.0f, 1.0f };

				RpHAnimHierarchy* hierarchy = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
				RwMatrix* boneMat = &RpHAnimHierarchyGetMatrixArray(hierarchy)[RpHAnimIDGetIndex(hierarchy, 24 + (10 * isLeftHand[i]))];
				memcpy(mat, boneMat, sizeof(RwMatrix));
				RwMatrixUpdate(mat);

				if (!isInVehicle)
				{
					if (ped->m_fMovingSpeed > 0.0f && needsCustomMat && !surfing)
					{
						ChangeOnFootOffsetForCustomMatWeapons(ped, mat);

						if (dualWeildingSKill)
						{
							particleTimeMult = dualWeildingTimeMult;
						}
						else
						{
							particleTimeMult = singleWeaponWeildingTimeMult;
						}

						if (isUsingJetpack)
						{
							particleTimeMult = jetpackTimeMult;
						}
					}

					if (surfing)
					{
						ChangeOnFootOffsetWhenSurfing(ped, mat);
						needsCustomMat = true;
						particleTimeMult = surfingTimeMult;
					}

				}
				else
				{
					if (ped->m_pVehicle->m_fMovingSpeed > 0.0f)
					{
						particleTimeMult = inVehicleTimeMult;
						
						RwReal reversingFactor = (ped->m_pVehicle->m_nCurrentGear == 0) ? -1.0f : 1.0f;

						if (!isInBike)
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
						else
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
					}
				}

				FxSystem_c* gunflashFx = g_fxMan.CreateFxSystem(fxName, &offset, mat, true);

				//if (MixSets::G_GunflashEmissionMult > -1.0f) gunflashFx->SetRateMult(MixSets::G_GunflashEmissionMult);
				
				if (gunflashFx)
				{
					if (isInVehicle || !needsCustomMat)
					{
						gunflashFx->m_pParentMatrix = boneMat;
					}
					//Set the final multipliers
					gunflashFx->SetTimeMult(particleTimeMult);

					RwMatrixRotate(&gunflashFx->m_localMatrix, &axis_z, -90.0f, rwCOMBINEPRECONCAT);
					if (rotate)
					{
						RwMatrixRotate(&gunflashFx->m_localMatrix, &axis_y, CGeneral::GetRandomNumberInRange(0.0f, 360.0f), rwCOMBINEPRECONCAT);
					}
					gunflashFx->PlayAndKill();

					if (gunflashLowerLight && !isInVehicle)
					{
						DrawGunflashLowerLight(ped);
					}
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