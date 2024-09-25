#include "aimbot_melee.hpp"

#include "../../aim_utils/aim_utils.hpp"
#include "../../trace_utils/trace_utils.hpp"
#include "../../game_movement/move_pred.hpp"
#include "../../tick_base/tick_base.hpp"
#include "../../hitboxes/hitboxes.hpp"
#include "../../player_list/player_list.hpp"
#include "../../cfg.hpp"

bool AimbotMelee::canSee(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const Target &target, const vec3 *const local_pos_override, const bool skip_lr)
{
	if (!local || !target.m_ent) {
		return false;
	}

	float swing_range{};

	vec3 hull_mins{};
	vec3 hull_maxs{};

	if (!aim_utils->getMeleeInfo(swing_range, hull_mins, hull_maxs)) {
		return false;
	}

	vec3 forward{};
	math::angleVectors(target.m_ang_to, &forward);

	const vec3 trace_start{ local_pos_override ? *local_pos_override : local->GetEyePos() };
	const vec3 trace_end{ trace_start + (forward * swing_range) };

	trace_filters::melee_aim.m_ignore_ent = local;

	trace_t trace{};

	//s: called from swing pred, dont override origin
	if (skip_lr) {
		tf_utils::traceHull(trace_start, trace_end, hull_mins, hull_maxs, &trace_filters::melee_aim, &trace, MASK_SOLID);
	}

	else
	{
		if (const LagRecord *const lr{ target.m_lag_record }) {
			lrm->set(target.m_ent->cast<C_TFPlayer>(), lr);
			tf_utils::traceHull(trace_start, trace_end, hull_mins, hull_maxs, &trace_filters::melee_aim, &trace, MASK_SOLID);
			lrm->reset();
		}

		else {
			tf_utils::traceHull(trace_start, trace_end, hull_mins, hull_maxs, &trace_filters::melee_aim, &trace, MASK_SOLID);
		}
	}

	return trace.m_pEnt == target.m_ent;
}

bool AimbotMelee::swingPred(C_TFPlayer *const local, C_TFWeaponBase *const weapon, Target &target)
{
	if (!cfg::aimbot_melee_swing_pred
		|| !local
		|| !weapon
		|| weapon->GetWeaponID() == TF_WEAPON_KNIFE
		|| !target.m_ent
		|| !target.m_can_swing_pred) {
		return false;
	}

	const int max_time_ticks{ tf_utils::timeToTicks(cfg::aimbot_melee_swing_pred_time + 0.0125f) };

	auto getPositions = [max_time_ticks](C_BaseEntity *const ent, std::vector<vec3> &out, const bool ignore_special_ability)
	{
		if (!ent) {
			return false;
		}

		if (ent->GetClassId() != class_ids::CTFPlayer) {
			out.push_back(ent->GetCenter());
			return true;
		}

		if (ent->cast<C_TFPlayer>()->m_vecVelocity().isZero()) {
			out.push_back(ent->m_vecOrigin());
			return true;
		}

		MovePred pred{};

		if (!pred.init(ent->cast<C_TFPlayer>(), ignore_special_ability)) {
			return false;
		}

		for (int n{}; n < max_time_ticks; n++) {
			pred.tick();
			out.push_back(pred.origin());
		}

		return true;
	};

	if (m_local_positions.empty() && !getPositions(local, m_local_positions, true)) {
		return false;
	}

	std::vector<vec3> target_positions{};

	if (!getPositions(target.m_ent, target_positions, false)) {
		return false;
	}

	for (int n{}; n < max_time_ticks; n++)
	{
		const size_t local_pos_idx{ std::clamp(static_cast<size_t>(n), 0ull, m_local_positions.size() - 1) };
		const size_t target_pos_idx{ std::clamp(static_cast<size_t>(n), 0ull, target_positions.size() - 1) };

		const float bounds{ (target.m_ent->m_vecMins().z + target.m_ent->m_vecMaxs().z) };

		const std::vector<vec3> target_positions_offset
		{
			target_positions[target_pos_idx] + vec3{ 0.0f, 0.0f, bounds * 0.0f },
			target_positions[target_pos_idx] + vec3{ 0.0f, 0.0f, bounds * 0.2f },
			target_positions[target_pos_idx] + vec3{ 0.0f, 0.0f, bounds * 0.5f },
			target_positions[target_pos_idx] + vec3{ 0.0f, 0.0f, bounds * 0.7f },
			target_positions[target_pos_idx] + vec3{ 0.0f, 0.0f, bounds * 1.0f }
		};

		const vec3 local_pos{ m_local_positions[local_pos_idx] + local->m_vecViewOffset() };

		const vec3 target_pos{ *std::min_element(target_positions_offset.begin(), target_positions_offset.end(), [&](const vec3 &a, const vec3 &b) {
			return a.distTo(local_pos) < b.distTo(local_pos);
		}) };

		const AimUtils::SetAbsOrigin set_abs_origin{ target.m_ent, target_positions[target_pos_idx] };

		const vec3 ang_backup{ target.m_ang_to };

		target.m_ang_to = math::calcAngle(local_pos, target_pos);

		if (canSee(local, weapon, target, &local_pos, true)) {
			target.m_pos = target_pos;
			target.m_was_swing_preded = true;
			return true;
		}

		target.m_ang_to = ang_backup;
	}

	return false;
}

bool AimbotMelee::processTarget(C_TFPlayer *const local, C_TFWeaponBase *const weapon, Target &target)
{
	if (!local || !target.m_ent) {
		return false;
	}

	float swing_range{};

	vec3 hull_mins{};
	vec3 hull_maxs{};

	if (!aim_utils->getMeleeInfo(swing_range, hull_mins, hull_maxs)) {
		return false;
	}

	float max_dist{ swing_range * 2.0f };

	if (cfg::aimbot_melee_swing_pred) {
		max_dist += local->m_vecVelocity().length() * 1.2f;
	}

	if (target.m_pos.distTo(local->GetEyePos()) > max_dist) {
		return false;
	}

	if (!canSee(local, weapon, target)) {
		return swingPred(local, weapon, target);
	}

	return true;
}

bool AimbotMelee::getTarget(C_TFPlayer *const local, C_TFWeaponBase *const weapon, Target &out)
{
	if (!local || !weapon) {
		return false;
	}

	std::vector<Target> targets{};

	const ECGroup player_group
	{
		tf_utils::attribHookValue(0.0f, "speed_buff_ally", weapon) > 0.0f
		? ECGroup::PLAYERS_ALL
		: ECGroup::PLAYERS_ENEMIES
	};

	for (const EntityHandle_t ehandle : ec->getGroup(player_group))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

		if (aim_utils->shouldSkipPlayer(local, pl, player_group == ECGroup::PLAYERS_ALL)) {
			continue;
		}

		const int target_hitbox_id{ HITBOX_BODY };

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
			const vec3 ang_to{ math::calcAngle(local->GetEyePos(), lr_pos) };
			const float fov_to{ math::calcFOV(i::engine->GetViewAngles(), ang_to) };
			const float dist_to{ local->GetEyePos().distTo(lr_pos) };

			if (cfg::aimbot_melee_mode == 0 && fov_to > cfg::aimbot_melee_fov) {
				continue;
			}

			targets.push_back({ pl, ang_to, lr_pos, fov_to, dist_to, lr, n == 0, false });
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

		if (cfg::aimbot_melee_mode == 0 && fov_to > cfg::aimbot_melee_fov) {
			continue;
		}

		targets.push_back({ obj, ang_to, obj_pos, fov_to, dist_to, nullptr, true, false });
	}

	if (targets.empty()) {
		return false;
	}

	std::sort(targets.begin(), targets.end(), [](const Target &a, const Target &b)
	{
		if (cfg::aimbot_melee_mode == 0) {
			return a.m_fov_to < b.m_fov_to;
		}

		if (cfg::aimbot_melee_mode == 1) {
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

	size_t swing_pred_attempts{};

	for (size_t n{}; n < targets.size(); n++)
	{
		Target &target{ targets[n] };

		if (swing_pred_attempts >= 6 && target.m_can_swing_pred) {
			continue;
		}

		if (!processTarget(local, weapon, target))
		{
			if (target.m_can_swing_pred) {
				swing_pred_attempts++;
			}

			continue;
		}

		out = target;

		return true;
	}

	return false;
}

void AimbotMelee::setAttack(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const Target &target)
{
	if (!cmd || !local || !weapon || !cfg::aimbot_auto_shoot) {
		return;
	}

	if (cfg::aimbot_melee_aim_method == 1)
	{
		vec3 forward{};
		math::angleVectors(cmd->viewangles, &forward);

		float swing_range{};

		vec3 hull_mins{};
		vec3 hull_maxs{};

		if (!aim_utils->getMeleeInfo(swing_range, hull_mins, hull_maxs)) {
			return;
		}

		const vec3 trace_start{ local->GetEyePos() };
		const vec3 trace_end{ trace_start + (forward * swing_range) };

		trace_filters::melee_aim.m_ignore_ent = local;

		trace_t trace{};

		if (target.m_was_swing_preded)
		{
			const AimUtils::SetAbsOrigin set_abs_origin{ target.m_ent, target.m_pos };

			tf_utils::traceHull(trace_start, trace_end, hull_mins, hull_maxs, &trace_filters::melee_aim, &trace, MASK_SOLID);
		}

		else
		{
			if (const LagRecord *const lr{ target.m_lag_record }) {
				lrm->set(target.m_ent->cast<C_TFPlayer>(), lr);
				tf_utils::traceHull(trace_start, trace_end, hull_mins, hull_maxs, &trace_filters::melee_aim, &trace, MASK_SOLID);
				lrm->reset();
			}

			else {
				tf_utils::traceHull(trace_start, trace_end, hull_mins, hull_maxs, &trace_filters::melee_aim, &trace, MASK_SOLID);
			}
		}

		if (trace.m_pEnt == target.m_ent) {
			cmd->buttons |= IN_ATTACK;
		}
	}

	else {
		cmd->buttons |= IN_ATTACK;
	}
}

void AimbotMelee::setAngles(CUserCmd *const cmd, const Target &target)
{
	if (!cmd) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	const vec3 target_angle{ target.m_ang_to - (local->m_vecPunchAngle() * 2.f) };

	switch (cfg::aimbot_melee_aim_method)
	{
		case 0: {
			cmd->viewangles = target_angle;
			i::engine->SetViewAngles(cmd->viewangles);
			break;
		}

		case 1: {
			aim_utils->smoothAim(cmd, target_angle, cfg::aimbot_melee_smooth);
			break;
		}

		case 2: {
			tf_utils::fixMovement(cmd, target_angle);
			cmd->viewangles = target_angle;
			break;
		}

		case 3:
		{
			if (aim_utils->isAttacking(cmd, i::global_vars->curtime) || tick_base->canDoubleTap(cmd, false))
			{
				tf_utils::fixMovement(cmd, target_angle);

				cmd->viewangles = target_angle;

				if (!tick_base->canDoubleTap(cmd, false)) {
					tf_globals::send_packet = false;
				}
			}

			break;
		}

		default: {
			break;
		}
	}
}

void AimbotMelee::run(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon)
{
	if (!cfg::aimbot_melee_active || !cmd || !local || !weapon) {
		return;
	}

	if (weapon->GetWeaponID() == TF_WEAPON_KNIFE && cfg::auto_backstab_active) {
		return;
	}

	m_local_positions.clear();

	Target target{};

	if (!getTarget(local, weapon, target)) {
		return;
	}

	if (cfg::aimbot_melee_walk_to_target) {
		tf_utils::walkTo(cmd, local->m_vecOrigin(), target.m_pos, 1.0f);
	}

	setAttack(cmd, local, weapon, target);
	setAngles(cmd, target);

	aim_utils->setTickCount(cmd, target.m_ent, target.m_lag_record);

	tf_globals::aimbot_target = target.m_ent ? target.m_ent->GetRefEHandle() : EntityHandle_t{};

	if (target.m_ent
		&& target.m_ent->GetClassId() == class_ids::CTFPlayer
		&& target.m_lag_record
		&& aim_utils->isAttacking(cmd, i::global_vars->curtime)) {
		hitboxes->add(target.m_ent->cast<C_TFPlayer>(), target.m_lag_record->bones);
	}
}