#pragma once

#include "../../../game/game.hpp"

class AntiAim final
{
public:
	vec3 m_last_sent_origin{};
	vec3 m_direction{};
	bool m_jitter_flip{};
	float m_yaw_real{};
	float m_yaw_fake{};
	bool m_fakewalk_active{};
public:
	void runFakelag(CUserCmd *const cmd) const;
	void runPreMove(CUserCmd *const cmd);

public:
	void run(CUserCmd *const cmd, bool final_tick = false);

public:
	void handlePitch();
	void handleFakePitch();
	void handleYaw();
	void handleFakeYaw();

public:
	float getJitterOffset() const;
	void microMove(CUserCmd *const cmd);
	void fakeWalk(CUserCmd *const cmd);
public:
	void paint() const;
};

MAKE_UNIQUE(AntiAim, anti_aim);