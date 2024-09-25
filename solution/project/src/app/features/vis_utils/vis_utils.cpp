#include "vis_utils.hpp"

#include "../icons/icons.hpp"
#include "../notifs/notifs.hpp"
#include "../player_list/player_list.hpp"
#include "../cfg.hpp"

bool VisualUtils::shouldSkipPlayer(
	C_TFPlayer *const local,
	C_TFPlayer *const pl,
	const bool ignore_local,
	const bool ignore_friends,
	const bool ignore_enemies,
	const bool ignore_teammates,
	const bool no_fp_check)
{
	if (!local || !pl) {
		return false;
	}

	if (!no_fp_check && pl == local && local->InFirstPersonView()) {
		return true;
	}

	const bool is_local{ pl == local };
	const bool is_friend{ pl->IsPlayerOnSteamFriendsList() };
	const bool is_enemy{ pl->m_iTeamNum() != local->m_iTeamNum() };
	const bool is_teammate{ pl->m_iTeamNum() == local->m_iTeamNum() };

	if (ignore_local && is_local) {
		return true;
	}

	if (ignore_friends && is_friend) {
		return true;
	}

	if (ignore_enemies && is_enemy && !is_local && !is_friend) {
		return true;
	}

	if (ignore_teammates && is_teammate && !is_local && !is_friend) {
		return true;
	}

	return false;
}

bool VisualUtils::shouldSkipBuilding(
	C_TFPlayer *const local,
	C_BaseObject *const obj,
	const bool ignore_local,
	const bool ignore_enemies,
	const bool ignore_teammates)
{
	if (!local || !obj) {
		return false;
	}

	if (obj->m_bCarried() || obj->m_bPlacing()) {
		return true;
	}

	const bool is_local{ tf_utils::isEntOwnedBy(obj, local) };
	const bool is_enemy{ obj->m_iTeamNum() != local->m_iTeamNum() };
	const bool is_teammate{ obj->m_iTeamNum() == local->m_iTeamNum() };

	if (ignore_local && is_local) {
		return true;
	}

	if (ignore_enemies && is_enemy && !is_local) {
		return true;
	}

	if (ignore_teammates && is_teammate && !is_local) {
		return true;
	}

	return false;
}

bool VisualUtils::shouldSkipProjectile(
	C_TFPlayer *const local,
	C_BaseEntity *const proj,
	const bool ignore_local,
	const bool ignore_enemies,
	const bool ignore_teammates)
{
	const bool is_local{ tf_utils::isEntOwnedBy(proj, local) };
	const bool is_enemy{ proj->m_iTeamNum() != local->m_iTeamNum() };
	const bool is_teammate{ proj->m_iTeamNum() == local->m_iTeamNum() };

	if (ignore_local && is_local) {
		return true;
	}

	if (ignore_enemies && is_enemy && !is_local) {
		return true;
	}

	if (ignore_teammates && is_teammate && !is_local) {
		return true;
	}

	return false;
}

bool VisualUtils::shouldRunVisuals()
{
	return i::engine->IsInGame() && i::engine->IsConnected() && !i::vgui->IsGameUIVisible();
}

bool VisualUtils::getScreenBounds(C_BaseEntity *const ent, float &x, float &y, float &w, float &h)
{
	if (!ent) {
		return false;
	}

	vec3 mins{};
	vec3 maxs{};

	const size_t class_id{ ent->GetClassId() };

	bool want_transform{};

	if (class_id == class_ids::CTFPlayer) {
		mins = ent->m_vecMins() * vec3{ 0.7f, 0.7f, 1.0f };
		maxs = ent->m_vecMaxs() * vec3{ 0.7f, 0.7f, 1.0f };
	}

	else if (class_id == class_ids::CObjectSentrygun
		|| class_id == class_ids::CObjectDispenser
		|| class_id == class_ids::CObjectTeleporter)
	{
		mins = ent->m_vecMins();
		maxs = ent->m_vecMaxs();

		if (class_id == class_ids::CObjectSentrygun && !ent->cast<C_ObjectSentrygun>()->m_bMiniBuilding())
		{
			const int level{ ent->cast<C_BaseObject>()->m_iUpgradeLevel() };

			if (level == 1) {
				maxs.z *= 0.7f;
			}

			else if (level == 3) {
				maxs.z *= 1.2f;
			}
		}
	}

	else {
		ent->GetRenderBounds(mins, maxs);
		want_transform = true;
	}

	const vec3 points[8]
	{
		{ mins.x, mins.y, mins.z },
		{ maxs.x, mins.y, mins.z },
		{ maxs.x, maxs.y, mins.z },
		{ mins.x, maxs.y, mins.z },
		{ mins.x, mins.y, maxs.z },
		{ maxs.x, mins.y, maxs.z },
		{ maxs.x, maxs.y, maxs.z },
		{ mins.x, maxs.y, maxs.z }
	};

	vec2 screen_points[8]{};

	for (int n{}; n < 8; n++)
	{
		if (want_transform)
		{
			vec3 transformed{};

			math::vectorTransform(points[n], ent->RenderableToWorldTransform(), transformed);

			if (!draw->worldToScreen(transformed, screen_points[n])) {
				return false;
			}
		}

		else
		{
			if (!draw->worldToScreen(ent->GetRenderOrigin() + points[n], screen_points[n])) {
				return false;
			}
		}
	}

	float min_x{ FLT_MAX };
	float min_y{ FLT_MAX };
	float max_x{ FLT_MIN };
	float max_y{ FLT_MIN };

	for (int n{}; n < 8; n++)
	{
		min_x = std::min(min_x, screen_points[n].x);
		min_y = std::min(min_y, screen_points[n].y);
		max_x = std::max(max_x, screen_points[n].x);
		max_y = std::max(max_y, screen_points[n].y);
	}

	x = std::floorf(min_x);
	y = std::floorf(min_y);
	w = std::floorf(max_x - min_x);
	h = std::floorf(max_y - min_y);
	
	return x <= draw->getScreenSize().x && (x + w) >= 0.0f && y <= draw->getScreenSize().y && (y + h) >= 0.0f;
}

Color VisualUtils::getHpColor(float hp, const float max_hp)
{
	if (hp > max_hp) {
		hp = max_hp;
	}

	if (cfg::esp_health_color == 1) {
		return cfg::color_esp_low_health.lerp(cfg::color_esp_high_health, hp / max_hp);
	}

	const int r{ static_cast<int>(std::min((510.0f * (max_hp - hp)) / max_hp, 210.0f)) };
	const int g{ static_cast<int>(std::min((510.0f * hp) / max_hp, 230.0f)) };

	return { static_cast<uint8_t>(r), static_cast<uint8_t>(g), 50, 255 };
}

Color VisualUtils::getEntColor(C_BaseEntity *const ent)
{
	const Color ERR_CLR{ 255, 255, 255, 255 };

	if (!ent) {
		return ERR_CLR;
	}

	if (ent->GetRefEHandle() == tf_globals::aimbot_target) {
		return cfg::color_target;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return ERR_CLR;
	}

	if (ent->GetClassId() == class_ids::CTFPlayer)
	{
		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

		if (!pl) {
			return ERR_CLR;
		}

		if (pl->IsInvulnerable()) {
			return cfg::color_invuln;
		}

		if (pl->IsInvisible()) {
			return cfg::color_invis;
		}

		if (pl != local && pl->IsPlayerOnSteamFriendsList()) {
			return cfg::color_friend;
		}

		player_info_t info{};
		if (i::engine->GetPlayerInfo(pl->entindex(), &info) && !info.fakeplayer) {
			if (PlayerListTag *const tag{ player_list->findPlayer(info.friendsID) }) {
				return tag->color;
			}
		}
	}

	if (ent == local || tf_utils::isEntOwnedBy(ent, local)) {
		return cfg::color_local;
	}

	if (ent->GetClassId() == class_ids::CBaseAnimating) {
		if (ec->isEntityAmmoPack(ent)) {
			return cfg::color_ammo_pack;
		}
		else if (ec->isEntityHealthPack(ent)) {
			return cfg::color_health_pack;
		}
	}

	if (ent->GetClassId() == class_ids::CHalloweenGiftPickup) {
		return cfg::color_halloween_gift;
	}

	if (ent->GetClassId() == class_ids::CCurrencyPack) {
		return cfg::color_mvm_money;
	}
	
	if (ent->GetClassId() == class_ids::CTFAmmoPack) {
		return cfg::color_ammo_pack;
	}

	if (ent->m_iTeamNum() != local->m_iTeamNum()) {
		return cfg::color_enemy;
	}

	if (ent->m_iTeamNum() == local->m_iTeamNum()) {
		return cfg::color_teammate;
	}

	return ERR_CLR;
}

std::string VisualUtils::getClassName(const int class_num)
{
	switch (class_num)
	{
		case TF_CLASS_SCOUT: {
			return "scout";
		}

		case TF_CLASS_SOLDIER: {
			return "soldier";
		}

		case TF_CLASS_PYRO: {
			return "pyro";
		}

		case TF_CLASS_DEMOMAN: {
			return "demoman";
		}

		case TF_CLASS_HEAVYWEAPONS: {
			return "heavy";
		}

		case TF_CLASS_ENGINEER: {
			return "engineer";
		}

		case TF_CLASS_MEDIC: {
			return "medic";
		}

		case TF_CLASS_SNIPER: {
			return "sniper";
		}

		case TF_CLASS_SPY: {
			return "spy";
		}

		default: {
			return "unknown";
		}
	}
}

std::wstring VisualUtils::getClassNameWide(const int class_num)
{
	switch (class_num)
	{
		case TF_CLASS_SCOUT:
		{
			return L"scout";
		}

		case TF_CLASS_SOLDIER:
		{
			return L"soldier";
		}

		case TF_CLASS_PYRO:
		{
			return L"pyro";
		}

		case TF_CLASS_DEMOMAN:
		{
			return L"demoman";
		}

		case TF_CLASS_HEAVYWEAPONS:
		{
			return L"heavy";
		}

		case TF_CLASS_ENGINEER:
		{
			return L"engineer";
		}

		case TF_CLASS_MEDIC:
		{
			return L"medic";
		}

		case TF_CLASS_SNIPER:
		{
			return L"sniper";
		}

		case TF_CLASS_SPY:
		{
			return L"spy";
		}

		default:
		{
			return L"unknown";
		}
	}
}

std::wstring VisualUtils::getProjectileName(C_BaseEntity *const projectile)
{
	if (!projectile) {
		return L"unknown";
	}

	const size_t class_id{ projectile->GetClassId() };

	if (class_id == class_ids::CTFProjectile_Rocket || class_id == class_ids::CTFProjectile_SentryRocket) {
		return L"rocket";
	}
	else if (class_id == class_ids::CTFProjectile_Jar) {
		return L"jar";
	}
	else if (class_id == class_ids::CTFProjectile_JarGas) {
		return L"jar gas";
	}
	else if (class_id == class_ids::CTFProjectile_JarMilk) {
		return L"jar milk";
	}
	else if (class_id == class_ids::CTFProjectile_Arrow) {
		return L"arrow";
	}
	else if (class_id == class_ids::CTFProjectile_Flare) {
		return L"flare";
	}
	else if (class_id == class_ids::CTFProjectile_Cleaver) {
		return L"cleaver";
	}
	else if (class_id == class_ids::CTFProjectile_HealingBolt) {
		return L"healing bolt";
	}
	else if (class_id == class_ids::CTFGrenadePipebombProjectile) {
		C_TFGrenadePipebombProjectile *const pipebomb{ projectile->cast<C_TFGrenadePipebombProjectile>() };

		if (pipebomb->HasStickyEffects()) {
			return L"stickybomb";
		}
		else if (pipebomb->m_iType() == TF_GL_MODE_CANNONBALL) {
			return L"cannonball";
		}

		return L"pipebomb";
	}
	else if (class_id == class_ids::CTFProjectile_BallOfFire) {
		return L"ball of fire";
	}
	else if (class_id == class_ids::CTFProjectile_EnergyRing) {
		return L"energy ring";
	}
	else if (class_id == class_ids::CTFProjectile_EnergyBall) {
		return L"energy ball";
	}

	return L"unknown";
}

int VisualUtils::createTextureRGBA(const unsigned char *rgba, int w, int h)
{
	const int texture_id{ i::surface->CreateNewTextureID(true) };

	i::surface->DrawSetTextureRGBAEx(texture_id, rgba, w, h, IMAGE_FORMAT_BGRA8888);

	return texture_id;
}

int VisualUtils::getClassIconTextureId(const int class_num)
{
	static const int icon_scout{ createTextureRGBA(icons::leaderboard_class_scout, 64, 64) };
	static const int icon_sniper{ createTextureRGBA(icons::leaderboard_class_sniper, 64, 64) };
	static const int icon_soldier{ createTextureRGBA(icons::leaderboard_class_soldier, 64, 64) };
	static const int icon_demo{ createTextureRGBA(icons::leaderboard_class_demo, 64, 64) };
	static const int icon_medic{ createTextureRGBA(icons::leaderboard_class_medic, 64, 64) };
	static const int icon_heavy{ createTextureRGBA(icons::leaderboard_class_heavy, 64, 64) };
	static const int icon_pyro{ createTextureRGBA(icons::leaderboard_class_pyro, 64, 64) };
	static const int icon_spy{ createTextureRGBA(icons::leaderboard_class_spy, 64, 64) };
	static const int icon_engineer{ createTextureRGBA(icons::leaderboard_class_engineer, 64, 64) };

	switch (class_num)
	{
		case TF_CLASS_SCOUT: {
			return icon_scout;
		}

		case TF_CLASS_SNIPER: {
			return icon_sniper;
		}

		case TF_CLASS_SOLDIER: {
			return icon_soldier;
		}

		case TF_CLASS_DEMOMAN: {
			return icon_demo;
		}

		case TF_CLASS_MEDIC: {
			return icon_medic;
		}

		case TF_CLASS_HEAVYWEAPONS: {
			return icon_heavy;
		}

		case TF_CLASS_PYRO: {
			return icon_pyro;
		}

		case TF_CLASS_SPY: {
			return icon_spy;
		}

		case TF_CLASS_ENGINEER: {
			return icon_engineer;
		}

		default: {
			return -1;
		}
	}
}

bool VisualUtils::isEntOnScreen(C_TFPlayer *const local, IClientEntity *const ent)
{
	const vec3 ent_pos{ ent->GetRenderOrigin() };

	if (ent_pos.distTo(local->m_vecOrigin()) < 300.0f) {
		return true;
	}

	vec2 ent_screen_pos{};

	if (!draw->worldToScreen(ent_pos, ent_screen_pos)) {
		return false;
	}

	if (ent_screen_pos.x < -400.0f || ent_screen_pos.x > draw->getScreenSize().x + 400.0f
		|| ent_screen_pos.y < -400.0f || ent_screen_pos.y > draw->getScreenSize().y + 400.0f)
	{
		return false;
	}

	return true;
}

bool VisualUtils::isPosOnScreen(C_TFPlayer *const local, const vec3 &pos)
{
	if (pos.distTo(local->m_vecOrigin()) < 300.0f) {
		return true;
	}

	vec2 ent_screen_pos{};

	if (!draw->worldToScreen(pos, ent_screen_pos)) {
		return false;
	}

	if (ent_screen_pos.x < -400.0f || ent_screen_pos.x > draw->getScreenSize().x + 400.0f
		|| ent_screen_pos.y < -400.0f || ent_screen_pos.y > draw->getScreenSize().y + 400.0f)
	{
		return false;
	}

	return true;
}

void VisualUtils::fireBeam(const vec3 &start, const vec3 &end, const Color &color, bool fade)
{
	BeamInfo_t beam_info{};

	beam_info.m_nType = 0;
	beam_info.m_pszModelName = "sprites/physbeam.vmt";
	beam_info.m_nModelIndex = -1;
	beam_info.m_flHaloScale = 0.0f;
	beam_info.m_flLife = 1.0f;
	beam_info.m_flWidth = 2.0f;
	beam_info.m_flEndWidth = 2.0f;
	beam_info.m_flFadeLength = 10.f;
	beam_info.m_flAmplitude = 0.0f;
	beam_info.m_flBrightness = 255.f;
	beam_info.m_flSpeed = 0.0f;
	beam_info.m_nStartFrame = 0;
	beam_info.m_flFrameRate = 0;

	// mfed: these values are 0.f to 255.f, weird
	beam_info.m_flRed = static_cast<float>(color.r);
	beam_info.m_flGreen = static_cast<float>(color.g);
	beam_info.m_flBlue = static_cast<float>(color.b);

	beam_info.m_nSegments = 2;

	if (fade) {
		beam_info.m_nFlags = FBEAM_FADEOUT;
	}

	beam_info.m_vecStart = start;
	beam_info.m_vecEnd = end;

	Beam_t *const beam{ i::beams->CreateBeamPoints(beam_info) };

	if (!beam) {
		notifs->warning("Failed to create beam");
		return;
	}

	i::beams->DrawBeam(beam);
}