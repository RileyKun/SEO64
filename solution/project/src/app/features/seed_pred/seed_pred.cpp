#include "seed_pred.hpp"

#include "../aim_utils/aim_utils.hpp"
#include "../vis_utils/vis_utils.hpp"
#include "../engine_pred/engine_pred.hpp"
#include "../cfg.hpp"

bool SeedPred::onLevelShutdown()
{
	m_send_time = 0.0f;
	m_recv_time = 0.0f;
	m_waiting_for_recv = false;
	m_pp_time = 0.0f;

	return true;
}

bool SeedPred::onLoad()
{
	for (int n{}; n <= 255; n++) 
	{
		tf_utils::randomSeed(n);

		float x{ tf_utils::randomFloat(-0.5f, 0.5f) + tf_utils::randomFloat(-0.5f, 0.5f) };
		float y{ tf_utils::randomFloat(-0.5f, 0.5f) + tf_utils::randomFloat(-0.5f, 0.5f) };

		m_spread_offsets[n] = {x, y};
	}

	m_spread_init = true;
	return true;
}

void SeedPred::sendPlayerPerf()
{
	if (tf_utils::isLoopback() || m_waiting_for_recv) {
		return;
	}

	if (m_pp_time > 0.0f && (static_cast<float>(tf_utils::Plat_FloatTime()) - m_recv_time) < cfg::seed_pred_resync_interval) {
		return;
	}

	i::client_state->SendStringCmd("playerperf");

	m_send_time = static_cast<float>(tf_utils::Plat_FloatTime());
	m_waiting_for_recv = true;
}

void SeedPred::recvPlayerPerf(bf_read &msg_data)
{
	if (!m_waiting_for_recv) {
		return;
	}

	m_recv_time = static_cast<float>(tf_utils::Plat_FloatTime());
	m_waiting_for_recv = false;

	char msg_str[256]{};

	if (!msg_data.ReadString(msg_str, sizeof(msg_str), true)) {
		return;
	}

	std::string msg{ msg_str };
	msg.erase(msg.begin());

	std::smatch matches{};
	std::regex_match(msg, matches, std::regex{ "(\\d+\\.\\d+) (\\d+) (\\d+) (\\d+\\.\\d+) (\\d+\\.\\d+) vel (\\d+\\.\\d+)" });

	if (matches.size() != 7) {
		return;
	}

	m_pp_time = std::stof(matches[1].str()) + ((m_recv_time - m_send_time) * 1.0f);
}

float SeedPred::predictServerTime() const
{
	if (tf_utils::isLoopback()) {
		return static_cast<float>(tf_utils::Plat_FloatTime());
	}

	if (m_send_time <= 0.0f || m_recv_time <= 0.0f || m_pp_time <= 0.0f) {
		return 0.0f;
	}

	return m_pp_time + (static_cast<float>(tf_utils::Plat_FloatTime()) - m_recv_time);
}

int SeedPred::getSeed(const float time) const
{
	const float seed_time{ time * 1000.0f };

	return *reinterpret_cast<int *>((char *)&seed_time) & 255;
}

// WHAT A FUCKING MESS
void SeedPred::findBestSolution(C_TFPlayer *const pl, CUserCmd *const cmd, const float wpn_spread, const bool perfect_shot, const int bullets_per_shot)
{
	const vec3 origin{ pl->m_vecOrigin() };
	vec3 new_origin{};

	const CUserCmd backup{ *cmd };

	engine_pred->end();

	const int seed{ getSeed(predictServerTime()) };

	std::vector<vec2> spread_multipliers{};

	vec2 average_multiplier{};

	for (int n{}; n < bullets_per_shot; n++) 
	{
		vec2 multiplier{};

		if (n == 0 && perfect_shot) 
		{
			spread_multipliers.emplace_back(multiplier);
			continue;
		}

		if (seed + n > 255) // like 3 extra random seed calls wont hurt you
		{
			tf_utils::randomSeed(seed);

			multiplier.x = tf_utils::randomFloat(-0.5f, 0.5f) + tf_utils::randomFloat(-0.5f, 0.5f);
			multiplier.y = tf_utils::randomFloat(-0.5f, 0.5f) + tf_utils::randomFloat(-0.5f, 0.5f);
		}
		else
		{
			multiplier = m_spread_offsets[seed + n];
		}

		average_multiplier += multiplier;

		spread_multipliers.emplace_back(multiplier);
	}

	average_multiplier /= bullets_per_shot;

	const vec2 final_multiplier{ *std::min_element(spread_multipliers.begin(), spread_multipliers.end(), [&](const vec2 &a, const vec2 &b) {
		  return a.distTo(average_multiplier) < b.distTo(average_multiplier);
	})};

	const vec3 final_spread{ final_multiplier.x * wpn_spread, final_multiplier.y * wpn_spread, 0.f };

	vec3 roll_angle { cmd->viewangles };
	roll_angle -= { -math::radToDeg(std::atan(final_spread.length2D())), 0.f, math::radToDeg(std::atan2(final_spread.x, final_spread.y))};

	tf_utils::fixMovement(cmd, roll_angle);
	cmd->viewangles = roll_angle;

	RUN_PREDICTION(cmd, [&]() {
		new_origin = pl->m_vecOrigin();
	})

	// if our new origin differs then the roll solution is not fixable for movement
	// pitch yaw might not be either but FUCK YOU
	if (std::fabs(new_origin.x - origin.x) > DIST_EPSILON
		|| std::fabs(new_origin.y - origin.y) > DIST_EPSILON)
	{
		*cmd = backup;

		vec3 forward{};
		vec3 right{};
		vec3 up{};

		math::angleVectors(cmd->viewangles, &forward, &right, &up);
		const vec3 fixed{ forward + (right * final_spread.x) + (up * final_spread.y) };

		vec3 final_angle{};
		math::vectorAngles(fixed, final_angle);

		final_angle = (cmd->viewangles * 2) - final_angle;
		math::clampAngles(final_angle);

		tf_utils::fixMovement(cmd, final_angle);
		cmd->viewangles = final_angle;
	}

	// we're already at the end of the function but fuck it compiler will optimize it out
	if (!engine_pred->start(cmd)) {
		return;
	}}

float SeedPred::step()
{
	float t{ m_pp_time * 1000.0f };
	int i{ *reinterpret_cast<int *>((char *)&t) };
	int e{ (i >> 23) & 0xFF };

	return powf(2.0f, static_cast<float>(e) - (127.0f + 23.0f));
}

void SeedPred::run(CUserCmd *const cmd)
{
	if (!cfg::seed_pred_active || !cmd || !m_spread_init) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon || !tf_utils::isWeaponHitscan(weapon)) {
		return;
	}

	const float weapon_spread{ weapon->GetWeaponSpread() };

	if (weapon_spread <= 0.0f) {
		return;
	}

	seed_pred->sendPlayerPerf();

	const float server_time_pred{ predictServerTime() };

	if (server_time_pred <= 0.0f) {
		return;
	}

	if (!aim_utils->isAttacking(cmd, i::global_vars->curtime)) {
		return;
	}

	if (cvars::tf_use_fixed_weaponspreads && weapon->GetWeaponID() != TF_WEAPON_MINIGUN) {
		return;
	}

	CTFWeaponInfo *const weapon_info{ weapon->m_pWeaponInfo() };

	if (!weapon_info) {
		return;
	}

	const int bullets_per_shot{ static_cast<int>(tf_utils::attribHookValue(weapon_info->GetWeaponData(0).m_nBulletsPerShot, "mult_bullets_per_shot", weapon))};

	if (bullets_per_shot <= 0) {
		return;
	}

	const float last_shot{ i::global_vars->curtime - weapon->m_flLastFireTime() };
	const bool perfect_shot{ (bullets_per_shot > 1 && last_shot > 0.25f) || (bullets_per_shot == 1 && last_shot > 1.25f) };

	if (bullets_per_shot == 1 && perfect_shot) {
		return;
	}

	findBestSolution(local, cmd, weapon_spread, perfect_shot, bullets_per_shot);
}

void SeedPred::paint()
{
	if (!cfg::seed_pred_active || !cfg::seed_pred_indicator) {
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon || !tf_utils::isWeaponHitscan(weapon)) {
		return;
	}

	const vec2 pos{ draw->getScreenSize().x * 0.5f, draw->getScreenSize().y * 0.6f };

	size_t idx{};

	auto getPos = [&]() {
		const vec2 pos{ draw->getScreenSize().x * 0.5f, draw->getScreenSize().y * cfg::seed_pred_indicator_pos };
		return pos + vec2{ 0.0f, fonts::indicators.getTall() * static_cast<float>(idx++) };
	};

	const std::chrono::hh_mm_ss time{ std::chrono::seconds(static_cast<int>(predictServerTime())) };

	const int total_mins{ time.hours().count() * 60 + time.minutes().count() };

	Color clr{ cfg::color_indicators_bad.lerp(cfg::color_indicators_good, math::remap(step(), 0.0f, 32.0f, 0.0f, 1.0f))};

	if (tf_utils::isLoopback()) {
		clr = cfg::color_indicators_bad.lerp(cfg::color_indicators_good, math::remap(static_cast<float>(total_mins), 0.0f, 20.0f, 0.0f, 1.0f));
	}

	if (cfg::seed_pred_indicator == 1)
	{
		const std::string str{ tf_utils::isLoopback() ? "step: loopback" : std::format("step: {:.0f}", step()) };
		const std::string str1{ std::format("{} ({}h {}m)", str, time.hours().count(), time.minutes().count()) };

		draw->stringOutlined(getPos(), fonts::indicators.get(), str1, clr, POS_CENTERX);
	}
}

void SeedPred::onPostDataUpdate(C_TEFireBullets *tempent)
{
	if (!tempent) {
		return;
	}

	INetChannelInfo *netchan{ i::engine->GetNetChannelInfo() };
	if (!netchan) {
		return;
	}

	const int server_tick{ i::global_vars->tickcount };
	const int seed{ tempent->m_iSeed & 0xFF };

	const std::string str{ std::format("received bullet msg! tick: {} | seed: {} | predicted server time: {}\n", server_tick, seed, predictServerTime()) };
	
	i::cvar->ConsoleColorPrintf({ 255, 0, 100, 255 }, str.c_str());
}