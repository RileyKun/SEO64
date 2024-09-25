#include "aimbot_hitscan.hpp"

#include "../../aim_utils/aim_utils.hpp"
#include "../../trace_utils/trace_utils.hpp"
#include "../../engine_pred/engine_pred.hpp"
#include "../../tick_base/tick_base.hpp"
#include "../../hitboxes/hitboxes.hpp"
#include "../../player_list/player_list.hpp"
#include "../../cfg.hpp"
#include "../../seed_pred/seed_pred.hpp"

bool AimbotHitscan::weaponHasHeadshot(C_TFPlayer *const local, C_TFWeaponBase *const weapon)
{
	switch (weapon->GetWeaponID())
	{
		case TF_WEAPON_SNIPERRIFLE:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC:
		case TF_WEAPON_SNIPERRIFLE_DECAP:
		{
			const int rifle_type{ weapon->cast<C_TFSniperRifle>()->GetRifleType() };

			if (rifle_type == RIFLE_CLASSIC) {
				return true;
			}

			return rifle_type != RIFLE_JARATE && local->InCond(TF_COND_ZOOMED);
		}

		case TF_WEAPON_REVOLVER: {
			return weapon->cast<C_TFRevolver>()->CanHeadshot();
		}

		default: {
			return false;
		}
	}
}

int AimbotHitscan::getTargetHitbox(C_TFPlayer *const local, C_TFWeaponBase *const weapon)
{
	if (!local || !weapon) {
		return 0;
	}

	if (cfg::aimbot_hitscan_aim_pos == 0)
	{
		if (local->IsCritBoosted()) {
			return HITBOX_PELVIS;
		}

		if (weaponHasHeadshot(local, weapon)) {
			return HITBOX_HEAD;
		}

		return HITBOX_PELVIS;
	}

	if (cfg::aimbot_hitscan_aim_pos == 1) {
		return HITBOX_PELVIS;
	}

	if (cfg::aimbot_hitscan_aim_pos == 2) {
		return HITBOX_HEAD;
	}

	return 0;
}

bool AimbotHitscan::canSee(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const Target &target)
{
	if (!local || !weapon || !target.m_ent) {
		return false;
	}

	CTFWeaponInfo *const info{ weapon->m_pWeaponInfo() };

	if (!info) {
		return false;
	}

	if (vec3(target.m_pos - local->GetEyePos()).length() > info->GetWeaponData(weapon->m_iWeaponMode()).m_flRange) {
		return false;
	}

	trace_filters::hitscan_aim.m_ignore_ent = local;

	trace_t trace{};

	if (const LagRecord *const lr{ target.m_lag_record }) {
		lrm->set(target.m_ent->cast<C_TFPlayer>(), lr);
		tf_utils::trace(local->GetEyePos(), target.m_pos, &trace_filters::hitscan_aim, &trace, MASK_SHOT | CONTENTS_GRATE);
		lrm->reset();
	}

	else {
		tf_utils::trace(local->GetEyePos(), target.m_pos, &trace_filters::hitscan_aim, &trace, MASK_SHOT | CONTENTS_GRATE);
	}

	return trace.m_pEnt == target.m_ent && !trace.allsolid && !trace.startsolid;
}

bool AimbotHitscan::scanBBox(C_TFPlayer *const local, C_TFWeaponBase* const weapon, Target &target)
{
	if (!local || !target.m_ent) {
		return false;
	}

	for (float point_scale{ 0.9f }; point_scale > 0.5f; point_scale -= 0.1f)
	{
		const vec3 scaled_mins{ target.m_ent->m_vecMins() * point_scale };
		const vec3 scaled_maxs{ target.m_ent->m_vecMaxs() * point_scale };

		const std::vector<vec3> offsets
		{
			{ (scaled_mins.x + scaled_maxs.x) * 0.5f, (scaled_mins.y + scaled_maxs.y) * 0.5f, scaled_mins.z },
			{ (scaled_mins.x + scaled_maxs.x) * 0.5f, (scaled_mins.y + scaled_maxs.y) * 0.5f, scaled_maxs.z },

			{ scaled_mins.x, scaled_mins.y, (scaled_mins.z + scaled_maxs.z) * 0.5f  },
			{ scaled_maxs.x, scaled_mins.y, (scaled_mins.z + scaled_maxs.z) * 0.5f  },
			{ scaled_mins.x, scaled_maxs.y, (scaled_mins.z + scaled_maxs.z) * 0.5f  },
			{ scaled_maxs.x, scaled_maxs.y, (scaled_mins.z + scaled_maxs.z) * 0.5f  },

			{ scaled_mins.x, scaled_mins.y, scaled_maxs.z },
			{ scaled_maxs.x, scaled_mins.y, scaled_maxs.z },
			{ scaled_mins.x, scaled_maxs.y, scaled_maxs.z },
			{ scaled_maxs.x, scaled_maxs.y, scaled_maxs.z },

			{ scaled_mins.x, scaled_mins.y, scaled_mins.z },
			{ scaled_maxs.x, scaled_mins.y, scaled_mins.z },
			{ scaled_mins.x, scaled_maxs.y, scaled_mins.z },
			{ scaled_maxs.x, scaled_maxs.y, scaled_mins.z },
		};

		for (size_t n{}; n < offsets.size(); n++)
		{
			const vec3 point{ target.m_ent->m_vecOrigin() + offsets[n] };

			target.m_ang_to = math::calcAngle(local->GetEyePos(), point).normalizeAngles();
			target.m_pos = point;

			if (canSee(local, weapon, target)) {
				return true;
			}
		}
	}

	return false;
}

bool AimbotHitscan::processTarget(C_TFPlayer *const local, C_TFWeaponBase *const weapon, Target &target, const bool second_check)
{
	if (!local || !weapon || !target.m_ent) {
		return false;
	}

	if (canSee(local, weapon, target)) {
		return true;
	}

	const size_t class_id{ target.m_ent->GetClassId() };

	if (class_id == class_ids::CTFPlayer && !second_check)
	{
		if (cfg::aimbot_hitscan_scan_hitboxes && target.m_lag_record && target.hitbox != HITBOX_HEAD)
		{
			for (int hitbox_iter{}; hitbox_iter < target.m_lag_record->hitboxes.size(); hitbox_iter++)
			{
				if (hitbox_iter == target.hitbox) {
					continue;
				}

				// never target limbs on moving people
				if (hitbox_iter > HITBOX_UPPER_CHEST && !target.m_lag_record->vel.isZero()) {
					continue;
				}

				const vec3 lr_pos{ target.m_lag_record->hitboxes[hitbox_iter] };
				const vec3 ang_to{ math::calcAngle(local->GetEyePos(), lr_pos).normalizeAngles() };

				const float fov_to{ math::calcFOV(i::engine->GetViewAngles(), ang_to) };
				const float dist_to{ local->GetEyePos().distTo(lr_pos) };

				const Target new_target{ target.m_ent, ang_to, lr_pos, fov_to, dist_to, target.m_lag_record, hitbox_iter };

				if (canSee(local, weapon, new_target)) {
					target = new_target;
					return true;
				}
			}
		}
	}

	else if (class_id == class_ids::CObjectSentrygun
		|| class_id == class_ids::CObjectDispenser
		|| class_id == class_ids::CObjectTeleporter) 
	{
		if (cfg::aimbot_hitscan_bbox_multipoint) {
			return scanBBox(local, weapon, target);
		}
	}

	return false;
}

bool AimbotHitscan::getTarget(C_TFPlayer *const local, CUserCmd* const cmd, C_TFWeaponBase *const weapon, Target &out)
{
	if (!local || !weapon) {
		return false;
	}

	std::vector<Target> targets{};

	for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PLAYERS_ENEMIES))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

		if (aim_utils->shouldSkipPlayer(local, pl)) {
			continue;
		}

		const int target_hitbox_id{ getTargetHitbox(local, weapon) };

		for (size_t n{}; n < player_data->getNumValidRecords(pl); n++)
		{
			const LagRecord *const lr{ player_data->getRecord(pl, n) };

			if (!lr || !lr->isRecordValid() || lr->hitboxes.size() <= target_hitbox_id) {
				continue;
			}

			if (lr->interpolated && !cfg::aimbot_fakelag_fix) {
				continue;
			}

			const vec3 lr_pos{ lr->hitboxes[target_hitbox_id] };
			const vec3 ang_to{ math::calcAngle(local->GetEyePos(), lr_pos).normalizeAngles()};
			const float fov_to{ math::calcFOV(i::engine->GetViewAngles(), ang_to) };
			const float dist_to{ local->GetEyePos().distTo(lr_pos) };

			if (cfg::aimbot_hitscan_mode == 0 && fov_to > cfg::aimbot_hitscan_fov) {
				continue;
			}

			targets.push_back({ pl, ang_to, lr_pos, fov_to, dist_to, lr, target_hitbox_id });
		}
	}

	for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::BUILDINGS_ENEMIES))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_BaseObject *const obj{ ent->cast<C_BaseObject>() };

		if (!obj || aim_utils->shouldSkipBuilding(local, obj)) {
			continue;
		}

		const vec3 obj_pos{ obj->GetCenter() };
		const vec3 ang_to{ math::calcAngle(local->GetEyePos(), obj_pos) };
		const float fov_to{ math::calcFOV(i::engine->GetViewAngles(), ang_to) };
		const float dist_to{ local->GetEyePos().distTo(obj_pos) };

		if (cfg::aimbot_hitscan_mode == 0 && fov_to > cfg::aimbot_hitscan_fov) {
			continue;
		}

		targets.push_back({ obj, ang_to, obj_pos, fov_to, dist_to, nullptr });
	}

	if (cfg::aimbot_hitscan_target_stickies)
	{
		for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PROJECTILES_ENEMIES))
		{
			IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

			if (!ent || ent->GetClassId() != class_ids::CTFGrenadePipebombProjectile) {
				continue;
			}

			C_TFGrenadePipebombProjectile *const sticky{ ent->cast<C_TFGrenadePipebombProjectile>() };

			if (!sticky || sticky->m_iType() != TF_GL_MODE_REMOTE_DETONATE || !sticky->m_bTouched()) {
				continue;
			}

			const vec3 sticky_pos{ sticky->GetCenter() };
			const vec3 ang_to{ math::calcAngle(local->GetEyePos(), sticky_pos) };
			const float fov_to{ math::calcFOV(i::engine->GetViewAngles(), ang_to) };
			const float dist_to{ local->GetEyePos().distTo(sticky_pos) };

			if (cfg::aimbot_hitscan_mode == 0 && fov_to > cfg::aimbot_hitscan_fov) {
				continue;
			}

			targets.push_back({ sticky, ang_to, sticky_pos, fov_to, dist_to, nullptr });
		}
	}

	if (cfg::aimbot_hitscan_target_rockets_minigun_mvm
		&& weapon->GetWeaponID() == TF_WEAPON_MINIGUN
		&& tf_utils::attribHookValue(0.0f, "attack_projectile", weapon))
	{
		for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PROJECTILES_ENEMIES))
		{
			IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

			if (!ent || ent->GetClassId() != class_ids::CTFProjectile_Rocket) {
				continue;
			}

			C_TFProjectile_Rocket *const rocket{ ent->cast<C_TFProjectile_Rocket>() };

			if (!rocket) {
				continue;
			}

			const vec3 rocket_pos{ rocket->GetCenter() };
			const vec3 ang_to{ math::calcAngle(local->GetEyePos(), rocket_pos) };
			const float fov_to{ math::calcFOV(i::engine->GetViewAngles(), ang_to) };
			const float dist_to{ local->GetEyePos().distTo(rocket_pos) };

			if (cfg::aimbot_hitscan_mode == 0 && fov_to > cfg::aimbot_hitscan_fov) {
				continue;
			}

			targets.push_back({ rocket, ang_to, rocket_pos, fov_to, dist_to, nullptr });
		}
	}

	if (targets.empty()) {
		return false;
	}

	std::sort(targets.begin(), targets.end(), [](const Target &a, const Target &b)
	{
		if (cfg::aimbot_hitscan_mode == 0) {
			return a.m_fov_to < b.m_fov_to;
		}

		if (cfg::aimbot_hitscan_mode == 1) {
			return a.m_dist_to < b.m_dist_to;
		}

		return false;
	});

	std::sort(targets.begin(), targets.end(), [](const Target &a, const Target &b)
	{
	  PlayerListTag *const tag_a{ player_list->findPlayer(a.m_ent->cast<C_TFPlayer>()) };
	  PlayerListTag *const tag_b{ player_list->findPlayer(b.m_ent->cast<C_TFPlayer>()) };

	  if (tag_a && tag_b) {
		  return tag_a->priority > tag_b->priority;
	  }

	  if (tag_a && !tag_b) {
		  return true;
	  }

	  if (!tag_a && tag_b) {
		  return false;
	  }

	  return false;
	});

	for (Target &target : targets)
	{
		if (!processTarget(local, weapon, target)) {
			continue;
		}

		//fix for back speed clamp potentially causing prediction errors
		const CUserCmd backup_cmd{ *cmd };
		vec3 backup_angles{ i::engine->GetViewAngles() };
		bool can_see{};

		//restore back to our last predicted command
		engine_pred->end(); 
		
		//set the cmd viewangles to our target ones
		setAngles(cmd, target);

		//predict the cmd again and check if we hit the target
		RUN_PREDICTION(cmd, [&](){
			can_see = processTarget(local, weapon, target, true);
		})

		//restore all data modified by this
		i::engine->SetViewAngles(backup_angles);
		*cmd = backup_cmd;

		if (!engine_pred->start(cmd)) {
			return false;	
		}

		//if back speed moveclamp fucked our shit skip this target
		if (!can_see) {
			continue;
		}

		out = target;

		return true;
	}

	return false;
}

void AimbotHitscan::setAttack(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const Target &target)
{
	if (!cmd
		|| !local
		|| !weapon
		|| !target.m_ent
		|| !weapon->HasPrimaryAmmoForShot()
		|| !cfg::aimbot_auto_shoot) {
		return;
	}

	if (cfg::aimbot_hitscan_wait_for_headshot
		&& target.m_ent->GetClassId() == class_ids::CTFPlayer
		&& weaponHasHeadshot(local, weapon)
		&& !local->IsCritBoosted())
	{
		CUserCmd dummy{ *cmd };
		dummy.buttons |= IN_ATTACK;

		float og_curtime{ i::global_vars->curtime };
		if (tick_base->canDoubleTap(&dummy, false))
		{
			const int ticks_to_shift{ tick_base->getShiftLimit() };
			const int server_tick{ tf_utils::timeToTicks(tf_utils::getServerTime() + tf_utils::getLatency()) - 1 };
			const float adjusted_time{ tf_utils::ticksToTime(tick_base->getAdjustedTick(ticks_to_shift + 1, local->m_nTickBase(), server_tick)) };
			i::global_vars->curtime = adjusted_time;
		}

		switch (weapon->GetWeaponID())
		{
			case TF_WEAPON_SNIPERRIFLE:
			case TF_WEAPON_SNIPERRIFLE_CLASSIC:
			case TF_WEAPON_SNIPERRIFLE_DECAP:
			{
				if (static_cast<int>(tf_utils::attribHookValue(0.0f, "sniper_no_headshot_without_full_charge", weapon)) != 0)
				{
					if (!weapon->cast<C_TFSniperRifle>()->IsFullyCharged()) {
						cmd->buttons |= IN_ATTACK;
						return;
					}
				}

				if (static_cast<int>(tf_utils::attribHookValue(0.0f, "sniper_crit_no_scope", weapon)) == 0)
				{
					if ((i::global_vars->curtime - local->m_flFOVTime()) < TF_WEAPON_SNIPERRIFLE_NO_CRIT_AFTER_ZOOM_TIME) {
						return;
					}
				}

				break;
			}

			case TF_WEAPON_REVOLVER:
			{
				if (weapon->cast<C_TFRevolver>()->GetWeaponSpread() > 0.0f) {
					return;
				}

				break;
			}

			default: {
				break;
			}
		}

		i::global_vars->curtime = og_curtime;
	}

	if (cfg::aimbot_hitscan_aim_method == 1)
	{
		vec3 forward{};
		math::angleVectors(cmd->viewangles, &forward);

		const vec3 trace_start{ local->GetEyePos() };
		const vec3 trace_end{ trace_start + (forward * 8192.0f) };

		trace_filters::hitscan_aim.m_ignore_ent = local;

		trace_t trace{};

		if (const LagRecord *const lr{ target.m_lag_record }) {
			lrm->set(target.m_ent->cast<C_TFPlayer>(), lr);
			tf_utils::trace(trace_start, trace_end, &trace_filters::hitscan_aim, &trace, MASK_SHOT | CONTENTS_GRATE);
			lrm->reset();
		}

		else {
			tf_utils::trace(trace_start, trace_end, &trace_filters::hitscan_aim, &trace, MASK_SHOT | CONTENTS_GRATE);
		}

		if (trace.m_pEnt == target.m_ent && !trace.allsolid && !trace.startsolid) {
			cmd->buttons |= IN_ATTACK;
		}
	}

	else {
		cmd->buttons |= IN_ATTACK;
	}

	if (weapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC && weapon->cast<C_TFSniperRifleClassic>()->m_bCharging()) {
		cmd->buttons &= ~IN_ATTACK;
	}
}

void AimbotHitscan::setAngles(CUserCmd *const cmd, const Target &target)
{
	if (!cmd) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	const vec3 target_angle{ target.m_ang_to - (local->m_vecPunchAngle() * 2.f) };

	switch (cfg::aimbot_hitscan_aim_method)
	{
		case 0: {
			cmd->viewangles = target_angle;
			i::engine->SetViewAngles(cmd->viewangles);
			break;
		}

		case 1: {
			aim_utils->smoothAim(cmd, target_angle, cfg::aimbot_hitscan_smooth);
			break;
		}

		case 2: {
			tf_utils::fixMovement(cmd, target_angle);
			cmd->viewangles = target_angle;
			break;
		}

		case 3:
		{
			if (aim_utils->isAttacking(cmd, i::global_vars->curtime)) {
				tf_utils::fixMovement(cmd, target_angle);
				cmd->viewangles = target_angle;
			}

			break;
		}

		default: {
			break;
		}
	}
}

bool AimbotHitscan::passesHitChance(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const Target &target, const float& hitchance, const bool ignore_perfect)
{
	CTFWeaponInfo *const info{ weapon->m_pWeaponInfo() };

	if (!info) {
		return false;
	}

	if (!seed_pred->m_spread_init) {
		return false;
	}

	const float time_since_last_shot{ i::global_vars->curtime - weapon->m_flLastFireTime() };

	bool perfect_shot{};

	int bullets_per_shot{ info->GetWeaponData(0).m_nBulletsPerShot };

	if (bullets_per_shot >= 1) {
		bullets_per_shot = tf_utils::attribHookValue(bullets_per_shot, "mult_bullets_per_shot", weapon);
	}

	else {
		bullets_per_shot = 1;
	}

	if (!ignore_perfect)
	{
		if (seed_pred->step() < 4.f) {
			if (bullets_per_shot > 1 && time_since_last_shot > 0.25f)
			{
				perfect_shot = true;

				if (weapon->GetWeaponID() == TF_WEAPON_MINIGUN) {
					return true;
				}
			}

			else if (bullets_per_shot == 1 && time_since_last_shot > 1.25f) {
				return true;
			}
		} 
		else {
			return true;
		}
	}

	Ray_t ray{};
	trace_t trace;

	int needed_hits{ static_cast<int>(std::ceilf((hitchance * 255.0f) / 100.0f)) };
	int total_hits{};

	if (perfect_shot) {
		total_hits += 1;
	}

	const float spread{ weapon->GetWeaponSpread() };
	const vec3 eye_pos{ local->GetEyePos() };

	for (int n{}; n <= 255; n++)
	{
		vec2 spread_offset{ seed_pred->m_spread_offsets[n] };

		vec3 spread_vec{ spread_offset.x * spread, spread_offset.y * spread, 0.0f };

		vec3 forward{};
		vec3 right{};
		vec3 up{};

		math::angleVectors(target.m_ang_to, &forward, &right, &up);

		spread_vec = forward + (right * spread_vec.x) + (up * spread_vec.y);
		spread_vec.normalize();

		ray.Init(eye_pos, { eye_pos + spread_vec * info->GetWeaponData(weapon->m_iWeaponMode()).m_flRange });

		i::trace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, target.m_ent, &trace);

		if (trace.m_pEnt == target.m_ent) {
			total_hits++;
		}

		if (total_hits >= needed_hits) {
			return true;
		}

		if ((255 - n + total_hits) < needed_hits) {
			return false;
		}
	}

	return false;
}

void AimbotHitscan::run(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon)
{
	if (!cfg::aimbot_hitscan_active || !cmd || !local || !weapon) {
		return;
	}

	if (cfg::aimbot_hitscan_auto_rev_minigun && weapon->GetWeaponID() == TF_WEAPON_MINIGUN) {
		cmd->buttons |= IN_ATTACK2;
	}

	Target target{};

	if (!getTarget(local, cmd, weapon, target)) {
		tick_base->m_delay_hitscan = false;
		return;
	}

	// if we are antiaiming never shoot on the first tick of choke cycle
	if (cfg::antiaim_active && cfg::antiaim_fake_type != 0 && !i::client_state->chokedcommands) {
		return;
	}

	if (cfg::aimbot_hitscan_hitchance > 0.0f && !passesHitChance(local, weapon, target, cfg::aimbot_hitscan_hitchance)) {
		return;
	}

	CUserCmd dummy{ *cmd };

	setAttack(&dummy, local, weapon, target);
	setAngles(&dummy, target);

	if (cfg::tb_dt_hitchance > 0.0f 
		&& tick_base->canDoubleTap(&dummy, false) 
		&& !passesHitChance(local, weapon, target, cfg::tb_dt_hitchance, true)
		&& !tick_base->m_delay_hitscan
		&& target.m_ent->GetClassId() != class_ids::CTFGrenadePipebombProjectile) {
		tick_base->m_delay_hitscan = true;
		setAttack(cmd, local, weapon, target);
		return;
	}

	else if (tick_base->m_delay_hitscan) {
		tick_base->m_delay_hitscan = false;
	}

	setAttack(cmd, local, weapon, target);
	setAngles(cmd, target);

	aim_utils->setTickCount(cmd, target.m_ent, target.m_lag_record);

	tf_globals::aimbot_target = target.m_ent ? target.m_ent->GetRefEHandle() : EntityHandle_t{};
	tf_globals::aimbot_target_hitbox = target.hitbox;

	if (target.m_ent
		&& target.m_ent->GetClassId() == class_ids::CTFPlayer
		&& target.m_lag_record
		&& aim_utils->isAttacking(cmd, i::global_vars->curtime)) {
		hitboxes->add(target.m_ent->cast<C_TFPlayer>(), target.m_lag_record->bones);
	}
}