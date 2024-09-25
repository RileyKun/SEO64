#pragma once

#include "../../../game/game.hpp"

class UI final : public HasLoad, public HasUnload
{
private:
	bool m_open{};

public:
	bool isOpen() const { return m_open; }

public:
	bool onLoad() override;
	bool onUnload() override;

private:
	void setColors(const float hue, const float sat, const float val);

public:
	void start();
	void end();
};

MAKE_UNIQUE(UI, ui);