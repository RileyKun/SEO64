#pragma once

#include "../../../game/game.hpp"

class Notifs final
{
private:
	struct Notif final
	{
		std::string m_str{};
		Color m_clr{};
		float m_expire_time{};
		vec2 m_pos{};
	};

	enum class NotifType
	{
		MSG,
		WRN
	};

private:
	std::vector<Notif> m_notifs{};

private:
	void add(std::string_view str, const NotifType type);

public:
	void message(std::string_view str);
	void warning(std::string_view str);

public:
	void run();
};

MAKE_UNIQUE(Notifs, notifs);