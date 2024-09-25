#pragma once

#include <algorithm>
#include <array>

#include "vec.hpp"

using matrix3x4 = float[3][4];

namespace math
{
	float degToRad(const float deg);
	float radToDeg(const float rad);
	inline float normalizeAngle(float ang);
	void sinCos(const float radians, float *const sine, float *const cosine);
	void clampAngles(vec3 &v);
	void vectorAngles(const vec3 &forward, vec3 &angles);
	void angleVectors(const vec3 &angles, vec3 *forward, vec3 *right = nullptr, vec3 *up = nullptr);
	vec3 calcAngle(const vec3 &from, const vec3 &to, bool clamp = true);
	float calcFOV(const vec3 &ang0, const vec3 &ang1);
	float remap(const float val, const float in_min, const float in_max, const float out_min, const float out_max);
	void vectorTransform(const vec3 &input, const matrix3x4 &matrix, vec3 &output);
	bool rayVsBox(const vec3 &ray_origin, const vec3 &ray_angles, const vec3 &box_origin, const vec3 &box_mins, const vec3 &box_maxs);
	void matrixGetColumn(const matrix3x4 &matrix, const int column, vec3 &out);
	float lerp(const float a, const float b, const float t);
	vec3 lerp(const vec3 a, const vec3 b, const float t);
	void angleMatrix(const vec3& angles, matrix3x4& matrix);
	void matrixAngles(const matrix3x4& matrix, vec3& angles);
	void rotTriangle2d(std::array<vec2, 3> &points, const float rotation);

	constexpr float pi()
	{
		return 3.14159265358979323846f;
	}
}