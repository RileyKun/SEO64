#include "spectator_list.hpp"

#include "../vis_utils/vis_utils.hpp"
#include "../cfg.hpp"

void SpectatorList::updateSpectators()
{
	m_spectators.clear();

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return;
	}

	C_TFPlayer *target{ local->deadflag() ? local->m_hObserverTarget().Get()->cast<C_TFPlayer>() : local };

	if (!target) {
		m_target_index = i::engine->GetLocalPlayer();
		return;
	}

	C_TFPlayerResource *const res{ ec->getPlayerResource() };

	m_target_index = target->entindex();

	for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PLAYERS_ALL))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

		if (!pl->deadflag()) {
			continue;
		}

		C_BaseEntity *const obs_target{ pl->m_hObserverTarget().Get() };

		if (!obs_target || obs_target->entindex() != m_target_index) {
			continue;
		}

		EObserverModes mode{ pl->m_iObserverMode() };

		// in_eye = 1st person, chase = 3rd person
		if (mode != OBS_MODE_IN_EYE && mode != OBS_MODE_CHASE) {
			continue;
		}

		player_info_t info{};

		if (!i::engine->GetPlayerInfo(pl->entindex(), &info)) {
			continue;
		}

		m_spectators.emplace_back(
			utils::utf8ToWide(info.name),
			res ? res->GetNextRespawnTime(pl->entindex()) : 0.0f,
			mode,
			vis_utils->getEntColor(pl)
		);
	}
}

void SpectatorList::run()
{
	if (!cfg::spectator_list_active || !vis_utils->shouldRunVisuals()) {
		return;
	}

	updateSpectators();
	
	if (m_spectators.empty()) {
		return;
	}

	player_info_t info{};

	if (!i::engine->GetPlayerInfo(m_target_index, &info)) {
		return;
	}

	const vec2 screen_size{ draw->getScreenSize() };
	vec2 cursor_pos{ screen_size.x * 0.5f, screen_size.y * 0.2f };
	float height{ fonts::indicators.getTall() + 2.0f };

	const std::wstring name{ m_target_index == i::engine->GetLocalPlayer() ? L"you" : utils::utf8ToWide(info.name) };

	draw->stringOutlined
	(
		cursor_pos,
		fonts::indicators.get(),
		std::format(L"spectating {}", name),
		{ 200, 200, 200, 255 },
		POS_CENTERX
	);

	cursor_pos.y += height;

	for (const Spectator &spec : m_spectators)
	{
		draw->stringOutlined
		(
			cursor_pos,
			fonts::indicators.get(),
			spec.toString(),
			spec.getColor(),
			POS_CENTERX
		);

		cursor_pos.y += height;
	}
}

const std::wstring SpectatorList::Spectator::toString() const
{
	const std::wstring observer_mode{ m_mode == OBS_MODE_IN_EYE ? L"1st" : L"3rd" };
	const float displayed_respawn_time{ fabsf(i::global_vars->curtime - m_respawn_time) };

	if (cfg::spectator_list_show_respawn_time && m_respawn_time > 0.0f && displayed_respawn_time < 2000.0f) { // mfed: fix for saxton hale mode where the respawn time is like 200k
		return std::format(L"[{}] {} ({:.1f}s)", observer_mode, m_name, displayed_respawn_time);
	}

	return std::format(L"[{}] {}", observer_mode, m_name);
}

const Color &SpectatorList::Spectator::getColor() const
{
	if (cfg::spectator_list_override_firstperson && m_mode == OBS_MODE_IN_EYE) {
		return cfg::spectator_list_firstperson_clr;
	}

	return m_color;
}