#include "math.hpp"

constexpr float PI{ 3.14159265f };
constexpr float M_RADPI{ 57.2957795f };

float math::degToRad(const float deg)
{
	return deg * (PI / 180.0f);
}

float math::radToDeg(const float rad)
{
	return rad * (180.0f / PI);
}

inline float math::normalizeAngle(float ang)
{
	float f = (ang * (1.0f / 360.0f)) + 0.5f;
	int i = (int)f;
	float fi = (float)i;
	f = (f < 0.0f && f != fi) ? fi - 1.0f : fi;
	ang -= f * 360.0f;
	return ang;
}

void math::sinCos(const float radians, float *const sine, float *const cosine)
{
	*sine = std::sinf(radians);
	*cosine = std::cosf(radians);
}

void math::clampAngles(vec3 &v)
{
	v.x = std::clamp(normalizeAngle(v.x), -89.0f, 89.0f);
	v.y = normalizeAngle(v.y);
	v.z = 0.0f;
}

void math::vectorAngles(const vec3 &forward, vec3 &angles)
{
	float yaw{};
	float pitch{};

	if (forward.y == 0.0f && forward.x == 0.0f)
	{
		yaw = 0.0f;

		if (forward.z > 0.0f) {
			pitch = 270.0f;
		}

		else {
			pitch = 90.0f;
		}
	}

	else
	{
		yaw = radToDeg(std::atan2f(forward.y, forward.x));

		if (yaw < 0.0f) {
			yaw += 360.0f;
		}

		pitch = radToDeg(std::atan2f(-forward.z, forward.length2D()));

		if (pitch < 0.0f) {
			pitch += 360.0f;
		}
	}

	angles.set(pitch, yaw, 0.0f);
}

void math::angleVectors(const vec3 &angles, vec3 *forward, vec3 *right, vec3 *up)
{
	float sr{};
	float sp{};
	float sy{};
	float cr{};
	float cp{};
	float cy{};

	sinCos(degToRad(angles.x), &sp, &cp);
	sinCos(degToRad(angles.y), &sy, &cy);
	sinCos(degToRad(angles.z), &sr, &cr);

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right) {
		right->x = -1.0f * sr * sp * cy + -1.0f * cr * -sy;
		right->y = -1.0f * sr * sp * sy + -1.0f * cr * cy;
		right->z = -1.0f * sr * cp;
	}

	if (up) {
		up->x = cr * sp * cy + -sr * -sy;
		up->y = cr * sp * sy + -sr * cy;
		up->z = cr * cp;
	}
}

vec3 math::calcAngle(const vec3 &from, const vec3 &to, bool clamp)
{
	const vec3 delta{ from - to };
	const float hyp{ std::sqrtf((delta.x * delta.x) + (delta.y * delta.y)) };

	vec3 out{ (std::atanf(delta.z / hyp) * M_RADPI), (std::atanf(delta.y / delta.x) * M_RADPI), 0.0f };

	if (delta.x >= 0.0f) {
		out.y += 180.0f;
	}

	if (clamp) {
		clampAngles(out);
	}

	return out;
}

float math::calcFOV(const vec3 &ang0, const vec3 &ang1)
{
	vec3 v_ang0{};
	angleVectors(ang0, &v_ang0);

	vec3 v_ang1{};
	angleVectors(ang1, &v_ang1);

	const float out{ radToDeg(acos(v_ang1.dot(v_ang0) / v_ang1.lengthSqr())) };

	if (!isfinite(out) || isinf(out) || isnan(out)) {
		return 0.0f;
	}

	return out;
}

float math::remap(const float val, const float in_min, const float in_max, const float out_min, const float out_max)
{
	if (in_min == in_max) {
		return val >= in_max ? out_max : out_min;
	}

	return out_min + (out_max - out_min) * std::clamp((val - in_min) / (in_max - in_min), 0.0f, 1.0f);
}

void math::vectorTransform(const vec3 &input, const matrix3x4 &matrix, vec3 &output)
{
	for (int n{}; n < 3; n++) {
		output[n] = input.dot((vec3 &)matrix[n]) + matrix[n][3];
	}
}

bool math::rayVsBox(const vec3 &ray_origin, const vec3 &ray_angles, const vec3 &box_origin, const vec3 &box_mins, const vec3 &box_maxs)
{
	vec3 ray_dir{};

	angleVectors(ray_angles, &ray_dir);

	const vec3 box_min_world{ box_origin + box_mins };
	const vec3 box_max_world{ box_origin + box_maxs };

	float tmin{ (box_min_world.x - ray_origin.x) / ray_dir.x };
	float tmax{ (box_max_world.x - ray_origin.x) / ray_dir.x };

	if (tmin > tmax) {
		std::swap(tmin, tmax);
	}

	float tymin{ (box_min_world.y - ray_origin.y) / ray_dir.y };
	float tymax{ (box_max_world.y - ray_origin.y) / ray_dir.y };

	if (tymin > tymax) {
		std::swap(tymin, tymax);
	}

	if ((tmin > tymax) || (tymin > tmax)) {
		return false;
	}

	if (tymin > tmin) {
		tmin = tymin;
	}

	if (tymax < tmax) {
		tmax = tymax;
	}

	float tzmin{ (box_min_world.z - ray_origin.z) / ray_dir.z };
	float tzmax{ (box_max_world.z - ray_origin.z) / ray_dir.z };

	if (tzmin > tzmax) {
		std::swap(tzmin, tzmax);
	}

	if ((tmin > tzmax) || (tzmin > tmax)) {
		return false;
	}

	if (tzmin > tmin) {
		tmin = tzmin;
	}

	if (tzmax < tmax) {
		tmax = tzmax;
	}

	return tmax >= 0.0f && tmin <= tmax;
}

void math::matrixGetColumn(const matrix3x4 &matrix, const int column, vec3 &out)
{
	out.x = matrix[0][column];
	out.y = matrix[1][column];
	out.z = matrix[2][column];
}

float math::lerp(const float a, const float b, const float t)
{
	return a + (b - a) * t;
}

vec3 math::lerp(const vec3 a, const vec3 b, const float t)
{
	vec3 ret{ std::lerp(a.x, b.x, t), std::lerp(a.y, b.y, t), std::lerp(a.z, b.z, t) };
	return ret;
}

void math::angleMatrix(const vec3& angles, matrix3x4& matrix)
{
	float sr, sp, sy, cr, cp, cy;

	sinCos(degToRad(angles.y), &sy, &cy);
	sinCos(degToRad(angles.x), &sp, &cp);
	sinCos(degToRad(angles.z), &sr, &cr);

	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;

	float crcy = cr * cy;
	float crsy = cr * sy;
	float srcy = sr * cy;
	float srsy = sr * sy;
	matrix[0][1] = sp * srcy - crsy;
	matrix[1][1] = sp * srsy + crcy;
	matrix[2][1] = sr * cp;

	matrix[0][2] = (sp * crcy + srsy);
	matrix[1][2] = (sp * crsy - srcy);
	matrix[2][2] = cr * cp;

	matrix[0][3] = 0.0f;
	matrix[1][3] = 0.0f;
	matrix[2][3] = 0.0f;
}

void math::matrixAngles(const matrix3x4& matrix, vec3& angles)
{
	float forward[3]{};
	float left[3]{};
	float up[3]{};

	//
	// Extract the basis vectors from the matrix. Since we only need the Z
	// component of the up vector, we don't get X and Y.
	//
	forward[0] = matrix[0][0];
	forward[1] = matrix[1][0];
	forward[2] = matrix[2][0];
	left[0] = matrix[0][1];
	left[1] = matrix[1][1];
	left[2] = matrix[2][1];
	up[2] = matrix[2][2];

	float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

	// enough here to get angles?
	if (xyDist > 0.001f)
	{
		// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
		angles[1] = radToDeg(atan2f(forward[1], forward[0]));

		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles[0] = radToDeg(atan2f(-forward[2], xyDist));

		// (roll)	z = ATAN( left.z, up.z );
		angles[2] = radToDeg(atan2f(left[2], up[2]));
	}
	else	// forward is mostly Z, gimbal lock-
	{
		// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
		angles[1] = radToDeg(atan2f(-left[0], left[1]));

		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles[0] = radToDeg(atan2f(-forward[2], xyDist));

		// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
		angles[2] = 0;
	}
}

void math::rotTriangle2d(std::array<vec2, 3> &points, const float rotation)
{
	const vec2 center{ (points[0] + points[1] + points[2]) / 3 };

	for (vec2 &point : points)
	{
		point -= center;

		const float temp_x{ point.x };
		const float temp_y{ point.y };
		const float theta{ degToRad(rotation) };
		const float c{ cosf(theta) };
		const float s{ sinf(theta) };

		point.x = temp_x * c - temp_y * s;
		point.y = temp_x * s + temp_y * c;

		point += center;
	}
}