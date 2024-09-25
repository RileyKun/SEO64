#pragma once

#include "../tf/interfaces.hpp"

class Font final : public InstTracker<Font>
{
private:
	HFont m_font{};
	std::string m_name{};
	int m_tall{};
	int m_weight{};
	int m_flags{};

public:
	Font(std::string_view name, const int tall, const int weight, const int flags);

public:
	HFont *const get();
	float getTall() const;
	vec2 getTextSize(std::string_view text) const;
	bool init();
};

#define MAKE_FONT(var_name, font_name, tall, weight, flags) \
namespace fonts { inline Font var_name{ font_name, tall, weight, flags }; }

MAKE_FONT(esp, "Small Fonts", 11, 0, FONTFLAG_OUTLINE);
MAKE_FONT(esp_conds, "Small Fonts", 9, 0, FONTFLAG_OUTLINE);
MAKE_FONT(indicators, "Bahnschrift", 16, 0, FONTFLAG_ANTIALIAS);