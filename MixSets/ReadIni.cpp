/*
	MixSets by Junior_Djjr and entire GTA 3D Era community
*/
#include "Common.h"
#include "CWorld.h"
#include "CTimer.h"
#include "CTheScripts.h"
#include "..\injector\assembly.hpp"
#include "IniReader/IniReader.h"
#include "CCam.h"
#include "CCamera.h"
#include "CWeather.h"
#include "CGame.h"
#include "Fx_c.h"
#include <filesystem>

using namespace plugin;
using namespace injector;
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////


// Globals
bool bEnabled, bReadOldINI, bParsePreserveComments, bErrorRename, inSAMP, rpSAMP, dtSAMP, bIMFX, bGunFuncs, bIniFailed, G_NoDensities, G_FixBicycleImpact,
G_NoStencilShadows, G_OpenedHouses, G_RandWheelDettach, G_TaxiLights, G_ParaLandingFix, G_NoEmergencyMisWanted, G_NoGarageRadioChange,
G_NoStuntReward, G_NoTutorials, G_EnableCensorship, G_HideWeaponsOnVehicle;

int G_i, G_FPSlimit, G_ProcessPriority, G_FreezeWeather, G_CameraPhotoQuality, G_UseHighPedShadows, G_StreamMemory, G_Anisotropic;

float G_f, G_CullDistNormalComps, G_CullDistBigComps, G_VehLodDist, G_VehDrawDist, G_PedDrawDist, G_VehMultiPassDist, G_GangWaveMinSpawnDist,
G_VehBulletDamage, G_VehFireDamage, G_VehExploDamage, G_HeliRotorSpeed, G_PedDensityExt, G_PedDensityInt, G_VehDensity,
G_VehDespawnOnScr, G_PedDespawnOnScr, G_PedDespawnOffScr, G_TowelSpawnOffScr, G_TrainSpawnDistance, G_ShadDist_Vehicles,
G_ShadDist_Vehicles_Sqr, G_ShadDist_SmallPlanes, G_ShadDist_SmallPlanes_Sqr, G_ShadDist_BigPlanes, G_ShadDist_BigPlanes_Sqr;
float G_ShadDist_CarLight, G_ShadDist_CarLight_Sqr, G_ShadDist_CarLight_Mid, G_ShadDist_CarLight_Min, G_ShadDist_CarLight_Scale, 
G_FootstepsDist, G_PickupsDrawDist, G_CarPedImpact, G_BikePedImpact, Default_BikePedImpact, Default_CarPedImpact,
G_HeliSensibility, G_PlaneTrailSegments, G_SkidHeight, G_SunSize, G_RhinoFireRange, G_VehOccupDrawDist,
G_VehOccupDrawDist_Boat, G_BrakePower, G_BrakeMin, G_TireEff_DustLife, G_TireEff_DustFreq, G_TireEff_DustSize;
float G_TireEff_DustUpForce, G_TireSmk_UpForce, G_PedWeaponDrawDist, G_PedWeaponDrawDist_Final, G_PropCollDist_NEG, G_PropCollDist_POS,
G_MediumGrassDistMult, G_DistBloodpoolTex, G_RainGroundSplashNum, G_RainGroundSplashArea, G_RainGroundSplashArea_HALF, G_RoadblockSpawnDist,
G_RoadblockSpawnDist_NEG, G_PedPopulationMult, G_VehPopulationMult, G_FxEmissionRateShare;
float zero = 0.0;

int numOldCfgNotFound = 0;

string G_NoMoneyZeros_Pos, G_NoMoneyZeros_Neg, G_ReloadCommand;

CVehicle *secPlayerVehicle = nullptr;

// External vars from root
extern fstream lg;
extern languages lang;


///////////////////////////////////////////////////////////////////////////////////////////////////

void ReadOldINI(CIniReader ini, fstream *lg, string section, string key) {

	bParsePreserveComments = true;

	CIniReader iniold("MixSets old.ini");
	string oldIniLine = iniold.ReadString(section, key, "");

	if (oldIniLine.length() <= 0) {

		if (lang == languages::PT)
			*lg << "Aviso: " << section << ": " << key << " n�o encontrado no 'MixSets old.ini'. Se n�o � fun��o nova, ent�o o nome foi alterado na nova vers�o. Verifique.\n";
		else
			*lg << "Warning: " << section << ": " << key << " not found in 'MixSets old.ini'. If isn't new feature, then the name has changed in new version. Check it.\n";

		bParsePreserveComments = false;

		numOldCfgNotFound++;
		return;
	}

	CIniReader ininew("MixSets.ini");

	string newIniLine = ininew.ReadString(section, key, "");

	bParsePreserveComments = false;

	int newIniLineLen = newIniLine.find_first_of(" #");
	int oldIniLineLen = oldIniLine.find_first_of(" #");

	newIniLine.replace(0, newIniLineLen, oldIniLine, 0, oldIniLineLen);

	int charsRemoved = (oldIniLineLen - newIniLineLen);

	if (charsRemoved < 0) {
		while (charsRemoved < 0) {
			newIniLine.insert(oldIniLineLen, " ");
			charsRemoved++;
		}
	}
	else {
		while (charsRemoved > 0) {
			if (&newIniLine[oldIniLineLen] == "#") break;
			newIniLine.replace(oldIniLineLen, 1, "");
			charsRemoved--;
		}
	}

	newIniLine.insert(0, " ");

	ininew.WriteString(section, key, newIniLine, false);
}

inline bool FileExists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool ReadIniFloat(CIniReader ini, fstream *lg, string section, string key, float* f) {
	if (bReadOldINI) ReadOldINI(ini, lg, section, key);
	*f = ini.ReadFloat(section, key, -1);
	if (*f != -1) {
		*lg << section << ": " << key << " = " << fixed << *f << "\n";
		return true;
	}
	else return false;
}

bool ReadIniInt(CIniReader ini, fstream *lg, string section, string key, int* i) {
	if (bReadOldINI) ReadOldINI(ini, lg, section, key);
	*i = ini.ReadInteger(section, key, -1);
	if (*i != -1) {
		*lg << section << ": " << key << " = " << *i << "\n";
		return true;
	}
	else return false;
}

bool ReadIniBool(CIniReader ini, fstream *lg, string section, string key) {
	if (bReadOldINI) ReadOldINI(ini, lg, section, key);
	//bool b = ini.ReadBoolean(section, key, 0);
	bool b = ini.ReadInteger(section, key, 0) == 1;
	if (b == true) {
		*lg << section << ": " << key << " = true \n";
		return true;
	}
	else return false; 
}

void show3dlog(float x, float y, float z) {
	lg << x << " " << y << " " << z << "\n";
	lg.flush();
}

void showintlog(int i) {
	lg << i << "\n";
	lg.flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t CustomMaxAnisotropic()
{
	return G_Anisotropic;
}

float G_VehFlipDamage = 3.5f;

void __declspec(naked) VehFlipDamage_ASM() {
	_asm {
		fstp st(0)
		push esi
		cmp dword ptr[esp + 4], 0x00570E84
		jnz _RemoveHealth // CPed vehicle already in ESI
		mov esi, ecx

		_RemoveHealth:
			push ebx
			xor ebx, ebx

		_RemoveHealth_Loop:
			lea eax, [ebx * 0x04]
			mov eax, dword ptr[esi + 0x460 + eax]
			push eax
			call _RemoveHealthFromPed
			inc ebx
			cmp ebx, 0x09
			jne _RemoveHealth_Loop

			pop ebx
			pop esi
			retn

		_RemoveHealthFromPed: // void __stdcall (*)(CPed*)                        
			mov eax, dword ptr[esp + 4]
			test eax, eax
			jz _RemoveHealthFromPed_Ret

			push eax
			push 0x0 // 0.0
			push G_VehFlipDamage
			mov ecx, 0x00B7CB5C // TimeStep
			fld dword ptr[ecx]
			fmul dword ptr[esp]
			fsubr dword ptr[eax + 0x540] // Ped.fHeath
			fld dword ptr[esp + 4] // 0.0
			fcomp st(1)
			fnstsw ax
			test ah, 0x41
			jnz _RemoveHealthFromPed_Ld // < 0.0
			fstp st(0)
			fld dword ptr[esp + 4] // 0.0

		_RemoveHealthFromPed_Ld:
			add esp, 0x08
			pop eax
			fstp dword ptr[eax + 0x540]

		_RemoveHealthFromPed_Ret:
			retn 4
	}
}

void __declspec(naked) BrakeReverseFix_ASM() {
	__asm {
		push   eax
		fld    dword ptr[esi + 0x49C]
		mov    edx, 0x00858B50
		fcomp  dword ptr[edx]
		fnstsw ax
		test   ah, 0x44
		jp     do_stuff
		mov    edx, 0x00B734A4
		mov    eax, dword ptr[edx]
		test   ax, ax
		jne    dont_do_stuff

		do_stuff:
		mov    dword ptr[esi + 0x4A0], 0
		pop    eax
		mov    dword ptr[esi + 0x49C], eax
		ret

		dont_do_stuff:
		pop    eax
		ret
	}
}

void __declspec(naked) SirenOnWithoutDriver_ASM() {
	__asm {
		cmp [esi + 0x328], 0
		jz back

		mov eax, 0x004EF850
		call eax

		mov[esi + 0x17C], ebx

		back:
		mov eax, 0x004F9DAF
		jmp eax
	}
}

void __declspec(naked) PedWeaponDrawDist_ASM() {
	G_PedWeaponDrawDist_Final = G_PedWeaponDrawDist * TheCamera.m_fLODDistMultiplier;
	__asm {
		fld     G_PedWeaponDrawDist_Final
		push    7336AEh
		ret
	}
}

DWORD _EAX;
void __declspec(naked) NoPauseWhenMinimize_AllowMouseMovement_ASM()
{
	_asm
	{
		mov _EAX, eax
		mov eax, dword ptr ds : [0x8D621C]
		cmp eax, 0
		jne label1
		mov eax, _EAX
		ret

		label1 :
		mov eax, _EAX
			mov _EAX, 0x7453F0
			jmp _EAX
	}
}

DWORD RETURN_FixMouseStuck = 0x74542B;
DWORD altRETURN_FixMouseStuck = 0x745433;
void _declspec(naked) FixMouseStuck_ASM()
{
	__asm
	{
		mov eax, [esp + 08h] // eax = y pos
		mov ecx, [esp + 04h] // ecx = x pos

		pushad
	}

	if (*(HWND*)0xC97C1C == GetForegroundWindow())
	{
		__asm
		{
			popad
			jmp RETURN_FixMouseStuck
		}
	}
	else
	{
		__asm
		{
			popad
			jmp altRETURN_FixMouseStuck
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void asm_fmul(float f) {
	_asm {fmul f}
}

///////////////////////////////////////////////////////////////////////////////////////////////////


void ReadIni() {
	if (!bEnabled) return;

	int i;
	float f;

	CIniReader ini("MixSets.ini");


	CIniReader iniold("MixSets old.ini");
	if (iniold.data.size() <= 0)
		bReadOldINI = false;
	else
		bReadOldINI = true;


	// -- System
	if (ReadIniInt(ini, &lg, "System", "StreamMemory", &i)) { 
		if (i > 2047) {
			G_StreamMemory = 2147483647;
		}
		else {
			G_StreamMemory = (i * 1048576);
		}
		WriteMemory<uint32_t>(0x8A5A80, G_StreamMemory, true);
	}
	else G_StreamMemory = -1;

	if (ReadIniInt(ini, &lg, "System", "FPSlimit", &i)) {
		WriteMemory<uint8_t>(0xC1704C, i, false);
		G_FPSlimit = i;
	}
	else G_FPSlimit = -1;
	
	if (ReadIniBool(ini, &lg, "System", "MouseFix")) {
		Call<0x7469A0>();
	}


	// -- Graphics
	if (ReadIniInt(ini, &lg, "Graphics", "MotionBlurAlpha", &i)) {
		WriteMemory<uint8_t>(0x8D5104, i, true);
	}
	
	if (ReadIniBool(ini, &lg, "Graphics", "NoMotionBlur")) {
		WriteMemory<uint8_t>(0x7030A0, 0xC3, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoVertigoEffect")) {
		WriteMemory<uint8_t>(0x524B3E, 0xEB, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoHeatEffect")) {
		WriteMemory<uint8_t>(0x72C1B7, 0xEB, true);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "LicenseTextFilter", &i)) {
		WriteMemory<uint8_t>(0x884958, i, false);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "TextureFilterMin", &i)) { 
		WriteMemory<uint8_t>(0x88498C, i, false);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "TextureFilterMax", &i)) { 
		WriteMemory<uint8_t>(0x884988, i, false);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "StencilShadowA", &i)) {
		WriteMemory<uint8_t>(0x71162C, i, true);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "StencilShadowR", &i)) {
		WriteMemory<uint8_t>(0x711631 + 1, i, true);
	}
	if (ReadIniInt(ini, &lg, "Graphics", "StencilShadowG", &i)) {
		WriteMemory<uint8_t>(0x71162F + 1, i, true);
	}
	if (ReadIniInt(ini, &lg, "Graphics", "StencilShadowB", &i)) {
		WriteMemory<uint8_t>(0x71162D + 1, i, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "UnderwaterEffect")) {
		WriteMemory<uint8_t>(0xC402D3, 1, false);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Graphics", "NoNitroBlur")) {
		MakeNOP(0x704E13, 17, true);
		WriteMemory<uint8_t>(0x704E24, 0xE9, true);
		WriteMemory<uint32_t>(0x704E25, 0xD4, true);
		WriteMemory<uint8_t>(0x704E29, 0x90, true);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "UseHighPedShadows", &i)) {
		G_UseHighPedShadows = i;
	}
	else G_UseHighPedShadows = -1;

	if (ReadIniBool(ini, &lg, "Graphics", "NoPlaneTrails")) {
		MakeNOP(0x7185B0, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "No3DGunflash")) {
		MakeNOP(0x5E5F2A, 20, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "Fix2DGunflash")) {
		if (!bIMFX && !bGunFuncs)
		{
			injector::MakeInline<0x73F3A5, 0x73F3A5 + 6>([](injector::reg_pack& regs)
			{
				//mov     eax, [esi+460h]
				regs.eax = *(uint32_t*)(regs.esi + 0x460);

				CVehicle *vehicle = (CVehicle *)regs.esi;
				CWeapon *weapon = *(CWeapon **)(regs.esp + 0x28);
				CVector *pointIn = (CVector *)(regs.esp + 0x2C);
				CVector *pointOut = (CVector *)(regs.esp + 0x44);
				CWeaponInfo *weaponInfo;

				CVector *gunshellPos;
				CVector gunshellDir;

				showintlog(weapon->m_nType);
				show3dlog(pointIn->x, 0.0, 0.0);

				float posOffset;
				float gunshellSize;

				switch (weapon->m_nType)
				{
				case WEAPON_PISTOL:
				case WEAPON_PISTOL_SILENCED:
				case WEAPON_DESERT_EAGLE:
				case WEAPON_SNIPERRIFLE:
					posOffset = 0.2;
					gunshellSize = 0x3E800000;
					goto LABEL_149;
				case WEAPON_SHOTGUN:
				case WEAPON_SAWNOFF:
				case WEAPON_SPAS12:
					posOffset = 0.30000001;
					gunshellSize = 0x3EE66666;
					goto LABEL_149;
				case WEAPON_MICRO_UZI:
				case WEAPON_MP5:
				case WEAPON_TEC9:
					posOffset = 0.2;
					gunshellSize = 0x3E99999A;
					goto LABEL_149;
				case WEAPON_AK47:
				case WEAPON_M4:
				case WEAPON_MINIGUN:
					weaponInfo = CWeaponInfo::GetWeaponInfo(weapon->m_nType, 1);
					if (((weaponInfo->m_fAnimLoopEnd - weaponInfo->m_fAnimLoopStart) * 900.0) >= 50 || (*(char*)0xC8A80C += 1, !(*(char*)0xC8A80C & 1)))
					{
						posOffset = 0.64999998;
						gunshellSize = 0x3E800000;
					LABEL_149:
						g_fx.TriggerGunshot(vehicle, *pointIn, *pointOut, true);
						g_fx.TriggerGunshot(vehicle, *pointOut, *pointIn, true);
					}
					break;
				default:
					break;
				}
			});
		}
		else {
			lg << "Fix2DGunflash disabled because it isn't required\n";
		}
	}
	
	
	if (ReadIniInt(ini, &lg, "Graphics", "PlaneTrailsSegments", &i)) {
		WriteMemory<uint32_t>(0x7172E6 + 2, i, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "RainGroundSplashNum", &f)) {
		G_RainGroundSplashNum = f;
		WriteMemory<float*>(0x72AB16+2, &G_RainGroundSplashNum, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "RainGroundSplashSize", &f)) {
		WriteMemory<float>(0x72AB87 + 1, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "RainGroundSplashArea", &f)) {
		G_RainGroundSplashArea = f;
		WriteMemory<float*>(0x72ACEB + 2, &G_RainGroundSplashArea, true);
		WriteMemory<float*>(0x72AD12 + 2, &G_RainGroundSplashArea, true);
		G_RainGroundSplashArea_HALF = G_RainGroundSplashArea * 0.5f;
		WriteMemory<float*>(0x72AD1C + 2, &G_RainGroundSplashArea_HALF, true);
		WriteMemory<float*>(0x72ACF5 + 2, &G_RainGroundSplashArea_HALF, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoRainSteam")) {
		MakeNOP(0x72ADF0, 37, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoSandstormSteam")) {
		MakeNOP(0x72AAE0, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoRainNoise")) {
		MakeNOP(0x705078, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoPointLights")) {
		WriteMemory<uint8_t>(0x7000E0, 0xC3, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoVehSpecular")) {
		WriteMemory<uint8_t>(0x5D9ABE, 0, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoVehLighting")) {
		WriteMemory<uint8_t>(0x5D9A8F, 0, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "HeadlightSmoothMov")) {
		WriteMemory<uint32_t>(0x70C6A9, 0, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "VehShadowSmoothMov")) {
		WriteMemory<uint32_t>(0x70C2D6, 0, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoSkyStuff")) {
		MakeNOP(0x53DCA2, 5, true);
		MakeNOP(0x53DFA0, 5, true);
		MakeNOP(0x53E121, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoSun")) {
		MakeNOP(0x53C136, 5, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "SunSize", &f)) {
		G_SunSize = f;
		WriteMemory<float*>(0x6FC6EA, &G_SunSize, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "VehSparkSpread", &f)) {
		WriteMemory<float>(0x5458E1, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "BulletSparkForce", &f)) {
		WriteMemory<float>(0x49F47B, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "BulletSparkSpread", &f)) {
		WriteMemory<float>(0x49F451, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "BulletDustSize", &f)) {
		WriteMemory<float>(0x49F57E, f, true);
		WriteMemory<float>(0x49F4A5, f, true);
	}
	 
	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_StartSize", &f)) {
		WriteMemory<float>(0x006DF1C8 + 1, f, true);
		f *= 0.7f;
		WriteMemory<float>(0x006DEF5A + 1, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_DriftSize", &f)) {
		WriteMemory<float>(0x006DF20D, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_BrakeSize", &f)) {
		f *= 0.7f;
		WriteMemory<float>(0x006DED24 + 1, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_Life", &f)) {
		WriteMemory<float>(0x006DF1BE + 1, f, true);
		f *= 0.6f;
		WriteMemory<float>(0x006DED1A + 1, f, true);
		WriteMemory<float>(0x006DEF50 + 1, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_UpForce", &f)) {
		G_TireSmk_UpForce = f;
		WriteMemory<float*>(0x006DF2B9 + 2, &G_TireSmk_UpForce, true);
	}

	
	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_LumMin", &f)) {
		WriteMemory<float>(0x006DEE06, f, true);
		WriteMemory<float>(0x006DF046, f, true);
		WriteMemory<float>(0x006DF2A7, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_LumMax", &f)) {
		WriteMemory<float>(0x006DEE01, f, true);
		WriteMemory<float>(0x006DF041, f, true);
		WriteMemory<float>(0x006DF2A2, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_Alpha", &f)) {
		WriteMemory<float>(0x006DED29 + 1, f, true);
		WriteMemory<float>(0x006DEF5F + 1, f, true);
		WriteMemory<float>(0x006DF1CD + 1, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireSmk_DriftAlpha", &f)) {
		WriteMemory<float>(0x006DF205, f, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireEff_DustLife", &f)) {
		G_TireEff_DustLife = f;
		WriteMemory<float*>(0x004A079A + 2, &G_TireEff_DustLife, true);
		WriteMemory<float*>(0x004A0B4A + 2, &G_TireEff_DustLife, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "TireEff_DustSize", &f)) {
		G_TireEff_DustSize = f;
		WriteMemory<float*>(0x004A07A4 + 2, &G_TireEff_DustSize, true);
		WriteMemory<float*>(0x004A0B5C + 2, &G_TireEff_DustSize, true);
	}
	
	if (ReadIniFloat(ini, &lg, "Graphics", "TireEff_DustUpForce", &f)) {
		G_TireEff_DustUpForce = f;
		WriteMemory<float*>(0x004A0868 + 2, &G_TireEff_DustUpForce, true);
		WriteMemory<float*>(0x004A0C18 + 2, &G_TireEff_DustSize, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "PlaneSmokeLife", &f)) {
		WriteMemory<float>(0x006CA9E5 + 1, f, true); //stuntplane
		WriteMemory<float>(0x006CA953 + 1, (f * 0.666666f), true); //cropduster
	}
	
	if (ReadIniBool(ini, &lg, "Graphics", "TaxiLights")) {
		G_TaxiLights = true;
	}
	else G_TaxiLights = false;

	if (!inSAMP && ReadIniFloat(ini, &lg, "Graphics", "BulletTraceThickness", &f)) {
		WriteMemory<float>(0x726CEA + 1, f, true);
		WriteMemory<float>(0x73AFB7 + 1, (f * 2), true);
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Graphics", "BulletTraceAlpha", &i)) {
		WriteMemory<uint8_t>(0x726CDD + 1, i, true);
		i = i * 2.142857f;
		if (i > 255) i = 255;
		WriteMemory<uint32_t>(0x73AFAD + 1, i, true);
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Graphics", "BulletTraceRGB", &i)) {
		WriteMemory<uint32_t>(0x723CBD + 1, i, true);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "RopeAlpha", &i)) {
		WriteMemory<uint8_t>(0x5568A0 - 1, i, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoWaterFog")) {
		WriteMemory<uint8_t>(0x8D37D4, 0, false);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "WaterFogDensity", &i)) {
		WriteMemory<uint32_t>(0x8D37E0, i, false);
	}


	if (ReadIniInt(ini, &lg, "Graphics", "CameraPhotoQuality", &i)) {
		G_CameraPhotoQuality = i;
		injector::MakeInline<0x5D04E1, 0x5D04E1 + 7>([](injector::reg_pack& regs)
		{
			*(uint32_t*)(regs.esp + 0x0E8) = regs.edi;
			((void(__cdecl *)(int, signed int, char))0x5C6FA0)(regs.ecx, G_CameraPhotoQuality, true);
		});
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoMirrors")) {
		MakeNOP(0x555854, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoColorFilter")) {
		WriteMemory<uint8_t>(0x8D518C, 0, false);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoRainStreaks")) {
		MakeNOP(0x53E126, 5, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Graphics", "NoCopHeliLight")) {
		WriteMemory<uint8_t>(0x006C712A, 0, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoGoggleNoise")) {
		MakeNOP(0x704EE8, 5, true);
		MakeNOP(0x704F59, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoVolumetricClouds")) {
		MakeNOP(0x53E1B4, 5, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Graphics", "NoMovingFog")) {
		MakeNOP(0x53E1AF, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoBirds")) {
		MakeNOP(0x53E170, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoLowClouds")) {
		MakeNOP(0x53E121, 5, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoGrass")) {
		WriteMemory<uint8_t>(0x5DBAE0, 0xC3, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "MediumGrassDistMult", &f)) {
		G_MediumGrassDistMult = f;
		WriteMemory<float*>(0x5DAD88 + 2, &G_MediumGrassDistMult, true);
	}

	if (ReadIniFloat(ini, &lg, "Graphics", "FireCoronaSize", &f)) {
		G_MediumGrassDistMult = f;
		WriteMemory<float*>(0x53B784 + 2, &G_MediumGrassDistMult, true);
	}
	
	if (ReadIniInt(ini, &lg, "Graphics", "FireLensflare", &i)) {
		WriteMemory<uint8_t>(0x53B759, i, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "FireGroundLight")) {
		WriteMemory<uint8_t>(0x53B65A, 50, true); // the intensity value doesn't matter, idkw
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoFireCoronas")) {
		MakeNOP(0x53B688, 10, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoDismemberment")) {
		WriteMemory<uint8_t>(0x4B3A3C, 0xEB, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoEnexCones")) {
		WriteMemory<uint8_t>(0x440D6D, 0xE9, true);
		WriteMemory<uint32_t>(0x440D6E, 0x200, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "HideWeaponsOnVehicle")) {
		G_HideWeaponsOnVehicle = true;
	}
	else G_HideWeaponsOnVehicle = false;

	


	// -- Gameplay

	if ((!inSAMP || (inSAMP && rpSAMP)) && ReadIniBool(ini, &lg, "Gameplay", "ScrollReloadFix")) {
		MakeNOP(0x60B4FA, 6, true);
	}

	if ((!inSAMP || (inSAMP && rpSAMP)) && ReadIniBool(ini, &lg, "Gameplay", "VehBurnEngineBroke")) {
		injector::MakeInline<0x006A70ED, 0x006A70F3>([](injector::reg_pack& regs) {
			auto vehicle = (CVehicle*)regs.esi;
			vehicle->m_nVehicleFlags.bEngineOn = false;
			regs.eax = *(DWORD*)(regs.esi + 0x57C); //original code
		});

		injector::MakeInline<0x006A75F9, 0x006A75FF>([](injector::reg_pack & regs) {
			auto vehicle = (CVehicle*)regs.esi;
			if (vehicle->m_fHealth > 0.0f) vehicle->m_nVehicleFlags.bEngineOn = true;
			*(DWORD*)(regs.esi + 0x57C) = regs.edi; //original code
		});
	}

	G_ParaLandingFix = false;
	G_NoGarageRadioChange = false;
	G_NoEmergencyMisWanted = false;
	G_NoTutorials = false;
	if (!inSAMP) {

		if (ReadIniBool(ini, &lg, "Gameplay", "ParaLandingFix")) {
			G_ParaLandingFix = true;
		}

		if (ReadIniBool(ini, &lg, "Gameplay", "NoGarageRadioChange")) {
			G_NoGarageRadioChange = true;
		}

		if (ReadIniBool(ini, &lg, "Gameplay", "NoEmergencyMisWanted")) {
			G_NoEmergencyMisWanted = true;
		}

		if (ReadIniBool(ini, &lg, "Gameplay", "NoFlyHeightLimit")) {
			WriteMemory<uint8_t>(0x6D261D, 0xEB, true);
		}

		if (ReadIniFloat(ini, &lg, "Gameplay", "JetpackHeightLimit", &f)) {
			WriteMemory<float>(0x8703D8, f, false);
		}

		if (ReadIniFloat(ini, &lg, "Gameplay", "BikePedImpact", &f)) {
			G_BikePedImpact = f;
			Default_BikePedImpact = ReadMemory<float>(0x8D22AC, false);
			injector::MakeInline<0x5F1200, 0x5F1200 + 6>([](injector::reg_pack& regs)
			{
				if (G_BikePedImpact != -1.0)
					asm_fmul(G_BikePedImpact);
				else
					asm_fmul(Default_BikePedImpact);
			});
		}

		if (ReadIniFloat(ini, &lg, "Gameplay", "CarPedImpact", &f)) {
			G_CarPedImpact = f;
			Default_CarPedImpact = ReadMemory<float>(0x8D22A8, false);
			injector::MakeInline<0x5F1208, 0x5F1208 + 6>([](injector::reg_pack& regs)
			{
				if (G_CarPedImpact != -1.0)
					asm_fmul(G_CarPedImpact);
				else
					asm_fmul(Default_CarPedImpact);
			});
		}

		if (ReadIniFloat(ini, &lg, "Gameplay", "VehPedImpactUpForce", &f)) {
			WriteMemory<float>(0x8D22A4, f, false);
		}

		ReadIniFloat(ini, &lg, "Gameplay", "VehExploDamage", &G_VehExploDamage);
		ReadIniFloat(ini, &lg, "Gameplay", "VehBulletDamage", &G_VehBulletDamage);

		if (G_VehBulletDamage != 1.0 || G_VehExploDamage != 1.0 || G_VehBulletDamage != -1.0 || G_VehExploDamage != -1.0) {
			injector::MakeInline<0x6D7FDA, 0x6D7FDA + 6>([](injector::reg_pack& regs)
			{
				regs.eax = *(uint32_t*)(regs.esi + 0x594); // mov     eax, [esi+594h]

				CVehicle* vehicle = (CVehicle*)regs.esi;
				if (!vehicle->m_nVehicleFlags.bIsRCVehicle && !(vehicle->m_pDriver && vehicle->m_pDriver->m_nCreatedBy == 2 && vehicle->m_nCreatedBy == eVehicleCreatedBy::MISSION_VEHICLE)) {
					if (regs.ebx == 51 || regs.ebx == 37) { // explosion or fire
						if (G_VehExploDamage != -1.0) { // check it because the var may be updated by ini reloading
							if (regs.ebx == 51) {
								*(float*)(regs.esp + 0x90) *= G_VehExploDamage;
							}
						}
					}
					else {
						if (G_VehBulletDamage != -1.0) {
							float thisDamage = *(float*)(regs.esp + 0x90);

							// For petrol cap (not 100% but this way we don't need to do another hook).
							if (thisDamage != vehicle->m_fHealth && thisDamage != 1000.0f) {
								*(float*)(regs.esp + 0x90) *= G_VehBulletDamage;
							}
						}
					}
				}
			});
		}

		if (ReadIniFloat(ini, &lg, "Gameplay", "BrakePower", &f)) {
			G_BrakePower = f;
			if (ReadIniFloat(ini, &lg, "Gameplay", "BrakeMin", &f)) {
				G_BrakeMin = f / 100.0f;
			}
			else {
				G_BrakeMin = -1.0f;
			}
			injector::MakeInline<0x6B269F, 0x6B269F + 6>([](injector::reg_pack& regs)
			{
				//fmul[eax + tHandlingData.fBrakeDeceleration]
				float brakeDeceleration = *(float*)(regs.eax + 0x94);
				brakeDeceleration *= G_BrakePower;
				if (G_BrakeMin > 0.0 && brakeDeceleration < G_BrakeMin) brakeDeceleration = G_BrakeMin;
				G_f = brakeDeceleration;
				asm_fmul(brakeDeceleration);
			});
		}

		if (ReadIniFloat(ini, &lg, "Gameplay", "VehFireDamage", &f)) {
			G_VehFireDamage = f;
			WriteMemory<float*>(0x53A6B7 + 2, &G_VehFireDamage, true);
		}

		if (ReadIniBool(ini, &lg, "Gameplay", "NoTutorials")) {
			WriteMemory<uint8_t>(0xC0BC15, 1, true);
			G_NoTutorials = true;
		}
		else G_NoTutorials = false;
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoSteerSpeedLimit")) {
		WriteMemory<uint8_t>(0x6B2A15, 0xEB, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoFixHandsToBars")) {
		MakeNOP(0x601B90, 6, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "EnableCensorship")) {
		G_EnableCensorship = true;
	}
	else G_EnableCensorship = false;

	if (ReadIniBool(ini, &lg, "Gameplay", "BrakeReverseFix")) {
		injector::MakeCALL(0x006ADB80, BrakeReverseFix_ASM);
		injector::MakeNOP(0x006ADB85);
		injector::WriteMemory<uint32_t>(0x006ADB87, 0x8D, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "NoCheats")) {
		MakeNOP(0x4384D0, 3, true);
		WriteMemory<uint8_t>(0x4384D3, 0xE9, true);
		WriteMemory<uint32_t>(0x4384D4, 0x000000CD, true);
	}

	if (ReadIniInt(ini, &lg, "Gameplay", "DeadPedFreezeDelay", &i)) {
		WriteMemory<uint32_t>(0x630D28, i, true);
		WriteMemory<uint32_t>(0x630D75, i, true);
	}

	if (!inSAMP && ReadIniFloat(ini, &lg, "Gameplay", "VehElevatorSpeed", &f)) {
		WriteMemory<float>(0x871008, f, false);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoWheelTurnBack")) {
		MakeNOP(0x6B5579, 6, true);
		MakeNOP(0x6B568A, 6, true);
	}

	if (!inSAMP && ReadIniFloat(ini, &lg, "Gameplay", "WheelTurnSpeed", &f)) {
		WriteMemory<float>(0x871058, f, false);
	}

	if (!inSAMP && ReadIniFloat(ini, &lg, "Gameplay", "HeliSensibility", &f)) {
		G_HeliSensibility = f;
		WriteMemory<float*>(0x6C4867 + 2, &G_HeliSensibility, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "VehFlipDontBurn")) {
		ReadIniFloat(ini, &lg, "Gameplay", "VehFlipDamage", &f);
		if (f > 0.0f) {
			G_VehFlipDamage = f;
			MakeCALL(0x006A776B, VehFlipDamage_ASM);
			MakeCALL(0x00570E7F, VehFlipDamage_ASM);
			MakeNOP(0x006A7770);
			MakeNOP(0x00570E84);
		}
		else {
			// Patch ped vehicles damage when flipped
			WriteMemory<uint16_t>(0x6A776B, 0xD8DD, true); // fstp st0
			MakeNOP(0x6A776D, 4, true);

			// Patch player vehicle damage when flipped
			WriteMemory<uint16_t>(0x570E7F, 0xD8DD, true); // fstp st0
			MakeNOP(0x570E81, 4, true);
		}
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "EnableTrainCams")) {
		WriteMemory<uint8_t>(0x52A52F, 0xAB, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoPauseWhenUnfocus")) {
		// from MTA
		// Disable MENU AFTER alt + tab
		//0053BC72   C605 7B67BA00 01 MOV BYTE PTR DS:[BA677B],1    
		injector::WriteMemory<uint8_t>(0x53BC78, 0x00, true);
		// ALLOW ALT+TABBING WITHOUT PAUSING
		injector::MakeNOP(0x748A8D, 6, true);
		injector::MakeJMP(0x6194A0, NoPauseWhenMinimize_AllowMouseMovement_ASM, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoPaintjobToWhite")) {
		MakeNOP(0x6D65C5, 11, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoIdleCam")) {
		WriteMemory<uint8_t>(0x522C80, 0xC3, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "NoPedsTalkingToYou")) {
		WriteMemory<uint8_t>(0x43B0F0, 0xC3, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoInteriorMusic")) {
		MakeNOP(0x508450, 6, true);
		MakeNOP(0x508817, 6, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "KeepLightEngineOff")) {
		MakeNOP(0x6E1DBC, 8, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoWayForSiren")) {
		MakeNOP(0x6B2BED, 5, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "NoUniqueJumps")) {
		MakeNOP(0x53C0C1, 5, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "NoUniqueJumpsCam")) {
		MakeNOP(0x49C524, 5, true);
		MakeNOP(0x49C533, 5, true);
		MakeNOP(0x49C87E, 20, true);
		MakeNOP(0x49C89C, 46, true);
	}
	
	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "NoUniqueJumpsSlow")) {
		MakeNOP(0x49C892, 10, true);
		MakeNOP(0x49C529, 10, true);
	}
	
	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "NoTrainDerail")) {
		WriteMemory<uint32_t>(0x006F8C2A, 0x00441F0F, true); // nop dword ptr [eax+eax*1+00h]
		WriteMemory<uint8_t>(0x006F8C2E, 0x00, true);
		WriteMemory<uint16_t>(0x006F8C41, 0xE990, true); // jmp near
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "FixMouseSensibility")) {
		if (ReadMemory<uint32_t>(0x50F048, true) == 0xB6EC18) {
			WriteMemory<uint32_t>(0x50F048, 0xB6EC1C, true);
			WriteMemory<uint32_t>(0x50FB28, 0xB6EC1C, true);
			WriteMemory<uint32_t>(0x510C28, 0xB6EC1C, true);
			WriteMemory<uint32_t>(0x511E0A, 0xB6EC1C, true);
			WriteMemory<uint32_t>(0x52228E, 0xB6EC1C, true);
		}
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "OpenedHouses")) {
		G_OpenedHouses = true;
	}
	else G_OpenedHouses = false;

	if (ReadIniBool(ini, &lg, "Gameplay", "RandWheelDettach")) {
		G_RandWheelDettach = true;
	}
	else G_RandWheelDettach = false;

	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "HostileGangs")) {
		WriteMemory<uint16_t>(0x5FC88F, 0x9066, true);
		WriteMemory<uint8_t>(0x5FC8A8, 0xEB, true);
		WriteMemory<uint16_t>(0x5FC8C6, 0x9066, true);
		WriteMemory<uint16_t>(0x5FC8DD, 0xF66, true);
		WriteMemory<uint32_t>(0x5FC8DF, 0x441F, true);
		WriteMemory<uint32_t>(0x5FC992, 0x1F0F01B0, true);
		WriteMemory<uint8_t>(0x5FC996, 0, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "NoStuntReward")) {
		G_NoStuntReward = true;
	}
	else G_NoStuntReward = false;

	if (ReadIniBool(ini, &lg, "Gameplay", "FixTwoPlayerVehSound")) {
		injector::MakeInline<0x4F570A, 0x4F570A + 6>([](injector::reg_pack& regs)
		{
			*(uint32_t*)(regs.esi + 0xA7) = 0; //mov     [esi+0A7h], bl
			CVehicle *veh = (CVehicle *)regs.eax;
			if (veh->m_pDriver == CWorld::Players[1].m_pPed) {
				secPlayerVehicle = veh;
			}
			else {
				secPlayerVehicle = nullptr;
			}
		});

		injector::MakeInline<0x5022A1, 0x5022A1 + 6>([](injector::reg_pack& regs)
		{
			CAEVehicleAudioEntity *audioEntity = (CAEVehicleAudioEntity *)regs.ecx;
			CVehicle *veh = (CVehicle *)regs.edi;

			audioEntity->m_bInhibitAccForLowSpeed = false;

			if (CWorld::Players[1].m_pPed && veh->m_pDriver == CWorld::Players[1].m_pPed) {
				CAEVehicleAudioEntity::s_pPlayerDriver = CWorld::Players[1].m_pPed;
			}
			else {
				if (CWorld::Players[0].m_pPed && veh->m_pDriver == CWorld::Players[0].m_pPed) {
					CAEVehicleAudioEntity::s_pPlayerDriver = CWorld::Players[0].m_pPed;
				}
			}

			regs.ecx = (uint32_t)CAEVehicleAudioEntity::s_pPlayerDriver;
		});

	}
	 
	

	if (ReadIniBool(ini, &lg, "Gameplay", "NoDoorCamera")) {
		WriteMemory<uint16_t>(0x440390, 0x46EB, true);
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Gameplay", "MaxStarToCreateEmVeh", &i)) {
		WriteMemory<uint8_t>(0x42F9F8 + 3, i, true);
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Gameplay", "PassTimeWasted", &i)) {
		WriteMemory<uint32_t>(0x442FCA + 1, i, true);
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Gameplay", "PassTimeBusted", &i)) {
		WriteMemory<uint32_t>(0x443375 + 1, i, true);
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Gameplay", "PassTimeSaved", &i)) {
		WriteMemory<uint32_t>(0x618F9B + 1, i, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoCinematicCam")) {
		WriteMemory<uint8_t>(0x005281EF + 2, 4, true);
		WriteMemory<uint32_t>(0x00528210, 4, true);
		WriteMemory<uint8_t>(0x52A52F, 0xAB, true); //EnableTrainCams
	}

	if (!inSAMP && ReadIniFloat(ini, &lg, "Gameplay", "HeliRotorSpeed", &f)) {
		G_HeliRotorSpeed = f;
		WriteMemory<float*>(0x6C4EFE + 2, &G_HeliRotorSpeed, true);
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Gameplay", "SniperBulletLife", &i)) {
		WriteMemory<uint32_t>(0x7360A2, i, true); //1000
		WriteMemory<uint32_t>(0x73AFB3, (i * 0.75f), true); //750
		WriteMemory<uint32_t>(0x726CE2, (i * 0.3f), true); //300
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Gameplay", "RocketLife", &i)) {
		WriteMemory<uint32_t>(0x738091+2, i, true);
	}

	if (!inSAMP && ReadIniInt(ini, &lg, "Gameplay", "HSRocketLife", &i)) {
		WriteMemory<uint32_t>(0x7380AA+1, i, true);
	}

	if (!inSAMP && ReadIniFloat(ini, &lg, "Gameplay", "HSRocketSpeed", &f)) {
		WriteMemory<float>(0x7380B3 + 4, f, true);
	}

	if (!inSAMP && ReadIniFloat(ini, &lg, "Gameplay", "RocketSpeed", &f)) {
		WriteMemory<float>(0x73809B + 4, f, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoPedVehDive")) {
		WriteMemory<uint32_t>(0x4C1197, 0xFFFF10E9, true);
		WriteMemory<uint8_t>(0x4C119B, 0xFF, true);

		WriteMemory<uint32_t>(0x4C1235, 0xFFFE72E9, true);
		WriteMemory<uint8_t>(0x4C1239, 0xFF, true);
	}

	if (ReadIniBool(ini, &lg, "Gameplay", "NoPedVehHandsUp")) {
		WriteMemory<uint32_t>(0x4C1175, 0, true);
	}
	



	// -- Densities
	if (!G_NoDensities) {
		if (ReadIniFloat(ini, &lg, "Densities", "VehLodDist", &f)) {
			G_VehLodDist = f;
			WriteMemory<float*>(0x732924 + 2, &G_VehLodDist, true);
		}

		if ((!inSAMP || (inSAMP && dtSAMP)) && ReadIniFloat(ini, &lg, "Densities", "VehDrawDist", &f)) {
			G_VehDrawDist = f;
			WriteMemory<float*>(0x73293E + 2, &G_VehDrawDist, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "CullDistNormalComps", &f)) {
			G_CullDistNormalComps = f;
			WriteMemory<float*>(0x7328CA + 2, &G_CullDistNormalComps, true);
		}

		if ((!inSAMP || (inSAMP && dtSAMP)) && ReadIniFloat(ini, &lg, "Densities", "VehOccupDrawDist", &f)) {
			G_VehOccupDrawDist = f;
			WriteMemory<float*>(0x5E77C6 + 2, &G_VehOccupDrawDist, true);
			G_VehOccupDrawDist_Boat = f * 1.6f;
			WriteMemory<float*>(0x5E77BE + 2, &G_VehOccupDrawDist_Boat, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "CullDistBigComps", &f)) {
			G_CullDistBigComps = f;
			WriteMemory<float*>(0x7328F2 + 2, &G_CullDistBigComps, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "VehMultiPassDist", &f)) {
			G_VehMultiPassDist = f;
			WriteMemory<float*>(0x73290A + 2, &G_VehMultiPassDist, true);
		}

		if ((!inSAMP || (inSAMP && dtSAMP)) && ReadIniFloat(ini, &lg, "Densities", "PedDrawDist", &f)) {
			G_PedDrawDist = f;
			WriteMemory<float*>(0x73295C + 2, &G_PedDrawDist, true);
		}

		if (!inSAMP) {

			if (ReadIniFloat(ini, &lg, "Densities", "RoadblockSpawnDist", &f)) {
				G_RoadblockSpawnDist = f;
				G_RoadblockSpawnDist_NEG = (f *= -1.0f);
				WriteMemory<float*>(0x462B1D + 2, &G_RoadblockSpawnDist_NEG, true);
				WriteMemory<float*>(0x462B76 + 2, &G_RoadblockSpawnDist_NEG, true);
				WriteMemory<float*>(0x462B32 + 2, &G_RoadblockSpawnDist, true);
				WriteMemory<float*>(0x462B8B + 2, &G_RoadblockSpawnDist, true);
				WriteMemory<float*>(0x462BB0 + 2, &G_RoadblockSpawnDist, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "GangWaveMaxSpawnDist", &f)) {
				WriteMemory<float>(0x444AFA + 1, f, true);
				WriteMemory<float>(0x444AFF + 1, f, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "GangWaveMinSpawnDist", &f)) {
				G_GangWaveMinSpawnDist = f;
				WriteMemory<float*>(0x444BB2 + 2, &G_GangWaveMinSpawnDist, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "PedDensityExt", &f))
			{
				G_PedDensityExt = f;
			}
			else G_PedDensityExt = -1.0f;

			if (ReadIniFloat(ini, &lg, "Densities", "PedDensityInt", &f)) {
				if (f > 1.0f) f = 1.0f;
				G_PedDensityInt = f;
			}
			else G_PedDensityInt = -1.0f;

			if (G_PedDensityExt >= 0.0f || G_PedDensityInt >= 0.0f) {
				injector::MakeInline<0x614937, 0x614937 + 6>([](injector::reg_pack& regs)
				{
					if (CGame::currArea != 0) {
						float dens;
						if (G_PedDensityInt >= 0.0f)
						{
							dens = *(float*)0x008D2530 * G_PedDensityInt;
						}
						else {
							dens = *(float*)0x008D2530;
						}
						if (dens > 1.0) dens = 1.0;
						asm_fmul(dens);
					}
					else {
						if (G_PedDensityExt >= 0.0f)
						{
							asm_fmul(*(float*)0x008D2530 * G_PedDensityExt);
						}
						else asm_fmul(*(float*)0x008D2530);
					}
				});
			}


			if (ReadIniFloat(ini, &lg, "Densities", "VehDensity", &f))
			{
				G_VehDensity = f;
				injector::MakeInline<0x4300E9>([](injector::reg_pack& regs)
				{
					if (G_VehDensity != -1.0f)
					{
						*(float*)&regs.eax = *(float*)0x008A5B20 * G_VehDensity;
					}
					else *(float*)&regs.eax = *(float*)0x008A5B20;
				});
			}
			else G_VehDensity = -1.0f;

			if (ReadIniFloat(ini, &lg, "Densities", "VehDespawnOnScr", &f)) {
				G_VehDespawnOnScr = f;
				WriteMemory<float*>(0x4250F0, &G_VehDespawnOnScr, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "VehDespawnOffScr", &f)) {
				WriteMemory<float>(0x42510F, f, true);
				f *= 0.6f;
				WriteMemory<float>(0x4250E2, f, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "PedDespawnOnScr", &f)) {
				G_PedDespawnOnScr = f;
				WriteMemory<float*>(0x6120FF, &G_PedDespawnOnScr, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "PedDespawnOffScr", &f)) {
				G_PedDespawnOffScr = f;
				WriteMemory<float*>(0x612128, &G_PedDespawnOffScr, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "PedSpawnOnScr", &f)) {
				WriteMemory<float>(0x86D284, f, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "PedSpawnOffScr", &f)) {
				WriteMemory<float>(0x86C850, f, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "TowelSpawnOnScr", &f)) {
				WriteMemory<float>(0x86D288, f, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "TowelSpawnOffScr", &f)) {
				G_TowelSpawnOffScr = f;
				WriteMemory<float*>(0x615E81 + 2, &G_TowelSpawnOffScr, true);
			}

			if (ReadIniFloat(ini, &lg, "Densities", "TrainSpawnDistance", &f)) {
				G_TrainSpawnDistance = f;
				WriteMemory<float*>(0x6F7AA8, &G_TrainSpawnDistance, true);
			}
		}

		if ((!inSAMP || (inSAMP && dtSAMP)) && ReadIniFloat(ini, &lg, "Densities", "MinGrassDist", &f)) {
			WriteMemory<float>(0x5DDB42 + 1, f, true);
		}

		if ((!inSAMP || (inSAMP && dtSAMP)) && ReadIniFloat(ini, &lg, "Densities", "MaxGrassDist", &f)) {
			WriteMemory<float>(0x5DDB3D + 1, f, true);
		}

		if ((!inSAMP || (inSAMP && dtSAMP)) && ReadIniFloat(ini, &lg, "Densities", "PedWeaponDrawDist", &f)) {
			G_PedWeaponDrawDist = f;
			if (G_PedWeaponDrawDist != 1.0f) {
				MakeJMP(0x7336A8, PedWeaponDrawDist_ASM, true);
			}
		}

		if (!inSAMP && ReadIniInt(ini, &lg, "Densities", "DeadPedDeleteDelay", &i)) {
			WriteMemory<uint32_t>(0x612026, i, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "ShadDist_Vehicles", &f)) {
			G_ShadDist_Vehicles = f;
			WriteMemory<float*>(0x70BEB6, &G_ShadDist_Vehicles, true);
			G_ShadDist_Vehicles_Sqr = G_ShadDist_Vehicles * G_ShadDist_Vehicles;
			WriteMemory<float*>(0x70BEA7, &G_ShadDist_Vehicles_Sqr, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "ShadDist_SmallPlanes", &f)) {
			G_ShadDist_SmallPlanes = f;
			WriteMemory<float*>(0x70BE79, &G_ShadDist_SmallPlanes, true);
			G_ShadDist_SmallPlanes_Sqr = G_ShadDist_SmallPlanes * G_ShadDist_SmallPlanes;
			WriteMemory<float*>(0x70BE88, &G_ShadDist_SmallPlanes_Sqr, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "ShadDist_BigPlanes", &f)) {
			G_ShadDist_BigPlanes = f;
			WriteMemory<float*>(0x0070BE9F, &G_ShadDist_BigPlanes, true);
			G_ShadDist_BigPlanes_Sqr = G_ShadDist_BigPlanes * G_ShadDist_BigPlanes;
			WriteMemory<float*>(0x0070BE90, &G_ShadDist_BigPlanes_Sqr, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "ShadDist_Peds", &f)) {
			WriteMemory<float>(0x8D5240, f, true);
			WriteMemory<float>(0xC4B6B0, (f * f), true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "LighDist_VehLight", &f)) {
			G_ShadDist_CarLight = f;
			G_ShadDist_CarLight_Sqr = (G_ShadDist_CarLight * G_ShadDist_CarLight);
			WriteMemory<float*>(0x0070C582, &G_ShadDist_CarLight_Sqr, true);

			G_ShadDist_CarLight_Mid = (G_ShadDist_CarLight * 0.75f);
			WriteMemory<float*>(0x0070C5E4, &G_ShadDist_CarLight_Mid, true);

			G_ShadDist_CarLight -= G_ShadDist_CarLight_Mid;
			G_ShadDist_CarLight /= 3.0f;
			G_ShadDist_CarLight_Min = G_ShadDist_CarLight_Mid - G_ShadDist_CarLight;
			WriteMemory<float*>(0x0070C5FE, &G_ShadDist_CarLight_Min, true);

			G_ShadDist_CarLight_Scale = 1.0f / (G_ShadDist_CarLight * 4.0f);
			WriteMemory<float*>(0x0070C608, &G_ShadDist_CarLight_Scale, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "ShadHeiLim_HeadLight", &f)) {
			WriteMemory<float>(0x0070C6B7, f, true);
			WriteMemory<float>(0x0070C719, f, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "ShadHeiLim_Vehicles", &f)) {
			WriteMemory<float>(0x0070BDD6, f, true);
			WriteMemory<float>(0x0070C2F8, f, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "ShadHeiLim_Aircraft", &f)) {
			WriteMemory<float>(0x0070C0ED, f, true);
			WriteMemory<float>(0x0070C113, f, true);
			WriteMemory<uint16_t>(0x0070C0D9, 0x0EEB, true);
			WriteMemory<uint16_t>(0x0070C0FF, 0x0EEB, true);
		}

		if (!inSAMP && ReadIniInt(ini, &lg, "Densities", "VehLimit", &i)) {
			WriteMemory<uint8_t>(0x434237, 0x73, true); // change condition to unsigned (0-255)
			WriteMemory<uint8_t>(0x434224, i, true);
			WriteMemory<uint8_t>(0x484D19, 0x83, true); // change condition to unsigned (0-255)
			WriteMemory<uint8_t>(0x484D17, i, true);
		}
		
		if (!inSAMP && ReadIniInt(ini, &lg, "Densities", "PedLimit", &i)) {
			WriteMemory<uint32_t>(0x8D2538, i, false);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "FootstepsDist", &f)) {
			G_FootstepsDist = f;
			WriteMemory<float*>(0x5E5550, &G_FootstepsDist, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "FootstepsTime", &i)) {
			WriteMemory<uint16_t>(0x5E5597, i, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "FootstepsTimeBlood", &i)) {
			WriteMemory<uint32_t>(0x5E546C, i, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "TimeExplosionTex", &i)) {
			WriteMemory<int>(0x73743E, i, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "TimeBloodstainNPC", &i)) {
			WriteMemory<int>(0x49ED69, i, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "TimeBloodstainPlayer", &i)) {
			WriteMemory<int>(0x49EDC7, i, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "TimeBloodpoolTex", &i)) {
			WriteMemory<int>(0x630E79, i, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "TimeBleedingTex", &i)) {
			WriteMemory<int>(0x5E943C, i, true);
		}

		if ((!inSAMP || (inSAMP && dtSAMP)) && ReadIniFloat(ini, &lg, "Densities", "PickupsDrawDist", &f)) {
			G_PickupsDrawDist = f;
			WriteMemory<float*>(0x70C0ED, &G_PickupsDrawDist, true);
		}

		if (!inSAMP && ReadIniInt(ini, &lg, "Densities", "TimePickupShort", &i)) {
			WriteMemory<int>(0x457236, i, true);
		}

		if (!inSAMP && ReadIniInt(ini, &lg, "Densities", "TimePickupLong", &i)) {
			WriteMemory<int>(0x457243, i, true);
		}

		if (!inSAMP && ReadIniInt(ini, &lg, "Densities", "TimePickupMoney", &i)) {
			WriteMemory<int>(0x457250, i, true);
		}

		if (ReadIniBool(ini, &lg, "Densities", "NoLODduringFly")) {
			MakeNOP(0x5557CF, 7, true);
		}
		
		if (ReadIniFloat(ini, &lg, "Densities", "ShadDist_AllPerm", &f)) {
			WriteMemory<float>(0x70C995 + 1, f, true); //CShadows::UpdatePermanentShadows
			WriteMemory<float>(0x70C9F3 + 1, f, true); //CShadows::UpdatePermanentShadows
			WriteMemory<float>(0x630E26 + 1, f, true); //CTaskSimpleDead::ProcessPed
		}

		if (ReadIniFloat(ini, &lg, "Densities", "ShadDist_Poles", &f)) {
			WriteMemory<float>(0x70C88A + 1, f, true);
		}
		
		if (ReadIniFloat(ini, &lg, "Densities", "LighDist_TrafficL", &f)) {
			WriteMemory<float>(0x49DF79 + 1, f, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "LighDist_Fire", &f)) {
			WriteMemory<float>(0x53B5E1 + 1, f, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "LighDist_Entities", &f)) {
			WriteMemory<float>(0x6FD3A5 + 1, f, true);
			WriteMemory<float>(0x6FD44E + 1, f, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "MinDesiredLoadedVeh", &i)) {
			WriteMemory<uint8_t>(0x40B6AA + 2, i, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "DesiredLoadedVeh", &i)) {
			WriteMemory<uint32_t>(0x8A5A84, i, false);
		}

		if (ReadIniInt(ini, &lg, "Densities", "DelayLoadDesiredVeh", &i)) {
			WriteMemory<uint8_t>(0x40B9B6 + 6, i, true);
		}

		if (ReadIniInt(ini, &lg, "Densities", "MinLoadedGangVeh", &i)) {
			WriteMemory<uint8_t>(0x40ACA5 + 2, i, true);
		}
		
		/*
		if (ReadIniFloat(ini, &lg, "Densities", "PropCollDist", &f)) {
			G_PropCollDist_NEG = f * -1.0f;
			G_PropCollDist_POS = f;

			WriteMemory<float*>(0x41047A + 2, &G_PropCollDist_NEG, true);
			WriteMemory<float*>(0x41048C + 2, &G_PropCollDist_POS, true);
			WriteMemory<float*>(0x41049E + 2, &G_PropCollDist_NEG, true);
			WriteMemory<float*>(0x4104B1 + 2, &G_PropCollDist_POS, true);
			WriteMemory<float*>(0x4084F5 + 2, &G_PropCollDist_POS, true);
			WriteMemory<float*>(0x4051BD + 2, &G_PropCollDist_POS, true);
			WriteMemory<float*>(0x4051EE + 2, &G_PropCollDist_POS, true);
			WriteMemory<float*>(0x5A2EE3 + 2, &G_PropCollDist_POS, true);
			WriteMemory<float*>(0x5A2EF0 + 4, &G_PropCollDist_POS, true);

			//WriteMemory<float>(0x858A14, G_PropCollDist_POS, true);
			//WriteMemory<float>(0x858BA0, G_PropCollDist_NEG, true);
		}

		if (ReadIniFloat(ini, &lg, "Densities", "PlantDist", &f)) {
			G_PlantDist = f;
			WriteMemory<float*>(0x5DC5F7 + 2, &G_PlantDist, true);
			WriteMemory<float*>(0x5DC64D + 2, &G_PlantDist, true);
			WriteMemory<float*>(0x5DC695 + 2, &G_PlantDist, true);
			WriteMemory<float*>(0x5DC6E6 + 2, &G_PlantDist, true);
		}
		WriteMemory<float>(0x86BF20, 1000000.0f, true);
		WriteMemory<float>(0x859AA4, 20000.0f, true);
		WriteMemory<float>(0x85F074, 500.0f, true);
		*/

	}


	// -- Skid marks
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkidRate", &i)) {
		WriteMemory<uint8_t>(0x720B22, i, true);
	}

	if (ReadIniFloat(ini, &lg, "Skid Marks", "SkidHeight", &f)) {
		G_SkidHeight = f;
		WriteMemory<float*>(0x720819, &G_SkidHeight, true);
	}

	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdVeryShort_FadeStr", &i)) {
		WriteMemory<uint16_t>(0x7205F6, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdVeryShort_FadeEnd", &i)) {
		WriteMemory<uint16_t>(0x7205FF, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdShort_FadeStr", &i)) {
		WriteMemory<uint16_t>(0x72060E, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdShort_FadeEnd", &i)) {
		WriteMemory<uint16_t>(0x720617, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdMedium_FadeStr", &i)) {
		WriteMemory<uint16_t>(0x72061F, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdMedium_FadeEnd", &i)) {
		WriteMemory<uint16_t>(0x720628, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdLong_FadeStr", &i)) {
		WriteMemory<uint16_t>(0x720CA6, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdLong_FadeEnd", &i)) {
		WriteMemory<uint16_t>(0x720CAB, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdThread_FadeStr", &i)) {
		WriteMemory<uint16_t>(0x720AF4, i, true);
	}
	if (ReadIniInt(ini, &lg, "Skid Marks", "SkdThread_FadeEnd", &i)) {
		WriteMemory<uint16_t>(0x720AF9, i, true);
	}



	// -- Interface
	if (!inSAMP && ReadIniBool(ini, &lg, "Interface", "NoStatsUpdateBox")) {
		WriteMemory<uint8_t>(0x55B980, 0xC3, true);
		WriteMemory<uint8_t>(0x559760, 0xC3, true);
	}
	if (ReadIniBool(ini, &lg, "Interface", "NoMoneyZeros")) {
		G_NoMoneyZeros_Pos = "$%d";
		WriteMemory<string*>(0x58F4C8, &G_NoMoneyZeros_Pos, true); //positive
		G_NoMoneyZeros_Neg = "-$%d";
		WriteMemory<string*>(0x58F50A, &G_NoMoneyZeros_Neg, true); //negative
	}
	if ((!inSAMP || (inSAMP && rpSAMP)) && ReadIniBool(ini, &lg, "Interface", "NoTargetBlip")) {
		WriteMemory<uint8_t>(0x53E1EC, 0xEB, true);
	}
	if ((!inSAMP || (inSAMP && rpSAMP)) && ReadIniBool(ini, &lg, "Interface", "NoCrosshair")) {
		MakeNOP(0x58FBBF, 5);
	}



	// -- Audio
	if (ReadIniBool(ini, &lg, "Audio", "NoAmbientGuns")) {
		MakeNOP(0x507818, 11, true);
	}
	if (ReadIniBool(ini, &lg, "Audio", "NoHelpBoxSound")) {
		WriteMemory<uint32_t>(0x58B81F, 0x900CC483, true);
		MakeNOP(0x58B823, 1, true);
	}
	if (ReadIniBool(ini, &lg, "Audio", "SirenOnWithoutDriver")) {
		MakeJMP(0x004F9DA4, SirenOnWithoutDriver_ASM, true);
		MakeNOP(0x004F9DA9, 5, true);
	}

	


	// -- Wanted
	if (!inSAMP) {
		/*if (ReadIniInt(ini, &lg, "Wanted", "RoadBlockVeh_4", &i)) {
			WriteMemory<uint32_t>(0x461BE7, i, true);
		}
		if (ReadIniInt(ini, &lg, "Wanted", "RoadBlockVeh_5", &i)) {
			WriteMemory<uint32_t>(0x461BCC, i, true);
		}
		if (ReadIniInt(ini, &lg, "Wanted", "RoadBlockVeh_6", &i)) {
			WriteMemory<uint32_t>(0x461BB1, i, true);
		}*/
		if (ReadIniInt(ini, &lg, "Wanted", "MaxHydras", &i)) {
			WriteMemory<uint8_t>(0x6CD91C, i, true);
		}
		if (ReadIniInt(ini, &lg, "Wanted", "DelayHydras", &i)) {
			WriteMemory<uint32_t>(0x6CD8E0, i, true);
		}
		if (ReadIniInt(ini, &lg, "Wanted", "MilitaryZoneStar", &i)) {
			WriteMemory<uint8_t>(0x72DF2A, i, true);
		}
		if (ReadIniInt(ini, &lg, "Wanted", "BannedSFZoneLevel", &i)) {
			WriteMemory<uint8_t>(0x4418B0+1, i, true);
		}
		if (ReadIniInt(ini, &lg, "Wanted", "BannedLVZoneLevel", &i)) {
			WriteMemory<uint8_t>(0x44183C+1, i, true);
		}
		if (ReadIniFloat(ini, &lg, "Wanted", "RandomHeliFireTime", &f)) {
			WriteMemory<float>(0x8D33A4, f, false);
		}
		if (ReadIniBool(ini, &lg, "Wanted", "NoCopHeliShots")) {
			MakeNOP(0x006C7773, 5, true);
			MakeNOP(0x006C777B, 18, true);
		}



		// -- Hydra
		if (ReadIniInt(ini, &lg, "Hydra", "HydraRocketDelay", &i)) {
			WriteMemory<uint32_t>(0x6D462E, i, true);
			WriteMemory<uint32_t>(0x6D4634, i, true);
		}
		if (ReadIniInt(ini, &lg, "Hydra", "HydraFlareDelay", &i)) {
			WriteMemory<uint32_t>(0x6E351B, i, true);
		}
		if (ReadIniInt(ini, &lg, "Hydra", "HydraLockDelay", &i)) {
			WriteMemory<uint32_t>(0x6E363A, i, true);
			WriteMemory<uint32_t>(0x6E36FB, i, true);
		}
		if (ReadIniBool(ini, &lg, "Hydra", "NoHydraSpeedLimit")) {
			WriteMemory<uint8_t>(0x6DADE8, 0xEB, true);
		}

		// -- Rhino
		if (ReadIniInt(ini, &lg, "Rhino", "RhinoFireDelay", &i)) {
			WriteMemory<uint32_t>(0x6AED10, i, true);
		}
		if (ReadIniFloat(ini, &lg, "Rhino", "RhinoFirePush", &f)) {
			f *= -1.0;
			WriteMemory<float>(0x871080, f, true);
		}
		if (ReadIniFloat(ini, &lg, "Rhino", "RhinoFireRange", &f)) {
			G_RhinoFireRange = f;
			WriteMemory<float*>(0x6AEF42, &G_RhinoFireRange, true);
			WriteMemory<float*>(0x6AEF56, &G_RhinoFireRange, true);
			WriteMemory<float*>(0x6AEF65, &G_RhinoFireRange, true);
		}
		if (ReadIniInt(ini, &lg, "Rhino", "RhinoFireType", &i)) {
			WriteMemory<uint8_t>(0x6AF0AC, i, true);
		}


		// -- World
		if (ReadIniBool(ini, &lg, "World", "LockHour")) {
			MakeNOP(0x53BFBD, 5, true);
		}

		if (ReadIniFloat(ini, &lg, "World", "Gravity", &f)) {
			WriteMemory<float>(0x863984, f, false);
		}

		if (ReadIniInt(ini, &lg, "World", "HowManyMinsInDay", &i)) {
			i *= 41.666666667f;
			WriteMemory<uint32_t>(0xB7015C, i, false);
			WriteMemory<uint32_t>(0x5BA35F, i, true);
			WriteMemory<uint32_t>(0x5BA35F, i, true);
			WriteMemory<uint32_t>(0x53BDEC, i, true);
		}

		if (ReadIniInt(ini, &lg, "World", "FreezeWeather", &i)) {
			G_FreezeWeather = i;
		}
		else {
			if (G_FreezeWeather >= 0) {
				CWeather::ReleaseWeather();
			}
			G_FreezeWeather = -1;
		}

		if (ReadIniBool(ini, &lg, "World", "NoWaterPhysics")) {
			WriteMemory<uint8_t>(0x6C2759, 1, true);
		}
	}

	// -- Post

	if (bReadOldINI) {

		if (numOldCfgNotFound > 0)
		{
			if (lang == languages::PT)
				lg << "\nAviso: " << numOldCfgNotFound << " configura��es n�o foram encontradas no .ini antigo. Verifique acima.\n";
			else
				lg << "\nWarning: " << numOldCfgNotFound << " configurations has not found on old ini. Check it above.\n";
		}

		bErrorRename = false;

		try {
			filesystem::rename(PLUGIN_PATH("MixSets old.ini"), PLUGIN_PATH("MixSets backup.ini"));
		}
		catch (std::filesystem::filesystem_error& e) {
			if (lang == languages::PT)
			{
				lg << "\nERRO: N�o foi poss�vel renomear o arquivo 'MixSets old.ini'. Provavelmente voc� est� com o jogo instalado na pasta Arquivos de Programas ou o arquivo est� em uso.\n";
				lg << "Mova seu jogo para outra pasta para o melhor funcionamento deste e outros mods. Ou verifique o arquivo, tente de novo, renomei-o ou delete-o manualmente.\n";
			}
			else {
				lg << "\nERROR: Unable to rename 'MixSets old.ini' file. You probably have the game installed in the Program Files folder or the file is in use.\n";
				lg << "Move your game to another folder for the best working of this and other mods. Or check the file, try again, renamed it, or delete it manually.\n";
			}
			lg << "Error message: " << e.what() << "\n";
			bErrorRename = true;
		}


	}

	lg.flush();
}


///////////////////////////////////////////////////////////////////////////////////////////////////


void ReadIni_BeforeFirstFrame() {
	int i;
	float f;

	numOldCfgNotFound = 0;

	CIniReader ini("MixSets.ini");

	
	if (ini.data.size() <= 0) {
		lg << "\nERROR: MixSets.ini not found - MixSets.ini n�o encontrado \n";
		bIniFailed = true;
		return;
	} bIniFailed = false;


	CIniReader iniold("MixSets old.ini");
	if (iniold.data.size() <= 0)
	{
		bReadOldINI = false;
	}
	else {
		bReadOldINI = true;
		if (lang == languages::PT)
			lg << "\n'MixSets old.ini' encontrado. As configura��es ser�o movidas para o 'MixSets.ini'.\n\n";
		else
			lg << "\n'MixSets old.ini' found. The settings will be moved to 'MixSets.ini'.\n\n";
	}
	 

	// -- Mod
	if (ReadIniInt(ini, &lg, "Mod", "Language", &i)) {
		switch (i)
		{
		case 1:
			lang = languages::PT;
			break;
		case 2:
			lang = languages::EN;
			break;
		default:
			break;
		}
	}
	else {
		lang = languages::EN;
		lg << "Language not read. Set to english\n";
	}
	

	if (ReadIniBool(ini, &lg, "Mod", "Enabled")) {
		bEnabled = true;
	}
	else {
		bEnabled = false;
		if (lang == languages::PT)
			lg << "Desativado" << "\n\n";
		else
			lg << "Disabled" << "\n\n";
		lg.flush();
		return;
	}

	if (ReadIniBool(ini, &lg, "Mod", "NoDensities")) {
		G_NoDensities = true;
	}


	if (ReadIniBool(ini, &lg, "Mod", "SAMPdisadvantage")) {
		rpSAMP = true;
	}
	else rpSAMP = false;


	if (ReadIniBool(ini, &lg, "Mod", "LoadDistancesOnSAMP")) {
		dtSAMP = true;
	}
	else dtSAMP = false;


	G_ReloadCommand = ini.ReadString("Mod", "ReloadCommand", "");


	// -- System
	if (ReadIniInt(ini, &lg, "System", "ProcessPriority", &i)) {
		G_ProcessPriority = i;
	}
	else G_ProcessPriority = -1;

	if (ReadIniInt(ini, &lg, "System", "FPSlimit", &i)) {
		WriteMemory<uint8_t>(0xC1704C, i, false);
		G_FPSlimit = i;
	}


	// -- Gameplay
	if (!inSAMP && ReadIniBool(ini, &lg, "Gameplay", "NoReplay")) {
		WriteMemory<uint8_t>(0x460500, 0xC3, true);
	}
	 
	if (ReadIniBool(ini, &lg, "Gameplay", "FixMouseStuck")) {
		MakeNOP(0x745423, 8, true);
		MakeJMP(0x745423, FixMouseStuck_ASM, true);
	}
	

	// -- Graphics
	if (ReadIniBool(ini, &lg, "Graphics", "DisplayDialogAnyAR")) {
		MakeNOP(0x7459E1, 2, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "ForceAnisotropic")) {
		WriteMemory<uint8_t>(0x730F9C, 0, true);
	}

	if (ReadIniInt(ini, &lg, "Graphics", "Anisotropic", &i)) {
		G_Anisotropic = i;
		MakeCALL(0x730F9F, CustomMaxAnisotropic, true);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "FxEmissionRateShare")) {
		G_FxEmissionRateShare = true;
	}
	else G_FxEmissionRateShare = false;

	if (ReadIniFloat(ini, &lg, "Graphics", "FxEmissionRateMult", &f)) {
		if (G_FxEmissionRateShare) {
			WriteMemory<float>(0x4A97B0 + 1, (0.5f * f), true);
			WriteMemory<float>(0x4A97C6 + 1, (0.5f * f), true);
			WriteMemory<float>(0x4A97DC + 1, (0.5f * f), true);
		}
		else {
			WriteMemory<float>(0x4A97B0 + 1, (0.5f * f), true);
			WriteMemory<float>(0x4A97C6 + 1, (0.75f * f), true);
			WriteMemory<float>(0x4A97DC + 1, (1.0f * f), true);
		}
	}
	else {
		if (G_FxEmissionRateShare) {
			WriteMemory<float>(0x4A97B0 + 1, 0.5f, true);
			WriteMemory<float>(0x4A97C6 + 1, 0.5f, true);
			WriteMemory<float>(0x4A97DC + 1, 0.5f, true);
		}
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Graphics", "NoCoronas")) {
		MakeNOP(0x53E18E, 5);
	}

	if (ReadIniBool(ini, &lg, "Graphics", "NoStencilShadows")) {
		G_NoStencilShadows = true;
		WriteMemory<uint8_t>(0x7113C0, 0xEB, true);
	}
	else G_NoStencilShadows = false;


	// -- Densities
	if (!G_NoDensities) {
		if (!inSAMP && ReadIniFloat(ini, &lg, "Densities", "PedPopulationMult", &f)) {
			G_PedPopulationMult = f;
			injector::MakeInline<0x5BC1E9, 0x5BC1E9 + 7>([](injector::reg_pack& regs)
			{
				regs.eax = *(uint8_t*)(regs.esp + 0x0BC) * G_PedPopulationMult; //mov al, [esp+0BCh]
			});
		}
		else G_PedPopulationMult = 1.0f;

		if (!inSAMP && ReadIniFloat(ini, &lg, "Densities", "VehPopulationMult", &f)) {
			G_VehPopulationMult = f;
			injector::MakeInline<0x5BC1F0, 0x5BC1F0 + 7>([](injector::reg_pack& regs)
			{
				regs.ecx = *(uint8_t*)(regs.esp + 0x0F8) * G_VehPopulationMult; //mov cl, [esp+0F8h]
			});
		}
		else G_VehPopulationMult = 1.0f;
	}


	// -- Experimental
	if (ReadIniBool(ini, &lg, "Experimental", "NoTextures")) {
		WriteMemory<uint8_t>(0x884900, 0, true);
	}

	if (!inSAMP && ReadIniBool(ini, &lg, "Experimental", "ForceIPLcarSection")) {
		WriteMemory<uint8_t>(0x6F2EE3, 0x0C, true);
		WriteMemory<uint8_t>(0x6F2EFF, 0xCA, true);
	}

	if (ReadIniBool(ini, &lg, "Experimental", "NoSound")) {
		WriteMemory<uint8_t>(0x507750, 0xC3, true);
	}

	if (ReadIniBool(ini, &lg, "Experimental", "NoParticles")) {
		WriteMemory<uint32_t>(0x4AA440, 0x000020C2, true);
	}

	if (ReadIniBool(ini, &lg, "Experimental", "NoPostFx")) {
		WriteMemory<uint32_t>(0xC402CF, 1, true);
	}
	

	lg.flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////////