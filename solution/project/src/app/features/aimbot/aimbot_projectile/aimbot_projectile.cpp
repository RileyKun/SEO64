#include "aimbot_projectile.hpp"

#include "../aimbot_snap/aimbot_snap.hpp"

#include "../../aim_utils/aim_utils.hpp"
#include "../../game_movement/move_pred.hpp"
#include "../../proj_sim/proj_sim.hpp"
#include "../../trace_utils/trace_utils.hpp"
#include "../../player_list/player_list.hpp"
#include "../../engine_pred/engine_pred.hpp"
#include "../../player_data/player_data.hpp"
#include "../../cfg.hpp"

//s: origin + vec3{ 0.0f, 0.0f, SPLASH_Z_OFFSET } passed to getPointsAroundPos()
//and then sorted by pos - vec3{ 0.0f, 0.0f, SPLASH_Z_OFFSET }
static constexpr float SPLASH_Z_OFFSET{ 50.0f };

bool AimbotProjectile::getProjInfo(C_TFPlayer *const local, C_TFWeaponBase *const weapon, ProjectileInfo &out)
{
	if (!local || !weapon) {
		return false;
	}

	const bool ducking{ !!(local->m_fFlags() & FL_DUCKING) };

	switch (weapon->GetWeaponID())
	{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_PARTICLE_CANNON:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		{
			out.m_speed = tf_utils::attribHookValue(1100.0f, "mult_projectile_speed", weapon);
			out.m_splash_radius = (weapon->m_iItemDefinitionIndex() == Soldier_m_TheAirStrike) ? 130.0f : 170.0f;
			out.m_offset = { 23.5f, tf_utils::attribHookValue(0, "centerfire_projectile", weapon) ? 0.0f : 12.0f, ducking ? 8.0f : -3.0f };

			if (const int rocket_specialist{ static_cast<int>(tf_utils::attribHookValue(0.0f, "rocket_specialist", local)) }) {
				out.m_speed *= math::remap(static_cast<float>(rocket_specialist), 1.0f, 4.0f, 1.15f, 1.6f);
				out.m_speed = std::min(out.m_speed, 3000.0f);
			}

			if (weapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT) {
				out.m_splash_radius = 70.0f;
			}

			return true;
		}

		case TF_WEAPON_GRENADELAUNCHER:
		{
			out.m_speed = tf_utils::attribHookValue(1200.0f, "mult_projectile_speed", weapon);
			out.m_gravity_mod = 1.0f;
			out.m_drag_mod = (weapon->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad) ? 0.07f : 0.11f;
			out.m_life_time = (weapon->m_iItemDefinitionIndex() == Demoman_m_TheIronBomber) ? 1.4f : 2.0f;
			out.m_offset = { 16.0f, 8.0f, -6.0f };

			if (out.m_speed > k_flMaxVelocity) {
				out.m_speed = k_flMaxVelocity;
			}

			return true;
		}

		case TF_WEAPON_PIPEBOMBLAUNCHER:
		{
			out.m_speed = 1000.0f;
			out.m_gravity_mod = 1.0f;
			out.m_drag_mod = 0.16f;
			out.m_splash_radius = 170.0f;
			out.m_offset = { 16.0f, 8.0f, -6.0f };

			if (const float charge_begin_time{ weapon->cast<C_TFPipebombLauncher>()->m_flChargeBeginTime() })
			{
				out.m_speed = math::remap
				(
					i::global_vars->curtime - charge_begin_time,
					0.0f,
					tf_utils::attribHookValue(4.0f, "stickybomb_charge_rate", weapon),
					1000.0f,
					2400.0f
				);
			}

			if (out.m_speed > k_flMaxVelocity) {
				out.m_speed = k_flMaxVelocity;
			}

			return true;
		}

		case TF_WEAPON_CANNON:
		{
			out.m_speed = 1100.0f;
			out.m_gravity_mod = 1.0f;
			out.m_drag_mod = 0.05f;
			out.m_life_time = 0.95f;
			out.m_offset = { 16.0f, 8.0f, -6.0f };

			if (out.m_speed > k_flMaxVelocity) {
				out.m_speed = k_flMaxVelocity;
			}

			return true;
		}

		case TF_WEAPON_COMPOUND_BOW:
		{
			out.m_speed = 1800.0f;
			out.m_gravity_mod = 0.5f;
			out.m_offset = { 23.5f, -8.0f, -3.0f };

			if (const float charge_begin_time{ weapon->cast<C_TFPipebombLauncher>()->m_flChargeBeginTime() }) {
				out.m_speed = 1800.0f + std::clamp<float>(i::global_vars->curtime - charge_begin_time, 0.0f, 1.0f) * 800.0f;
				out.m_gravity_mod = math::remap(i::global_vars->curtime - charge_begin_time, 0.0f, 1.0f, 0.5f, 0.1f);
			}

			return true;
		}

		case TF_WEAPON_CROSSBOW:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
		{
			out.m_speed = 2400.0f;
			out.m_gravity_mod = 0.2f;
			out.m_offset = { 23.5f, 8.0f, -3.0f };

			return true;
		}

		case TF_WEAPON_SYRINGEGUN_MEDIC:
		{
			out.m_speed = 1000.0f;
			out.m_gravity_mod = 0.3f;
			out.m_offset = { 16.0f, 6.0f, -8.0f };

			return true;
		}

		case TF_WEAPON_FLAREGUN:
		{
			out.m_speed = 2000.0f;
			out.m_gravity_mod = 0.3f;
			out.m_offset = { 23.5f, 12.0f, ducking ? 8.0f : -3.0f };
			
			if (weapon->m_iItemDefinitionIndex() == Pyro_s_TheScorchShot) {
				out.m_splash_radius = 150.0f;
			}

			return true;
		}

		case TF_WEAPON_FLAREGUN_REVENGE:
		{
			out.m_speed = 3000.0f;
			out.m_gravity_mod = 0.45f;
			out.m_offset = { 23.5f, 12.0f, ducking ? 8.0f : -3.0f };

			return true;
		}

		case TF_WEAPON_FLAME_BALL:
		{
			out.m_speed = 3000.0f;
			out.m_gravity_mod = 0.0f;
			out.m_life_time = 0.18f;
			out.m_offset = { 23.5f, 12.0f, ducking ? 8.0f : -3.0f };

			return true;
		}

		case TF_WEAPON_FLAMETHROWER:
		{
			out.m_speed = 2000.0f;
			out.m_gravity_mod = 0.0f;
			out.m_life_time = 0.18f;
			out.m_offset = { 23.5f, 12.0f, ducking ? 8.0f : -3.0f };

			return true;
		}

		case TF_WEAPON_RAYGUN:
		case TF_WEAPON_DRG_POMSON:
		{
			out.m_speed = 1200.0f;
			out.m_gravity_mod = 0.0f;
			out.m_offset = { 23.5f, 12.0f, ducking ? 8.0f : -3.0f };

			return true;
		}

		default: {
			out = {};
			return false;
		}
	}
}

bool AimbotProjectile::calcProjAngle(const vec3 &from, const vec3 &to, const ProjectileInfo &proj_info, vec3 &ang_out, float &t_out)
{
	if (proj_info.m_gravity_mod)
	{
		const vec3 v{ to - from };
		const float dx{ sqrtf(v.x * v.x + v.y * v.y) };
		const float dy{ v.z };
		const float v0{ proj_info.m_speed };
		const float g{ tf_utils::getGravity() * proj_info.m_gravity_mod };

		const float root{ v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0) };

		if (root < 0.0f) {
			return false;
		}

		ang_out = { -math::radToDeg(atanf((v0 * v0 - sqrtf(root)) / (g * dx))), math::radToDeg(atan2f(v.y, v.x)), 0.0f };
		t_out = dx / (cosf(-math::degToRad(ang_out.x)) * v0);

		//TODO: use dist instead of time to avoid this mess (?)
		if (proj_info.m_drag_mod)
		{
			const float v0{ proj_info.m_speed - ((proj_info.m_speed * t_out) * proj_info.m_drag_mod) };
			const float root{ v0 * v0 * v0 * v0 - g * (g * dx * dx + 2.0f * dy * v0 * v0) };

			if (root < 0.0f) {
				return false;
			}

			ang_out = { -math::radToDeg(atanf((v0 * v0 - sqrtf(root)) / (g * dx))), math::radToDeg(atan2f(v.y, v.x)), 0.0f };
			t_out = dx / (cosf(-math::degToRad(ang_out.x)) * v0);

			vec3 forward{};
			vec3 up{};

			math::angleVectors(ang_out, &forward, nullptr, &up);
			math::vectorAngles((forward * proj_info.m_speed) - (up * 200.0f), ang_out);
		}
	}

	else {
		ang_out = math::calcAngle(from, to);
		t_out = from.distTo(to) / proj_info.m_speed;
	}

	if (proj_info.m_life_time > 0.0f && t_out > proj_info.m_life_time) {
		return false;
	}

	return true;
}

AimbotProjectile::BBoxPoint AimbotProjectile::getBBoxPoint(C_TFWeaponBase *const weapon, C_TFPlayer *const pl)
{
	if (!weapon || !pl) {
		return BBOX_BODY;
	}

	BBoxPoint out{ static_cast<BBoxPoint>(std::max(0, cfg::aimbot_projectile_aim_pos - 1)) };

	if (cfg::aimbot_projectile_aim_pos == 0)
	{
		switch (weapon->GetWeaponID())
		{
			case TF_WEAPON_ROCKETLAUNCHER:
			case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
			case TF_WEAPON_GRENADELAUNCHER:
			case TF_WEAPON_PIPEBOMBLAUNCHER: {
				out = (pl->m_fFlags() & FL_ONGROUND) ? BBOX_FEET : BBOX_BODY;
				break;
			}

			case TF_WEAPON_COMPOUND_BOW: {
				out = BBOX_HEAD;
				break;
			}

			default: {
				out = BBOX_BODY;
				break;
			}
		}
	}

	return out;
}

bool AimbotProjectile::getBBoxOffset(C_TFPlayer *const target_pl, const BBoxPoint point, const bool dynamic_head, vec3 &offset_out)
{
	if (!target_pl) {
		return false;
	}

	const LagRecord *const lr{ player_data->getRecord(target_pl, 0) };

	if (!lr || !lr->hasSetupBones()) {
		return false;
	}

	const float maxs_z{ target_pl->m_vecMaxs().z * target_pl->m_flModelScale() };

	switch (point) 
	{
		case BBOX_FEET:
		{
			offset_out = { 0.0f, 0.0f, maxs_z * 0.1f };
			return true;
		}

		case BBOX_BODY:
		{
			offset_out = { 0.0f, 0.0f, maxs_z * 0.5f };
			return true;
		}

		case BBOX_HEAD:
		{
			if (dynamic_head)
			{
				vec3 head{};

				if (!tf_utils::getHitbox(target_pl, lr->bones, HITBOX_HEAD, head)) {
					return false;
				}

				const vec3 delta{ head - target_pl->m_vecOrigin() };

				offset_out = { delta.x, delta.y, maxs_z * 0.93f };
			}

			else
			{
				offset_out = { 0.0f, 0.0f, maxs_z * 0.93f };
			}

			return true;
		}

		default: return false;
	}

	return false;
}

std::vector<vec3> AimbotProjectile::getPointsAroundPos(const vec3 &pos, const float radius)
{
	//i::debug_overlay->ClearAllOverlays();

	std::vector<vec3> points_out{};

	const size_t num_points{ 50 };

	for (size_t n{}; n < num_points; n++)
	{
		const float a1{ acosf(1.0f - 2.0f * (static_cast<float>(n) / static_cast<float>(num_points))) };
		const float a2{ (static_cast<float>(math::pi()) * (3.0f - sqrtf(5.0f))) * static_cast<float>(n) };

		const vec3 point{ pos + vec3{ sinf(a1) * cosf(a2), sinf(a1) * sinf(a2), cosf(a1) }.scale(radius) };

		trace_t trace{};

		tf_utils::trace(pos, point, &trace_filters::world, &trace, MASK_SOLID);

		if (trace.fraction >= 1.0f) {
			continue;
		}

		points_out.push_back(trace.endpos);
	}

	std::sort(points_out.begin(), points_out.end(), [&](const vec3 &a, const vec3 &b) {
		return a.distTo(pos - vec3{ 0.0f, 0.0f, SPLASH_Z_OFFSET }) < b.distTo(pos - vec3{ 0.0f, 0.0f, SPLASH_Z_OFFSET });
	});

	/*for (const vec3 &p : points_out) {
		const Color clr{ cfg::color_indicators_extra_good.lerp(cfg::color_indicators_mid, math::remap(p.distTo(pos), 0.0f, radius, 0.0f, 1.0f)) };
		i::debug_overlay->AddBoxOverlay(p, { -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }, {}, clr.r, clr.g, clr.b, 255, 10.0f);
	}*/

	return points_out;
}

bool AimbotProjectile::canSee(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const ProjectileInfo &proj_info, Target &target, const vec3 &target_pos)
{
	if (!cmd || !local || !weapon) {
		return false;
	}

	auto testVis = [&]()
	{
		trace_t trace{};

		if (proj_info.m_gravity_mod)
		{
			for (int n{}; n < 2; n++)
			{
				vec3 local_pos{ local->GetEyePos() };

				if (!proj_sim->init(local, weapon, target.m_ang_to, n != 0)) {
					return false;
				}

				const int ticks{ tf_utils::timeToTicks(target.m_time_to) };
				const int steps{ 8 };
				const int step_size{ std::max(ticks / steps, 1) };

				for (int n{}; n < ticks; n += step_size)
				{
					const vec3 pre{ proj_sim->origin() };
					
					for (int m{}; m < step_size; m++) {
						proj_sim->tick();
					}

					const vec3 post{ proj_sim->origin() };

					tf_utils::traceHull(pre, post, { -4.0f, -4.0f, -4.0f }, { 4.0f, 4.0f, 12.0f }, &trace_filters::world, &trace, MASK_SOLID);

					if (trace.endpos.distTo(local_pos) > local_pos.distTo(target_pos)) {
						break;
					}

					if ((trace.fraction < 1.0f || trace.startsolid)) {
						return false;
					}

					if (n + step_size >= ticks) {
						n = ticks - 1;
					}
				}

				tf_utils::trace(trace.endpos, target_pos, &trace_filters::world, &trace, MASK_SOLID);

				if ((trace.fraction < 1.0f || trace.startsolid)) {
					return false;
				}
			}

			return true;
		}

		else
		{
			for (int n{}; n < 2; n++)
			{
				vec3 trace_start{ local->GetEyePos() };
				
				if (n == 0) {
					tf_utils::getProjectileFireSetup(target.m_ang_to, proj_info.m_offset, trace_start);
				}

				tf_utils::trace(trace_start, target_pos, &trace_filters::world, &trace, MASK_SOLID | CONTENTS_GRATE);

				if (trace.fraction < 1.0f || trace.startsolid) {
					return false;
				}
			}

			return true;
		}

		return false;
	};

	bool can_see{ testVis() };

	if (can_see)
	{
		const CUserCmd backup_cmd{ *cmd };
		vec3 backup_angles{ i::engine->GetViewAngles() };

		engine_pred->end();

		const bool og_send_packet{ tf_globals::send_packet };
		setAngles(cmd, weapon, proj_info, target);
		tf_globals::send_packet = og_send_packet;

		RUN_PREDICTION(cmd, [&]() {
			can_see = testVis();
		})

		i::engine->SetViewAngles(backup_angles);
		*cmd = backup_cmd;
		if (!engine_pred->start(cmd)) {
			return false;
		}
	}

	return can_see;
}

bool AimbotProjectile::solveTarget(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const ProjectileInfo &proj_info, Target &target)
{
	if (!cmd || !local || !weapon || !target.m_ent) {
		return false;
	}

	vec3 local_pos{ local->GetEyePos() };

	if (proj_info.m_drag_mod) {
		tf_utils::getProjectileFireSetup(target.m_ang_to, proj_info.m_offset, local_pos);
	}

	if (target.m_ent->GetClassId() == class_ids::CTFPlayer)
	{
		C_TFPlayer *const pl{ target.m_ent->cast<C_TFPlayer>() };

		if (!pl) {
			return false;
		}
		
		const bool pl_on_ground{ !!(pl->m_fFlags() & FL_ONGROUND) };

		const BBoxPoint start_bbox_point{ getBBoxPoint(weapon, pl) };

		vec3 bbox_offset{};

		if (!getBBoxOffset(pl, start_bbox_point, weapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW, bbox_offset)) {
			return false;
		}

		MovePred pred{};

		if (!pred.init(pl)) {
			return false;
		}

		for (int n{}; n < tf_utils::timeToTicks(cfg::aimbot_projectile_max_sim_time); n++)
		{
			pred.tick();

			m_pred_path.push_back(pred.origin());

			const vec3 target_pos{ pred.origin() + bbox_offset };

			if (!calcProjAngle(local_pos, target_pos, proj_info, target.m_ang_to, target.m_time_to)) {
				continue;
			}

			int target_tick{ tf_utils::timeToTicks(target.m_time_to) };

			//account for latency
			target_tick += tf_utils::timeToTicks(tf_utils::getLatency());

			//account for sticky det time
			if (weapon->GetWeaponID() == TF_WEAPON_PIPEBOMBLAUNCHER)
			{
				const float det_time{ tf_utils::attribHookValue(0.8f, "sticky_arm_time", local) };

				if (target_tick < tf_utils::timeToTicks(det_time)) {
					target_tick += tf_utils::timeToTicks(fabsf(target.m_time_to - det_time));
				}
			}

			if (n == target_tick || n == (target_tick - 1))
			{
				auto runSplash = [&]()
				{
					if (cfg::aimbot_projectile_splash_on_ground_only && !pl_on_ground) {
						return false;
					}

					const std::vector<vec3> points{ getPointsAroundPos(pred.origin() + vec3{ 0.0f, 0.0f, SPLASH_Z_OFFSET }, proj_info.m_splash_radius) };

					for (const vec3 &point : points)
					{
						if (!calcProjAngle(local_pos, point, proj_info, target.m_ang_to, target.m_time_to)) {
							continue;
						}

						if (!canSee(cmd, local, weapon, proj_info, target, point)) {
							continue;
						}

						return true;
					}

					return false;
				};

				if (cfg::aimbot_projectile_splash_multipoint == 2 && proj_info.m_splash_radius)
				{
					const Target target_backup{ target };

					if (runSplash()) {
						return true;
					}

					target = target_backup;
				}

				if (canSee(cmd, local, weapon, proj_info, target, target_pos)) {
					return true;
				}

				else
				{
					if (cfg::aimbot_projectile_bbox_multipoint && weapon->GetWeaponID() != TF_WEAPON_COMPOUND_BOW)
					{
						for (int n{}; n < static_cast<int>(BBOX_POINT_COUNT); n++)
						{
							const BBoxPoint bbox_point{ static_cast<BBoxPoint>(n) };

							if (n == start_bbox_point) {
								continue;
							}

							vec3 bbox_offset{};

							if (!getBBoxOffset(pl, bbox_point, false, bbox_offset)) {
								continue;
							}

							const vec3 target_pos{ pred.origin() + bbox_offset };

							if (!calcProjAngle(local_pos, target_pos, proj_info, target.m_ang_to, target.m_time_to)) {
								continue;
							}

							if (!canSee(cmd, local, weapon, proj_info, target, target_pos)) {
								continue;
							}

							return true;
						}
					}

					if (cfg::aimbot_projectile_splash_multipoint == 1 && proj_info.m_splash_radius)
					{
						if (runSplash()) {
							return true;
						}
					}
				}
			}
		}
	}

	else
	{
		if (!calcProjAngle(local_pos, target.m_init_pos, proj_info, target.m_ang_to, target.m_time_to)) {
			return false;
		}

		auto runSplash = [&]()
		{
			C_BaseAnimating *const animating{ target.m_ent->cast<C_BaseAnimating>() };

			if (!animating) {
				return false;
			}

			const std::vector<vec3> points{ getPointsAroundPos(animating->m_vecOrigin() + vec3{ 0.0f, 0.0f, SPLASH_Z_OFFSET }, proj_info.m_splash_radius) };

			for (const vec3 &point : points)
			{
				if (!calcProjAngle(local_pos, point, proj_info, target.m_ang_to, target.m_time_to)) {
					continue;
				}

				if (!canSee(cmd, local, weapon, proj_info, target, point)) {
					continue;
				}

				return true;
			}

			return false;
		};

		if (cfg::aimbot_projectile_splash_multipoint == 2 && proj_info.m_splash_radius)
		{
			const Target target_backup{ target };

			if (runSplash()) {
				return true;
			}

			target = target_backup;
		}

		if (canSee(cmd, local, weapon, proj_info, target, target.m_ent->m_vecOrigin())) {
			return true;
		}

		else
		{
			if (cfg::aimbot_projectile_bbox_multipoint)
			{
				C_BaseEntity *const ent{ target.m_ent->cast<C_BaseEntity>() };

				if (!ent) {
					return false;
				}

				const float point_scale{ static_cast<float>(cfg::aimbot_projectile_multipoint_scale) / 100.0f };

				const vec3 scaled_mins{ ent->m_vecMins() * point_scale };
				const vec3 scaled_maxs{ ent->m_vecMaxs() * point_scale };

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
					const vec3 point{ ent->m_vecOrigin() + offsets[n] };

					if (!calcProjAngle(local_pos, point, proj_info, target.m_ang_to, target.m_time_to)) {
						continue;
					}

					if (canSee(cmd, local, weapon, proj_info, target, point)) {
						return true;
					}
				}
			}

			if (cfg::aimbot_projectile_splash_multipoint == 1 && proj_info.m_splash_radius)
			{
				if (runSplash()) {
					return true;
				}
			}
		}
	}

	return false;
}

bool AimbotProjectile::getTarget(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const ProjectileInfo &proj_info, Target &out)
{
	if (!cmd || !local || !weapon) {
		return false;
	}

	std::vector<Target> targets{};

	const ECGroup player_group
	{
		weapon->GetWeaponID() == TF_WEAPON_CROSSBOW
		? ECGroup::PLAYERS_ALL
		: ECGroup::PLAYERS_ENEMIES
	};

	const bool friendly_fire{ player_group == ECGroup::PLAYERS_ALL };

	for (const EntityHandle_t ehandle : ec->getGroup(player_group))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };


		if (aim_utils->shouldSkipPlayer(local, pl, friendly_fire, friendly_fire)) {
			continue;
		}

		vec3 bbox_offset{};

		if (!getBBoxOffset(pl, getBBoxPoint(weapon, pl), weapon->GetWeaponID() == TF_WEAPON_COMPOUND_BOW, bbox_offset)) {
			continue;
		}

		const vec3 pl_pos{ pl->m_vecOrigin() + bbox_offset };
		const vec3 ang_to{ math::calcAngle(local->GetEyePos(), pl_pos) };
		const float fov_to{ math::calcFOV(i::engine->GetViewAngles(), ang_to) };
		const float dist_to{ local->GetEyePos().distTo(pl->m_vecOrigin()) };

		if (cfg::aimbot_projectile_mode == 0 && fov_to > cfg::aimbot_projectile_fov) {
			continue;
		}

		if (player_data->getNumRecords(pl) >= 2)
		{
			const LagRecord *const cur{ player_data->getRecord(pl, 0) };
			const LagRecord *const pre{ player_data->getRecord(pl, 1) };

			if ((cur && fabsf((pl->m_vecVelocity() - cur->vel).length()) > 500.0f)
				|| (pre && fabsf((pl->m_vecVelocity() - pre->vel).length()) > 500.0f)
				|| (cur && pre) && fabsf((cur->vel - pre->vel).length()) > 500.0f) {
				continue;
			}
		}

		targets.push_back({ pl, ang_to, pl_pos, fov_to, dist_to, 0.0f });
	}

	const ECGroup building_group
	{
		tf_utils::attribHookValue(0.0f, "arrow_heals_buildings", weapon) > 0.0f
		? ECGroup::BUILDINGS_ALL
		: ECGroup::BUILDINGS_ENEMIES
	};

	for (const EntityHandle_t ehandle : ec->getGroup(building_group))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_BaseObject *const obj{ ent->cast<C_BaseObject>() };

		if (!obj || (obj->m_iTeamNum() != local->m_iTeamNum() && aim_utils->shouldSkipBuilding(local, obj))) {
			continue;
		}

		const vec3 target_pos{ obj->GetClassId() == class_ids::CObjectTeleporter ? obj->m_vecOrigin() : obj->GetCenter() };

		const vec3 ang_to{ math::calcAngle(local->GetEyePos(), target_pos) };
		const float fov_to{ math::calcFOV(i::engine->GetViewAngles(), ang_to) };
		const float dist_to{ local->GetEyePos().distTo(target_pos) };

		if (cfg::aimbot_projectile_mode == 0 && fov_to > cfg::aimbot_projectile_fov) {
			continue;
		}

		targets.push_back({ obj, ang_to, target_pos, fov_to, dist_to, 0.0f });
	}

	if (targets.empty()) {
		return false;
	}

	std::sort(targets.begin(), targets.end(), [](const Target &a, const Target &b)
	{
		if (cfg::aimbot_projectile_mode == 0 || cfg::aimbot_projectile_mode == 1) {
			return a.m_fov_to < b.m_fov_to;
		}

		if (cfg::aimbot_projectile_mode == 2) {
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

	size_t num_solve_attempts{};

	for (size_t n{}; n < targets.size(); n++)
	{
		m_pred_path.clear();

		Target &target{ targets[n] };

		//allows really close targets to be processed
		if (num_solve_attempts >= cfg::aimbot_projectile_max_sim_targets && target.m_init_pos.distTo(local->GetEyePos()) > 400.0f) {
			continue;
		}

		if (!solveTarget(cmd, local, weapon, proj_info, target)) {
			num_solve_attempts++;
			continue;
		}

		if (cfg::aimbot_projectile_mode == 1 && math::calcFOV(i::engine->GetViewAngles(), target.m_ang_to) > cfg::aimbot_projectile_fov) {
			continue;
		}

		out = target;

		return true;
	}

	return false;
}

void AimbotProjectile::setAttack(CUserCmd *const cmd, C_TFWeaponBase *const weapon)
{
	if (!cmd || !weapon || !cfg::aimbot_auto_shoot) {
		return;
	}

	const bool can_overload{ weapon->CanOverload() };
	const bool has_ammo{ weapon->HasPrimaryAmmoForShot() };

	if (!can_overload && !has_ammo) {
		return;
	}

	cmd->buttons |= IN_ATTACK;

	switch (weapon->GetWeaponID())
	{
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		{
			if (weapon->cast<C_TFPipebombLauncher>()->m_flChargeBeginTime() > 0.0f) {
				cmd->buttons &= ~IN_ATTACK;
			}

			break;
		}

		case TF_WEAPON_CANNON:
		{
			if (weapon->cast<C_TFGrenadeLauncher>()->m_flDetonateTime() > 0.0f) {
				cmd->buttons &= ~IN_ATTACK;
			}

			break;
		}

		default:
		{
			if (can_overload && has_ammo) {
				cmd->buttons &= ~IN_ATTACK;
			}

			break;
		}
	}
}

void AimbotProjectile::setAngles(CUserCmd *const cmd, C_TFWeaponBase *const weapon, const ProjectileInfo &proj_info, const Target &target)
{
	if (!cmd || !weapon) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	const vec3 target_angle{ target.m_ang_to - (local->m_vecPunchAngle() * 2.f) };

	if (cfg::aimbot_projectile_smooth_flamethrowers_active && weapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER) {
		return aim_utils->smoothAim(cmd, target_angle, cfg::aimbot_projectile_smooth_flamethrowers);
	}

	switch (cfg::aimbot_projectile_aim_method)
	{
		case 0: {
			cmd->viewangles = target_angle;
			cmd->viewangles.normalizeAngles();
			i::engine->SetViewAngles(cmd->viewangles);
			break;
		}

		case 2: {
			tf_utils::fixMovement(cmd, target_angle);
			cmd->viewangles = target_angle;
			cmd->viewangles.normalizeAngles();
			break;
		}

		case 3:
		{
			if (aim_utils->isAttacking(cmd, i::global_vars->curtime))
			{
				tf_utils::fixMovement(cmd, target_angle);

				cmd->viewangles = target_angle;
				cmd->viewangles.normalizeAngles();

				if (weapon->GetWeaponID() != TF_WEAPON_FLAMETHROWER) {
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

void AimbotProjectile::run(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon)
{
	m_pred_path.clear();

	if (!cfg::aimbot_projectile_active || !cmd || !local || !weapon) {
		aimbot_snap->reset();
		return;
	}

	ProjectileInfo proj_info{};

	if (!getProjInfo(local, weapon, proj_info)) {
		aimbot_snap->reset();
		return;
	}

	Target target{};

	if (getTarget(cmd, local, weapon, proj_info, target) && target.m_ent)
	{
		tf_globals::aimbot_target = target.m_ent->GetRefEHandle();

		// if we are antiaiming never shoot on the first tick of choke cycle
		if (cfg::antiaim_active && cfg::antiaim_fake_type != 0 && !i::client_state->chokedcommands) {
			return;
		}

		if (cfg::aimbot_projectile_aim_method == 1)
		{
			static EntityHandle_t last_target{};

			if (target.m_ent->GetRefEHandle() != last_target) {
				aimbot_snap->reset();
			}

			last_target = target.m_ent->GetRefEHandle();

			aimbot_snap->begin(local, weapon, target.m_ang_to);
			aimbot_snap->update(target.m_ang_to);
		}

		else {
			setAttack(cmd, weapon);
			setAngles(cmd, weapon, proj_info, target);
		}
	}

	if (aimbot_snap->think(cmd, weapon)) {
		setAttack(cmd, weapon);
	}

	if (aim_utils->isAttacking(cmd, i::global_vars->curtime)) {
		tf_globals::pred_path = m_pred_path;
		tf_globals::pred_path_expire_time = i::global_vars->curtime + cfg::prediction_path_duration;
		aimbot_snap->reset();
	}
}