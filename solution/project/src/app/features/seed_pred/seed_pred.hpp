#pragma once

#include "../../../game/game.hpp"

class SeedPred final : public HasLevelShutdown, public HasLoad
{
private:
	float m_send_time{};
	float m_recv_time{};
	bool m_waiting_for_recv{};
	float m_pp_time{};
public:
	std::array<vec2, UINT8_MAX + 1> m_spread_offsets{};
	bool m_spread_init{};
public:
	bool onLevelShutdown() override;
	bool onLoad() override;
private:
	void sendPlayerPerf();

public:
	void recvPlayerPerf(bf_read &msg_data);

private:
	float predictServerTime() const;
	int getSeed(const float time) const;
	void findBestSolution(C_TFPlayer* pl, CUserCmd *cmd, const float wpn_spread, const bool perfect_shot, const int bullets_per_shot);
public:
	float step();

public:
	void run(CUserCmd *const cmd);
	void paint();
	void onPostDataUpdate(C_TEFireBullets *tempent);
};

MAKE_UNIQUE(SeedPred, seed_pred);