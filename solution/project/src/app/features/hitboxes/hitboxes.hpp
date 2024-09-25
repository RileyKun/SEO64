#pragma once

#include "../player_data/player_data.hpp"

class Hitboxes final : public HasLevelShutdown
{
private:
	struct Hitbox final
	{
		vec3 pos{};
		vec3 ang{};
		vec3 min{};
		vec3 max{};
		Color col{};
		float expire_time{};
		bool target{};
	};

private:
	std::vector<Hitbox> m_hitboxes{};

public:
	bool onLevelShutdown() override;

private:
	bool getHitboxes(C_BaseAnimating *const animating, matrix3x4_t *const bones, std::vector<Hitbox> &out, const Color col, const bool no_expire = false);
	void renderHitbox(const Hitbox &hbox);

public:
	void renderHitboxesLR(const LagRecord *const lr);
	void renderFakeHitboxes();
public:
	void add(C_BaseAnimating *const animating, matrix3x4_t *const bones);
	void render();
};

MAKE_UNIQUE(Hitboxes, hitboxes);