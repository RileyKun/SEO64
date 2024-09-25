#pragma once

#include "../player_data/player_data.hpp"

class AutoBackstab final
{
private:
	bool canSex(const vec3 &player_center, const vec3 &target_center, const vec3 &player_eye_angs, const vec3 &target_eye_angs, const int health, const int dmg);
	bool canSee(C_TFPlayer *const local, C_TFWeaponBase *const weapon, C_TFPlayer *const target, const LagRecord *const lr, const vec3 ang_to_target);

public:
	void setAngles(CUserCmd *const cmd, const vec3 &ang, const bool no_packet = false);

public:
	void run(CUserCmd *const cmd);
};

MAKE_UNIQUE(AutoBackstab, auto_backstab);