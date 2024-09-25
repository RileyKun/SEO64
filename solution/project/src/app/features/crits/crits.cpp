#include "crits.hpp"
#include "crits_damage.hpp"

#include "../aim_utils/aim_utils.hpp"
#include "../vis_utils/vis_utils.hpp"
#include "../networking/networking.hpp"
#include "../notifs/notifs.hpp"
#include "../cfg.hpp"

//#define DEBUG_CRITS

#ifdef DEBUG_CRITS

MAKE_SIG(server_UTIL_EntityByIndex, mem::findBytes("server.dll", "E8 ? ? ? ? 8B 53 40 45 33 C0").fixRip());
MAKE_SIG(server_CTF_GameStats_FindPlayerStats, mem::findBytes("server.dll", "E8 ? ? ? ? 48 85 C0 74 ? 8B 88 ? ? ? ? 8B 90").fixRip());

static void *player_stats{ nullptr };

MAKE_HOOK(
	server_CTF_GameStats_FindPlayerStats,
	s::server_CTF_GameStats_FindPlayerStats.get(),
	void *,
	void *const rcx, C_TFPlayer *const player)
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return CALL_ORIGINAL(rcx, player);
	}

	C_TFPlayer *const server_local{ s::server_UTIL_EntityByIndex.call<C_TFPlayer *>(local->entindex()) };

	if (!server_local) {
		return CALL_ORIGINAL(rcx, player);
	}

	if (player == server_local) {
		player_stats = CALL_ORIGINAL(rcx, player);
		return player_stats;
	}

	return CALL_ORIGINAL(rcx, player);
}

#endif

int Crits::getWeaponMode(C_TFWeaponBase *const weapon) const
{
	if (!weapon
		|| weapon->GetWeaponID() == TF_WEAPON_MINIGUN
		|| weapon->GetWeaponID() == TF_WEAPON_FLAMETHROWER) {
		return 0;
	}

	return weapon->m_iWeaponMode();
}

Crits::CritType Crits::getCritType(C_TFWeaponBase *const weapon) const
{
	if (!weapon || !weapon->m_pWeaponInfo()) {
		return CritType::UNKNOWN;
	}

	return tf_utils::isWeaponMelee(weapon)
		? CritType::MELEE
		: weapon->m_pWeaponInfo()->GetWeaponData(getWeaponMode(weapon)).m_bUseRapidFireCrits
		? CritType::RAPIDFIRE
		: CritType::REGULAR;
}

float Crits::getCritChance(C_TFPlayer *const local, C_TFWeaponBase *const weapon) const
{
	if (!local || !weapon) {
		return 0.0f;
	}

	switch (getCritType(weapon))
	{
		case CritType::REGULAR: {
			return tf_utils::attribHookValue(TF_DAMAGE_CRIT_CHANCE * local->GetCritMult(), "mult_crit_chance", weapon);
		}

		case CritType::RAPIDFIRE:
		{
			const float total_crit_chance{ std::clamp(TF_DAMAGE_CRIT_CHANCE_RAPID * local->GetCritMult(), 0.01f, 0.99f) };
			const float non_crit_duration{ (TF_DAMAGE_CRIT_DURATION_RAPID / total_crit_chance) - TF_DAMAGE_CRIT_DURATION_RAPID };

			return tf_utils::attribHookValue(1.0f / non_crit_duration, "mult_crit_chance", weapon);
		}

		case CritType::MELEE: {
			return tf_utils::attribHookValue(TF_DAMAGE_CRIT_CHANCE_MELEE * local->GetCritMult(), "mult_crit_chance", weapon);
		}

		default: {
			return 0.0f;
		}
	}
}

float Crits::getDamage(C_TFWeaponBase *const weapon, const bool for_withdraw) const
{
	if (!weapon || !weapon->m_pWeaponInfo()) {
		return 0.0f;
	}

	const CritType crit_type{ getCritType(weapon) };

	if (crit_type == CritType::UNKNOWN) {
		return 0.0f;
	}

	const WeaponData_t &weapon_data{ weapon->m_pWeaponInfo()->GetWeaponData(getWeaponMode(weapon)) };

	float damage{ tf_utils::attribHookValue(static_cast<float>(weapon_data.m_nDamage), "mult_dmg", weapon) };

	if (crit_type != CritType::MELEE)
	{
		int projectiles_per_shot{ weapon_data.m_nBulletsPerShot };

		if (projectiles_per_shot >= 1) {
			projectiles_per_shot = static_cast<int>(tf_utils::attribHookValue(projectiles_per_shot, "mult_bullets_per_shot", weapon));
		}

		else {
			projectiles_per_shot = 1;
		}

		damage *= static_cast<float>(projectiles_per_shot);
	}

	if (for_withdraw && crit_type == CritType::RAPIDFIRE)
	{
		damage *= TF_DAMAGE_CRIT_DURATION_RAPID / weapon_data.m_flTimeFireDelay;

		const int cap{ cvars::tf_weapon_criticals_bucket_cap->GetInt() };

		if (damage * TF_DAMAGE_CRIT_MULTIPLIER > cap) {
			damage = static_cast<float>(cap) / TF_DAMAGE_CRIT_MULTIPLIER;
		}
	}

	return damage;
}

bool Crits::isSeedCrit(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const int seed) const
{
	if (!local || !weapon) {
		return false;
	}

	const int mask
	{
		getCritType(weapon) == CritType::MELEE
		? ((weapon->entindex() << 16) | (local->entindex() << 8))
		: ((weapon->entindex() << 8) | local->entindex())
	};

	tf_utils::randomSeed(seed ^ mask);

	return tf_utils::randomInt(0, WEAPON_RANDOM_RANGE - 1) < (getCritChance(local, weapon) * WEAPON_RANDOM_RANGE);
}

int Crits::findCritCmdNum(C_TFPlayer *const local, C_TFWeaponBase *const weapon, const int start_cmd_num, const bool want_crit)
{
	if (!local || !weapon) {
		return -1;
	}

	for (int n{ start_cmd_num + 1 }; n < start_cmd_num + 4096; n++)
	{
		if (n == m_last_crit_cmd) {
			continue;
		}

		const bool is_crit{ isSeedCrit(local, weapon, MD5_PseudoRandom(n) & 0x7FFFFFFF) };

		if (want_crit ? is_crit : !is_crit) {
			m_last_crit_cmd = n;
			return n;
		}
	}

	notifs->warning("failed to find a crit cmd");

	return -1; //if we cant find a random seed return -1 so the hack doesnt try to force a crit seed at the cmd num
}

bool Crits::canWeaponRandomCrit(C_TFPlayer *const local, C_TFWeaponBase *const weapon) const
{
	if (!local
		|| !weapon
		|| !weapon->AreRandomCritsEnabled()
		|| !weapon->CanFireCriticalShot(false)
		|| getCritChance(local, weapon) <= 0.0f) {
		return false;
	}

	switch (weapon->GetWeaponID())
	{
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_KNIFE:
		case TF_WEAPON_LUNCHBOX:
		case TF_WEAPON_MEDIGUN:
		case TF_WEAPON_FLAME_BALL:
		case TF_WEAPON_PDA:
		case TF_WEAPON_PDA_ENGINEER_BUILD:
		case TF_WEAPON_PDA_ENGINEER_DESTROY:
		case TF_WEAPON_LASER_POINTER:
		case TF_WEAPON_PDA_SPY:
		case TF_WEAPON_BUILDER:
		case TF_WEAPON_BUFF_ITEM: {
			return false;
		}

		default: {
			break;
		}
	}

	C_TFGameRulesProxy *const game_rules{ ec->getGameRules() };

	if (!game_rules || ((game_rules->m_iRoundState() == GR_STATE_TEAM_WIN) && (game_rules->m_iWinningTeam() == local->m_iTeamNum()))) {
		return false;
	}

	return true;
}

void Crits::addToCritBucket(C_TFWeaponBase *const weapon, const float amount)
{
	if (!weapon) {
		return;
	}

	const float bucket_cap{ cvars::tf_weapon_criticals_bucket_cap->GetFloat() };

	if (weapon->m_flCritTokenBucket() < bucket_cap) {
		weapon->m_flCritTokenBucket() += amount;
		weapon->m_flCritTokenBucket() = std::min(weapon->m_flCritTokenBucket(), bucket_cap);
	}
}

bool Crits::isAllowedToWithdrawFromCritBucket(C_TFWeaponBase *const weapon, const float amount)
{
	if (!weapon) {
		return false;
	}

	const float mult
	{
		getCritType(weapon) == CritType::MELEE
		? 0.5f
		: math::remap((static_cast<float>(weapon->m_nCritSeedRequests() + 1) / static_cast<float>(weapon->m_nCritChecks())), 0.1f, 1.0f, 1.0f, 3.0f)
	};

	const float cost{ (amount * TF_DAMAGE_CRIT_MULTIPLIER) * mult };

	if (cost > weapon->m_flCritTokenBucket()) {
		return false;
	}

	weapon->m_flCritTokenBucket() -= cost;

	const float bucket_bottom{ cvars::tf_weapon_criticals_bucket_bottom->GetFloat() };

	if (weapon->m_flCritTokenBucket() < bucket_bottom) {
		weapon->m_flCritTokenBucket() = bucket_bottom;
	}

	weapon->m_nCritSeedRequests()++;

	return true;
}

bool Crits::onLevelShutdown()
{
	m_crit_cmd = 0;
	m_last_crit_cmd = 0;
	m_want_crit = false;

	return true;
}

bool Crits::think(bool want_crit)
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->IsCritBoosted()) {
		return false;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon) {
		return false;
	}

	const CritType crit_type{ getCritType(weapon) };

	if (crit_type == CritType::UNKNOWN) {
		return false;
	}

	if (crit_type == CritType::RAPIDFIRE && weapon->m_flCritTime() > i::global_vars->curtime) {
		return false;
	}

	addToCritBucket(weapon, getDamage(weapon, false));

	if (crit_type == CritType::RAPIDFIRE)
	{
		if (i::global_vars->curtime < weapon->m_flLastRapidFireCritCheckTime() + 1.0f) {
			return false;
		}

		weapon->m_flLastRapidFireCritCheckTime() = i::global_vars->curtime;
	}

	weapon->m_nCritChecks()++;

	if (want_crit)
	{
		if (crit_type != CritType::MELEE) {
			want_crit = crits_dmg->canFireRandomCriticalShot(getCritChance(local, weapon));
		}

		if (want_crit)
		{
			want_crit = isAllowedToWithdrawFromCritBucket(weapon, getDamage(weapon, true));

			if (want_crit && crit_type == CritType::RAPIDFIRE) {
				weapon->m_flCritTime() = i::global_vars->curtime + TF_DAMAGE_CRIT_DURATION_RAPID;
			}
		}
	}

	return want_crit;
}

void Crits::run(CUserCmd *const cmd)
{
	m_crit_cmd = -1;
	m_want_crit = false;

	if (!cmd) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon) {
		return;
	}

	if (aim_utils->isAttacking(cmd, i::global_vars->curtime, true)) {
		crits_dmg->updateLastWeaponId(weapon);
	}

	if (!canWeaponRandomCrit(local, weapon)) {
		return;
	}

	bool want_crit{ cfg::crits_force_crit };
	
	if (want_crit)
	{
		const float tokens{ weapon->m_flCritTokenBucket() };
		const int crit_checks{ weapon->m_nCritChecks() };
		const int seed_requests{ weapon->m_nCritSeedRequests() };
		const float crit_time{ weapon->m_flCritTime() };
		const float last_rf_check{ weapon->m_flLastRapidFireCritCheckTime() };

		want_crit = think(true);

		weapon->m_flCritTokenBucket() = tokens;
		weapon->m_nCritChecks() = crit_checks;
		weapon->m_nCritSeedRequests() = seed_requests;
		weapon->m_flCritTime() = crit_time;
		weapon->m_flLastRapidFireCritCheckTime() = last_rf_check;
	}

	m_crit_cmd = findCritCmdNum(local, weapon, cmd->command_number, want_crit);
	m_want_crit = want_crit && m_crit_cmd != -1;

	if (m_crit_cmd != -1) {
		cmd->command_number = m_crit_cmd;
		cmd->random_seed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;
	}
}

void Crits::paint()
{
	if (!cfg::crits_indicator || !vis_utils->shouldRunVisuals()) {
		return;
	}

	C_TFPlayer *const player{ ec->getLocal() };

	if (!player || player->deadflag()) {
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon) {
		return;
	}

	const CritType crit_type{ getCritType(weapon) };

	if (crit_type == CritType::UNKNOWN) {
		return;
	}

	size_t idx{};

	Font *const font{ &fonts::indicators };

	auto getPos = [&]() {
		const vec2 pos{ draw->getScreenSize().x * 0.5f, draw->getScreenSize().y * cfg::crits_indicator_pos };
		return pos + vec2{ 0.0f, font->getTall() * static_cast<float>(idx++) };
	};

	if (!canWeaponRandomCrit(player, weapon)) {
		draw->stringOutlined(getPos(), font->get(), "weapon cant random crit", cfg::color_indicators_bad, POS_CENTERX);
		return;
	}

#ifdef DEBUG_CRITS
	if (tf_utils::isLoopback())
	{
		C_TFPlayer *const player{ ec->getLocal() };

		if (!player) {
			return;
		}

		C_TFWeaponBase *const weapon{ ec->getWeapon() };

		if (!weapon) {
			return;
		}

		C_BaseEntity *const server_local{ s::server_UTIL_EntityByIndex.call<C_BaseEntity *>(player->entindex()) };
		C_BaseEntity *const server_weapon{ s::server_UTIL_EntityByIndex.call<C_BaseEntity *>(weapon->entindex()) };

		if (!server_local || !server_weapon) {
			return;
		}

		constexpr size_t offsets[]
		{
			2235, // m_bCurrentCritIsRandom
			2260, // m_flCritTime
			2276, // m_flLastRapidFireCritCheckTime
			2264, // m_flLastCritCheckTime
			2272, // m_iCurrentSeed
			1888, // m_nCritChecks
			1892, // m_nCritSeedRequests
			1884, // m_flCritTokenBucket
		};

		const vec2 pos{ draw->getScreenSize().x * 0.5f, draw->getScreenSize().y * 0.2f };

		size_t idx{};

		auto getPos = [&]() {
			return pos + vec2{ 0.0f, fonts::esp.getTall() * static_cast<float>(idx++) };
		};

		draw->stringOutlined
		(
			getPos(),
			fonts::esp.get(),
			std::format("crit_time: {} vs {}", weapon->m_flCritTime(), *reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(server_weapon) + offsets[1])),
			{ 100, 200, 50, 255 },
			POS_CENTERX
		);

		draw->stringOutlined
		(
			getPos(),
			fonts::esp.get(),
			std::format("crit_token_bucket: {} vs {}", weapon->m_flCritTokenBucket(), *reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(server_weapon) + offsets[7])),
			{ 100, 200, 50, 255 },
			POS_CENTERX
		);

		draw->stringOutlined
		(
			getPos(),
			fonts::esp.get(),
			std::format("last_rapid_fire_crit_check_time: {} vs {}", weapon->m_flLastRapidFireCritCheckTime(), *reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(server_weapon) + offsets[2])),
			{ 100, 200, 50, 255 },
			POS_CENTERX
		);

		draw->stringOutlined
		(
			getPos(),
			fonts::esp.get(),
			std::format("crit_seed_requests: {} vs {}", weapon->m_nCritSeedRequests(), *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(server_weapon) + offsets[6])),
			{ 100, 200, 50, 255 },
			POS_CENTERX
		);

		draw->stringOutlined
		(
			getPos(),
			fonts::esp.get(),
			std::format("crit_checks: {} vs {}", weapon->m_nCritChecks(), *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(server_weapon) + offsets[5])),
			{ 100, 200, 50, 255 },
			POS_CENTERX
		);

		if (!player_stats) {
			return;
		}

		constexpr size_t TFSTAT_DAMAGE_RANGED{ 324 };
		constexpr size_t TFSTAT_DAMAGE_RANGED_CRIT_RANDOM{ 328 };

		const int total_damage{ *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(player_stats) + TFSTAT_DAMAGE_RANGED) };
		const int random_ranged_crit_damage{ *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(player_stats) + TFSTAT_DAMAGE_RANGED_CRIT_RANDOM) };

		getPos();

		bool equal_total_damage{ crits_dmg->getDamageRanged() == total_damage };

		const float total_difference{ crits_dmg->getDamageRanged() - total_damage };

		draw->stringOutlined
		(
			getPos(),
			fonts::esp.get(),
			std::format("total_damage: {} vs {} ({})", crits_dmg->getDamageRanged(), total_damage, total_difference),
			equal_total_damage ? Color{ 100, 200, 50, 255 } : Color{ 200, 100, 50, 255 },
			POS_CENTERX
		);

		bool equal_random_ranged_crit_damage{ crits_dmg->getDamageRangedCritRandom() == random_ranged_crit_damage };

		const float random_ranged_crit_difference{ crits_dmg->getDamageRangedCritRandom() - random_ranged_crit_damage };

		draw->stringOutlined
		(
			getPos(),
			fonts::esp.get(),
			std::format("random_ranged_crit_damage: {} vs {} ({})", crits_dmg->getDamageRangedCritRandom(), random_ranged_crit_damage, random_ranged_crit_difference),
			equal_random_ranged_crit_damage ? Color{ 100, 200, 50, 255 } : Color{ 200, 100, 50, 255 },
			POS_CENTERX
		);
	}
#endif

	const bool crit_boosted{ player->IsCritBoosted() };
	const float bucket_cap{ cvars::tf_weapon_criticals_bucket_cap->GetFloat() };

	const float base_damage{ getDamage(weapon, false) };
	const float actual_damage{ getDamage(weapon, true) };

	const int potential_crits{
		static_cast<int>((bucket_cap - base_damage) / (TF_DAMAGE_CRIT_MULTIPLIER * actual_damage / (crit_type == CritType::MELEE ? 2.0f : 1.0f) - base_damage))
	};

	const float cost_no_mult{ actual_damage * TF_DAMAGE_CRIT_MULTIPLIER };

	int shots{ weapon->m_nCritChecks() };
	int crits{ weapon->m_nCritSeedRequests() };
	float bucket{ weapon->m_flCritTokenBucket() };

	int available_crits{ 0 };

	for (int i{}; i < std::min(potential_crits, 227); i++) // mfed: highest i can find is 226 on fan o' war at 9 damage
	{
		shots++;
		crits++;

		const float test_mult{ crit_type == CritType::MELEE ? 0.5f : math::remap((static_cast<float>(crits) / static_cast<float>(shots)), 0.1f, 1.0f, 1.0f, 3.0f) };

		bucket = std::min(bucket + base_damage, bucket_cap) - cost_no_mult * test_mult;

		if (bucket < 0.0f) {
			break;
		}

		available_crits++;
	}

	const float observed_crit_chance{ crits_dmg->getObservedCriticalChance() };
	const float crit_chance{ getCritChance(player, weapon) + 0.1f };
	const float tokens{ weapon->m_flCritTokenBucket() < bucket_cap ? std::min(weapon->m_flCritTokenBucket() + base_damage, bucket_cap) : weapon->m_flCritTokenBucket() };
	const float mult{ crit_type == CritType::MELEE ? 0.5f : math::remap((static_cast<float>(weapon->m_nCritSeedRequests() + 1) / static_cast<float>(weapon->m_nCritChecks() + 1)), 0.1f, 1.0f, 1.0f, 3.0f) };
	const float cost{ actual_damage * TF_DAMAGE_CRIT_MULTIPLIER * mult };

	const bool tokens_too_low{ cost > tokens };
	const bool crit_chance_too_high{ crit_type != CritType::MELEE && observed_crit_chance > crit_chance };
	const bool is_crit_time{ crit_type == CritType::RAPIDFIRE && weapon->m_flCritTime() > i::global_vars->curtime };
	const bool waiting_for_crit{ crit_type == CritType::RAPIDFIRE && i::global_vars->curtime < weapon->m_flLastRapidFireCritCheckTime() + 1.0f };

	if (cfg::crits_indicator == 1)
	{
		const bool crit_ready{ (crit_boosted || (!crit_chance_too_high && !tokens_too_low && !waiting_for_crit) || is_crit_time) && available_crits > 0 };

		draw->stringOutlined
		(
			getPos(),
			font->get(),
			std::format("crits: {} / {}", available_crits, potential_crits),
			{ 200, 200, 200, 255 },
			POS_CENTERX
		);

		if (crit_boosted) {
			draw->stringOutlined(getPos(), font->get(), "crit boosted", cfg::color_indicators_extra_good, POS_CENTERX);
		}

		else if (crit_ready)
		{
			Color clr{ cfg::color_indicators_good };

			if (cfg::crits_force_crit) {
				clr = cfg::color_indicators_extra_good;
			}

			draw->stringOutlined(getPos(), font->get(), "crits ready!", clr, POS_CENTERX);
		}

		else if (is_crit_time) {
			const float t{ math::remap(i::global_vars->curtime, weapon->m_flCritTime() - TF_DAMAGE_CRIT_DURATION_RAPID, weapon->m_flCritTime(), 0.0f, TF_DAMAGE_CRIT_DURATION_RAPID) };
			draw->stringOutlined(getPos(), font->get(), std::format("crit time! ({:.1f})", t), cfg::color_indicators_good, POS_CENTERX);
		}

		else if (crit_chance_too_high)
		{
			draw->stringOutlined(getPos(), font->get(), std::format("crit banned ({:.2f} > {:.2f})", observed_crit_chance, crit_chance), cfg::color_indicators_bad, POS_CENTERX);

			if (!is_crit_time) {
				const float dmg{ crits_dmg->calculateNonCritDamage(crit_chance) };
				draw->stringOutlined(getPos(), font->get(), std::format("deal {:.0f} damage", dmg), cfg::color_indicators_bad, POS_CENTERX);
			}
		}

		else if (tokens_too_low) {
			const int needed_tokens{ std::clamp(static_cast<int>(cost - tokens), 0, static_cast<int>(bucket_cap)) };
			draw->stringOutlined(getPos(), font->get(), std::format("low tokens (-{})", needed_tokens), cfg::color_indicators_bad, POS_CENTERX);
		}

		else if (waiting_for_crit) {
			const float t{ (i::global_vars->curtime - weapon->m_flLastRapidFireCritCheckTime()) / 1.0f };
			draw->stringOutlined(getPos(), font->get(), std::format("wait! ({:.1f})", 1.0f - t), cfg::color_indicators_mid, POS_CENTERX);
		}

		else {
			draw->stringOutlined(getPos(), font->get(), "crits not ready", cfg::color_indicators_bad, POS_CENTERX); // how
		}
	}

	//if (cfg::crits_indicator == 1)
	//{
	//	bool crit_ready{};

	//	if (crit_boosted) {
	//		draw->stringOutlined(getPos(), font->get(), "crit boosted", cfg::color_indicators_extra_good, POS_CENTERX);
	//	}

	//	else if (!crit_chance_too_high && !tokens_too_low && !waiting_for_crit)
	//	{
	//		Color clr{ cfg::color_indicators_good };

	//		if (cfg::crits_force_crit) {
	//			clr = cfg::color_indicators_extra_good;
	//		}

	//		draw->stringOutlined(getPos(), font->get(), "crits ready!", clr, POS_CENTERX);

	//		crit_ready = true;
	//	}

	//	else if (is_crit_time) {
	//		const float t{ math::remap(i::global_vars->curtime, weapon->m_flCritTime() - TF_DAMAGE_CRIT_DURATION_RAPID, weapon->m_flCritTime(), 0.0f, TF_DAMAGE_CRIT_DURATION_RAPID) };
	//		draw->stringOutlined(getPos(), font->get(), std::format("crit time! ({:.1f})", t), cfg::color_indicators_good, POS_CENTERX);
	//		crit_ready = true;
	//	}

	//	else if (crit_chance_too_high && getCritType(weapon) != CritType::MELEE)
	//	{
	//		draw->stringOutlined(getPos(), font->get(), std::format("crit banned ({:.2f} > {:.2f})", observed_crit_chance, crit_chance), cfg::color_indicators_bad, POS_CENTERX);

	//		if (!is_crit_time)
	//		{
	//			const float dmg{ crits_dmg->calculateNonCritDamage(crit_chance) };

	//			draw->stringOutlined
	//			(
	//				getPos(),
	//				font->get(),
	//				std::format("deal {:.0f} damage", dmg),
	//				cfg::color_indicators_bad,
	//				POS_CENTERX
	//			);
	//		}
	//	}

	//	else if (tokens_too_low) {
	//		const int needed_tokens{ std::clamp(static_cast<int>(cost - tokens), 0, static_cast<int>(bucket_cap)) };
	//		draw->stringOutlined(getPos(), font->get(), std::format("low tokens (-{})", needed_tokens), cfg::color_indicators_bad, POS_CENTERX);
	//	}

	//	else if (waiting_for_crit) {
	//		const float t{ (i::global_vars->curtime - weapon->m_flLastRapidFireCritCheckTime()) / 1.0f };
	//		draw->stringOutlined(getPos(), font->get(), std::format("wait! ({:.1f})", 1.0f - t), cfg::color_indicators_mid, POS_CENTERX);
	//	}

	//	else {
	//		draw->stringOutlined(getPos(), font->get(), "crits not ready", cfg::color_indicators_bad, POS_CENTERX); // how
	//	}

	//	if (available_crits <= 0) {
	//		crit_ready = false;
	//	}

	//	draw->stringOutlined
	//	(
	//		getPos(),
	//		font->get(),
	//		std::format("crits: {} / {}", available_crits, potential_crits),
	//		crit_ready ? cfg::color_indicators_good : cfg::color_indicators_bad,
	//		POS_CENTERX
	//	);
	//}
}

int Crits::getCritCmd() const
{
	return m_crit_cmd;
}

bool Crits::getWantCrit() const
{
	return m_want_crit;
}