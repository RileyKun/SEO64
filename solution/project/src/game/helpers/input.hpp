#pragma once

#include "../tf/interfaces.hpp"

class Input final
{
private:
	class Key
	{
	public:
		bool pressed{};
		bool held{};
		bool released{};
	};

	class Wheel
	{
	public:
		bool up{};
		bool down{};
	};

	enum KeyState
	{
		NONE,
		PRESSED,
		HELD,
		RELEASED
	};

private:
	bool key_states[256]{};
	KeyState key_kstates[256]{};
	int mouse_wheel_delta_cached{};
	int mouse_wheel_delta{};
	int mouse_pos_x{};
	int mouse_pos_y{};
	char key_char{};

public:
	Key getKey(const size_t key) const;
	Key getKeyInGame(const size_t key) const;
	char getChar() const;
	Wheel getMouseWheel() const;
	vec2 getMousePos() const;
	bool isMouseWithin(const vec2 rect_pos, const vec2 &rect_size) const;

public:
	void wndproc(const UINT msg, const WPARAM wparam, const LPARAM lparam);
	void processStart();
	void processEnd();
};

MAKE_UNIQUE(Input, input_game);		//used in game hooks
MAKE_UNIQUE(Input, input_tick);		//used in createmove
MAKE_UNIQUE(Input, input_dx);		//used in dx hooks