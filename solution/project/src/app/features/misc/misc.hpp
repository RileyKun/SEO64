#pragma once

#include "../../../game/game.hpp"

class Misc final
{
public:
	void bhop(CUserCmd *const cmd);
	void autoStrafe(CUserCmd *const cmd);
	void tauntSpin(CUserCmd *const cmd);
	void sentryWiggler(CUserCmd *const cmd);
	void fastStop(CUserCmd *const cmd);
	void autoTurn(CUserCmd *const cmd);
	void noisemakerSpam();
	void mvmRespawn();
	void autoMvmReadyUp();
	void autoDisguise();
public:
	int getRegionPing(const unsigned int pop_id);

public:
	void paintFOVCircle();
	void paintPredPath();
	void paintSniperSightlines();
};

MAKE_UNIQUE(Misc, misc);