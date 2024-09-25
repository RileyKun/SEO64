#include "aimbot_snap.hpp"

#include "../../aim_utils/aim_utils.hpp"
#include "../../cfg.hpp"

AimbotSnapInst::AimbotSnapInst(const vec3 &start_ang, const vec3 &target_ang)
{
	m_start_ang = start_ang;
	m_target_ang = target_ang;
	m_start_time = i::global_vars->curtime;
	m_end_time = m_start_time + cfg::aimbot_projectile_snap_time;
}

float AimbotSnapInst::progress() const
{
	return math::remap(i::global_vars->curtime, m_start_time, m_end_time, 0.0f, 1.0f);
}

vec3 AimbotSnapInst::angles()
{
	vec3 delta_ang{ m_target_ang - m_start_ang };

	for (int n{}; n < 2; n++)
	{
		if (delta_ang[n] > 180.0f) {
			delta_ang[n] -= 360.0f;
		}

		else if (delta_ang[n] < -180.0f) {
			delta_ang[n] += 360.0f;
		}
	}

	const float t{ progress() };

	vec3 lerp_angs{ m_start_ang + delta_ang * t };

	math::clampAngles(lerp_angs);

	if (cfg::aimbot_projectile_snap_smooth)
	{
		const float smooth_start{ math::remap(cfg::aimbot_projectile_snap_smooth_start, 1.0f, 100.0f, 2.0f, 20.0f) };
		const float smooth_end{ math::remap(cfg::aimbot_projectile_snap_smooth_end, 1.0f, 100.0f, 2.0f, 20.0f) };

		if (const float smooth{ math::remap(t, 0.0f, 1.0f, smooth_start, smooth_end) })
		{
			vec3 smooth_ang{ lerp_angs - i::engine->GetViewAngles() };

			math::clampAngles(smooth_ang);

			if (smooth_ang.length() > 0.0f) {
				return i::engine->GetViewAngles() + (smooth_ang / smooth);
			}
		}
	}

	return lerp_angs;
}

void AimbotSnapInst::update(const vec3 &target_ang)
{
	m_target_ang = target_ang;
}

//==========================================================================================================================================================

void AimbotSnap::begin(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const vec3 &target_ang)
{
	if (m_snap
		|| !local /*|| (local->m_flNextAttack() > i::global_vars->curtime)*/
		|| !weapon /*|| (weapon->m_flNextPrimaryAttack() > i::global_vars->curtime)*/
		|| !weapon->HasPrimaryAmmoForShot()) {
		return;
	}

	const float delay{ weapon->m_pWeaponInfo()->GetWeaponData(weapon->m_iWeaponMode()).m_flTimeFireDelay - (cfg::aimbot_projectile_snap_time * 0.9f) };

	if ((i::global_vars->curtime - weapon->m_flLastFireTime()) < delay) {
		return;
	}

	m_snap = std::make_unique<AimbotSnapInst>(i::engine->GetViewAngles(), target_ang);
}

void AimbotSnap::reset()
{
	if (!m_snap) {
		return;
	}

	m_snap.reset();
}

void AimbotSnap::update(const vec3 &target_ang)
{
	if (!m_snap) {
		return;
	}

	m_snap->update(target_ang);
}

bool AimbotSnap::think(CUserCmd *const cmd, C_TFWeaponBase *const weapon)
{
	if (!cmd || !weapon || !m_snap) {
		return false;
	}

	if (!weapon->HasPrimaryAmmoForShot()) {
		reset();
		return false;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		reset();
		return false;
	}

	i::engine->SetViewAngles(cmd->viewangles = m_snap->angles());

	return m_snap->progress() >= 1.0f;
}