#include "color.hpp"

Color::Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

Color Color::scaled(const float scale) const
{
	return
	{
		static_cast<uint8_t>(static_cast<float>(r) * scale),
		static_cast<uint8_t>(static_cast<float>(g) * scale),
		static_cast<uint8_t>(static_cast<float>(b) * scale),
		a
	};
}

Color Color::lerp(const Color &clr, const float t) const
{
	return
	{
		static_cast<uint8_t>(std::clamp(static_cast<int>(r + t * (clr.r - r)), 0, 255)),
		static_cast<uint8_t>(std::clamp(static_cast<int>(g + t * (clr.g - g)), 0, 255)),
		static_cast<uint8_t>(std::clamp(static_cast<int>(b + t * (clr.b - b)), 0, 255)),
		static_cast<uint8_t>(std::clamp(static_cast<int>(a + t * (clr.a - a)), 0, 255))
	};
}

float Color::getHue() const
{
	const float lowest{ static_cast<float>(std::min(r, std::min(g, b))) };
	const float highest{ static_cast<float>(std::max(r, std::max(g, b))) };

	if (lowest == highest) {
		return 0.0f;
	}

	float hue{};

	if (highest == r) {
		hue = (g - b) / (highest - lowest);
	}

	else if (highest == g) {
		hue = 2.0f + (b - r) / (highest - lowest);
	}

	else{
		hue = 4.0f + (r - g) / (highest - lowest);
	}

	hue *= 60.0f;

	if (hue < 0.0f) {
		hue += 360.0f;
	}

	return std::round(hue);
}

float Color::getSat() const
{
	const float lowest{ static_cast<float>(std::min(r, std::min(g, b))) };
	const float highest{ static_cast<float>(std::max(r, std::max(g, b))) };

	return highest == 0.0f ? 0.0f : 1.0f - lowest / highest;
}

float Color::getVal() const
{
	return std::max(r, std::max(g, b)) / 255.0f;
}