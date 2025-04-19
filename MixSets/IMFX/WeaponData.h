#pragma once

#include <string>

struct WeaponData {
	std::string particleName;
	bool        rotate = true;
	bool        smoke = true;
	bool        underFlash = true;

	WeaponData(
		const std::string& pName = "gunflash",
		bool               r = true,
		bool               s = true,
		bool               u = true
	);
};
