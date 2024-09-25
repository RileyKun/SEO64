#pragma once

#include "../../../game/game.hpp"

class AutoDetonate final
{
private:
	float m_live_time{}; // time until sticky is live (can be detonated) - see CTFGrenadePipebombProjectile::GetLiveTime

private:
    bool canSee(C_BaseEntity *const target, C_BaseEntity *const projectile, const float radius ) const;
	void detonate(CUserCmd *const cmd, C_TFPlayer *const local, C_TFGrenadePipebombProjectile *const sticky = nullptr );

	void handleFlares(CUserCmd *const cmd, C_TFPlayer *const local);
	void handleStickies(CUserCmd *const cmd, C_TFPlayer *const local);

public:
	void run(CUserCmd *const cmd);
};

MAKE_UNIQUE(AutoDetonate, auto_detonate);