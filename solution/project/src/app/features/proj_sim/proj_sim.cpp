#include "proj_sim.hpp"

#include "../aimbot/aimbot_projectile/aimbot_projectile.hpp"
#include "../vis_utils/vis_utils.hpp"
#include "../trace_utils/trace_utils.hpp"
#include "../cfg.hpp"

bool ProjSim::getProjInfo(C_TFPlayer *const pl, C_TFWeaponBase *const wep, const vec3 &angs, ProjectileInfo &out, const bool no_fire_setup)
{
	if (!pl || !wep) {
		return false;
	}

	const float cur_time{ static_cast<float>(pl->m_nTickBase()) * i::global_vars->interval_per_tick };
	const bool ducking{ !!(pl->m_fFlags() & FL_DUCKING) };

	vec3 pos{};
	vec3 ang{};

	switch (wep->GetWeaponID())
	{
		case TF_WEAPON_GRENADELAUNCHER:
		{
			projFireSetup(pl, { 16.0f, 8.0f, -6.0f }, angs, pos, ang, true, no_fire_setup);

			const bool is_lochnload{ wep->m_iItemDefinitionIndex() == Demoman_m_TheLochnLoad };
			const float speed{ tf_utils::attribHookValue(1200.0f, "mult_projectile_speed", wep) };

			out = { TF_PROJECTILE_PIPEBOMB, pos, ang, speed, 1.0f, is_lochnload };

			return true;
		}
			
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		{
			projFireSetup(pl, { 16.0f, 8.0f, -6.0f }, angs, pos, ang, true, no_fire_setup);

			const float charge_begin_time{ wep->cast<C_TFPipebombLauncher>()->m_flChargeBeginTime() };
			const float charge{ cur_time - charge_begin_time };
			const float speed{ math::remap(charge, 0.0f, tf_utils::attribHookValue(4.0f, "stickybomb_charge_rate", wep), 900.0f, 2400.0f) };

			out = { TF_PROJECTILE_PIPEBOMB_REMOTE, pos, ang, charge_begin_time <= 0.0f ? 900.0f : speed, 1.0f, false };

			return true;
		}

		case TF_WEAPON_CANNON: {
			projFireSetup(pl, { 16.0f, 8.0f, -6.0f }, angs, pos, ang, true, no_fire_setup);
			out = { TF_PROJECTILE_CANNONBALL, pos, ang, 1454.0f, 1.0f, false };
			return true;
		}

		case TF_WEAPON_FLAREGUN: {
			projFireSetup(pl, { 23.5f, 12.0f, ducking ? 8.0f : -3.0f }, angs, pos, ang, false, no_fire_setup);
			out = { TF_PROJECTILE_FLARE, pos, ang, 2000.0f, 0.3f, true };
			return true;
		}

		case TF_WEAPON_FLAREGUN_REVENGE: {
			projFireSetup(pl, { 23.5f, 12.0f, ducking ? 8.0f : -3.0f }, angs, pos, ang, false, no_fire_setup);
			out = { TF_PROJECTILE_FLARE, pos, ang, 3000.0f, 0.45f, true };
			return true;
		}

		case TF_WEAPON_COMPOUND_BOW:
		{
			projFireSetup(pl, { 23.5f, 8.0f, -3.0f }, angs, pos, ang, false, no_fire_setup);

			const float charge_begin_time{ wep->cast<C_TFPipebombLauncher>()->m_flChargeBeginTime() };
			const float charge{ cur_time - charge_begin_time };
			const float speed{ math::remap(charge, 0.0f, 1.0f, 1800.0f, 2600.0f) };
			const float grav_mod{ math::remap(charge, 0.0f, 1.0f, 0.5f, 0.1f) };

			out = { TF_PROJECTILE_ARROW, pos, ang, charge_begin_time <= 0.0f ? 1800.0f : speed, charge_begin_time <= 0.0f ? 0.5f : grav_mod, true };

			return true;
		}

		case TF_WEAPON_CROSSBOW:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE: {
			projFireSetup(pl, { 23.5f, 8.0f, -3.0f }, angs, pos, ang, false, no_fire_setup);
			out = { TF_PROJECTILE_ARROW, pos, ang, 2400.0f, 0.2f, true };
			return true;
		}

		case TF_WEAPON_SYRINGEGUN_MEDIC: {
			projFireSetup(pl, { 16.0f, 6.0f, -8.0f }, angs, pos, ang, false, no_fire_setup);
			out = { TF_PROJECTILE_SYRINGE, pos, ang, 1000.0f, 0.3f, true };
			return true;
		}

		default: {
			return false;
		}
	}
}

void ProjSim::projFireSetup(C_TFPlayer *const pl, vec3 offset, const vec3 &ang_in, vec3 &pos_out, vec3 &ang_out, bool pipes, const bool no_fire_setup)
{
	if (no_fire_setup) {
		offset.y = 0.0f;
	}

	if (cvars::cl_flipviewmodels->GetBool()) {
		offset.y *= -1.0f;
	}

	vec3 forward{};
	vec3 right{};
	vec3 up{};

	math::angleVectors(ang_in, &forward, &right, &up);

	const vec3 shoot_pos{ pl->GetEyePos() };

	pos_out = shoot_pos + (forward * offset.x) + (right * offset.y) + (up * offset.z);

	if (pipes) {
		ang_out = ang_in;
	}

	else {
		math::vectorAngles(shoot_pos + (forward * 2000.0f) - pos_out, ang_out);
	}
}

bool ProjSim::init(C_TFPlayer *const pl, C_TFWeaponBase *const wep, const vec3 &angs, const bool no_fire_setup)
{
	ProjectileInfo proj_info{};

	if (!getProjInfo(pl, wep, angs, proj_info, no_fire_setup)) {
		return false;
	}

	if (!m_phys_env) {
		m_phys_env = i::physics->CreateEnvironment();
	}

	if (!m_phys_obj)
	{
		CPhysCollide *const col{ i::physics_col->BBoxToCollide({ -2.0f, -2.0f, -2.0f }, { 2.0f, 2.0f, 2.0f }) };

		objectparams_t params{ g_PhysDefaultObjectParams };
		{
			params.damping = 0.0f;
			params.rotdamping = 0.0f;
			params.inertia = 0.0f;
			params.rotInertiaLimit = 0.0f;
			params.enableCollisions = false;
		}
		
		m_phys_obj = m_phys_env->CreatePolyObject(col, 0, proj_info.m_pos, proj_info.m_ang, &params);
		m_phys_obj->Wake();
	}

	//pos & vel
	{
		vec3 forward{};
		vec3 up{};

		math::angleVectors(proj_info.m_ang, &forward, nullptr, &up);

		vec3 vel{ forward * proj_info.m_speed };
		vec3 ang_vel{};

		switch (proj_info.m_type)
		{
			case TF_PROJECTILE_PIPEBOMB:
			case TF_PROJECTILE_PIPEBOMB_REMOTE:
			case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			case TF_PROJECTILE_CANNONBALL:
			{
				vel += up * 200.0f;
				ang_vel = { 600.0f, -1200.0f, 0.0f };

				break;
			}

			default: {
				break;
			}
		}

		if (proj_info.m_no_spin) {
			ang_vel.set();
		}

		m_phys_obj->SetPosition(proj_info.m_pos, proj_info.m_ang, true);
		m_phys_obj->SetVelocity(&vel, &ang_vel);
	}

	//drag
	{
		float drag{};

		vec3 drag_basis{};
		vec3 ang_drag_basis{};

		switch (proj_info.m_type)
		{
			case TF_PROJECTILE_PIPEBOMB:
			{
				drag = 1.0f;
				drag_basis = { 0.003902f, 0.009962f, 0.009962f };
				ang_drag_basis = { 0.003618f, 0.001514f, 0.001514f };

				break;
			}

			case TF_PROJECTILE_PIPEBOMB_REMOTE:
			case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			{
				drag = 1.0f;
				drag_basis = { 0.007491f, 0.007491f, 0.007306f };
				ang_drag_basis = { 0.002777f, 0.002842f, 0.002812f };

				break;
			}

			case TF_PROJECTILE_CANNONBALL:
			{
				drag = 1.0f;
				drag_basis = { 0.020971f, 0.019420f, 0.020971f };
				ang_drag_basis = { 0.012997f, 0.013496f, 0.013714f };

				break;
			}

			default: {
				break;
			}
		}

		m_phys_obj->SetDragCoefficient(&drag, &drag);
		m_phys_obj->m_dragBasis = drag_basis;
		m_phys_obj->m_angDragBasis = ang_drag_basis;
	}

	//env
	{
		float max_vel{ 1000000.0f };
		float max_ang_vel{ 1000000.0f };

		switch (proj_info.m_type)
		{
			case TF_PROJECTILE_PIPEBOMB:
			case TF_PROJECTILE_PIPEBOMB_REMOTE:
			case TF_PROJECTILE_PIPEBOMB_PRACTICE:
			case TF_PROJECTILE_CANNONBALL:
			{
				max_vel = k_flMaxVelocity;
				max_ang_vel = k_flMaxAngularVelocity;

				break;
			}

			default: {
				break;
			}
		}

		physics_performanceparams_t params{};

		params.Defaults();

		params.maxVelocity = max_vel;
		params.maxAngularVelocity = max_ang_vel;

		m_phys_env->SetPerformanceSettings(&params);
		m_phys_env->SetAirDensity(2.0f);
		m_phys_env->SetGravity({ 0.0f, 0.0f, -(800.0f * proj_info.m_gravity_mod) });
		m_phys_env->ResetSimulationClock();
	}

	return true;
}

void ProjSim::tick()
{
	if (!m_phys_env) {
		return;
	}

	m_phys_env->Simulate(i::global_vars->interval_per_tick);
}

vec3 ProjSim::origin()
{
	if (!m_phys_obj) {
		return {};
	}

	vec3 out{};

	m_phys_obj->GetPosition(&out, nullptr);

	return out;
}

void ProjSim::render()
{
	if (!cfg::proj_sim_visual_active || !vis_utils->shouldRunVisuals()) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || !local->InFirstPersonView()) {
		return;
	}

	C_TFWeaponBase *const wep{ ec->getWeapon() };

	if (!wep) {
		return;
	}

	const vec3 og_origin{ local->m_vecOrigin() };

	local->m_vecOrigin() = local->GetRenderOrigin();

	AimbotProjectile::ProjectileInfo data{};

	if (!aimbot_projectile->getProjInfo(local, wep, data) || !init(local, wep, i::engine->GetViewAngles())) {
		local->m_vecOrigin() = og_origin;
		return;
	}

	local->m_vecOrigin() = og_origin;

	const float max_time{ data.m_life_time != 0 ? data.m_life_time : 10.f};

	Color col{ cfg::proj_sim_visual_color };

	if (cfg::proj_sim_visual_mode == 1) {
		col = utils::rainbow(i::global_vars->curtime * cfg::proj_sim_visual_rainbow_speed);
	}

	for (int n{}; n < tf_utils::timeToTicks(max_time); n++)
	{
		const vec3 start{ origin() };
		tick();
		const vec3 end{ origin() };

		trace_t trace{};

		tf_utils::traceHull(start, end, { -2.0f, -2.0f, -2.0f }, { 2.0f, 2.0f, 2.0f }, &trace_filters::world, &trace, MASK_SOLID);

		render_utils::RenderLine(start, trace.endpos, col, false);

		if (trace.fraction < 1.0f || trace.allsolid || trace.startsolid)
		{
			vec3 angles{};
			math::vectorAngles(trace.plane.normal, angles);

			render_utils::RenderWireframeBox(trace.endpos, angles, { -1.0f, -10.0f, -10.0f }, { 1.0f, 10.0f, 10.0f }, col, false);
			col.a = 50;
			render_utils::RenderBox(trace.endpos, angles, { -1.0f, -10.0f, -10.0f }, { 1.0f, 10.0f, 10.0f }, col, false);

			break;
		}
	}

	local->m_vecOrigin() = og_origin;
}