#include "notifs.hpp"

#include "../cfg.hpp"

void Notifs::add(std::string_view str, const NotifType type)
{
	if (!cfg::notifs_active) {
		return;
	}

	const float NOTIF_SPACING_X{ 25.0f };
	const float NOTIF_SPACING_Y{ 25.0f };

	Color clr{};

	if (type == NotifType::MSG) {
		clr = cfg::color_notifs_style_message;
	}

	if (type == NotifType::WRN) {
		clr = cfg::color_notifs_style_warning;
	}

	float pos_y{ 50.0f + NOTIF_SPACING_Y };

	if (!m_notifs.empty())
	{
		const float potential_pos{ m_notifs.back().m_pos.y + NOTIF_SPACING_Y };

		if (potential_pos < draw->getScreenSize().y * 0.98f) {
			pos_y = potential_pos;
		}
	}

	m_notifs.push_back
	({
		std::string{ str },
		clr,
		i::engine->Time() + cfg::notifs_duration,
		{ (draw->getScreenSize().x - NOTIF_SPACING_X) - fonts::indicators.getTextSize(str).x, pos_y }
	});

	if (cfg::notifs_print_to_con) {
		i::cvar->ConsoleColorPrintf(clr, std::format("[seo64] {}\n", str).c_str());
	}
}

void Notifs::message(std::string_view str)
{
	add(str, NotifType::MSG);
}

void Notifs::warning(std::string_view str)
{
	add(str, NotifType::WRN);
}

void Notifs::run()
{
	for (size_t n{}; n < m_notifs.size(); n++)
	{
		Notif &notif{ m_notifs[n] };

		if (i::engine->Time() >= notif.m_expire_time) {
			m_notifs.erase(m_notifs.begin() + n);
			n--;
			continue;
		}

		const float time_left{ notif.m_expire_time - i::engine->Time() };

		auto easeInExpo = [](const float t) {
			return (powf(2.0f, 8.0f * t) - 1.0f) / 255.0f;
		};

		notif.m_clr.a = 255;

		if (time_left >= cfg::notifs_duration - cfg::notifs_fade_in_time) {
			notif.m_clr.a = static_cast<uint8_t>(math::remap(time_left, cfg::notifs_duration - cfg::notifs_fade_in_time, cfg::notifs_duration, 1.0f, 0.0f) * 255.0f);
		}

		if (time_left <= cfg::notifs_fade_out_time)
		{
			const float t_clr{ easeInExpo(math::remap(time_left, 0.0f, cfg::notifs_fade_out_time, 0.0f, 1.7f)) };
			const float t_pos{ easeInExpo(math::remap(time_left, 0.0f, cfg::notifs_fade_out_time, 1.0f, 0.3f)) };

			notif.m_clr.a = static_cast<uint8_t>(std::min(t_clr, 1.0f) * 255.0f);
			notif.m_pos.x += (2000.0f * t_pos) * i::global_vars->frametime;
		}

		const vec2 rect_pos{ notif.m_pos - vec2{ 2.0f, 2.0f } };
		const vec2 rect_size{ fonts::indicators.getTextSize(notif.m_str) + vec2{ 5.0f, 6.0f } };
		
		Color clr_bg{ cfg::color_ui_background };
		Color clr_outline{ cfg::color_ui_outline };

		clr_bg.a = notif.m_clr.a;
		clr_outline.a = notif.m_clr.a;

		const float bar_w{ math::remap(time_left, cfg::notifs_fade_out_time, cfg::notifs_duration, rect_size.x - 4.0f, 0.0f) };

		draw->rectFilled(rect_pos, rect_size, clr_bg);
		draw->rect(rect_pos, rect_size, clr_outline);
		draw->rectFilled(rect_pos + vec2{ 2.0f, rect_size.y - 5.0f }, { bar_w, 2.0f }, notif.m_clr);
		draw->string(notif.m_pos, fonts::indicators.get(), notif.m_str, notif.m_clr);
	}
}