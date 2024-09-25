#pragma once

#include "../../player_data/player_data.hpp"

class AimbotHitscan final
{
private:
	struct Target final
	{
		C_BaseEntity *m_ent{};
		vec3 m_ang_to{};
		vec3 m_pos{};
		float m_fov_to{};
		float m_dist_to{};
		const LagRecord *m_lag_record{};
		int hitbox{};
	};

private:
	bool weaponHasHeadshot(C_TFPlayer *const local, C_TFWeaponBase *const weapon);
	int getTargetHitbox(C_TFPlayer *const local, C_TFWeaponBase *const weapon);
	bool canSee(C_TFPlayer *const local, C_TFWeaponBase* const weapon, const Target &target);
	bool scanBBox(C_TFPlayer *const local, C_TFWeaponBase* const weapon, Target &target);
	bool processTarget(C_TFPlayer *const local, C_TFWeaponBase *const weapon, Target &target, const bool second_check = false);
	bool getTarget(C_TFPlayer *const local, CUserCmd* const cmd,C_TFWeaponBase *const weapon, Target &out);
	void setAttack(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const Target &target);
	void setAngles(CUserCmd *const cmd, const Target &target);
    bool passesHitChance( C_TFPlayer* const local, C_TFWeaponBase* const weapon, const Target& target, const float& hitchance, const bool ignore_perfect = false);

public:
	void run(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon);
};

MAKE_UNIQUE(AimbotHitscan, aimbot_hitscan);