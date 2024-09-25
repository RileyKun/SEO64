#pragma once

#include "../../player_data/player_data.hpp"

class AimbotMelee final
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
		bool m_can_swing_pred{};
		bool m_was_swing_preded{};
	};

private:
	std::vector<vec3> m_local_positions{}; //s: for swing pred, resimed once per tick

private:
	bool canSee(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const Target &target, const vec3 *const local_pos_override = nullptr, const bool skip_lr = false);
	bool swingPred(C_TFPlayer *const local, C_TFWeaponBase *const weapon, Target &target);
	bool processTarget(C_TFPlayer *const local, C_TFWeaponBase *const weapon, Target &target);
	bool getTarget(C_TFPlayer *const local, C_TFWeaponBase *const weapon, Target &out);
	void setAttack(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const Target &target);
	void setAngles(CUserCmd *const cmd, const Target &target);

public:
	void run(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon);
};

MAKE_UNIQUE(AimbotMelee, aimbot_melee);