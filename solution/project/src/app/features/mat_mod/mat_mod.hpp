#pragma once

#include "../../../game/game.hpp"

class MatMod final : public HasUnload
{
private:
	bool m_mod_applied{};

private:
	void modMats(const Color clr, const bool mod_world, const bool mod_sky);

public:
	bool onUnload() override;
	
public:
	void run();
};

MAKE_UNIQUE(MatMod, mat_mod);