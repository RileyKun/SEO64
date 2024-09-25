#pragma once

#include "../../../game/game.hpp"

class Aimbot final
{
public:
	void run(CUserCmd *const cmd);
};

MAKE_UNIQUE(Aimbot, aimbot);