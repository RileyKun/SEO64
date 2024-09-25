#pragma once

#include <cmath>

class vec2
{
public:
	float x{ 0.0f };
	float y{ 0.0f };

public:
	vec2()
	{
		x = 0.0f;
		y = 0.0f;
	}

	vec2(const float x, const float y)
	{
		this->x = x;
		this->y = y;
	}

	vec2(float *const v)
	{
		x = v[0];
		y = v[1];
	}

	vec2(const float *const v)
	{
		x = v[0];
		y = v[1];
	}

	vec2(const vec2 &v)
	{
		x = v.x;
		y = v.y;
	}

public:
	float &operator[](const int i)
	{
		return ((float *)this)[i];
	}

	float operator[](const int i) const
	{
		return ((float *)this)[i];
	}

public:
	vec2 &operator=(const vec2 &v)
	{
		x = v.x;
		y = v.y;

		return *this;
	}

	vec2 &operator+=(const vec2 &v)
	{
		x += v.x;
		y += v.y;

		return *this;
	}

	vec2 &operator-=(const vec2 &v)
	{
		x -= v.x;
		y -= v.y;

		return *this;
	}

	vec2 &operator*=(const vec2 &v)
	{
		x *= v.x;
		y *= v.y;

		return *this;
	}

	vec2 &operator/=(const vec2 &v)
	{
		x /= v.x;
		y /= v.y;

		return *this;
	}

	vec2 &operator+=(const float v)
	{
		x += v;
		y += v;

		return *this;
	}

	vec2 &operator-=(const float v)
	{
		x -= v;
		y -= v;

		return *this;
	}

	vec2 &operator*=(const float v)
	{
		x *= v;
		y *= v;

		return *this;
	}

	vec2 &operator/=(const float v)
	{
		x /= v;
		y /= v;

		return *this;
	}

	vec2 operator+(const vec2 &v) const
	{
		return { x + v.x, y + v.y };
	}

	vec2 operator-(const vec2 &v) const
	{
		return { x - v.x, y - v.y };
	}

	vec2 operator*(const vec2 &v) const
	{
		return { x * v.x, y * v.y };
	}

	vec2 operator/(const vec2 &v) const
	{
		return { x / v.x, y / v.y };
	}

	vec2 operator+(const float v) const
	{
		return { x + v, y + v };
	}

	vec2 operator-(const float v) const
	{
		return { x - v, y - v };
	}

	vec2 operator*(const float v) const
	{
		return { x * v, y * v };
	}

	vec2 operator/(const float v) const
	{
		return { x / v, y / v };
	}

public:
	void set(const float x = 0.0f, const float y = 0.0f)
	{
		this->x = x;
		this->y = y;
	}

	bool isZero() const
	{
		constexpr float epsilon{ 0.01f };

		return x > -epsilon && x < epsilon && y > -epsilon && y < epsilon;
	}

public:
	float length() const
	{
		return sqrtf(x * x + y * y);
	}

	float lengthSqr() const
	{
		return x * x + y * y;
	}

	float distTo(const vec2 &v) const
	{
		return (*this - v).length();
	}

	float distToSqr(const vec2 &v) const
	{
		return (*this - v).lengthSqr();
	}

	float dot(const vec2 &v) const
	{
		return x * v.x + y * v.y;
	}
};

class vec3
{
public:
	float x{ 0.0f };
	float y{ 0.0f };
	float z{ 0.0f };

public:
	vec3()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	vec3(const float x, const float y, const float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	vec3(float *const v)
	{
		x = v[0];
		y = v[1];
		z = v[2];
	}

	vec3(const float *const v)
	{
		x = v[0];
		y = v[1];
		z = v[2];
	}

	vec3(const vec3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

public:
	float &operator[](const int i)
	{
		return ((float *)this)[i];
	}

	float operator[](const int i) const
	{
		return ((float *)this)[i];
	}

public:
	vec3 &operator=(const vec3 &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;

		return *this;
	}

	vec3 &operator+=(const vec3 &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}

	vec3 &operator-=(const vec3 &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}

	vec3 &operator*=(const vec3 &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;

		return *this;
	}

	vec3 &operator/=(const vec3 &v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;

		return *this;
	}

	vec3 &operator+=(const float v)
	{
		x += v;
		y += v;
		z += v;

		return *this;
	}

	vec3 &operator-=(const float v)
	{
		x -= v;
		y -= v;
		z -= v;

		return *this;
	}

	vec3 &operator*=(const float v)
	{
		x *= v;
		y *= v;
		z *= v;

		return *this;
	}

	vec3 &operator/=(const float v)
	{
		x /= v;
		y /= v;
		z /= v;

		return *this;
	}

	vec3 operator+(const vec3 &v) const
	{
		return { x + v.x, y + v.y, z + v.z };
	}

	vec3 operator-(const vec3 &v) const
	{
		return { x - v.x, y - v.y, z - v.z };
	}

	vec3 operator*(const vec3 &v) const
	{
		return { x * v.x, y * v.y, z * v.z };
	}

	vec3 operator/(const vec3 &v) const
	{
		return { x / v.x, y / v.y, z / v.z };
	}

	vec3 operator+(const float v) const
	{
		return { x + v, y + v, z + v };
	}

	vec3 operator-(const float v) const
	{
		return { x - v, y - v, z - v };
	}

	vec3 operator*(const float v) const
	{
		return { x * v, y * v, z * v };
	}

	vec3 operator/(const float v) const
	{
		return { x / v, y / v, z / v };
	}

public:
	void set(const float x = 0.0f, const float y = 0.0f, const float z = 0.0f)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	bool isZero() const
	{
		constexpr float epsilon{ 0.01f };

		return x > -epsilon && x < epsilon && y > -epsilon && y < epsilon && z > -epsilon && z < epsilon;
	}

public:
	float length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float lengthSqr() const
	{
		return x * x + y * y + z * z;
	}

	float normalize()
	{
		const float len{ length() };
		const float len_normal{ 1.0f / (FLT_EPSILON + len) };

		x = x * len_normal;
		y = y * len_normal;
		z = z * len_normal;

		return len;
	}

	vec3 normalized()
	{
		normalize();

		return *this;
	}

	float length2D() const
	{
		return sqrtf(x * x + y * y);
	}

	float length2DSqr() const
	{
		return x * x + y * y;
	}

	float distTo(const vec3 &v) const
	{
		return (*this - v).length();
	}

	float distToSqr(const vec3 &v) const
	{
		return (*this - v).lengthSqr();
	}

	float dot(const vec3 &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}
public:
	vec3 cross(const vec3 &v) const
	{
		return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x };
	}

	vec3 scale(const float scale)
	{
		return { x * scale, y * scale, z * scale };
	}

	vec3 normalizeAngles()
	{
		auto normalizeAngle = [&](float& ang) -> void
		{
			float f = (ang * (1.f / 360.f)) + 0.5f;
			int i = (int)f;
			float fi = (float)i;
			f = (f < 0.f && f != fi) ? fi - 1.f : fi;
			ang -= f * 360.f;
		};

		normalizeAngle(x);
		normalizeAngle(y);
		normalizeAngle(z);

		return *this;
	}
};