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
//#include <Windows.h>
//#include <shlobj.h>
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

/*
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

static void GetBikePassengerDrivebyAnimations(unsigned short id, unsigned int& front, unsigned int& left, unsigned int& right, unsigned int& back)
{
    front = 826417989;
    left = 1918643658;
    right = 1692705712;
    back = 910920601;
}
*/

//BIKEV animation group
const unsigned int PIZZABOY = 448;
const unsigned int FAGGIO = 462;

//BIKEH animation group
const unsigned int FREEWAY = 463;

//BIKED animation group
const unsigned int SANCHEZ = 468;

//QUAD animation group
const unsigned int QUAD = 471;

//BMX animation group
const unsigned int BMX = 481;

//MTB animation group
const unsigned int BIKE = 509;
const unsigned int MBIKE = 510;

//BIKES animation group
const unsigned int FCR = 521;
const unsigned int NRG = 522;
const unsigned int PCJ = 461;
const unsigned int BF = 581;
const unsigned int HPV = 523;

//WAYFARER animation group
const unsigned int WAYFARER = 586;

static bool IsPedInVehicle(CPed* ped, bool& driverDB)
{
    if (!ped->m_pIntelligence) {
        return false;
    }

    CTask* activeTask = ped->m_pIntelligence->m_TaskMgr.GetSimplestActiveTask();

    if (activeTask)
    {
        eTaskType taskType = activeTask->GetId();
        if (taskType == TASK_SIMPLE_CAR_DRIVE)
        {
            driverDB = true;
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


const int BIKE_APPEREANCE = 2;
static bool IsPedInBike(CPed* ped)
{
    return (ped->m_pVehicle->GetVehicleAppearance()) == BIKE_APPEREANCE ? true : false;
}

static void GetBikeDriverDrivebyAnimations(unsigned short id, unsigned int& front, unsigned int& left, unsigned int& right)
{
    switch (id)
    {
    case PIZZABOY: case FAGGIO:
    {
        front = 407980065;
        left = 1442027987;
        right = 1128994729;
        break;
    }
    case FREEWAY:
    {
        front = 3838945260;
        left = 2967119665;
        right = 2791450955;
        break;
    }
    case SANCHEZ:
    {
        front = 4006294827;
        left = 3050367664;
        right = 2741527754;
        break;
    }
    case QUAD:
    {
        front = 3751445549;
        left = 1552031284;
        right = 1245352014;
        break;
    }
    case BMX:
    {
        front = 2659948553;
        left = 3673166122;
        right = 3427311440;
        break;
    }
    case BIKE: case MBIKE:
    {
        front = 1678968350;
        left = 3368341863;
        right = 3732692765;
        break;

    }
    case FCR: case NRG: case PCJ: case BF: case HPV:
    {
        front = 1762059363;
        left = 3444633283;
        right = 3689439417;
        break;

    }
    case WAYFARER:
    {
        front = 760534612;
        left = 1974016510;
        right = 1662097284;
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

const RwReal bikeDriverOffsetFactor = 2.0f;
static void ChangeOffsetForDriverBikeDriveBy(CPed* ped, RwV3d& offset, RwReal reversingFactor)
{

    unsigned int carModel = ped->m_pVehicle->m_nModelIndex;
    unsigned int dFrontAnim, dLeftAnim, dRightAnim;
    GetBikeDriverDrivebyAnimations(carModel, dFrontAnim, dLeftAnim, dRightAnim);

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

const unsigned int ANIM_HASH_PASSENGER_DBFRONT_BIKE = 826417989;
const unsigned int ANIM_HASH_PASSENGER_DBLEFT_BIKE = 1918643658;
const unsigned int ANIM_HASH_PASSENGER_DBRIGHT_BIKE = 1692705712;
const unsigned int ANIM_HASH_PASSENGER_DBBACK_BIKE = 910920601;

const RwReal bikePassengerOffsetFactor = 1.15f;
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


// Named constants for animation hash keys
const unsigned int ANIM_HASH_DBRIGHT_CAR = 2872216017;
const unsigned int ANIM_HASH_DBLEFT_CAR = 1362998450;
const unsigned int ANIM_HASH_DBLEFT2_CAR = 115705905;
const unsigned int ANIM_HASH_DBRIGHT2_CAR = 4243240274;

const unsigned int ANIM_HASH_TOP_DRIVEBY_RIGHT_SHOOTING_LEFT = 2338707153;
const unsigned int ANIM_HASH_TOP_DRIVEBY_LEFT_SHOOTING_RIGHT = 2648529067;
const unsigned int ANIM_HASH_RIGHT_DRIVEBY_SHOOTING_RIGHT = 2664255580;
const unsigned int ANIM_HASH_LEFT_DRIVEBY_SHOOTING_FRONT = 3887475062;
const unsigned int ANIM_HASH_LEFT_DRIVEBY_SHOOTING_BACK = 3770646954;
const unsigned int ANIM_HASH_LEFT_DRIVEBY_SHOOTING_LEFT = 2289425958;
const unsigned int ANIM_HASH_LEFT_DRIVEBY_SHOOTING_FRONT_2 = 3495415525;
const unsigned int ANIM_HASH_LEFT_DRIVEBY_SHOOTING_BACK_2 = 3613287993;

static void ChangeOffsetForCarDriverDriveBy(CPed* ped, RwV3d& offset, RwReal& reversingFactor)
{
    if (ped->m_pRwClump)
    {
        unsigned int totalAnims = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
        if (totalAnims > 0)
        {
            const RwReal CAR_DRIVER_OFFSET_FACTOR = 2.0f;
            const RwReal posDeltaDriver = ped->m_pVehicle->m_fMovingSpeed * CAR_DRIVER_OFFSET_FACTOR * reversingFactor;
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
            const RwReal CAR_PASSENGER_OFFSET_FACTOR = 1.15f;
            const RwReal posDeltaPassenger = ped->m_pVehicle->m_fMovingSpeed * CAR_PASSENGER_OFFSET_FACTOR * reversingFactor;
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


const unsigned int GUNMOVE_BWD_HASHKEY = 205664729;
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
                if (association->m_pHierarchy->m_hashKey == GUNMOVE_BWD_HASHKEY)
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
    const RwReal onFootOffset = 2.00f * reverseFactor;

    mat->pos.x += ped->m_vecMoveSpeed.x * onFootOffset;
    mat->pos.y += ped->m_vecMoveSpeed.y * onFootOffset;
    mat->pos.z += ped->m_vecMoveSpeed.z * onFootOffset;
}

static bool CanWeaponBeDualWielded(const int model)
{
    switch (model)
    {
    case MODEL_COLT45:
    {
        return true;
    }
    case MODEL_SAWNOFF:
    {
        return true;
    }
    case MODEL_MICRO_UZI:
    {
        return true;
    }
    case MODEL_TEC9:
    {
        return true;
    }
    default:
    {
        return false;
    }
    }
}

const float inVehMult = 1.35f;
const float dualWeildingMult = 1.25;
void Gunflashes::CreateGunflashEffectsForPed(CPed* ped) {
    bool ary[2];
    ary[0] = pedExt.Get(ped).bLeftHandGunflashThisFrame;
    ary[1] = pedExt.Get(ped).bRightHandGunflashThisFrame;

    bool driverDB = false;
    const bool isInVehicle = IsPedInVehicle(ped, driverDB);
    const bool needsCustomMat = CanWeaponBeDualWielded(ped->m_nWeaponModelId);
    const bool isInBike = isInVehicle ? IsPedInBike(ped) : false;

    for (int i = 0; i < 2; i++) {
        if (ary[i]) {

            if (pedExt.Get(ped).pMats[i] == nullptr) pedExt.Get(ped).pMats[i] = new RwMatrix();
            RwMatrix* mat = pedExt.Get(ped).pMats[i];
            if (!mat) break;

            bool leftHand = i == 0;
            if (ped->m_pRwObject && ped->m_pRwObject->type == rpCLUMP) {
                bool rotate = true;
                bool smoke = true;
                char* fxName = "gunflash";

                char weapSkill = ped->GetWeaponSkill(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType);
                CWeaponInfo* weapInfo = CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, weapSkill);
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
                else
                {
                    if (ped->m_pVehicle->m_fMovingSpeed > 0.0f)
                    {
                        RwReal reversingFactor = (ped->m_pVehicle->m_nCurrentGear == 0) ? -1.0f : 1.0f;
                        if (isInBike)
                        {
                            if (driverDB)
                            {
                                ChangeOffsetForDriverBikeDriveBy(ped, offset, reversingFactor);
                            }
                            else
                            {
                                ChangeOffsetForPassengerBikeDriveBy(ped, offset, reversingFactor);
                            }
                        }
                        else
                        {
                            if (driverDB)
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
                    if (isInVehicle || !needsCustomMat || (!leftHand && ped->m_nWeaponSkill > (char)2))
                    {
                        gunflashFx->m_pParentMatrix = boneMat;
                        if (isInVehicle)
                            gunflashFx->SetTimeMult(inVehMult);
                    }
                    if (needsCustomMat)
                    {
                        gunflashFx->SetTimeMult(dualWeildingMult);
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