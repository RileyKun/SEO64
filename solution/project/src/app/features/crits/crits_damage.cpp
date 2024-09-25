#include "crits_damage.hpp"

#include "../aim_utils/aim_utils.hpp"

//#define DEBUG_CRITS

MAKE_NETVAR_HOOK(critHealthTracker, "DT_BasePlayer", "m_iHealth")
{
	C_BasePlayer *const pl{ static_cast<C_BasePlayer *>(pStruct) };

	if (pl) {
		crits_dmg->getHealthSync(pl).onNetvarUpdate(pData, pOut);
	}

	CALL_ORIGINAL_PROXY(pData, pStruct, pOut);
}

MAKE_NETVAR_ARRAY_HOOK(playerResourceDamage, "DT_TFPlayerResource", "m_iDamage")
{
	if (GET_ARRAY_INDEX(int) == i::engine->GetLocalPlayer()) {
		crits_dmg->updateTotalDamage(*reinterpret_cast<int *>(pOut), pData->m_Value.m_Int);
	}

	CALL_ORIGINAL_PROXY(pData, pStruct, pOut);
}

void HealthSyncData::onNetvarUpdate(const CRecvProxyData *const data, void *const out)
{
	if (!data) {
		return;
	}

	m_old_health = *reinterpret_cast<int *>(out);
	m_new_health = data->m_Value.m_Int;
	m_health_difference = m_new_health - m_old_health;

	if (m_new_health > m_current_health) {
		m_current_health = m_new_health;
	}
}

HealthSyncData &CritsDamage::getHealthSync(C_BasePlayer *const ent)
{
	const unsigned long ent_index{ ent->GetRefEHandle().GetIndex() };

	if (m_health_sync.find(ent_index) == m_health_sync.end()) {
		m_health_sync[ent_index] = HealthSyncData{};
		m_health_sync[ent_index].m_current_health = ent->m_iHealth();
	}

	return m_health_sync[ent_index];
}

float CritsDamage::getDamageRangedCritRandom() const
{
	return m_damage_ranged_crit_random;
}

float CritsDamage::getDamageRanged() const
{
	return m_damage_ranged;
}

float CritsDamage::getDamage() const
{
	return m_damage;
}

float CritsDamage::getDamageMelee() const
{
	return m_damage_melee;
}

void CritsDamage::updateTotalDamage(const int old_damage, const int new_damage)
{
	if (new_damage > old_damage && m_last_damage_was_melee) {
		m_damage_melee += new_damage - old_damage;
	}

	m_damage_ranged = new_damage - m_damage_melee;
}

float CritsDamage::getObservedCriticalChance() const
{
	if (m_damage_ranged <= 0.0f || m_damage_ranged_crit_random <= 0.0f) {
		return 0.0f;
	}

	const float normalized_damage{ m_damage_ranged_crit_random / TF_DAMAGE_CRIT_MULTIPLIER };

	return normalized_damage / (normalized_damage + (m_damage_ranged - m_damage_ranged_crit_random));
}

float CritsDamage::calculateNonCritDamage(const float desired_crit_chance) const
{
	if (desired_crit_chance <= 0.0f || m_damage_ranged_crit_random <= 0.0f) {
		return -1.0f;
	}

	const float normalized_damage{ m_damage_ranged_crit_random / TF_DAMAGE_CRIT_MULTIPLIER };

	return (normalized_damage / desired_crit_chance) + (2.0f * normalized_damage) - m_damage_ranged;
}

bool CritsDamage::canFireRandomCriticalShot(const float crit_chance) const
{
	if (getObservedCriticalChance() > crit_chance + 0.1f) {
		return false;
	}

	return true;
}

bool CritsDamage::customDamageTypeSuggestsNotMelee(int custom_damage_type) const
{
	// mfed: majority of cases shouldn't even be able to happen, like the taunts for example but worth doing.
	// testing these is a PITA

	switch (custom_damage_type)
	{
		case TF_DMG_CUSTOM_BLEEDING: // bleed in general counts as ranged
		case TF_DMG_CUSTOM_HEADSHOT: // any headshot
		case TF_DMG_CUSTOM_MINIGUN: // heavy minigun
		case TF_DMG_CUSTOM_TAUNTATK_HADOUKEN: // pyro taunt, can be melee with 
		case TF_DMG_CUSTOM_BURNING: // fire in general counts as ranged
		case TF_DMG_CUSTOM_BURNING_FLARE: // flare gun
		case TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON: // pootis POW haha
		case TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM: // scout melee taunt
		case TF_DMG_CUSTOM_PENETRATE_MY_TEAM: // machina
		case TF_DMG_CUSTOM_PENETRATE_ALL_PLAYERS: // machina
		case TF_DMG_CUSTOM_TAUNTATK_FENCING: // spy melee taunt
		case TF_DMG_CUSTOM_PENETRATE_NONBURNING_TEAMMATE: // sydney sleeper
		case TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB: // huntsman
		case TF_DMG_CUSTOM_TELEFRAG: // bot_mimic my beloved
		case TF_DMG_CUSTOM_BURNING_ARROW:
		case TF_DMG_CUSTOM_FLYINGBURN: // also burning arrow
		case TF_DMG_CUSTOM_PUMPKIN_BOMB:
		case TF_DMG_CUSTOM_BASEBALL: // sandman
		case TF_DMG_CUSTOM_CHARGE_IMPACT: // shield bash
		case TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING: // eyelander
		case TF_DMG_CUSTOM_AIR_STICKY_BURST: // bomb that was in the air
		case TF_DMG_CUSTOM_DEFENSIVE_STICKY: // scottish resistance
		case TF_DMG_CUSTOM_ROCKET_DIRECTHIT: // direct hit
		case TF_DMG_CUSTOM_STANDARD_STICKY: // stickybomb launcher
		case TF_DMG_CUSTOM_SHOTGUN_REVENGE_CRIT: // revenge crits
		case TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH: // frontier justice
		case TF_DMG_CUSTOM_TAUNTATK_UBERSLICE: // ubersaw
		case TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KIL: // gunslinger
		case TF_DMG_CUSTOM_PLAYER_SENTRY: // wrangler on non mini sentry
		case TF_DMG_CUSTOM_FLARE_EXPLOSION: // you cant det on detonator with melee out
		case TF_DMG_CUSTOM_BOOTS_STOMP: // mantreads
		case TF_DMG_CUSTOM_PLASMA: // bison, pomson, cow mangler - energy weapons
		case TF_DMG_CUSTOM_PLASMA_CHARGED: // cow mangler charged shot
		case TF_DMG_CUSTOM_HEADSHOT_DECAPITATION: // bazaar bargain
		case TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON: // pyroland boom circle taunt
		case TF_DMG_CUSTOM_FLARE_PELLET: // good luck switching to melee in time for this to hit, i mean theoretically you can but this will count as ranged damage anyway
		case TF_DMG_CUSTOM_CLEAVER: // flying guillotine
		case TF_DMG_CUSTOM_CLEAVER_CRIT: // flying guillotine crit
		case TF_DMG_CUSTOM_CANNONBALL_PUSH:
		case TF_DMG_CUSTOM_SPELL_TELEPORT:		// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_SKELETON:		// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_MIRV:			// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_METEOR:		// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_LIGHTNING:		// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_FIREBALL:		// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_MONOCULUS:		// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_BLASTJUMP:		// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_BATS:			// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_SPELL_TINY:			// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_KART:				// untested, but probably counts as ranged damage
		case TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE: // kinda says it in the name
		case TF_DMG_CUSTOM_DRAGONS_FURY_BONUS_BURNING: // ^
		case TF_DMG_CUSTOM_RUNE_REFLECT: // reflect in mann power is a whole ass particle - adds to ranged
		case TF_DMG_CUSTOM_TAUNTATK_GASBLAST: {
			return true;
		}

		default: {
			return false;
		}
	}

	/* Remaining cases - unless specified, these count as melee and are handled by the default case.
	case TF_DMG_CUSTOM_BACKSTAB:
	case TF_DMG_CUSTOM_DECAPITATION:
	case TF_DMG_CUSTOM_TAUNTATK_GRENADE: // interestingly, seems to not use your melee weapon anyway?:
	case TF_DMG_CUSTOM_PICKAXE:
	case TF_DMG_CUSTOM_GOLD_WRENCH: // dont have one of these to test :P
	case TF_DMG_CUSTOM_CARRIED_BUILDING: // literally what
	case TF_DMG_CUSTOM_COMBO_PUNCH: // gunslinger 3rd hit
	case TF_DMG_CUSTOM_FISH_KILL: // never happens, seems to be related to disguised players? but cant get it to work
	case TF_DMG_CUSTOM_TRIGGER_HURT: // i dont think we will ever become a trigger_hurt entity as much as i want to
	case TF_DMG_CUSTOM_DECAPITATION_BOSS: // TODO:
	case TF_DMG_CUSTOM_STICKBOMB_EXPLOSION: // TODO: Tested this, seems to sometimes split into two damage events: one counting as ranged on the server: the other as melee
	case TF_DMG_CUSTOM_AEGIS_ROUND: // unused
	case TF_DMG_CUSTOM_PLASMA_GIB: // unused
	case TF_DMG_CUSTOM_PRACTICE_STICKY: // doesnt do damage
	case TF_DMG_CUSTOM_EYEBALL_ROCKET: // we cant be the eye, unless server plugin? probably not worth pursuing
	case TF_DMG_CUSTOM_SAPPER_RECORDER_DEATH: // building damage, doesn't affect any metric
	case TF_DMG_CUSTOM_MERASMUS_PLAYER_BOMB: // TODO: don't know the name of the map where he does prop hunt and cant find on google
	case TF_DMG_CUSTOM_MERASMUS_GRENADE: // ^
	case TF_DMG_CUSTOM_MERASMUS_ZAP:  // ^
	case TF_DMG_CUSTOM_MERASMUS_DECAPITATION: // ^
	case TF_DMG_CUSTOM_TAUNTATK_ALLCLASS_GUITAR_RIFF: // unused
	case TF_DMG_CUSTOM_THROWABLE: // unused
	case TF_DMG_CUSTOM_THROWABLE_KILL: // unused
	case TF_DMG_CUSTOM_GIANT_HAMMER: // wouldnt apply to players
	case TF_DMG_CUSTOM_RUNE_REFLECT:
	case TF_DMG_CUSTOM_SLAP_KILL: // any of these that are kill (and not taunts) are melee
	case TF_DMG_CUSTOM_CROC: // new, isnt used from what i can tell though
	case TF_DMG_CUSTOM_TAUNTATK_GASBLAST:
	case TF_DMG_CUSTOM_AXTINGUISHER_BOOSTED:
	case TF_DMG_CUSTOM_KRAMPUS_MELEE:
	case TF_DMG_CUSTOM_KRAMPUS_RANGED
	*/
}

void CritsDamage::updateLastWeaponId(C_TFWeaponBase *const weapon)
{
	if (!weapon) {
		return;
	}

	m_last_attacked_weapon_id = weapon->GetWeaponID();
}

bool CritsDamage::onGameEvent(IGameEvent *const event)
{
	if (!event) {
		return true;
	}

	const hash_t event_hash{ HASH_RT(event->GetName()) };

	if (event_hash == HASH_CT("player_hurt"))
	{
		C_TFPlayer *const local{ ec->getLocal() };

		if (!local) {
			return true;
		}

		const int attacker_entindex{ i::engine->GetPlayerForUserID(event->GetInt("attacker")) };

		if (attacker_entindex != local->entindex()) {
			return true;
		}

		const int victim_entindex{ i::engine->GetPlayerForUserID(event->GetInt("userid")) };

		if (victim_entindex == attacker_entindex) {
			return true;
		}

		C_TFPlayer *const victim{ i::ent_list->GetClientEntity(victim_entindex)->cast<C_TFPlayer>() };

		if (!victim) {
			return true;
		}

		const int health{ event->GetInt("health") };
		int damage_amount{ event->GetInt("damageamount") };

		HealthSyncData &health_sync{ getHealthSync(victim) };

		const int health_difference{ health_sync.m_current_health - health };

		health_sync.m_current_health = health;

		if (health == 0 && damage_amount > health_difference) { // they've died with this one
			damage_amount = health_difference;
		}

		C_TFWeaponBase *weapon{ local->Weapon_OwnsThisID(event->GetInt("weaponid")) };

		if (m_last_attacked_weapon_id != event->GetInt("weaponid")) {
			weapon = local->Weapon_OwnsThisID(m_last_attacked_weapon_id);
		}

		bool cannot_be_melee{ true };

		// mfed: the weapon given here is the *active* weapon at time of damage, not necessarily the weapon that was used to create the damage source - a key difference between this and CTFGameStats::Event_PlayerDamage
		// if the weapon is melee, we can't really tell if it was a melee hit or not
		if (tf_utils::isWeaponMelee(weapon))
		{
			cannot_be_melee = false;

			// but if the custom damage type suggests it wasn't a melee hit, we can be sure to an extent
			if (customDamageTypeSuggestsNotMelee(event->GetInt("custom"))) {
				cannot_be_melee = true;
			}

			// otherwise, we can check the range of the weapon
			else
			{
				float swing_range{};
				vec3 hull_mins{};
				vec3 hull_maxs{};

				if (aim_utils->getMeleeInfo(swing_range, hull_mins, hull_maxs))
				{
					const vec3 victim_pos{ victim->GetCenter() };

					vec3 local_pos{};
					local->Weapon_ShootPosition(local_pos);

					if (local_pos.distTo(victim_pos) > swing_range * 3.f) {
						cannot_be_melee = true;
					}
				}
			}
		}

		m_damage += damage_amount;

		if (cannot_be_melee)
		{
			m_damage_ranged += damage_amount;

			if (event->GetBool("crit")) {
				if (!local->IsCritBoosted()) {
					m_damage_ranged_crit_random += damage_amount;
				}
				else {
					m_damage_ranged_crit_boosted += damage_amount;
				}
			}
		}

		m_last_damage_was_melee = !cannot_be_melee;

//#ifdef DEBUG_CRITS
//		const int damage_custom{ event->GetInt("custom") };
//
//		if (damage_custom > 0)
//		{
//			const char *const damage_name{ s::GetCustomDamageName.call<const char *>(damage_custom) };
//
//			if (damage_name) {
//				notifs->warning(std::format("custom damage: {}", damage_name));
//			}
//		}
//
//		if (event->GetBool("crit")) {
//			notifs->warning(std::format("did {} crit damage", damage_amount));
//		}
//#endif
	}

	if (event_hash == HASH_CT("teamplay_round_start"))
	{
		m_damage_ranged = 0.0f;
		m_damage_ranged_crit_random = 0.0f;
		m_damage_ranged_crit_boosted = 0.0f;
		m_damage_melee = 0.0f;
		m_damage = 0.0f;

		m_health_sync.clear();
	}

	if (event_hash == HASH_CT("client_beginconnect")
		|| event_hash == HASH_CT("client_disconnect")
		|| event_hash == HASH_CT("game_newmap"))
	{
		m_damage_ranged = 0.0f;
		m_damage_ranged_crit_random = 0.0f;
		m_damage_ranged_crit_boosted = 0.0f;
		m_damage_melee = 0.0f;
		m_damage = 0.0f;

		m_health_sync.clear();
	}

	return true;
}