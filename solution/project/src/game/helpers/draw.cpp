#include "draw.hpp"

void Draw::start()
{
	i::surface->StartDrawing();
	i::surface->DisableClipping(true);
}

void Draw::end()
{
	i::surface->FinishDrawing();
}

void Draw::updateW2S()
{
	CViewSetup view{};

	if (!i::client->GetPlayerView(view)) {
		return;
	}

	VMatrix world_to_view{};
	VMatrix view_to_projection{};
	VMatrix world_to_pixels{};

	i::render_view->GetMatricesForView(view, &world_to_view, &view_to_projection, &m_w2s_mat, &world_to_pixels);
}

vec2 Draw::getScreenSize()
{
	return vec2
	{
		static_cast<float>(i::client->GetScreenWidth()),
		static_cast<float>(i::client->GetScreenHeight())
	};
}

bool Draw::worldToScreen(const vec3 &world, vec2 &screen)
{
	const matrix3x4_t &w2s{ m_w2s_mat.As3x4() };

#pragma warning (push)
#pragma warning (disable : 6385)

	const float w{ w2s[3][0] * world[0] + w2s[3][1] * world[1] + w2s[3][2] * world[2] + w2s[3][3] };

#pragma warning (pop)

	if (w <= 0.001f) {
		return false;
	}

	const vec2 ss{ getScreenSize() };

	screen.x = (ss.x / 2.0f) + (0.5f * ((w2s[0][0] * world[0] + w2s[0][1] * world[1] + w2s[0][2] * world[2] + w2s[0][3]) * (1.0f / w)) * ss.x + 0.5f);
	screen.y = (ss.y / 2.0f) - (0.5f * ((w2s[1][0] * world[0] + w2s[1][1] * world[1] + w2s[1][2] * world[2] + w2s[1][3]) * (1.0f / w)) * ss.y + 0.5f);

	return true;
}

bool Draw::clipTransformWithProjection(const matrix3x4_t &w2s_mat, const vec3 &point, vec2 &clip)
{
	clip.x = w2s_mat[0][0] * point[0] + w2s_mat[0][1] * point[1] + w2s_mat[0][2] * point[2] + w2s_mat[0][3];
	clip.y = w2s_mat[1][0] * point[0] + w2s_mat[1][1] * point[1] + w2s_mat[1][2] * point[2] + w2s_mat[1][3];

#pragma warning (push)
#pragma warning (disable : 6385)
	const float w{ w2s_mat[3][0] * point[0] + w2s_mat[3][1] * point[1] + w2s_mat[3][2] * point[2] + w2s_mat[3][3] };
#pragma warning (pop)

	const bool behind{ w < 0.001f };

	if (behind) {
		clip.x *= 100000.0f;
		clip.y *= 100000.0f;
	}

	else {
		clip.x *= (1.0f / w);
		clip.y *= (1.0f / w);
	}

	return behind;
}

bool Draw::clipTransform(const vec3 &point, vec2 &clip)
{
	return clipTransformWithProjection(m_w2s_mat.As3x4(), point, clip);
}

bool Draw::screenPosition(const vec3 &world, vec2 &screen)
{
	const int result{ clipTransform(world, screen) };

	const vec2 ss{ getScreenSize() };

	screen.x = (0.5f * screen.x * ss.x) + 0.5f * ss.x;
	screen.y = (-0.5f * screen.y * ss.y) + 0.5f * ss.y;

	return result;
}

HFont Draw::getDefaultFont()
{
	static HFont default_font{};

	if (!default_font) {
		default_font = i::surface->CreateFont();
		i::surface->SetFontGlyphSet(default_font, "Arial", 16, 0, 0, 0, FONTFLAG_OUTLINE);
	}

	return default_font;
}

void Draw::startClip(const vec2 &pos, const vec2 &size)
{
	i::surface->DisableClipping(false);

	i::surface->SetClippingRect
	(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		static_cast<int>(pos.x + size.x),
		static_cast<int>(pos.y + size.y)
	);
}

void Draw::endClip()
{
	i::surface->DisableClipping(true);
}

void Draw::line(const vec2 &from, const vec2 &to, const Color clr)
{
	i::surface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);

	i::surface->DrawLine
	(
		static_cast<int>(from.x),
		static_cast<int>(from.y),
		static_cast<int>(to.x),
		static_cast<int>(to.y)
	);
}

void Draw::rect(const vec2 &pos, const vec2 &size, const Color clr)
{
	i::surface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);

	i::surface->DrawOutlinedRect
	(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		static_cast<int>(pos.x + size.x),
		static_cast<int>(pos.y + size.y)
	);
}

void Draw::rectFilled(const vec2 &pos, const vec2 &size, const Color clr)
{
	i::surface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);

	i::surface->DrawFilledRect
	(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		static_cast<int>(pos.x + size.x),
		static_cast<int>(pos.y + size.y)
	);
}

void Draw::rectFadeFilled(const vec2 &pos, const vec2 &size, const Color clr_a, const Color clr_b, const bool horizontal)
{
	i::surface->DrawSetColor(clr_a.r, clr_a.g, clr_a.b, clr_a.a);

	i::surface->DrawFilledRectFade
	(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		static_cast<int>(pos.x + size.x),
		static_cast<int>(pos.y + size.y),
		255,
		255,
		horizontal
	);

	i::surface->DrawSetColor(clr_b.r, clr_b.g, clr_b.b, clr_b.a);

	i::surface->DrawFilledRectFade
	(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		static_cast<int>(pos.x + size.x),
		static_cast<int>(pos.y + size.y),
		0,
		255,
		horizontal
	);
}

void Draw::rectHSV(const vec2 &pos, const vec2 &size, const Color clr)
{
	Draw::rectFilled(pos, size, { 0, 0, 0, 255 });

	const int x{ static_cast<int>(pos.x) };
	const int y{ static_cast<int>(pos.y) };
	const int w{ static_cast<int>(size.x) };
	const int h{ static_cast<int>(size.y) };

	i::surface->DrawSetColor(255, 255, 255, 255);
	i::surface->DrawFilledRectFade(x, y, x + w, y + h, 255, 0, true);

	i::surface->DrawSetColor(clr.r, clr.g, clr.b, 255);
	i::surface->DrawFilledRectFade(x, y, x + w, y + h, 0, 255, true);
	i::surface->DrawFilledRectFade(x, y, x + w, y + h, 0, 128, true);

	i::surface->DrawSetColor(0, 0, 0, 255);
	i::surface->DrawFilledRectFade(x, y, x + w, y + h, 0, 255, false);
	i::surface->DrawFilledRectFade(x, y, x + w, y + h, 0, 255, false);
	i::surface->DrawFilledRectFade(x, y, x + w, y + h, 0, 128, false);
}

void Draw::circle(const vec2 pos, const int radius, const Color clr)
{
	i::surface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);

	i::surface->DrawOutlinedCircle
	(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		radius,
		100
	);
}

void Draw::circleFilled(const vec2 pos, const int radius, const Color clr)
{
	std::vector<Vertex_t> vertices{};

	vertices.reserve(100);

	constexpr float step{ math::pi() * 2.0f / 100.0f };

	for (int i{}; i < 100; i++) {
		const float angle{ step * i };
		vertices.emplace_back(Vertex_t{ vec2{ pos.x + cosf(angle) * radius, pos.y + sinf(angle) * radius } });
	}

	filledPolygon(vertices, clr);
}

void Draw::filledPolygon(std::vector<Vertex_t> vertices, const Color clr)
{
	static int texture_id{ i::surface->CreateNewTextureID(true) };

	i::surface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);
	i::surface->DrawSetTexture(texture_id);
	i::surface->DrawTexturedPolygon(static_cast<int>(vertices.size()), vertices.data(), true);
}

void Draw::filledTriangle(const vec2 &pos, const float size, const float angle, const Color clr)
{
	std::vector<Vertex_t> tri
	{
		{{ pos.x, pos.y - size }},
		{{ pos.x - size, pos.y + size }},
		{{ pos.x + size, pos.y + size }}
	};

	const float rad{ math::degToRad(angle) };
	const float s{ std::sinf(rad) };
	const float c{ std::cosf(rad) };

	for (int i{}; i < 3; i++)
	{
		tri[i].m_Position -= pos;

		const float xnew{ tri[i].m_Position.x * c - tri[i].m_Position.y * s };
		const float ynew{ tri[i].m_Position.x * s + tri[i].m_Position.y * c };

		tri[i].m_Position.x = xnew + pos.x;
		tri[i].m_Position.y = ynew + pos.y;
	}

	filledPolygon(tri, clr);
}

void Draw::string(const vec2 &pos, HFont *const font, std::string_view str, const Color clr, const int align)
{
	const HFont str_font{ font ? *font : getDefaultFont() };

	wchar_t wstr[1024]{ '\0' };

	wsprintfW(wstr, L"%hs", str.data());

	int x{ static_cast<int>(pos.x) };
	int y{ static_cast<int>(pos.y) };

	if (align)
	{
		int w{};
		int h{};

		i::surface->GetTextSize(str_font, wstr, w, h);

		if (align & POS_LEFT) {
			x -= w;
		}

		if (align & POS_TOP) {
			y -= h;
		}

		if (align & POS_CENTERX) {
			x -= w / 2;
		}

		if (align & POS_CENTERY) {
			y -= h / 2;
		}
	}

	i::surface->DrawSetTextPos(x, y);
	i::surface->DrawSetTextFont(str_font);
	i::surface->DrawSetTextColor(clr.r, clr.g, clr.b, clr.a);
	i::surface->DrawPrintText(wstr, static_cast<int>(wcslen(wstr)));
}

void Draw::string(const vec2 &pos, HFont *const font, std::wstring_view str, const Color clr, const int align)
{
	const HFont str_font{ (font && *font) ? *font : getDefaultFont() };

	int x{ static_cast<int>(pos.x) };
	int y{ static_cast<int>(pos.y) };

	if (align)
	{
		int w{};
		int h{};

		i::surface->GetTextSize(str_font, str.data(), w, h);

		if (align & POS_LEFT) {
			x -= w;
		}

		if (align & POS_TOP) {
			y -= h;
		}

		if (align & POS_CENTERX) {
			x -= w / 2;
		}

		if (align & POS_CENTERY) {
			y -= h / 2;
		}
	}

	i::surface->DrawSetTextPos(x, y);
	i::surface->DrawSetTextFont(str_font);
	i::surface->DrawSetTextColor(clr.r, clr.g, clr.b, clr.a);
	i::surface->DrawPrintText(str.data(), static_cast<int>(wcslen(str.data())));
}

void Draw::stringOutlined(const vec2 &pos, HFont *const font, std::string_view str, const Color clr, const int align)
{
	const HFont str_font{ font ? *font : getDefaultFont() };

	wchar_t wstr[1024]{ '\0' };

	wsprintfW(wstr, L"%hs", str.data());

	int x{ static_cast<int>(pos.x) };
	int y{ static_cast<int>(pos.y) };

	if (align)
	{
		int w{};
		int h{};

		i::surface->GetTextSize(str_font, wstr, w, h);

		if (align & POS_LEFT) {
			x -= w;
		}

		if (align & POS_TOP) {
			y -= h;
		}

		if (align & POS_CENTERX) {
			x -= w / 2;
		}

		if (align & POS_CENTERY) {
			y -= h / 2;
		}
	}

	const int offsets[8][2]
	{
		{ -1, -1 },	{ 0, -1 },	{ 1, -1 },
		{ -1, 0 },				{ 1, 0 },
		{ -1, 1 },	{ 0, 1 },	{ 1, 1 }
	};

	const int len{ static_cast<int>(wcslen(wstr)) };

	i::surface->DrawSetTextFont(str_font);
	i::surface->DrawSetTextColor(0, 0, 0, 255);

	for (int n{}; n < 8; n++) {
		i::surface->DrawSetTextPos(x + offsets[n][0], y + offsets[n][1]);
		i::surface->DrawPrintText(wstr, len);
	}

	i::surface->DrawSetTextColor(clr.r, clr.g, clr.b, clr.a);
	i::surface->DrawSetTextPos(x, y);
	i::surface->DrawPrintText(wstr, len);
}

void Draw::stringOutlined(const vec2 &pos, HFont *const font, std::wstring_view str, const Color clr, const int align)
{
	const HFont str_font{ (font && *font) ? *font : getDefaultFont() };

	int x{ static_cast<int>(pos.x) };
	int y{ static_cast<int>(pos.y) };

	if (align)
	{
		int w{};
		int h{};

		i::surface->GetTextSize(str_font, str.data(), w, h);

		if (align & POS_LEFT) {
			x -= w;
		}

		if (align & POS_TOP) {
			y -= h;
		}

		if (align & POS_CENTERX) {
			x -= w / 2;
		}

		if (align & POS_CENTERY) {
			y -= h / 2;
		}
	}

	const int offsets[8][2]
	{
		{ -1, -1 },	{ 0, -1 },	{ 1, -1 },
		{ -1, 0 },				{ 1, 0 },
		{ -1, 1 },	{ 0, 1 },	{ 1, 1 }
	};

	const int len{ static_cast<int>(wcslen(str.data())) };

	i::surface->DrawSetTextFont(str_font);
	i::surface->DrawSetTextColor(0, 0, 0, 255);

	for (int n{}; n < 8; n++) {
		i::surface->DrawSetTextPos(x + offsets[n][0], y + offsets[n][1]);
		i::surface->DrawPrintText(str.data(), len);
	}

	i::surface->DrawSetTextColor(clr.r, clr.g, clr.b, clr.a);
	i::surface->DrawSetTextPos(x, y);
	i::surface->DrawPrintText(str.data(), len);
}

void Draw::texture(const vec2 &pos, const vec2 &size, const int texture_id, const Color clr, const int align)
{
	int x{ static_cast<int>(pos.x) };
	int y{ static_cast<int>(pos.y) };
	int w{ static_cast<int>(size.x) };
	int h{ static_cast<int>(size.y) };

	if (align)
	{
		if (align & POS_LEFT) {
			x -= w;
		}

		if (align & POS_TOP) {
			y -= h;
		}

		if (align & POS_CENTERX) {
			x -= w / 2;
		}

		if (align & POS_CENTERY) {
			y -= h / 2;
		}
	}

	i::surface->DrawSetTexture(texture_id);
	i::surface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);
	i::surface->DrawTexturedRect(x, y, x + w, y + h);
}

void Draw::texture(const vec2 &pos, const vec2 &size, const int texture_id, const int align)
{
	int x{ static_cast<int>(pos.x) };
	int y{ static_cast<int>(pos.y) };
	int w{ static_cast<int>(size.x) };
	int h{ static_cast<int>(size.y) };

	if (align)
	{
		if (align & POS_LEFT) {
			x -= w;
		}

		if (align & POS_TOP) {
			y -= h;
		}

		if (align & POS_CENTERX) {
			x -= w / 2;
		}

		if (align & POS_CENTERY) {
			y -= h / 2;
		}
	}

	i::surface->DrawSetTexture(texture_id);
	i::surface->DrawSetColor(255, 255, 255, 255);
	i::surface->DrawTexturedRect(x, y, x + w, y + h);
}

void Draw::texturedSubrect(const vec2 &pos, const vec2 &size, const int texture_id, float subrect[4], const Color clr)
{
	i::surface->DrawSetTexture(texture_id);
	i::surface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);

	i::surface->DrawTexturedSubRect
	(
		static_cast<int>(pos.x),
		static_cast<int>(pos.y),
		static_cast<int>(pos.x + size.x),
		static_cast<int>(pos.y + size.y),
		subrect[0],
		subrect[1],
		subrect[2],
		subrect[3]
	);
}

void Draw::hudTexture(CHudTexture *const texture, const vec2 &pos, const vec2 &size, const Color clr)
{
	if (!texture) {
		return;
	}

	if (texture->bRenderUsingFont)
	{
		i::surface->DrawSetTextFont(texture->hFont);
		i::surface->DrawSetTextColor(clr.r, clr.g, clr.b, clr.a);
		i::surface->DrawSetTextPos(static_cast<int>(pos.x), static_cast<int>(pos.y));
		i::surface->DrawUnicodeChar(texture->cCharacterInFont);
	}

	else
	{
		if (texture->textureId == -1) {
			return;
		}

		i::surface->DrawSetTexture(texture->textureId);
		i::surface->DrawSetColor(clr.r, clr.g, clr.b, clr.a);

		i::surface->DrawTexturedSubRect
		(
			static_cast<int>(pos.x), 
			static_cast<int>(pos.y), 
			static_cast<int>(pos.x + size.x), 
			static_cast<int>(pos.y + size.y),
			texture->texCoords[0], 
			texture->texCoords[1], 
			texture->texCoords[2], 
			texture->texCoords[3]
		);
	}
}