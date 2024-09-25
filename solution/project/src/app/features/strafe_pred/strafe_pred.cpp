#include "strafe_pred.hpp"

//#define DEBUG_STRAFES

bool StrafePred::pointsSane(const std::vector<vec3> &world_points, float &dir)
{
	if (world_points.size() < 3) {
		return false;
	}

	for (size_t n{}; n < world_points.size(); n++)
	{
		for (size_t j{ n + 1 }; j < world_points.size(); j++)
		{
			if (world_points[n].distTo(world_points[j]) < 1.5f) {
				return false;
			}
		}
	}

	float prev_diff{};
	bool first_diff{ true };

	for (size_t n{}; n < world_points.size() - 2; n++)
	{
		const float yaw0{ math::calcAngle(world_points[n], world_points[n + 1]).y };
		const float yaw1{ math::calcAngle(world_points[n + 1], world_points[n + 2]).y };

		if (fabsf(yaw1 - yaw0) < 0.2f) {
			return false;
		}

		const float cur_diff{ yaw1 - yaw0 };

		if (!first_diff)
		{
			if ((cur_diff > 0.0f && prev_diff < 0.0f) || (cur_diff < 0.0f && prev_diff > 0.0f)) {
				return false;
			}
		}

		prev_diff = cur_diff;
		first_diff = false;
	}
	
	if (prev_diff > 0.0f) {
		dir = -1.0f;
	}

	if (prev_diff < 0.0f) {
		dir = 1.0f;
	}

	return true;
}

bool StrafePred::projectAndNormalizePoints(const std::vector<vec3> &world_points, vec3 &offset, std::vector<vec2> &points)
{
	if (world_points.size() < 3) {
		return false;
	}

	vec3 centroid{};

	for (const vec3 &p : world_points) {
		centroid += p;
	}

	offset = centroid / static_cast<float>(world_points.size());

	for (const vec3 &p : world_points) {
		points.push_back(vec2{ p.x - offset.x, p.y - offset.y });
	}

	return true;
}

bool StrafePred::fitCircleToPoints(const std::vector<vec2> &points, float &cx, float &cy, float &cr)
{
	if (points.size() < 3) {
		return false;
	}

	float sumx{};
	float sumy{};
	float sumx2{};
	float sumy2{};
	float sumxy{};
	float sumx3{};
	float sumy3{};
	float sumx1y2{};
	float sumx2y1{};

	for (const vec2 &point : points)
	{
		const float x{ point.x };
		const float y{ point.y };
		const float x2{ x * x };
		const float y2{ y * y };
		const float xy{ x * y };
		const float x3{ x2 * x };
		const float y3{ y2 * y };
		const float x1y2{ x * y2 };
		const float x2y1{ x2 * y };

		sumx += x;
		sumy += y;
		sumx2 += x2;
		sumy2 += y2;
		sumxy += xy;
		sumx3 += x3;
		sumy3 += y3;
		sumx1y2 += x1y2;
		sumx2y1 += x2y1;
	}

	const float N{ static_cast<float>(points.size()) };

	const float C{ N * sumx2 - sumx * sumx };
	const float D{ N * sumxy - sumx * sumy };
	const float E{ N * sumx3 + N * sumx1y2 - (sumx2 + sumy2) * sumx };
	const float G{ N * sumy2 - sumy * sumy };
	const float H{ N * sumy3 + N * sumx2y1 - (sumx2 + sumy2) * sumy };

	const float a{ (H * D - E * G) / (C * G - D * D) };
	const float b{ (H * C - E * D) / (D * D - G * C) };
	const float c{ -(a * sumx + b * sumy + sumx2 + sumy2) / N };

	cx = a / -2.0f;
	cy = b / -2.0f;
	cr = sqrtf(a * a + b * b - 4.0f * c) / 2.0f;

	return true;
}

bool StrafePred::generateCirclePoints(const float cx, const float cy, const float cr, const float z, const vec3 &offset, const int num_points, std::vector<vec3> &out)
{
	if (cr < 5.0f) {
		return false;
	}

	for (int n{}; n < num_points; n++) {
		const float angle{ 2.0f * math::pi() * static_cast<float>(n) / static_cast<float>(num_points) };
		out.push_back(vec3{ cx + cr * cosf(angle) + offset.x, cy + cr * sinf(angle) + offset.y, z });
	}

	return true;
}

bool StrafePred::calc(const std::vector<vec3> &points, const float player_speed, float &yaw_per_tick)
{
	float strafe_dir{};

	if (!pointsSane(points, strafe_dir)) {
		return false;
	}

#ifdef DEBUG_STRAFES
	i::debug_overlay->ClearAllOverlays();

	for (const vec3 &p : points) {
		i::debug_overlay->AddSweptBoxOverlay(p, p, { -0.7f, -0.7f, -0.7f }, { 0.7f, 0.7f, 0.7f }, {}, 0, 255, 100, 255, 0.1f);
	}
#endif

	vec3 offset{};
	std::vector<vec2> normalized_points{};

	if (!projectAndNormalizePoints(points, offset, normalized_points)) {
		return false;
	}

	float cx{};
	float cy{};
	float cr{};

	if (!fitCircleToPoints(normalized_points, cx, cy, cr)) {
		return false;
	}

	yaw_per_tick = std::clamp((player_speed / (cr * 1.0f)) * strafe_dir, -5.0f, 5.0f);

#ifdef DEBUG_STRAFES
	std::vector<vec3> cp{};

	if (!generateCirclePoints(cx, cy, cr, points[0].z, offset, 100, cp)) {
		return false;
	}

	for (size_t n{ 1 }; n < cp.size(); n += 2) {
		i::debug_overlay->AddLineOverlay(cp[n], cp[n - 1], 255, 255, 255, false, 0.1f);
	}
#endif

	return true;
}