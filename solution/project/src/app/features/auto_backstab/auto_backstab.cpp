#include "auto_backstab.hpp"

#include "../aim_utils/aim_utils.hpp"
#include "../engine_pred/engine_pred.hpp"
#include "../trace_utils/trace_utils.hpp"
#include "../player_data/player_data.hpp"
#include "../hitboxes/hitboxes.hpp"
#include "../cfg.hpp"

bool AutoBackstab::canSex(const vec3 &player_center, const vec3 &target_center, const vec3 &player_eye_angs, const vec3 &target_eye_angs, const int health, const int dmg)
{
	if (cfg::auto_backstab_lethal_active && health <= dmg)
	{
		return true;
	}

	vec3 player_forward{};
	math::angleVectors(player_eye_angs, &player_forward);

	player_forward.z = 0.0f;
	player_forward.normalize();

	vec3 target_forward{};
	math::angleVectors(target_eye_angs, &target_forward);

	target_forward.z = 0.0f;
	target_forward.normalize();

	vec3 to_target{ target_center - player_center };

	to_target.z = 0.0f;
	to_target.normalize();

	return to_target.dot(target_forward) > 0.0f
		&& to_target.dot(player_forward) > 0.5f
		&& target_forward.dot(player_forward) > -0.3f;
}

bool AutoBackstab::canSee(C_TFPlayer *const local, C_TFWeaponBase *const weapon, C_TFPlayer *const target, const LagRecord *const lr, const vec3 ang_to_target)
{
	if (!local || !weapon || !target || !lr) {
		return false;
	}

	float swing_range{};

	vec3 hull_mins{};
	vec3 hull_maxs{};

	if (!aim_utils->getMeleeInfo(swing_range, hull_mins, hull_maxs)) {
		return false;
	}

	vec3 forward{};
	math::angleVectors(ang_to_target, &forward);

	const vec3 trace_start{ local->GetEyePos() };
	const vec3 trace_end{ trace_start + (forward * swing_range) };

	trace_filters::melee_aim.m_ignore_ent = local;

	trace_t trace{};

	lrm->set(target, lr);
	tf_utils::traceHull(trace_start, trace_end, hull_mins, hull_maxs, &trace_filters::melee_aim, &trace, MASK_SOLID);
	lrm->reset();

	return trace.m_pEnt == target && !trace.startsolid && !trace.allsolid;
}

void AutoBackstab::setAngles(CUserCmd *const cmd, const vec3 &ang, const bool no_packet)
{
	if (cfg::auto_backstab_legit_mode) {
		return;
	}

	if (cfg::auto_backstab_aim_method == 0) {
		cmd->viewangles = ang;
		i::engine->SetViewAngles(cmd->viewangles);
	}

	if (cfg::auto_backstab_aim_method == 1) {
		tf_utils::fixMovement(cmd, ang);
		cmd->viewangles = ang;
	}

	if (cfg::auto_backstab_aim_method == 2)
	{
		tf_utils::fixMovement(cmd, ang);
		cmd->viewangles = ang;

		if (!no_packet) {
			tf_globals::send_packet = false;
		}
	}
}

void AutoBackstab::run(CUserCmd *const cmd)
{
	if (!cfg::auto_backstab_active || !cmd) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local
		|| local->deadflag()
		|| local->m_iClass() != TF_CLASS_SPY
		|| i::global_vars->curtime < local->m_flNextAttack()) {
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon
		|| weapon->GetWeaponID() != TF_WEAPON_KNIFE
		|| i::global_vars->curtime < weapon->m_flNextPrimaryAttack()) {
		return;
	}

	CTFWeaponInfo *const info{ weapon->m_pWeaponInfo() };
	if (!info) {
		return;
	}

	const WeaponData_t *const data{ &info->GetWeaponData(weapon->m_iWeaponMode()) };
	if (!data) {
		return;
	}

	for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PLAYERS_ENEMIES))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

		if (!pl
			|| pl->deadflag()
			|| (cfg::auto_backstab_ignore_friends && pl->IsPlayerOnSteamFriendsList())
			|| (cfg::auto_backstab_ignore_invisible && pl->IsInvisible())
			|| (cfg::auto_backstab_ignore_invulnerable && pl->IsInvulnerable())) {
			continue;
		}

		const PlayerDataVars *const pd{ player_data->get(pl) };

		if (!pd) {
			continue;
		}

		const size_t num_records{ player_data->getNumValidRecords(pl) };

		if (num_records <= 0) {
			continue;
		}

		for (size_t n{}; n < num_records; n++)
		{
			const LagRecord *const lr{ player_data->getRecord(pl, n) };

			if (!lr || !lr->isRecordValid()) {
				continue;
			}

			if (lr->interpolated && !cfg::aimbot_fakelag_fix) {
				continue;
			}

			vec3 ang_to{ cmd->viewangles };

			if (!cfg::auto_backstab_legit_mode) {
				ang_to = math::calcAngle(local->GetEyePos(), lr->center);
			}

			if (!canSex(local->GetCenter(), lr->center, ang_to, pd->eye_angles, lr->pl_ptr->m_iHealth(), data->m_nDamage)
				|| !canSee(local, weapon, pl, lr, ang_to)) {
				continue;
			}

			{
				//fix for back speed clamp potentially causing prediction errors
				const CUserCmd backup_cmd{ *cmd };
				vec3 backup_angles{ i::engine->GetViewAngles() };
				bool can_sex_and_see{};

				//restore back to our last predicted command
				engine_pred->end();

				//set the cmd viewangles to our target ones
				setAngles(cmd, ang_to, true);

				//predict the cmd again and check if we hit the target
				RUN_PREDICTION(cmd, [&]() {
					can_sex_and_see = canSex(local->GetCenter(), lr->center, ang_to, pd->eye_angles, lr->pl_ptr->m_iHealth(), data->m_nDamage) && canSee(local, weapon, pl, lr, ang_to);
				})

				//restore all data modified by this
				i::engine->SetViewAngles(backup_angles);
				*cmd = backup_cmd;
				if (!engine_pred->start(cmd)) {
					return;
				}

				if (!can_sex_and_see) {
					continue;
				}
			}

			cmd->buttons |= IN_ATTACK;

			setAngles(cmd, ang_to);

			aim_utils->setTickCount(cmd, pl, lr);

			if (pl && lr && aim_utils->isAttacking(cmd, i::global_vars->curtime)) {
				hitboxes->add(pl, lr->bones);
			}

			tf_globals::aimbot_target = pl->GetRefEHandle();

			return;
		}
	}
}