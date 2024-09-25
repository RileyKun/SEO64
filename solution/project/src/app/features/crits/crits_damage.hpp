#pragma once

#include "../../../game/game.hpp"

class HealthSyncData final
{
public:
	int m_old_health{};
	int m_new_health{};
	int m_current_health{};
	int m_health_difference{};

public:
	void onNetvarUpdate(const CRecvProxyData *const data, void *const out);
};

class CritsDamage final : public HasGameEvent
{
private:
	std::unordered_map<unsigned long, HealthSyncData> m_health_sync{};

private:
	int m_last_attacked_weapon_id{};

private:
	float m_damage_ranged_crit_random{};
	float m_damage_ranged_crit_boosted{};
	float m_damage_ranged{};
	float m_damage{};

private:
	float m_damage_melee{};
	bool m_last_damage_was_melee{};

public:
	HealthSyncData &getHealthSync(C_BasePlayer *const ent);

public:
	float getDamageRangedCritRandom() const;
	float getDamageRanged() const;
	float getDamage() const;
	float getDamageMelee() const;

public:
	void updateTotalDamage(const int old_damage, const int new_damage);

public:
	float getObservedCriticalChance() const;
	float calculateNonCritDamage(const float desired_crit_chance) const;
	bool canFireRandomCriticalShot(const float crit_chance) const;

public:
	bool customDamageTypeSuggestsNotMelee(int custom_damage_type) const;
	void updateLastWeaponId(C_TFWeaponBase *const weapon);

public:
	bool onGameEvent(IGameEvent *const event) override;
};

MAKE_UNIQUE(CritsDamage, crits_dmg);