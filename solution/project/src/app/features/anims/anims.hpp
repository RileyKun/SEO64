#pragma once

#include "../../../game/game.hpp"

class Anims final : public HasLoad, public HasLevelInit, public HasLevelShutdown, public HasUnload
{
public:
	CTFPlayerAnimState* m_fake_animstate{};
	matrix3x4_t* m_fake_bones{};
	vec3 m_fake_origin{};
	int m_last_sent_tickbase{};
	std::array<float, 24> m_fake_poses{};
public:
	std::array<matrix3x4_t, 128> m_local_bones{};
	Vector m_bone_origin{};
	bool m_allow_local_bone_setup{};
public:
	void onNetworkUpdate(C_TFPlayer *const pl);
	void onRunSimulation(C_BasePlayer* const base_pl, CUserCmd* const cmd, float curtime);
	void postCreateMove();
public:
	bool onLoad() override;
	bool onLevelInit() override;
	bool onUnload() override;
	bool onLevelShutdown() override;
};

MAKE_UNIQUE(Anims, anims);