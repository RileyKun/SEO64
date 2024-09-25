#pragma once

#include "game_movement.hpp"

class MovePred final
{
public:
	TFPlayerProxy player{};
	CMoveData move_data{};
	GameMovement game_movement{};

private:
	float m_yaw_per_tick{};
	bool m_air_strafing{};

private:
	bool setupMoveData(C_TFPlayer *const pl, CMoveData *const move_data, const bool ignore_special_ability = false);
	bool setupMoveData(C_TFPlayer *const pl, CUserCmd* const cmd, CMoveData *const move_data, const bool ignore_special_ability = false);
public:
	bool init(C_TFPlayer *const pl, const bool ignore_special_ability = false);
	void tick();
	vec3 origin() const;
};