#include "aimbot.hpp"

#include "aimbot_hitscan/aimbot_hitscan.hpp"
#include "aimbot_melee/aimbot_melee.hpp"
#include "aimbot_projectile/aimbot_projectile.hpp"
#include "aimbot_snap/aimbot_snap.hpp"

#include "../aim_utils/aim_utils.hpp"
#include "../tick_base/tick_base.hpp"
#include "../cfg.hpp"

void Aimbot::run(CUserCmd *const cmd)
{
	const bool active{ cmd && (cfg::aimbot_active || (cfg::aimbot_active_on_attack && aim_utils->isAttacking(cmd, i::global_vars->curtime))) };

	if (!active) {
		tick_base->m_delay_hitscan = false;
		aimbot_snap->reset();
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local
		|| local->deadflag()
		|| local->InCond(TF_COND_TAUNTING)
		|| local->m_bFeignDeathReady() 
		|| local->m_flInvisibility() > 0.0f) {
		tick_base->m_delay_hitscan = false;
		aimbot_snap->reset();
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon) {
		return;
	}

	const int weapon_id{ weapon->GetWeaponID() };

	if (weapon_id == TF_WEAPON_MEDIGUN
		|| weapon_id == TF_WEAPON_BUILDER
		|| weapon_id == TF_WEAPON_PDA_ENGINEER_BUILD
		|| weapon_id == TF_WEAPON_PDA_ENGINEER_DESTROY
		|| weapon_id == TF_WEAPON_PDA_SPY) {
		tick_base->m_delay_hitscan = false;
		aimbot_snap->reset();
		return;
	}

	if (tf_utils::isWeaponHitscan(weapon)) {
		aimbot_hitscan->run(cmd, local, weapon);
		aimbot_snap->reset();
	}

	else if (tf_utils::isWeaponMelee(weapon)) {
		tick_base->m_delay_hitscan = false;
		aimbot_snap->reset();
		aimbot_melee->run(cmd, local, weapon);
	}

	else if (tf_utils::isWeaponProjectile(weapon)) {
		tick_base->m_delay_hitscan = false;
		aimbot_projectile->run(cmd, local, weapon);
	}
}