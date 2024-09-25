#pragma once

#include "../../../game/game.hpp"

class SpectatorList final
{
private:
	class Spectator
	{
	private:
		std::wstring m_name{};
		float m_respawn_time{};
		EObserverModes m_mode{};
		Color m_color{};

	public:
		Spectator(const std::wstring &name, float respawn_time, EObserverModes mode, const Color &color)
		{
			m_name = name;
			m_respawn_time = respawn_time;
			m_mode = mode;
			m_color = color;
		}

	public:
		const std::wstring toString() const;
		const Color &getColor() const;
	};

private:
	std::vector<Spectator> m_spectators{};
	int m_target_index{};

private:
	void updateSpectators();

public:
	void run();
};

MAKE_UNIQUE(SpectatorList, spectator_list);