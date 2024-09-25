#pragma once

#include "entity_visuals_effects.hpp"

enum struct EVClass
{
	NONE = 0,
	PLAYER = 1 << 0,
	SENTRYGUN = 1 << 1,
	DISPENSER = 1 << 2,
	TELEPORTER = 1 << 3,
	PROJECTILE = 1 << 4,
	HEALTHPACK = 1 << 5,
	AMMOPACK = 1 << 6,
	HALLOWEEN_GIFTS = 1 << 7,
	MVM_MONEY = 1 << 8,
    CTF_FLAGS = 1 << 9
};

enum struct EVCond
{
	NONE = 0,
	LOCAL = 1 << 0,
	TEAMMATE = 1 << 1,
	ENEMY = 1 << 2,
	FRIEND = 1 << 3,
	INVISIBLE = 1 << 4,
	INVULNERABLE = 1 << 5,
	AIMBOT_TARGET = 1 << 6,
	CLASS_SCOUT = 1 << 7,
	CLASS_SOLDIER = 1 << 8,
	CLASS_PYRO = 1 << 9,
	CLASS_DEMOMAN = 1 << 10,
	CLASS_HEAVY = 1 << 11,
	CLASS_ENGINEER = 1 << 12,
	CLASS_MEDIC = 1 << 13,
	CLASS_SNIPER = 1 << 14,
	CLASS_SPY = 1 << 15
};

class EVGroup final
{
public:
	std::string name{};

public:
	int classes{};
	int conds{};
	int effects{};

	float esp_alpha{ 1.0f };

public:
	EVMaterialType mat_type{ EVMaterialType::ORIGINAL };
	float mat_alpha{ 1.0f };
	bool mat_depth{ true };

public:
	EVWeaponType weapon_type{ EVWeaponType::ACTIVE_NAME };

public:
	float outline_alpha{ 1.0f };

public:
	bool entPass(C_BaseEntity *const ent) const;
};