#include "tick_base.hpp"

#include "../engine_pred/engine_pred.hpp"
#include "../aim_utils/aim_utils.hpp"
#include "../vis_utils/vis_utils.hpp"
#include "../notifs/notifs.hpp"
#include "../misc/misc.hpp"
#include "../anims/anims.hpp"
#include "../networking/networking.hpp"
#include "../crits/crits.hpp"
#include "../anti_aim/anti_aim.hpp"
#include "../cfg.hpp"

//#define DEBUG_DT

#ifdef DEBUG_DT
MAKE_SIG(server_UTIL_EntityByIndex, mem::findBytes("server.dll", "E8 ? ? ? ? 8B 53 40 45 33 C0").fixRip());
#endif

bool TickBase::onLoad()
{
	m_processing_ticks = 0;
	m_tick_fixes.clear();

	return true;
}

bool TickBase::onLevelInit()
{
	return onLoad();
}

const int TickBase::getAdjustedTick(const int sim_ticks, const int tickbase, const int server_tick)
{
	if (sim_ticks < 0) {
		return tickbase;
	}

	if (i::global_vars->maxClients == 1) {
		return i::global_vars->tickcount - sim_ticks + 1;
	}

	else
	{
		const int correction_ticks{ tf_utils::timeToTicks(std::clamp(cvars::sv_clockcorrection_msecs->GetFloat() / 1000.0f, 0.0f, 1.0f)) };
		const int ideal_final_tick{ server_tick + correction_ticks };
		const int estimated_final_tick{ tickbase + sim_ticks };
		const int fast_limit{ ideal_final_tick + correction_ticks };

		if (estimated_final_tick > fast_limit || estimated_final_tick < server_tick) {
			return ideal_final_tick - sim_ticks + 1;
		}

		return tickbase;
	}
}

const int TickBase::getShiftLimit(const bool warp)
{
	const int host_frameticks{ *s::host_frameticks.cast<int *>() };
	const int ticks_to_shift{ std::min(m_processing_ticks - i::client_state->chokedcommands, 21) - (host_frameticks - 1) };

	if (!warp) {
		return ticks_to_shift;
	}

	else {
		return std::min(ticks_to_shift, cfg::tb_warp_ticks);
	}
}

void TickBase::sendMoreMove(CUserCmd *const cmd)
{
	if (!m_should_doubletap && !m_should_warp) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	m_should_antiwarp = shouldAntiWarp();

	const int latest_cmd{ networking->getLatestCommandNumber() };

	int new_outgoing_cmd{ latest_cmd + 1 };
	int ticks_to_shift{ getShiftLimit(m_should_warp) };

	if (tf_globals::final_tick) {
		tf_globals::send_packet = true;
	}

	const int server_tick{ tf_utils::timeToTicks(tf_utils::getServerTime() + tf_utils::getLatency()) - 1 };
	const int adjusted_tick{ getAdjustedTick(ticks_to_shift + 1, local->m_nTickBase(), server_tick) };
	
	local->m_nTickBase() = adjusted_tick;

	addTickShift(latest_cmd, adjusted_tick);

	//notifs->message(std::format("shifting {} ticks", ticks_to_shift));

	//do some out of the ass black magic
	{
		const float mod{ math::remap(local->m_vecVelocity().length2D(), 0.0f, 400.0f, 0.0f, 0.4f) };

		m_shift_start_origin = local->m_vecOrigin() + ((local->m_vecVelocity() * mod) * tf_utils::ticksToTime(2));

		m_shift_velocity = local->GetAbsVelocity();
	}

	while (ticks_to_shift >= 1 && i::client_state->chokedcommands < 22)
	{
		const bool final_tick{ (ticks_to_shift == 1 || i::client_state->chokedcommands == 21) && tf_globals::final_tick};

		i::engine->GetNetChannelInfo()->SetChoked();

		i::client_state->chokedcommands++;

		engine_pred->update();

		addTickShift(new_outgoing_cmd, adjusted_tick + new_outgoing_cmd - latest_cmd);

		CUserCmd *const new_cmd{ &i::input->m_pCommands[new_outgoing_cmd % 90] };
		//copy og usercmd to new one
		memcpy(new_cmd, cmd, sizeof(CUserCmd));

		//mark new cmd as not predicted so fx works properly
		new_cmd->hasbeenpredicted = false;

		//run antiwarp if we're doubletapping and not warping
		if (m_should_antiwarp) {
			antiWarp(new_cmd);
		}

		//otherwise run all of our movement features like normal
		else {
			misc->bhop(new_cmd);
			misc->fastStop(new_cmd);
			engine_pred->edgeJump(new_cmd);
		}

		anti_aim->run(new_cmd, final_tick);

		new_outgoing_cmd++;
		ticks_to_shift--;
	}

	m_disable_recharge = false;
	m_should_doubletap = false;
}

bool TickBase::rechargeTicks()
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		m_recharge = false;
		return false;
	}

	if (m_processing_ticks - i::client_state->chokedcommands < 24 - m_reserved_ticks)
	{
		if (m_processing_ticks < 22 - m_reserved_ticks) {
			m_disable_recharge = false;
		}

		if (cfg::tb_recharge
			&& !m_recharge
			&& !m_disable_recharge
			&& !m_should_doubletap
			&& !m_should_warp
			&& !m_queue_warp) {
			m_recharge = true;
			{
				const int ticks_recharged{ 24 - m_processing_ticks };

				const int server_tick{ tf_utils::timeToTicks(tf_utils::getServerTime() + tf_utils::getLatency()) - 1 + ticks_recharged};
				const int adjusted_tick{ getAdjustedTick(1, local->m_nTickBase(), server_tick) };

				addTickShift(networking->getLatestCommandNumber() + ticks_recharged, adjusted_tick);
			}
		}

		if (m_recharge) {
			return true;
		}
	}

	else {
		m_recharge = false;
		m_disable_recharge = true; //stop users from recharging those bled ticks
	}

	return false;
}

bool TickBase::canDoubleTap(CUserCmd *const cmd, bool set_dt_state)
{
	if (m_should_warp) //if we're warping dont doubletap
		return false;

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return false;
	}

	C_TFWeaponBase *const wep{ ec->getWeapon() };

	if (!wep) {
		return false;
	}

	//normie - we should use fast_limit which would be around 8 ticks
	//since anything lower would maybe make tickbase not shift
	if (!cmd || m_processing_ticks < 8 || !cfg::tb_doubletap) {
		return false;
	}

	const int ticks_to_shift{ getShiftLimit() };
	const int server_tick{ tf_utils::timeToTicks(tf_utils::getServerTime() + tf_utils::getLatency()) - 1 };
	const float adjusted_time{ tf_utils::ticksToTime(getAdjustedTick(ticks_to_shift + 1, local->m_nTickBase(), server_tick)) };

	if (cfg::tb_dt_aimbot_only && tf_globals::aimbot_target == EntityHandle_t{}) {
		return false;
	}

	if (tf_globals::aimbot_target != EntityHandle_t{}) {
		C_BaseEntity *const ent{ i::ent_list->GetClientEntityFromHandle(tf_globals::aimbot_target)->cast<C_BaseEntity>() };
		if (ent && ent->GetClassId() == class_ids::CTFGrenadePipebombProjectile) {
			return false;
		}
	}

	if (isAttacking(cmd, adjusted_time) && !m_delay_hitscan)
	{
		if (set_dt_state) {
			m_should_doubletap = true;
		}

		return true;
	}

	if (cfg::tb_dt_lock || m_delay_hitscan)
	{
		if (!tf_utils::isWeaponChargeable(wep)
			|| (wep->GetWeaponID() == TF_WEAPON_MINIGUN && wep->cast<C_TFMinigun>()->m_iWeaponState() == AC_STATE_FIRING))
		{
			cmd->buttons &= ~IN_ATTACK;

			if (wep->GetWeaponID() == TF_WEAPON_MINIGUN) {
				cmd->buttons |= IN_ATTACK2;
			}
		}
	}

	return false;
}

bool TickBase::checkWarp(CUserCmd *const cmd)
{
	m_should_warp = false;

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return false;
	}

	if (!cmd || m_processing_ticks - i::client_state->chokedcommands <= 1) {
		return false;
	}

	if (!cfg::tb_warp && !m_queue_warp) {
		return false;
	}

	m_should_warp = true;
	m_queue_warp = false;

	return true;
}

void TickBase::updateTickbaseFixes()
{
	//dont store a crazy amount of tickbase fixes
	while (m_tick_fixes.size() > 24) {
		m_tick_fixes.pop_front();
	}

	m_tick_fixes.erase(
		std::ranges::remove_if(m_tick_fixes, 
			[&](const TickShift& shift) { 
				return i::client_state->command_ack >= shift.command_number; 
			}
		).begin(), m_tick_fixes.end());
}

void TickBase::antiWarp(CUserCmd *const cmd) const
{
	if (!cmd || !m_should_antiwarp) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local
		|| local->deadflag()
		|| local->m_MoveType() != MOVETYPE_WALK
		|| !(local->m_fFlags() & FL_ONGROUND)) {
		return;
	}

	if (cfg::tb_antiwarp_style == 0) {
		cmd->forwardmove = cmd->sidemove = 0.0f;
	}

	//TODO make this shit instantly stop
	else if (cfg::tb_antiwarp_style == 1) {
		const vec3 difference{ m_shift_start_origin - local->m_vecOrigin()};

		const vec3 og_angles{ cmd->viewangles };

		if (difference.length() == 0.0f) {
			cmd->forwardmove = 0.0f;
			cmd->sidemove = 0.0f;
			return;
		}

		vec3 direction{};
		math::vectorAngles({ difference.x, difference.y, 0.0f }, direction);

		cmd->viewangles = direction;

		cmd->forwardmove = 450.f;
		cmd->sidemove = 0.f;

		tf_utils::fixMovement(cmd, og_angles);
		cmd->viewangles = og_angles;
	}

	else if (cfg::tb_antiwarp_style == 2) 
	{
		if (m_shift_velocity.length2D() == 0.f) {
			return;
		}

		const vec3 og_angles{ cmd->viewangles };

		vec3 dir{};
		math::vectorAngles(m_shift_velocity, dir);

		dir *= -1;

		cmd->viewangles = dir;
		cmd->forwardmove = 450.f;

		tf_utils::fixMovement(cmd, og_angles);
		cmd->viewangles = og_angles;
	}
}

bool TickBase::shouldAntiWarp() const
{
	if (m_should_warp && !m_should_doubletap) {
		return false;
	}

	C_TFWeaponBase *const wep{ ec->getWeapon() };

	if (!wep || tf_utils::isWeaponMelee(wep)) {
		return false;
	}

	switch (cfg::tb_antiwarp_mode)
	{
		case 0: {
			return false;
		}

		case 1:
		{
			//normie: todo we should just check the attack interval and whether or not its less than the ticks we are shifting
			if (tf_utils::isWeaponProjectile(wep) || tf_utils::isWeaponMelee(wep)) {
				return false;
			}

			else {
				return true;
			}
		}

		case 2: {
			return true;
		}

		default: {
			return false;
		}
	}

	return false;
}

bool TickBase::isAttacking(CUserCmd *const cmd, float curtime, const bool vis_indicator)
{
	if (!cmd) {
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

	//adjust our curtime to our non-shifted tickbase if we have a charge weapon
	if (tf_utils::isWeaponChargeable(weapon)) {
		curtime = local->m_nTickBase() * i::global_vars->interval_per_tick;
	}

	const bool can_primary_attack{
		(curtime >= local->m_flNextAttack()) && (curtime >= weapon->m_flNextPrimaryAttack())
	};

	if (vis_indicator) {
		return can_primary_attack;
	}

	const float charge_begin_time{ weapon->cast<C_TFPipebombLauncher>()->m_flChargeBeginTime() };

	switch (weapon->GetWeaponID())
	{
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		{
			if (charge_begin_time)
			{
				const float charge{ curtime - charge_begin_time };
				const float max_charge{ tf_utils::attribHookValue(4.0f, "stickybomb_charge_rate", weapon) };

				if ((charge > max_charge) && (cmd->buttons & IN_ATTACK)) {
					return true;
				}
			}

			return (!(cmd->buttons & IN_ATTACK) && charge_begin_time);
		}

		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_CANNON:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: {
			return (!(cmd->buttons & IN_ATTACK) && charge_begin_time);
		}

		default: {
			return aim_utils->isAttacking(cmd, curtime, true);
		}
	}

	return false;
}

void TickBase::addTickShift(const int command_number, const int tickbase)
{
	for (auto &shifts : m_tick_fixes)
	{
		if (command_number == shifts.command_number) {
			shifts.tickbase = tickbase;
			break;
		}
	}

	m_tick_fixes.emplace_back(command_number, tickbase);
}

void TickBase::paint()
{
	if (!cfg::tb_indicator || !vis_utils->shouldRunVisuals()) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	Font *const font{ &fonts::indicators };

	size_t idx{};

	vec2 pos{ draw->getScreenSize().x * 0.5f, draw->getScreenSize().y * cfg::tb_indicator_pos };

	auto getPos = [&]() {
		return pos + vec2{ 0.0f, font->getTall() * static_cast<float>(idx++) };
	};

	const int ticks_to_shift{ getShiftLimit() };
	const int server_tick{ tf_utils::timeToTicks(tf_utils::getServerTime() + tf_utils::getLatency()) - 1 };
	const float adjusted_time{ tf_utils::ticksToTime(getAdjustedTick(ticks_to_shift + 1, local->m_nTickBase(), server_tick)) };

#ifndef DEBUG_DT
	const int processing_ticks{ std::min(m_visual_ticks.load(), 22)};
#elif
	const int processing_ticks{ m_visual_ticks.load() };
#endif

	static float pt_lerped{ static_cast<float>(processing_ticks) };

	if (cfg::tb_indicator_lerp) {
		pt_lerped += (static_cast<float>(processing_ticks) - pt_lerped) * (20.0f * i::global_vars->frametime);
		pt_lerped = std::clamp(pt_lerped, 0.0f, 22.0f);
	}
	else {
		pt_lerped = static_cast<float>(processing_ticks);
	}

	if (cfg::tb_indicator == 1)
	{
		//stupid game decides to bleed off 2 ticks if we recharge past 22 ticks so
		std::string str{ std::format("ticks: {:.0f} / 22", pt_lerped > 1.0f ? ceilf(pt_lerped) : pt_lerped) };

#ifdef DEBUG_DT
		str = std::format("ticks: {} / 24", processing_ticks);
#endif

		draw->stringOutlined(getPos(), font->get(), str, { 200, 200, 200, 255 }, POS_CENTERX);

#ifdef DEBUG_DT
		if (i::engine->GetNetChannelInfo()->IsLoopback())
		{
			uinptr_t server_local{ s::server_UTIL_EntityByIndex.call<uintptr_t>(local->entindex()) };

			if (server_local)
			{
				//8B 9A 08 0A 00 00 - CPlayerMove::RunCommand
				int ticks_for_processing{ *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(server_local) + 2568) };

				const std::string server_str{ std::format("server ticks {}/24", ticks_for_processing) };

				pos.y += font->getTall();

				draw->stringOutlined(getPos(), font->get(), server_str, { 200, 200, 200, 255 }, POS_CENTERX);
			}
		}
#endif

		CUserCmd dummy{};

		dummy.buttons |= IN_ATTACK;

		if (isAttacking(&dummy, adjusted_time, true) && m_processing_ticks - i::client_state->chokedcommands >= 8 && !m_recharge)
		{
			Color clr{ cfg::color_indicators_good };

			if (cfg::tb_doubletap || m_should_doubletap) {
				clr = cfg::color_indicators_extra_good;
			}

			draw->stringOutlined(getPos(), font->get(), "doubletap ready!", clr, POS_CENTERX);
		}

		else {
			draw->stringOutlined(getPos(), font->get(), "doubletap not ready", cfg::color_indicators_bad, POS_CENTERX);
		}

		if (m_disable_recharge) {
			draw->stringOutlined(getPos(), font->get(), "recharge locked", cfg::color_indicators_bad, POS_CENTERX);
		}
	}

	else if (cfg::tb_indicator == 2)
	{
		const std::string str{ std::format("ticks: {:.0f} ({:.2f}s)", pt_lerped, tf_utils::ticksToTime(pt_lerped > 1.0f ? ceilf(pt_lerped) : pt_lerped)) };

		const vec2 rect_size{ 120.0f, 20.0f };
		const vec2 rect_pos{ pos - vec2{ rect_size.x * 0.5f, 0.0f } };

		draw->rectFilled(rect_pos, rect_size, { 5, 5, 5, 127 });
		draw->string(rect_pos + vec2{ rect_size.x * 0.5f, 2.0f }, font->get(), str.c_str(), { 180, 180, 180, 255 }, POS_CENTERX);

		const vec2 top_rect_pos{ rect_pos - vec2{ 0.0f, 4.0f } };

		draw->rectFilled(top_rect_pos, { rect_size.x, 4.0f }, cfg::color_indicators_bad);

		CUserCmd dummy{};

		dummy.buttons |= IN_ATTACK;

		Color top_clr{ cfg::color_indicators_good };

		if (isAttacking(&dummy, adjusted_time, true) && m_processing_ticks - i::client_state->chokedcommands >= 8 && !m_recharge)
		{
			if (cfg::tb_doubletap || m_should_doubletap) {
				top_clr = cfg::color_indicators_extra_good;
			}
		}

		float pt_lerped_bar{ pt_lerped };

		if (cfg::tb_indicator_lerp) {
			if (pt_lerped <= 1.0f) {
				pt_lerped_bar = 0.0f;
			}
		}

		draw->rectFilled(top_rect_pos, { (rect_size.x / 21.0f) * std::min(pt_lerped_bar, 21.0f), 4.0f }, top_clr);
	}
}