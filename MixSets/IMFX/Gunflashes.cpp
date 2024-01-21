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
/*added by m0b
#include <Windows.h>
#include <shlobj.h>
*/
#include <CAnimManager.h>
#include <CAnimBlendAssociation.h>
#include <CAnimBlendHierarchy.h>
#include <CAnimBlock.h>
#include <CAnimBlendAssocGroup.h>

#include "..\MixSets.h"


using namespace plugin;

//RwMatrix Gunflashes::matrixAry[20];
unsigned int Gunflashes::matrixCounter = 0;
PedExtendedData<Gunflashes::PedExtension> Gunflashes::pedExt;
//std::vector<GunflashInfo> Gunflashes::gunflashInfos;
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

/*void Gunflashes::ReadSettings() {
    std::ifstream settingsFile(PLUGIN_PATH("imfx\\gunflash.dat"));
    if (settingsFile.is_open()) {
        for (std::string line; getline(settingsFile, line); ) {
            if (line[0] != ';' && line[0] != '#') {
                GunflashInfo info;
                unsigned int rotation; unsigned int smoke;
                if (sscanf(line.c_str(), "%d %s %d %d", &info.weapId, info.fxName, &rotation, &smoke) == 4) {
                    info.rotate = rotation ? true : false;
                    info.smoke = smoke ? true : false;
                    gunflashInfos.push_back(info);
                }
            }
        }
    }
}*/

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

////////////////
//added by m0b//
////////////////

const unsigned int PIZZABOY = 448;
const unsigned int FAGGIO = 462;

const unsigned int FREEWAY = 463;

const unsigned int SANCHEZ = 468;

const unsigned int QUAD = 471;

const unsigned int BMX = 481;

const unsigned int BIKE = 509;
const unsigned int MBIKE = 510;

const unsigned int FCR = 521;
const unsigned int NRG = 522;
const unsigned int PCJ = 461;
const unsigned int BF = 581;
const unsigned int HPV = 523;

const unsigned int WAYFARER = 586;

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
        unsigned int numAnimations = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
        if (numAnimations > 0)
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

static void ChangeOnFootMuzzleFlashOffset(CPed* ped, RwV3d& offset)
{
    int weaponModel = ped->m_nWeaponModelId;
    if (ped->m_nWeaponModelId == MODEL_MICRO_UZI ||
        ped->m_nWeaponModelId == MODEL_TEC9 ||
        ped->m_nWeaponModelId == MODEL_SAWNOFF
        )
    {
        const float pedOffsetFactor = 5.5f;
        offset.x += ped->m_fMovingSpeed * pedOffsetFactor;
    }
    else
    {
        if (ped->m_pRwClump)
        {
            float pedOffsetFactor = 3.0;

            if( ped->m_nWeaponModelId == MODEL_DESERT_EAGLE ||
                ped->m_nWeaponModelId == MODEL_SILENCED
                )
            {
                pedOffsetFactor = 5.5f;
            }

            const float pedSidewaysOffsetFactor = 3.0f;
            unsigned int numAnimations = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
            if (numAnimations > 0)
            {
                CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
                while (association)
                {
                    if (association->m_pHierarchy->m_hashKey == 189503749) //ANIM_DEFAULT_GUNMOVE_FWD hashkey
                    {
                        offset.x += ped->m_fMovingSpeed * pedOffsetFactor;
                    }
                    else
                    if (association->m_pHierarchy->m_hashKey == 205664729) //ANIM_DEFAULT_GUNMOVE_BWD hashkey
                    {
                        offset.x -= ped->m_fMovingSpeed * pedOffsetFactor;
                    }
                    if (association->m_pHierarchy->m_hashKey == 434959174) //ANIM_DEFAULT_GUNMOVE_L hashkey
                    {
                        offset.y += ped->m_fMovingSpeed * pedSidewaysOffsetFactor;
                    }
                    else
                    if (association->m_pHierarchy->m_hashKey == 3823357477) //ANIM_DEFAULT_GUNMOVE_R hashkey
                    {
                        offset.y -= ped->m_fMovingSpeed * pedSidewaysOffsetFactor;
                    }
                    association = RpAnimBlendGetNextAssociation(association);
                }
            }
        }
    }
}
*/
static bool IsPedInBike(CPed* ped, bool inVehicle)
{
    const int BIKE_APPEREANCE = 2;
    if (!inVehicle)
        return false;
    return ( ped->m_pVehicle->GetVehicleAppearance() ) == BIKE_APPEREANCE ? true : false;
}

static void IsPedSideCarLeftDrivebying(CPed* ped, bool& left, bool& driver)
{
    driver = false;
    if (ped->m_pRwClump)
    {
        unsigned int numAnimations = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
        if (numAnimations > 0)
        {
            CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
            while (association)
            {
                if (association->m_pHierarchy->m_hashKey == 2872216017) //DBRIGHT CAR
                {
                    left = false;
                    driver = true;
                }
                if (association->m_pHierarchy->m_hashKey == 1362998450) //DBLEFT CAR
                {
                    left = true;
                    driver = true;
                }
                association = RpAnimBlendGetNextAssociation(association);
            }
        }
    }
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

static void GetBikePassengerDrivebyAnimations(unsigned short id, unsigned int& front, unsigned int& left, unsigned int& right, unsigned int& back)
{
    front = 826417989;
    left = 1918643658;
    right = 1692705712;
    back = 910920601;
}

static void ChangeOffsetForBikeDriveBy(CPed* ped, RwV3d& offset, float reversingFactor)
{
    const float bikeOffsetFactor = 2.0f;

    unsigned int carModel = ped->m_pVehicle->m_nModelIndex;
    unsigned int dFrontAnim, dLeftAnim, dRightAnim;
    unsigned int pFrontAnim, pLeftAnim, pRightAnim, pBackAnim;
    GetBikeDriverDrivebyAnimations(carModel, dFrontAnim, dLeftAnim, dRightAnim);
    GetBikePassengerDrivebyAnimations(carModel, pFrontAnim, pLeftAnim, pRightAnim, pBackAnim);

    if (ped->m_pRwClump)
    {
        unsigned int numAnimations = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
        if (numAnimations > 0)
        {
            CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
            while (association)
            {
                if (association->m_pHierarchy->m_hashKey == dFrontAnim)
                {
                    offset.x += ped->m_pVehicle->m_fMovingSpeed * bikeOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == dLeftAnim)
                {
                    offset.z -= ped->m_pVehicle->m_fMovingSpeed * bikeOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == dRightAnim)
                {
                    offset.z += ped->m_pVehicle->m_fMovingSpeed * bikeOffsetFactor * reversingFactor;
                    break;
                }
                if (association->m_pHierarchy->m_hashKey == pFrontAnim)
                {
                    offset.x += ped->m_pVehicle->m_fMovingSpeed * bikeOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == pLeftAnim)
                {
                    offset.z -= ped->m_pVehicle->m_fMovingSpeed * bikeOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == pRightAnim)
                {
                    offset.z += ped->m_pVehicle->m_fMovingSpeed * bikeOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == pBackAnim)
                {
                    offset.x -= ped->m_pVehicle->m_fMovingSpeed * bikeOffsetFactor * reversingFactor;
                    break;
                }
                association = RpAnimBlendGetNextAssociation(association);
            }
        }
    }
}

static void ChangeOffsetForCarDriveBy(CPed* ped, RwV3d& offset, float& reversingFactor)
{
    const float carDriverOffsetFactor = 2.0f;
    const float carPassengerOffsetFactor = 1.15f;

    if (ped->m_pRwClump)
    {
        unsigned int numAnimations = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
        if (numAnimations > 0)
        {
            CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
            while (association)
            {
                //DRIVER DRIVEBY
                if (association->m_pHierarchy->m_hashKey == 2872216017) //DBRIGHT CAR
                {
                    offset.z += ped->m_pVehicle->m_fMovingSpeed * carDriverOffsetFactor * reversingFactor;
                    break;
                }
                if (association->m_pHierarchy->m_hashKey == 1362998450) //DBLEFT CAR
                {
                    offset.z -= ped->m_pVehicle->m_fMovingSpeed * carDriverOffsetFactor * reversingFactor;
                    break;
                }

                //PASSGENGER TOP RIGHT LEFT DRIVEBY
                if (association->m_pHierarchy->m_hashKey == 2338707153) // TOP_DRIVEBY_RIGHT_SHOOTING_LEFT
                {
                    offset.z -= ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == 2648529067) // TOP_DRIVEBY_LEFT_SHOOTING_RIGHT
                {
                    offset.z += ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor * reversingFactor;
                    break;
                }

                //PASSGENGER TRIGHT SEATS DRIVEBY
                else if (association->m_pHierarchy->m_hashKey == 2664255580) // RIGHT_DRIVEBY_SHOOTING_RIGHT
                {
                    offset.z += ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == 3887475062) // LEFT_DRIVEBY_SHOOTING_FRONT
                {
                    offset.x += ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == 3770646954) // LEFT_DRIVEBY_SHOOTING_BACK
                {
                    offset.x -= ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor * reversingFactor;
                    break;
                }

                //PASSGENGER TLEFT SEATS DRIVEBY
                else if (association->m_pHierarchy->m_hashKey == 2289425958) // LEFT_DRIVEBY_SHOOTING_LEFT
                {
                    offset.z -= ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == 3495415525) // LEFT_DRIVEBY_SHOOTING_FRONT
                {
                    offset.x += ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor  * reversingFactor;
                    break;
                }
                else if (association->m_pHierarchy->m_hashKey == 3613287993) // LEFT_DRIVEBY_SHOOTING_BACK
                {
                    offset.x -= ped->m_pVehicle->m_fMovingSpeed * carPassengerOffsetFactor * reversingFactor;
                    break;
                }
                association = RpAnimBlendGetNextAssociation(association);
            }
        }
    }
}


static bool ChangeOnBikeMuzzleFlashOffset(CPed* ped, RwV3d& offset)
{
    bool driver = false;
    if (ped->m_pRwClump)
    {
        unsigned int numAnimations = RpAnimBlendClumpGetNumAssociations(ped->m_pRwClump);
        if (numAnimations > 0)
        {
            float reversingFactor = 1.0f;
            CAnimBlendAssociation* association = RpAnimBlendClumpGetFirstAssociation(ped->m_pRwClump);
            while (association)
            {
                association = RpAnimBlendGetNextAssociation(association);
            }
        }
    }
    return false;
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
///////////////
///end added///
///////////////

void Gunflashes::CreateGunflashEffectsForPed(CPed* ped) {
    bool ary[2];
    ary[0] = pedExt.Get(ped).bLeftHandGunflashThisFrame;
    ary[1] = pedExt.Get(ped).bRightHandGunflashThisFrame;
    bool inVehicle = pedExt.Get(ped).bInVehicle;

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

                /*for (GunflashInfo &info : gunflashInfos) {
                    if (info.weapId == ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_nType) {
                        rotate = info.rotate;
                        smoke = info.smoke;
                        fxName = info.fxName;
                        break;
                    }
                }*/

                char weapSkill = ped->GetWeaponSkill(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType);
                CWeaponInfo* weapInfo = CWeaponInfo::GetWeaponInfo(ped->m_aWeapons[ped->m_nActiveWeaponSlot].m_eWeaponType, weapSkill);
                RwV3d offset = weapInfo->m_vecFireOffset.ToRwV3d();
                if (leftHand)
                {
                    offset.z *= -1.0f;
                
                }
                /*this is my old function, not really accurate
                if (ped->m_nPedFlags.bInVehicle == false)
                {
                    if(ped->m_fMovingSpeed > 0.0f)
                    {
                        ChangeOnFootMuzzleFlashOffset(ped, offset);
                    }
                }
                else
                if(IsVehiclePointerValid(ped->m_pVehicle))
                {
                    if (ped->m_pVehicle->m_fMovingSpeed > 0.0f)
                    {
                        float reversingFactor = (ped->m_pVehicle->m_nCurrentGear == 0) ? -1.0f : 1.0f;
                        if (IsPedInBike(ped, inVehicle))
                        {
                            ChangeOffsetForBikeDriveBy(ped, offset, reversingFactor);
                        }
                        else
                        {
                            ChangeOffsetForCarDriveBy(ped, offset, reversingFactor);
                        }
                    }
                }
                */

                //DebugPedAnim(ped);
                static RwV3d axis_y = { 0.0f, 1.0f, 0.0f };
                static RwV3d axis_z = { 0.0f, 0.0f, 1.0f };
                RpHAnimHierarchy* hierarchy = GetAnimHierarchyFromSkinClump(ped->m_pRwClump);
                RwMatrix* boneMat = &RpHAnimHierarchyGetMatrixArray(hierarchy)[RpHAnimIDGetIndex(hierarchy, 24 + (10 * leftHand))];
                memcpy(mat, boneMat, sizeof(RwMatrix));
                RwMatrixUpdate(mat);

                //added by m0b
                bool needsCustomMat = CanWeaponBeDualWielded(ped->m_nWeaponModelId);

                if (!IsVehiclePointerValid(ped->m_pVehicle))
                {
                    //Fix offset while moving
                    if (ped->m_fMovingSpeed > 0.0f && needsCustomMat)
                    {
                        const float onFootOffset = 1.75f;
                        mat->pos.x += ped->m_vecMoveSpeed.x * onFootOffset;
                        mat->pos.y += ped->m_vecMoveSpeed.y * onFootOffset;
                        mat->pos.z += ped->m_vecMoveSpeed.z * onFootOffset;
                    }
                }
                else
                {
                    if (ped->m_pVehicle->m_fMovingSpeed > 0.0f)
                    {
                        float reversingFactor = (ped->m_pVehicle->m_nCurrentGear == 0) ? -1.0f : 1.0f;
                        if (IsPedInBike(ped, inVehicle))
                        {
                            ChangeOffsetForBikeDriveBy(ped, offset, reversingFactor);
                        }
                        else
                        {
                            ChangeOffsetForCarDriveBy(ped, offset, reversingFactor);
                        }
                    }
                }

                FxSystem_c* gunflashFx = g_fxMan.CreateFxSystem(fxName, &offset, mat, true);
                //if (MixSets::G_GunflashEmissionMult > -1.0f) gunflashFx->SetRateMult(MixSets::G_GunflashEmissionMult);
                if (gunflashFx) 
                {
                    if (ped->m_nPedFlags.bInVehicle || !needsCustomMat)
                    {
                        gunflashFx->m_pParentMatrix = boneMat;
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