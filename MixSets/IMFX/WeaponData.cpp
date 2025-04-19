#include "WeaponData.h"

WeaponData::WeaponData(
	const std::string& pName,
	bool               r,
	bool               s,
	bool               u
)
	: particleName(pName)
	, rotate(r)
	, smoke(s)
	, underFlash(u)
{
}
