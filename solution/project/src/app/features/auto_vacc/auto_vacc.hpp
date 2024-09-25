#pragma once

#include "../../../game/game.hpp"

class AutoVacc final : public HasGameEvent
{
private:
	int m_event_desired_res{};
	bool m_event_want_pop{};

private:
	void resetEvent();

private:
	bool isVisible(const vec3 &trace_start, const vec3 &trace_end);
	bool processEnvironment(C_TFPlayer *const pl, int &desired_res, bool &want_pop);

public:
	bool onGameEvent(IGameEvent *const event) override;

public:
	void run(CUserCmd *const cmd);
};

MAKE_UNIQUE(AutoVacc, auto_vacc);