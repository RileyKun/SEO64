#pragma once

#include "../tf/interfaces.hpp"

constexpr int POS_DEFAULT{ 0 };
constexpr int POS_LEFT{ 1 << 1 };
constexpr int POS_TOP{ 1 << 2 };
constexpr int POS_CENTERX{ 1 << 3 };
constexpr int POS_CENTERY{ 1 << 4 };
constexpr int POS_CENTERXY{ POS_CENTERX | POS_CENTERY };

class Draw final
{
private:
	VMatrix m_w2s_mat{};

public:
	void start();
	void end();
	void updateW2S();
	vec2 getScreenSize();
	bool worldToScreen(const vec3 &world, vec2 &screen);
	bool clipTransformWithProjection(const matrix3x4_t &w2s_mat, const vec3 &point, vec2 &clip);
	bool clipTransform(const vec3 &point, vec2 &clip);
	bool screenPosition(const vec3 &world, vec2 &screen);
	HFont getDefaultFont();

public:
	void startClip(const vec2 &pos, const vec2 &size);
	void endClip();

public:
	void line(const vec2 &from, const vec2 &to, const Color clr);
	void rect(const vec2 &pos, const vec2 &size, const Color clr);
	void rectFilled(const vec2 &pos, const vec2 &size, const Color clr);
	void rectFadeFilled(const vec2 &pos, const vec2 &size, const Color clr_a, const Color clr_b, const bool horizontal = false);
	void rectHSV(const vec2 &pos, const vec2 &size, const Color clr);
	void circle(const vec2 pos, const int radius, const Color clr);
	void circleFilled(const vec2 pos, const int radius, const Color clr);
	void filledPolygon(std::vector<Vertex_t> vertices, const Color clr);
	void filledTriangle(const vec2 &pos, const float size, const float angle, const Color clr);
	void string(const vec2 &pos, HFont *const font, std::string_view str, const Color clr, const int align = 0);
	void string(const vec2 &pos, HFont *const font, std::wstring_view str, const Color clr, const int align = 0);
	void stringOutlined(const vec2 &pos, HFont *const font, std::string_view str, const Color clr, const int align = 0);
	void stringOutlined(const vec2 &pos, HFont *const font, std::wstring_view str, const Color clr, const int align = 0);
	void texture(const vec2 &pos, const vec2 &size, const int texture_id, const Color clr, const int align = 0);
	void texture(const vec2 &pos, const vec2 &size, const int texture_id, const int align = 0);
	void texturedSubrect(const vec2 &pos, const vec2 &size, const int texture_id, float subrect[4], const Color clr);
	void hudTexture(CHudTexture *const texture, const vec2 &pos, const vec2 &size, const Color clr);
};

MAKE_UNIQUE(Draw, draw);