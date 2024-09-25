#pragma once

#include "../../../game/game.hpp"

class Networking final
{
private:
	void sendMove();
public:
	void move(float accumulated_extra_samples, bool final_tick);
public:
	bool shouldReadPackets();
	void fixNetworking(bool final_tick);
	int getLatestCommandNumber();
public:
};

MAKE_UNIQUE(Networking, networking);

struct ReadPacketsState
{
	float m_frametime_client{};
	float m_frametime_global{};
	float m_curtime{};
	int m_tick_count{};

	void store()
	{
		m_frametime_client = i::client_state->m_frameTime;
		m_frametime_global = i::global_vars->frametime;
		m_curtime = i::global_vars->curtime;
		m_tick_count = i::global_vars->tickcount;
	}

	void restore() const
	{
		i::client_state->m_frameTime = m_frametime_client;
		i::global_vars->frametime = m_frametime_global;
		i::global_vars->curtime = m_curtime;
		i::global_vars->tickcount = m_tick_count;
	}
};

MAKE_UNIQUE(ReadPacketsState, read_packets_state_backup);
MAKE_UNIQUE(ReadPacketsState, read_packets_state);