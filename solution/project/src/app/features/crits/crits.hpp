#pragma once

#include "../../../game/game.hpp"

class Crits final : public HasLevelShutdown
{
private:
	enum class CritType
	{
		UNKNOWN,
		REGULAR,
		RAPIDFIRE,
		MELEE
	};

private:
	int m_crit_cmd{};
	int m_last_crit_cmd{};
	bool m_want_crit{};

private:
	int getWeaponMode(C_TFWeaponBase *const weapon) const;
	CritType getCritType(C_TFWeaponBase *const weapon) const;
	float getCritChance(C_TFPlayer *const local, C_TFWeaponBase *const weapon) const;
	float getDamage(C_TFWeaponBase *const weapon, const bool for_withdraw) const;
	bool isSeedCrit(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const int seed) const;
	int findCritCmdNum(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const int start_cmd_num, const bool want_crit);
	bool canWeaponRandomCrit(C_TFPlayer *const local, C_TFWeaponBase *const weapon) const;

private:
	void addToCritBucket(C_TFWeaponBase *const weapon, const float amount);
	bool isAllowedToWithdrawFromCritBucket(C_TFWeaponBase *const weapon, const float amount);

public:
	bool onLevelShutdown() override;

public:
	bool think(bool want_crit);

public:
	void run(CUserCmd *const cmd);
	void paint();

public:
	int getCritCmd() const;
	bool getWantCrit() const;
};

MAKE_UNIQUE(Crits, crits);