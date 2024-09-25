#pragma once

#include "../../../game/game.hpp"

class AutoQueue final
{
private:
	float m_last_check{};
	bool m_loaded{};

private:
	void queueForCasual();

public:
	void run();
};

MAKE_UNIQUE(AutoQueue, auto_queue);