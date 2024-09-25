#pragma once

#include <stdint.h>
#include <algorithm>
#include <cmath>

class Color final
{
public:
	Color() = default;
	Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

public:
	uint8_t r{};
	uint8_t g{};
	uint8_t b{};
	uint8_t a{};

public:
	Color scaled(const float scale) const;
	Color lerp(const Color &clr, const float t) const;

public:
	float getHue() const;
	float getSat() const;
	float getVal() const;
};