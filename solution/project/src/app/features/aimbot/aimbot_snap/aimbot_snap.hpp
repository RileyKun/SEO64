#pragma once

#include "../../../../game/game.hpp"

class AimbotSnapInst final
{
private:
	vec3 m_start_ang{};
	vec3 m_target_ang{};
	float m_start_time{};
	float m_end_time{};

public:
	AimbotSnapInst(const vec3 &start_ang, const vec3 &target_ang);

public:
	float progress() const;
	vec3 angles();
	void update(const vec3 &target_ang);
};

class AimbotSnap final
{
private:
	std::unique_ptr<AimbotSnapInst> m_snap{};

public:
	void begin(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const vec3 &target_ang);
	void reset();
	void update(const vec3 &target_ang);
	bool think(CUserCmd *const cmd, C_TFWeaponBase *const weapon);
};

MAKE_UNIQUE(AimbotSnap, aimbot_snap);