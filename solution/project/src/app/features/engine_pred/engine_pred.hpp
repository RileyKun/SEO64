#pragma once

#include "../../../game/game.hpp"

struct datamap_copy final
{
	uint8_t *data{};
	size_t size{};
};

class EnginePred final : public HasUnload, public HasLevelShutdown
{
private:
	float m_og_curtime{};
	float m_og_frametime{};
	bool m_og_in_prediction{};
	bool m_og_first_prediction{};
private:
	bool m_in_engine_pred{};
	datamap_copy m_local_copy{};
	datamap_copy m_weapon_copy{};
public:
	int current_prediction_cmd{};
public:
	void restoreEntityToPredictedFrame(int pred_frame) const;

public:
	void update();

public:
	void suppressEvents(C_BaseEntity *ent);
	[[nodiscard]] bool start(CUserCmd *const cmd);
	void runCommand(C_TFPlayer *const local, CUserCmd *const cmd);
	void end() const;
public:
	void edgeJump(CUserCmd *const cmd);

public:
	bool isInEnginePred() const { return m_in_engine_pred; }

public:
	bool onUnload() override;
	bool onLevelShutdown() override;
};

MAKE_UNIQUE(EnginePred, engine_pred);

#define RUN_PREDICTION(cmd, func) \
if (engine_pred->start(cmd)) { \
	func(); \
	engine_pred->end(); \
} \
