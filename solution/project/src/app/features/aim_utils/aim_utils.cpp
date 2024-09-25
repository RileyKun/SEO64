#include "aim_utils.hpp"

#include "../player_list/player_list.hpp"
#include "../cfg.hpp"

void AimUtils::blockAttack(CUserCmd *const cmd)
{
	m_block_attack = false;

	if (!cmd) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon) {
		return;
	}

	const CTFWeaponInfo *const weapon_info{ weapon->m_pWeaponInfo() };

	if (!weapon_info) {
		return;
	}

	//don't attack if we're dead
	if (local->deadflag()) {
		m_block_attack = true;
	}

	//don't attack if we're taunting
	if (local->InCond(TF_COND_TAUNTING)) {
		m_block_attack = true;
	}

	//don't attack if spy's watch is out
	//BUG: if you time it right, by activating the watch as it's unactivating it can cause fake isAttacking
	if (local->m_bFeignDeathReady() || local->m_flInvisibility() > 0.0f) {
		m_block_attack = true;
	}

	//don't attack on an empty clip if the weapon uses primary ammo
	if (!weapon->HasPrimaryAmmoForShot() 
		&& weapon_info->GetWeaponData(0).m_nBulletsPerShot > 0 
		&& weapon->GetWeaponID() != TF_WEAPON_LASER_POINTER )
	{
		m_block_attack = true;

		if (!weapon->CanOverload()) {
			cmd->buttons &= ~IN_ATTACK;
		}
	}

	//TODO:
	//beggars overload
	//loose cannon overload
}

bool AimUtils::isAttacking(CUserCmd *const cmd, float curtime, const bool melee_no_swing)
{
	if (!cmd || m_block_attack) {
		return false;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return false;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon) {
		return false;
	}

	const bool can_primary_attack{
		(curtime >= local->m_flNextAttack()) && (curtime >= weapon->m_flNextPrimaryAttack())
	};

	if (tf_utils::isWeaponMelee(weapon))
	{
		if (melee_no_swing || weapon->GetWeaponID() == TF_WEAPON_KNIFE) {
			return (cmd->buttons & IN_ATTACK) && can_primary_attack;
		}

		return fabsf(weapon->m_flSmackTime() - curtime) < (i::global_vars->interval_per_tick * 2.0f);
	}

	switch (weapon->GetWeaponID())
	{
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		{
			if (const float charge_begin_time{ weapon->cast<C_TFPipebombLauncher>()->m_flChargeBeginTime() })
			{
				const float charge{ curtime - charge_begin_time };
				const float max_charge{ tf_utils::attribHookValue(4.0f, "stickybomb_charge_rate", weapon) };

				if ((charge > max_charge)
					&& (cmd->buttons & IN_ATTACK)
					&& (tf_globals::last_cmd.buttons & IN_ATTACK)
					&& can_primary_attack) {
					return true;
				}
			}

			return (!(cmd->buttons & IN_ATTACK) && (tf_globals::last_cmd.buttons & IN_ATTACK))
				&& can_primary_attack
				&& weapon->HasPrimaryAmmoForShot()
				&& weapon->m_iReloadMode() == TF_RELOAD_START;
		}

		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_CANNON:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: {
			return (!(cmd->buttons & IN_ATTACK) && (tf_globals::last_cmd.buttons & IN_ATTACK)) && can_primary_attack;
		}

		case TF_WEAPON_FLAME_BALL:
		{
			//s: should be using 100.0f but that fucks with it
			return (cmd->buttons & IN_ATTACK) && (curtime >= local->m_flNextAttack()) && local->m_flItemChargeMeter() >= 80.0f;
		}

		default:
		{
			if (weapon->CanOverload())
			{
				//missfire
				//FIX: only works on the first missfire
				if (weapon->IsReloading()) {
					return weapon->m_iClip1() >= 3 && can_primary_attack;
				}

				return weapon->HasPrimaryAmmoForShot() && can_primary_attack;
			}

			if (weapon->GetWeaponID() == TF_WEAPON_MINIGUN && weapon->cast<C_TFMinigun>()->m_iWeaponState() < 2) {
				return false;
			}

			const bool result{ (cmd->buttons & IN_ATTACK) && can_primary_attack };

			//some weapons dont fire when inattack2 >:(
			if (result
				&& weapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER 
				|| weapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT) {
				cmd->buttons &= ~IN_ATTACK2;
			}

			return result;
		}
	}
}

void AimUtils::setTickCount(CUserCmd *const cmd, C_BaseEntity *const ent, const LagRecord *const lr)
{
	if (!cmd || !ent || ent->GetClassId() != class_ids::CTFPlayer || !isAttacking(cmd, i::global_vars->curtime)) {
		return;
	}

	C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

	if (!pl || pl->deadflag()) {
		return;
	}

	cmd->tick_count = tf_utils::timeToTicks((lr ? lr->sim_time : pl->m_flSimulationTime()) + tf_utils::getLerp());
}

void AimUtils::smoothAim(CUserCmd *const cmd, const vec3 &target_angle, const float smooth_factor)
{
	if (!cmd) {
		return;
	}

	if (smooth_factor >= 100.0f) {
		return;
	}

	vec3 delta_ang{ target_angle - cmd->viewangles };

	math::clampAngles(delta_ang);

	const float smooth{ math::remap(smooth_factor, 1.0f, 100.0f, 1.5f, 30.0f) };

	if (delta_ang.length() > 0.0f && smooth) {
		cmd->viewangles += delta_ang / smooth;
		i::engine->SetViewAngles(cmd->viewangles);
	}
}

bool AimUtils::getMeleeInfo(float &swing_range, vec3 &hull_mins, vec3 &hull_maxs)
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return false;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon || !tf_utils::isWeaponMelee(weapon)) {
		return false;
	}

	swing_range = weapon->GetSwingRange();

	hull_mins = { -18.0f, -18.0f, -18.0f };
	hull_maxs = { 18.0f, 18.0f, 18.0f };

	const float hull_scale{ tf_utils::attribHookValue(1.0f, "melee_bounds_multiplier", weapon) };

	hull_mins *= hull_scale;
	hull_maxs *= hull_scale;

	const float model_scale{ local->m_flModelScale() };

	//l3: for some reason midgets still get normal people's range and hull, where is equality ???
	if (model_scale > 1.0f) {
		swing_range *= model_scale;
		hull_mins *= model_scale;
		hull_maxs *= model_scale;
	}

	swing_range = tf_utils::attribHookValue(swing_range, "melee_range_multiplier", weapon);

	return true;
}

bool AimUtils::shouldSkipPlayer(C_TFPlayer *const local, C_TFPlayer *const pl, const bool friendly_fire, const bool health_check)
{
	if (!local || !pl || pl->deadflag() || pl == local) {
		return true;
	}

	if (cfg::aimbot_ignore_sentry_busters && ec->isSentryBuster(pl)) {
		return true;
	}

	if (cfg::aimbot_ignore_friends && pl->IsPlayerOnSteamFriendsList()) {
		if (local->m_iTeamNum() == pl->m_iTeamNum()) { // mfed: if we're on the same team, we want to use our weapon if its friendly fire (aka medigun or disciplinary action)			
			if (!friendly_fire) {
				return true;
			}
		}
		else {
			return true;
		}
	}

	if (cfg::aimbot_ignore_invisible && pl->IsInvisible()) {
		return true;
	}

	if (cfg::aimbot_ignore_invulnerable && pl->IsInvulnerable()) {
		return true;
	}

	if (health_check) {
		if (local->m_iTeamNum() == pl->m_iTeamNum()
			&& pl->m_iHealth() >= pl->GetMaxHealth()) 
		{
			return true;
		}
	}

	if (PlayerListTag *const tag{ player_list->findPlayer(pl) }) {
		if (tag->priority < 0) {
			return true;
		}
	}

	return false;
}

bool AimUtils::shouldSkipBuilding(C_TFPlayer *const local, C_BaseObject *const obj)
{
	if (!local || !obj || obj->m_iHealth() <= 0) {
		return true;
	}

	if (cfg::aimbot_ignore_sentries && obj->GetClassId() == class_ids::CObjectSentrygun) {
		return true;
	}

	if (cfg::aimbot_ignore_dispensers && obj->GetClassId() == class_ids::CObjectDispenser) {
		return true;
	}

	if (cfg::aimbot_ignore_teleporters && obj->GetClassId() == class_ids::CObjectTeleporter) {
		return true;
	}

	if (cfg::aimbot_ignore_disabled_buildings && obj->IsDisabled()) {
		return true;
	}

	return false;
}