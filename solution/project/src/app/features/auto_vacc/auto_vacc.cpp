#include "auto_vacc.hpp"

#include "../trace_utils/trace_utils.hpp"
#include "../player_data/player_data.hpp"
#include "../cfg.hpp"

//#define DEBUG_AUTO_VACC

#ifdef DEBUG_AUTO_VACC
#include "../notifs/notifs.hpp"
#endif

//if arrow is inside this radius we consider it dangerous
static constexpr float DIST_ARROW{ 100.0f };

//if health is below this value we want to pop
static constexpr float HEALTH_ARROW_POP{ 100.0f };

//if flare is inside this radius we consider it dangerous
static constexpr float DIST_FLARE{ 100.0f };

//if health is below this value we want to pop
static constexpr float HEALTH_FLARE_POP{ 60.0f };

//if blast is inside this radius we consider it dangerous
static constexpr float DIST_BLAST{ 200.0f };

//if blasts's owner is inside this radius we want to pop (rockets)
static constexpr float DIST_BLAST_POP{ 900.0f };

//if health is below this value we want to pop
static constexpr float HEALTH_BLAST_POP{ 100.0f };

//if health is below this value we want to pop because we've just taken damage
static constexpr float HEALTH_EVENT_POP{ 100.0f };

//if health is below this value we want to pop because we've just taken critical damage
static constexpr float HEALTH_EVENT_POP_CRIT{ 250.0f };

void AutoVacc::resetEvent()
{
	m_event_desired_res = -1;
	m_event_want_pop = false;
}

bool AutoVacc::isVisible(const vec3 &trace_start, const vec3 &trace_end)
{
	trace_t trace{};

	tf_utils::trace(trace_start, trace_end, &trace_filters::world, &trace, MASK_SOLID);

	return trace.fraction >= 1.0f;
}

bool AutoVacc::processEnvironment(C_TFPlayer *const pl, int &desired_res, bool &want_pop)
{
	if (!pl || pl->deadflag() || pl->IsInvulnerable()) {
		return false;
	}

	//projectiles
	{
		const float latency{ tf_utils::getLatency() };

		auto checkDist = [&](const vec3 &pos, const float dist, const vec3 &proj_pos, const vec3 &proj_vel)
		{
			for (float t{}; t < 1.0f; t += 0.05f)
			{
				const vec3 pred_pos{ proj_pos + ((proj_vel * latency) * t) };

				if (pred_pos.distTo(pos) < dist) {
					return true;
				}
			}

			return false;
		};

		//pipebombs are going to be handled a bit differently (so we can count them)
		std::vector<C_TFGrenadePipebombProjectile *> pipebombs{};

		for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PROJECTILES_ENEMIES))
		{
			IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

			if (!ent) {
				continue;
			}

			const size_t class_id{ ent->GetClassId() };

			if (class_id == class_ids::CTFProjectile_Arrow || class_id == class_ids::CTFProjectile_HealingBolt)
			{
				C_TFProjectile_Arrow *const arrow{ ent->cast<C_TFProjectile_Arrow>() };

				if (!arrow || arrow->m_MoveType() == MOVETYPE_NONE) {
					continue;
				}

				const vec3 arrow_pos{ arrow->GetCenter() };

				if (!checkDist(pl->GetCenter(), DIST_ARROW, arrow_pos, arrow->m_vInitialVelocity())) {
					continue;
				}

				if (!isVisible(arrow_pos, pl->GetCenter())) {
					continue;
				}

				desired_res = MEDIGUN_BULLET_RESIST;

				if (arrow->m_bCritical()) {
					want_pop = true;
				}

				//headshot danger
				if (class_id == class_ids::CTFProjectile_Arrow) {
					want_pop = true;
				}

				if (pl->m_iHealth() < HEALTH_ARROW_POP) {
					want_pop = true;
				}

				return true;
			}

			else if (class_id == class_ids::CTFProjectile_Flare)
			{
				C_TFProjectile_Flare *const flare{ ent->cast<C_TFProjectile_Flare>() };

				if (!flare) {
					continue;
				}

				const vec3 flare_pos{ flare->GetCenter() };

				if (!checkDist(pl->GetCenter(), DIST_FLARE, flare_pos, flare->m_vInitialVelocity())) {
					continue;
				}

				if (!isVisible(flare_pos, pl->GetCenter())) {
					continue;
				}

				desired_res = MEDIGUN_BULLET_RESIST;

				if (flare->m_bCritical()) {
					want_pop = true;
				}

				if (pl->m_iHealth() < HEALTH_FLARE_POP) {
					want_pop = true;
				}

				if (pl->InCond(TF_COND_BURNING)) {
					want_pop = true;
				}

				return true;
			}

			else if (class_id == class_ids::CTFProjectile_Rocket || class_id == class_ids::CTFProjectile_SentryRocket)
			{
				C_TFProjectile_Rocket *const rocket{ ent->cast<C_TFProjectile_Rocket>() };

				if (!rocket || rocket->isRocketJumper()) {
					continue;
				}

				const vec3 rocket_pos{ rocket->GetCenter() };

				if (!checkDist(pl->GetCenter(), DIST_BLAST, rocket_pos, rocket->m_vInitialVelocity())) {
					continue;
				}

				if (!isVisible(rocket_pos, pl->GetCenter())) {
					continue;
				}

				desired_res = MEDIGUN_BLAST_RESIST;

				if (rocket->m_bCritical()) {
					want_pop = true;
				}

				if (C_BaseEntity *const launcher{ rocket->m_hLauncher() })
				{
					if (C_BaseEntity *const owner{ launcher->m_hOwnerEntity() })
					{
						if (owner->GetCenter().distTo(pl->GetCenter()) < DIST_BLAST_POP) {
							want_pop = true;
						}
					}
				}

				if (pl->m_iHealth() < HEALTH_BLAST_POP) {
					want_pop = true;
				}

				return true;
			}

			else if (class_id == class_ids::CTFGrenadePipebombProjectile)
			{
				C_TFGrenadePipebombProjectile *const pipebomb{ ent->cast<C_TFGrenadePipebombProjectile>() };

				if (!pipebomb || pipebomb->m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE) {
					continue;
				}

				const vec3 pipebomb_pos{ pipebomb->GetCenter() };

				if (!checkDist(pl->GetCenter(), DIST_BLAST, pipebomb_pos, pipebomb->m_bTouched() ? vec3{} : pipebomb->m_vInitialVelocity())) {
					continue;
				}

				if (!isVisible(pipebomb_pos, pl->GetCenter())) {
					continue;
				}

				desired_res = MEDIGUN_BLAST_RESIST;

				pipebombs.push_back(pipebomb);
			}
		}

		if (!pipebombs.empty())
		{
			if (pipebombs.size() > 1) {
				want_pop = true;
			}

			else
			{
				for (C_TFGrenadePipebombProjectile *const pipebomb : pipebombs)
				{
					if (!pipebomb) {
						continue;
					}

					if (pipebomb->m_bCritical()) {
						want_pop = true;
					}

					//direct hit danger
					if (!pipebomb->m_bTouched()) {
						want_pop = true;
					}

					if (pl->m_iHealth() < HEALTH_BLAST_POP) {
						want_pop = true;
					}
				}
			}

			return true;
		}
	}

	//snipers
	{
		for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PLAYERS_ENEMIES))
		{
			IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

			if (!ent) {
				continue;
			}

			C_TFPlayer *const player{ ent->cast<C_TFPlayer>() };

			if (!player
				|| player->deadflag()
				|| player->m_iClass() != TF_CLASS_SNIPER
				|| player->InCond(TF_COND_TAUNTING)) {
				continue;
			}

			const LagRecord *const lr{ player_data->getRecord(player, 0) };

			if (!lr || !lr->hasSetupBones()) {
				continue;
			}

			C_TFWeaponBase *const pl_weapon{ player->m_hActiveWeapon()->cast<C_TFWeaponBase>() };

			if (!pl_weapon) {
				continue;
			}

			if (pl_weapon->GetWeaponID() == TF_WEAPON_SNIPERRIFLE_CLASSIC)
			{
				if (!pl_weapon->cast<C_TFSniperRifleClassic>()->m_bCharging()) {
					continue;
				}
			}

			else
			{
				if (!player->InCond(TF_COND_ZOOMED)) {
					continue;
				}
			}

			const vec3 ray_origin{ player->m_vecOrigin() + vec3{ 0.0f, 0.0f, ((player->m_fFlags() & FL_DUCKING) ? 45.0f : 75.0f) * player->m_flModelScale() } };

			//TODO/MAYBE: if we have someone marked as cheater skip this?
			if (!math::rayVsBox(ray_origin, player->m_angEyeAngles(), pl->m_vecOrigin(), pl->m_vecMins() * 1.2f, pl->m_vecMaxs() * 1.2f)) {
				continue;
			}

			std::vector<vec3> vischeck_positions{};

			for (int n{}; n < tf_utils::getHitboxCount(pl); n++)
			{
				vec3 hitbox{};

				if (!tf_utils::getHitbox(pl, lr->bones, n, hitbox)) {
					continue;
				}

				vischeck_positions.push_back(hitbox);
			}

			for (const vec3 &pos : vischeck_positions)
			{
				if (!isVisible(ray_origin, pos)) {
					continue;
				}

				desired_res = MEDIGUN_BULLET_RESIST;
				want_pop = true;

				return true;
			}
		}
	}

	return false;
}

bool AutoVacc::onGameEvent(IGameEvent *const event)
{
	if (!cfg::auto_vacc_active || !event || HASH_RT(event->GetName()) != HASH_CT("player_hurt")) {
		return true;
	}

	if (m_event_desired_res != -1 || m_event_want_pop) {
		return true;
	}

	C_BaseEntity *const victim{ i::ent_list->GetClientEntity(i::engine->GetPlayerForUserID(event->GetInt("userid")))->cast<C_BaseEntity>() };
	C_BaseEntity *const attacker{ i::ent_list->GetClientEntity(i::engine->GetPlayerForUserID(event->GetInt("attacker")))->cast<C_BaseEntity>() };

	if (!victim || !attacker) {
		return true;
	}

	if (victim == attacker) {
		return true;
	}

	if (victim->GetClassId() != class_ids::CTFPlayer || attacker->GetClassId() != class_ids::CTFPlayer) {
		return true;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || local->m_iClass() != TF_CLASS_MEDIC) {
		return true;
	}

	C_WeaponMedigun *const medigun { local->Weapon_OwnsThisID(TF_WEAPON_MEDIGUN)->cast<C_WeaponMedigun>() };

	if (!medigun || medigun->GetMedigunType() != MEDIGUN_RESIST || (medigun->m_hHealingTarget() != victim && victim != local)) {
		return true;
	}

	C_TFWeaponBase *const attacker_weapon{ attacker->cast<C_TFPlayer>()->m_hActiveWeapon().Get()->cast<C_TFWeaponBase>() };

	if (!attacker_weapon) {
		return true;
	}

	int desired_res{ -1 };

	switch (attacker_weapon->GetWeaponID())
	{
		case TF_WEAPON_ROCKETLAUNCHER:
		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_GRENADELAUNCHER:
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_PARTICLE_CANNON:
		case TF_WEAPON_CANNON: {
			desired_res = MEDIGUN_BLAST_RESIST;
			break;
		}

		case TF_WEAPON_SYRINGEGUN_MEDIC:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE:
		case TF_WEAPON_RAYGUN:
		case TF_WEAPON_CLEAVER:
		case TF_WEAPON_FLAREGUN_REVENGE:
		case TF_WEAPON_DRG_POMSON:
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_CROSSBOW: {
			desired_res = MEDIGUN_BULLET_RESIST;
			break;
		}

		case TF_WEAPON_FLAME_BALL:
		case TF_WEAPON_FLAMETHROWER: {
			desired_res = MEDIGUN_FIRE_RESIST;
			break;
		}

		default:
		{
			const int damage_type{ attacker_weapon->GetDamageType() };

			if ((damage_type & DMG_BULLET) || (damage_type & DMG_BUCKSHOT)) {
				desired_res = MEDIGUN_BULLET_RESIST;
			}

			else if (damage_type & DMG_BLAST) {
				desired_res = MEDIGUN_BLAST_RESIST;
			}

			else if (damage_type & DMG_BURN) {
				desired_res = MEDIGUN_FIRE_RESIST;
			}

			break;
		}
	}

	const int custom_damage{ event->GetInt("custom") };

	if (custom_damage == TF_DMG_CUSTOM_BURNING || custom_damage == TF_DMG_CUSTOM_BURNING_FLARE) {
		desired_res = MEDIGUN_FIRE_RESIST;
	}

	C_TFPlayer *const victim_pl{ victim->cast<C_TFPlayer>() };

	if (desired_res == MEDIGUN_BULLET_RESIST && victim_pl->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST)) {
		resetEvent();
		return true;
	}

	else if (desired_res == MEDIGUN_BLAST_RESIST && victim_pl->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST)) {
		resetEvent();
		return true;
	}

	else if (desired_res == MEDIGUN_FIRE_RESIST && victim_pl->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST)) {
		resetEvent();
		return true;
	}

	m_event_desired_res = desired_res;

#ifdef DEBUG_AUTO_VACC
	notifs->message(std::format("desired_res: {}", desired_res));
#endif

	const bool is_crit{ event->GetBool("crit") };
	const bool is_mini_crit{ event->GetBool("minicrit") };

	const int health{ event->GetInt("health") };

	if (((is_crit || is_mini_crit) && health < HEALTH_EVENT_POP_CRIT) || health < HEALTH_EVENT_POP) {
		m_event_want_pop = true;
	} 

#ifdef DEBUG_AUTO_VACC
	notifs->message(std::format("want_pop: {}", m_event_want_pop));
#endif

	return true;
}

void AutoVacc::run(CUserCmd *const cmd)
{
	if (!cfg::auto_vacc_active || !cmd) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || local->m_iClass() != TF_CLASS_MEDIC) {
		resetEvent();
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon || weapon->GetWeaponID() != TF_WEAPON_MEDIGUN) {
		resetEvent();
		return;
	}

	C_WeaponMedigun *const medigun{ weapon->cast<C_WeaponMedigun>() };

	if (!medigun || medigun->GetMedigunType() != MEDIGUN_RESIST) {
		resetEvent();
		return;
	}

	int desired_res{ -1 };
	bool want_pop{ false };
	bool use_event{ true };

	C_TFPlayer *const targets[2] {
		cfg::auto_vacc_prioritise_local ? local : medigun->m_hHealingTarget()->cast<C_TFPlayer>(),
		cfg::auto_vacc_prioritise_local ? medigun->m_hHealingTarget()->cast<C_TFPlayer>() : local
	};

	auto isResisted = [&](C_TFPlayer *const target) -> bool
	{
		if (!target) {
			return false;
		}

		return (desired_res == MEDIGUN_BULLET_RESIST && target->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST))
			|| (desired_res == MEDIGUN_BLAST_RESIST && target->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST))
			|| (desired_res == MEDIGUN_FIRE_RESIST && target->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST));
	};

	for (C_TFPlayer *const target : targets)
	{
		if (!target || !processEnvironment(target, desired_res, want_pop) || isResisted(target)) {
			continue;
		}
		
		use_event = false;

		break;
	}

	if (use_event)
	{
#ifdef DEBUG_AUTO_VACC
		notifs->message("using event");
#endif
		desired_res = m_event_desired_res;
		want_pop = m_event_want_pop;

		resetEvent();
	}

	if (desired_res == -1) {
		return;
	}

	if (medigun->m_nChargeResistType() != desired_res)
	{
		if (!(tf_globals::last_cmd.buttons & IN_RELOAD)) {
			cmd->buttons |= IN_RELOAD;
		}

		else {
			cmd->buttons &= ~IN_RELOAD;
		}

		if (want_pop) {
			cmd->buttons &= ~IN_ATTACK2;
		}

		return;
	}

	if (want_pop /*&& medigun->m_flChargeLevel() >= 0.25f*/) {
		cmd->buttons |= IN_ATTACK2;
	}
}