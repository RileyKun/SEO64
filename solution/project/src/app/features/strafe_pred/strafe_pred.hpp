#pragma once

#include "../../../game/game.hpp"

class StrafePred final
{
private:
	bool pointsSane(const std::vector<vec3> &world_points, float &dir);
	bool projectAndNormalizePoints(const std::vector<vec3> &world_points, vec3 &offset, std::vector<vec2> &points);
	bool fitCircleToPoints(const std::vector<vec2> &points, float &cx, float &cy, float &cr);
	bool generateCirclePoints(const float cx, const float cy, const float cr, const float z, const vec3 &offset, const int num_points, std::vector<vec3> &out);

public:
	bool calc(const std::vector<vec3> &points, const float player_speed, float &yaw_per_tick);
};

MAKE_UNIQUE(StrafePred, strafe_pred);