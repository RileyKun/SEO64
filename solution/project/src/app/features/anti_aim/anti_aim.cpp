#include "anti_aim.hpp"

#include "../aim_utils/aim_utils.hpp"
#include "../tick_base/tick_base.hpp"
#include "../misc/misc.hpp"
#include "../vis_utils/vis_utils.hpp"
#include "../game_movement/move_pred.hpp"
#include "../cfg.hpp"

void AntiAim::runFakelag(CUserCmd *const cmd) const
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || !cmd) {
		return;
	}

	if (!cfg::fakelag_active || cfg::antiaim_fakewalk_active) {
		return;
	}

	if (aim_utils->isAttacking(cmd, i::global_vars->curtime) && !tick_base->m_delay_hitscan) {
		tf_globals::send_packet = true;
		return;
	}

	if (tick_base->m_processing_ticks - i::client_state->chokedcommands >= cfg::fakelag_ticks) {
		tf_globals::send_packet = true;
		return;
	}

	if (local->m_fFlags() & FL_ONGROUND)
	{
		if (!(cmd->buttons & (IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK))) {
			return;
		}
	}

	const float teleport_dist_sqr{ cvars::sv_lagcompensation_teleport_dist->GetFloat() * cvars::sv_lagcompensation_teleport_dist->GetFloat() };

	if (vec3{ local->m_vecOrigin() - m_last_sent_origin }.lengthSqr() > teleport_dist_sqr && cfg::fakelag_break_lc) {
		tf_globals::send_packet = true;
	}

	else {
		tf_globals::send_packet = false;
	}

	if (i::client_state->chokedcommands >= cfg::fakelag_ticks) {
		tf_globals::send_packet = true;
	}

	const int host_frameticks{ *s::host_frameticks.cast<int *>() };

	if (i::client_state->chokedcommands >= 21 - (host_frameticks - 1)) {
		tf_globals::send_packet = true;
	}
}

void AntiAim::runPreMove(CUserCmd *const cmd)
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || !cmd) {
		tick_base->m_reserved_ticks = 0;
		return;
	}

	m_yaw_real = cmd->viewangles.y;

	fakeWalk(cmd);

	if (!cfg::antiaim_active || cfg::antiaim_fake_type == 0) {
		tick_base->m_reserved_ticks = 0;
		return;
	}

	microMove(cmd);

	tick_base->m_reserved_ticks = 2;
}

void AntiAim::run(CUserCmd *const cmd, bool final_tick)
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || !cmd) {
		return;
	}

	if (!cfg::antiaim_active) {
		return;
	}

	if ((aim_utils->isAttacking(cmd, tf_utils::ticksToTime(local->m_nTickBase())) && !tick_base->m_delay_hitscan) || tick_base->m_should_doubletap) {
		return;
	}

	if (local->m_iClass() == TF_CLASS_ENGINEER && (cmd->buttons & IN_ATTACK2)) {
		return;
	}

	if (!(tick_base->m_queue_warp || tick_base->m_should_warp) && cfg::antiaim_fake_type != 0)
	{
		if (i::client_state->chokedcommands < 2) {
			tf_globals::send_packet = false;
		}
	}

	m_direction = cmd->viewangles;

	handlePitch();
	handleFakePitch();
	handleYaw();

	if ((!(tick_base->m_queue_warp || tick_base->m_should_warp) && tf_globals::send_packet) || final_tick) {
		handleFakeYaw();
	}

	m_direction.y = math::normalizeAngle(m_direction.y);

	tf_utils::fixMovement(cmd, m_direction);

	cmd->viewangles = m_direction;
}

void AntiAim::handlePitch()
{
	switch (cfg::antiaim_pitch)
	{
		case 1: {
			m_direction.x = -89.0f;
			break;
		}

		case 2: {
			m_direction.x = 89.0f;
			break;
		}

		case 3: {
			m_direction.x = 0.0f;
			break;
		}

		case 4: {
			m_direction.x = m_jitter_flip ? 89.0f : -89.0f;
			break;
		}

		default: {
			break;
		}
	}
}

void AntiAim::handleFakePitch()
{
	switch (cfg::antiaim_pitch_fake)
	{
		case 1: {
			m_direction.x -= 360.0f;
			break;
		}

		case 2: {
			m_direction.x += 360.0f;
			break;
		}

		case 3: {
			m_direction.x += m_direction.x > 0.0f ? 360.0f : -360.0f;
			break;
		}

		case 4: {
			m_direction.x += m_direction.x > 0.0f ? -360.0f : 360.0f;
			break;
		}

		case 5: {
			m_direction.x += m_jitter_flip ? -360.0f : 360.0f;
			break;
		}

		default: {
			break;
		}
	}
}

void AntiAim::handleYaw()
{
	m_direction.y -= cfg::antiaim_yaw;

	switch (cfg::antiaim_yaw_type)
	{
		case 1: {
			m_direction.y += std::fmodf(i::global_vars->realtime * (cfg::antiaim_yaw_spin_speed * 20.0f), 360.0f);
			break;
		}

		case 2: {
			m_direction.y -= getJitterOffset();
			break;
		}

		default: {
			break;
		}
	}

	m_yaw_real = m_direction.y;
}

void AntiAim::handleFakeYaw()
{
	switch (cfg::antiaim_fake_type)
	{
		case 1: {
			m_yaw_fake = m_direction.y = i::engine->GetViewAngles().y;
			return;
		}

		case 2: {
			m_direction.y += 180.0f;
			break;
		}

		case 3: {
			m_direction.y += std::fmodf(i::global_vars->realtime * (cfg::antiaim_fake_spin_speed * 20.0f), 360.0f);
			break;
		}

		case 4: {
			m_direction.y = tf_utils::randomFloat(-180.0f, 180.0f);
			break;
		}

		case 5: {
			m_direction.y += 180.0f - getJitterOffset();
			break;
		}

		default: {
			return;
		}
	}

	m_direction.y += cfg::antiaim_fake_yaw;

	m_yaw_fake = m_direction.y;
}

float AntiAim::getJitterOffset() const
{
	float base_yaw{};

	switch (cfg::antiaim_jitter_type)
	{
		case 0: {
			base_yaw += m_jitter_flip ? 0.0f : cfg::antiaim_jitter_offset;
			break;
		}

		case 1: {
			base_yaw += m_jitter_flip ? -cfg::antiaim_jitter_offset : cfg::antiaim_jitter_offset;
			break;
		}
	}

	return base_yaw;
}

void AntiAim::microMove(CUserCmd *const cmd)
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || !cmd || cfg::antiaim_fakewalk_active) {
		return;
	}

	if (!(local->m_fFlags() & FL_ONGROUND) || cmd->buttons & IN_JUMP) {
		return;
	}

	// magic ass number
	// lower values causes ur real to jitter so fucking much
	const float move_amount{ 6.0f * ((local->m_fFlags() & FL_DUCKING) ? 3.0f : 1.0f) };

	MovePred pred{};

	if (!pred.init(local)) {
		return;
	}

	C_BaseEntity *const world{ i::ent_list->GetClientEntity(0)->cast<C_BaseEntity>() };

	if (world) {
		pred.player.SetGroundEntity(world);
	}

	pred.game_movement.Friction();

	if (pred.move_data.m_vecVelocity.length() >= move_amount) {
		return;
	}

	if (std::fabs(cmd->forwardmove) > move_amount || std::fabs(cmd->sidemove) > move_amount) {
		return;
	}

	cmd->sidemove = cmd->command_number % 2 ? -move_amount : move_amount;
}

void AntiAim::fakeWalk(CUserCmd *const cmd)
{
	m_fakewalk_active = false;

	if (!cfg::antiaim_fakewalk_active) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || !cmd) {
		return;
	}

	if (!(local->m_fFlags() & FL_ONGROUND)) {
		return;
	}

	const int host_frameticks{ *s::host_frameticks.cast<int *>() };

	if (tick_base->m_processing_ticks - i::client_state->chokedcommands >= cfg::antiaim_fakewalk_ticks) {
		return;
	}

	m_fakewalk_active = true;

	if (i::client_state->chokedcommands >= cfg::antiaim_fakewalk_ticks || i::client_state->chokedcommands >= 21 - (host_frameticks - 1)) {
		tf_globals::send_packet = true;
	}

	else {
		tf_globals::send_packet = false;
	}

	int ticks{};
	int max_ticks{ cfg::antiaim_fakewalk_ticks };

	max_ticks = std::min(max_ticks, 21 - (host_frameticks - 1));

	MovePred pred{};

	if (!pred.init(local)) {
		return;
	}
	
	C_BaseEntity *const world{ i::ent_list->GetClientEntity(0)->cast<C_BaseEntity>() };

	if (world) {
		pred.player.SetGroundEntity(world);
	}

	for (; ticks < 21; ticks++)
	{
		const float speed{ pred.move_data.m_vecVelocity.length2D() };

		if (speed <= 0.1f) {
			break;
		}

		pred.game_movement.Friction();
	}

	if (ticks > ((max_ticks - 1) - i::client_state->chokedcommands)) {
		cmd->forwardmove = cmd->sidemove = 0.0f;
	}
}

void AntiAim::paint() const
{
	if (!cfg::antiaim_active) {
		return;
	}

	if (!cfg::antiaim_render_lines || !vis_utils->shouldRunVisuals()) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || local->InFirstPersonView()) {
		return;
	}

	const vec3 origin{ local->GetAbsOrigin() };

	vec3 forward{};

	math::angleVectors({ 0.0f, m_yaw_real, 0.0f }, &forward);

	vec3 offset{ origin + (forward * 50.f) };

	vec2 src{};
	vec2 dst{};

	if (draw->worldToScreen(origin, src) && draw->worldToScreen(offset, dst)) {
		draw->line(src, dst, { 255, 0, 0, 255 });
		draw->stringOutlined(dst, fonts::indicators.get(), "R", { 255, 0, 0, 255 }, POS_CENTERXY);
	}

	if (cfg::antiaim_fake_type == 0) {
		return;
	}

	math::angleVectors(vec3(0.f, m_yaw_fake, 0.f), &forward);

	offset = origin + (forward * 50.f);

	if (draw->worldToScreen(offset, dst)) {
		draw->line(src, dst, { 0, 255, 0, 255 });
		draw->stringOutlined(dst, fonts::indicators.get(), "F", { 0, 255, 0, 255 }, POS_CENTERXY);
	}
}