#pragma once

#include "../../../game/game.hpp"

class Menu final : public HasUnload
{
public:
	bool aa_hovered{};

public:
	void run();
};

MAKE_UNIQUE(Menu, menu);