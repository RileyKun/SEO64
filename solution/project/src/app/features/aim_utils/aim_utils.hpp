#pragma once

#include "../player_data/player_data.hpp"

class AimUtils final
{
public:
	class SetAbsOrigin final
	{
	private:
		C_BaseEntity *m_ent{};
		vec3 m_og_abs_origin{};

	public:
		SetAbsOrigin() = default;

		SetAbsOrigin(C_BaseEntity *const ent, const vec3 &new_abs_origin)
		{
			if (!ent) {
				return;
			}

			m_ent = ent;
			m_og_abs_origin = ent->GetAbsOrigin();

			m_ent->SetAbsOrigin(new_abs_origin);
		}

		~SetAbsOrigin()
		{
			if (!m_ent) {
				return;
			}

			m_ent->SetAbsOrigin(m_og_abs_origin);
		}
	};

private:
	bool m_block_attack{};

public:
	void blockAttack(CUserCmd *const cmd);
	bool isAttacking(CUserCmd *const cmd, float curtime, const bool melee_no_swing = false);
	void setTickCount(CUserCmd *const cmd, C_BaseEntity *const ent, const LagRecord *const lr);
	void smoothAim(CUserCmd *const cmd, const vec3 &target_angle, const float smooth_factor);
	bool getMeleeInfo(float &swing_range, vec3 &hull_mins, vec3 &hull_maxs);

public:
	bool shouldSkipPlayer(
		C_TFPlayer *const local,
		C_TFPlayer *const pl,
		const bool friendly_fire = false,
		const bool health_check = false);

	bool shouldSkipBuilding(
		C_TFPlayer *const local,
		C_BaseObject *const obj);
};

MAKE_UNIQUE(AimUtils, aim_utils);