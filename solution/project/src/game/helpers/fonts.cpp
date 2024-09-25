#include "fonts.hpp"

#include "../../app/features/cfg.hpp"

Font::Font(std::string_view name, const int tall, const int weight, const int flags) : InstTracker(this)
{
	m_name = name;
	m_tall = tall;
	m_weight = weight;
	m_flags = flags;
}

HFont *const Font::get()
{
	return &m_font;
}

float Font::getTall() const
{
	return static_cast<float>(m_tall);
}

vec2 Font::getTextSize(std::string_view text) const
{
	int w{};
	int h{};

	i::surface->GetTextSize(m_font, utils::utf8ToWide(text).c_str(), w, h);

	return { static_cast<float>(w), static_cast<float>(h) };
}

bool Font::init()
{
	m_font = i::surface->CreateFont();

	if (!m_font) {
		return false;
	}
	
	return i::surface->SetFontGlyphSet(m_font, m_name.c_str(), m_tall, m_weight, 0, 0, m_flags);
}