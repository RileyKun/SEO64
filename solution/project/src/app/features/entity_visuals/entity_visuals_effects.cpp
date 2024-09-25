#include "entity_visuals_effects.hpp"
#include "entity_visuals_group.hpp"

#include "../vis_utils/vis_utils.hpp"
#include "../player_list/player_list.hpp"
#include "../cfg.hpp"

bool EntityVisualsEffects::onLoad()
{
	if (!i::engine->IsInGame() || !i::engine->IsConnected()) {
		return true;
	}

	if (!m_mat_flat)
	{
		KeyValues *const kv{ new KeyValues("VertexLitGeneric") };
		{
			kv->SetString("$basetexture", "vgui/white_additive");
			kv->SetString("$bumpmap", "vgui/white_additive");
			kv->SetString("$selfillum", "1");
			kv->SetString("$selfillumFresnel", "1");
			kv->SetString("$selfillumFresnelMinMaxExp", "[0.4999 0.5 1]");
			kv->SetString("$cloakPassEnabled", "1");
			kv->SetString("$nodecal", "1");
			kv->SetString("$model", "1");

			if (KeyValues *const proxies{ kv->FindKey("Proxies", true) }) {
				proxies->FindKey("invis", true);
			}
		}

		m_mat_flat = i::mat_sys->CreateMaterial("m_mat_flat", kv);
	}

	if (!m_mat_shaded)
	{
		KeyValues *const kv{ new KeyValues("VertexLitGeneric") };
		{
			kv->SetString("$basetexture", "vgui/white_additive");
			kv->SetString("$bumpmap", "models/player/shared/shared_normal");
			kv->SetString("$selfillum", "1");
			kv->SetString("$selfillumFresnel", "1");
			kv->SetString("$selfillumFresnelMinMaxExp", "[0.1 0.5 2]");
			kv->SetString("$cloakPassEnabled", "1");
			kv->SetString("$nodecal", "1");
			kv->SetString("$model", "1");

			if (KeyValues *const proxies{ kv->FindKey("Proxies", true) }) {
				proxies->FindKey("invis", true);
			}
		}

		m_mat_shaded = i::mat_sys->CreateMaterial("m_mat_shaded", kv);
	}

	if (!m_mat_glossy)
	{
		KeyValues *const kv{ new KeyValues("VertexLitGeneric") };
		{
			kv->SetString("$basetexture", "vgui/white_additive");
			kv->SetString("$bumpmap", "models/player/shared/shared_normal");
			kv->SetString("$envmap", "cubemaps/cubemap_sheen002");
			kv->SetString("$envmapfresnel", "1");
			kv->SetString("$phong", "1");
			kv->SetString("$phongfresnelranges", "[0 1 2]");
			kv->SetString("$selfillum", "1");
			kv->SetString("$selfillumFresnel", "1");
			kv->SetString("$selfillumFresnelMinMaxExp", "[0 0.1 1]");
			kv->SetString("$cloakPassEnabled", "1");
			kv->SetString("$nodecal", "1");
			kv->SetString("$model", "1");

			if (KeyValues *const proxies{ kv->FindKey("Proxies", true) }) {
				proxies->FindKey("invis", true);
			}
		}

		m_mat_glossy = i::mat_sys->CreateMaterial("m_mat_glossy", kv);
	}

	if (!m_mat_glow)
	{
		KeyValues *const kv{ new KeyValues("VertexLitGeneric") };
		{
			kv->SetString("$basetexture", "vgui/white_additive");
			kv->SetString("$bumpmap", "models/player/shared/shared_normal");
			kv->SetString("$envmap", "effects/saxxy_gold");
			kv->SetString("$envmapfresnel", "1");
			kv->SetString("$phong", "1");
			kv->SetString("$phongfresnelranges", cvars::mat_hdr_level->GetInt() > 1 ? "[0 0.05 0.1]" : "[0 1 2]");
			kv->SetString("$selfillum", "1");
			kv->SetString("$selfillumFresnel", "1");
			kv->SetString("$selfillumFresnelMinMaxExp", "[0.4999 0.5 0]");
			kv->SetString("$envmaptint", "[0 0 0]");
			kv->SetString("$selfillumtint", "[0.03 0.03 0.03]");
			kv->SetString("$cloakPassEnabled", "1");
			kv->SetString("$nodecal", "1");
			kv->SetString("$model", "1");

			if (KeyValues *const proxies{ kv->FindKey("Proxies", true) }) {
				proxies->FindKey("invis", true);
			}
		}

		m_mat_glow = i::mat_sys->CreateMaterial("m_mat_glow", kv);
		m_mat_glow_envmap_tint = m_mat_glow->FindVar("$envmaptint", nullptr);
		m_mat_glow_selfillum_tint = m_mat_glow->FindVar("$selfillumtint", nullptr);

		m_mat_glow_selfillum_tint->SetVecValue(0.03f, 0.03f, 0.03f); // TODO: var
	}

	if (!m_mat_plastic)
	{
		KeyValues *const kv{ new KeyValues("VertexLitGeneric") };
		{
			kv->SetString("$basetexture", "models/player/shared/ice_player");
			kv->SetString("$bumpmap", "models/player/shared/shared_normal");
			kv->SetString("$phong", "1");
			kv->SetString("$phongexponent", "10");
			kv->SetString("$phongboost", "1");
			kv->SetString("$phongfresnelranges", "[0 0 0]");
			kv->SetString("$basemapalphaphongmask", "1");
			kv->SetString("$phongwarptexture", "models/player/shared/ice_player_warp");
			kv->SetString("$cloakPassEnabled", "1");
			kv->SetString("$nodecal", "1");
			kv->SetString("$model", "1");

			if (KeyValues *const proxies{ kv->FindKey("Proxies", true) }) {
				proxies->FindKey("invis", true);
			}
		}

		m_mat_plastic = i::mat_sys->CreateMaterial("m_mat_plastic", kv);
	}

	if (!m_mat_glow_color) {
		m_mat_glow_color = i::mat_sys->FindMaterial("dev/glow_color", TEXTURE_GROUP_OTHER);
	}

	if (!m_tex_rt_buff_0)
	{
		m_tex_rt_buff_0 = i::mat_sys->CreateNamedRenderTargetTextureEx
		(
			"m_tex_rt_buff_0",
			i::client->GetScreenWidth(),
			i::client->GetScreenHeight(),
			RT_SIZE_LITERAL,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
			CREATERENDERTARGETFLAGS_HDR
		);

		m_tex_rt_buff_0->IncrementReferenceCount();
	}

	if (!m_tex_rt_buff_1)
	{
		m_tex_rt_buff_1 = i::mat_sys->CreateNamedRenderTargetTextureEx
		(
			"m_tex_rt_buff_1",
			i::client->GetScreenWidth(),
			i::client->GetScreenHeight(),
			RT_SIZE_LITERAL,
			IMAGE_FORMAT_RGB888,
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT | TEXTUREFLAGS_EIGHTBITALPHA,
			CREATERENDERTARGETFLAGS_HDR
		);

		m_tex_rt_buff_1->IncrementReferenceCount();
	}

	if (!m_mat_halo_add_to_screen)
	{
		KeyValues *const kv{ new KeyValues("UnlitGeneric") }; {
			kv->SetString("$basetexture", "m_tex_rt_buff_0");
			kv->SetString("$additive", "1");
		}

		m_mat_halo_add_to_screen = i::mat_sys->CreateMaterial("m_mat_halo_add_to_screen", kv);
	}

	if (!m_mat_halo_add_to_screen_fat)
	{
		KeyValues *const kv{ new KeyValues("screenspace_general") };
		{
			kv->SetString("$PIXSHADER", "haloaddoutline_ps20");
			kv->SetString("$ALPHA_BLEND_COLOR_OVERLAY", "1");
			kv->SetString("$basetexture", "m_tex_rt_buff_0");
			kv->SetString("$ignorez", "1");
			kv->SetString("$linearread_basetexture", "1");
			kv->SetString("$linearwrite", "1");
			kv->SetString("$texture1", "dev/glow_red");
			kv->SetString("$texture2", "dev/glow_white");
			kv->SetString("$texture3", "dev/glow_blue");
			kv->SetString("$additive", "1");
		}

		m_mat_halo_add_to_screen_fat = i::mat_sys->CreateMaterial("m_mat_halo_add_to_screen_fat", kv);
	}

	if (!m_mat_blur_x)
	{
		KeyValues *const kv{ new KeyValues("BlurFilterX") }; {
			kv->SetString("$basetexture", "m_tex_rt_buff_0");
		}

		m_mat_blur_x = i::mat_sys->CreateMaterial("m_mat_blur_x", kv);
	}

	if (!m_mat_blur_y)
	{
		KeyValues *const kv{ new KeyValues("BlurFilterY") }; {
			kv->SetString("$basetexture", "m_tex_rt_buff_1");
		}

		m_mat_blur_y = i::mat_sys->CreateMaterial("m_mat_blur_y", kv);
		m_bloom_amount = m_mat_blur_y->FindVar("$bloomamount", nullptr);
	}

	return true;
}

bool EntityVisualsEffects::onUnload()
{
	if (m_mat_flat) {
		m_mat_flat->DecrementReferenceCount();
		m_mat_flat->DeleteIfUnreferenced();
		m_mat_flat = nullptr;
	}

	if (m_mat_shaded) {
		m_mat_shaded->DecrementReferenceCount();
		m_mat_shaded->DeleteIfUnreferenced();
		m_mat_shaded = nullptr;
	}

	if (m_mat_glossy) {
		m_mat_glossy->DecrementReferenceCount();
		m_mat_glossy->DeleteIfUnreferenced();
		m_mat_glossy = nullptr;
	}

	if (m_mat_glow) {
		m_mat_glow->DecrementReferenceCount();
		m_mat_glow->DeleteIfUnreferenced();
		m_mat_glow = nullptr;
	}

	if (m_mat_plastic) {
		m_mat_plastic->DecrementReferenceCount();
		m_mat_plastic->DeleteIfUnreferenced();
		m_mat_plastic = nullptr;
	}

	if (m_mat_halo_add_to_screen) {
		m_mat_halo_add_to_screen->DecrementReferenceCount();
		m_mat_halo_add_to_screen->DeleteIfUnreferenced();
		m_mat_halo_add_to_screen = nullptr;
	}

	if (m_mat_halo_add_to_screen_fat) {
		m_mat_halo_add_to_screen_fat->DecrementReferenceCount();
		m_mat_halo_add_to_screen_fat->DeleteIfUnreferenced();
		m_mat_halo_add_to_screen_fat = nullptr;
	}

	if (m_tex_rt_buff_0) {
		m_tex_rt_buff_0->DecrementReferenceCount();
		m_tex_rt_buff_0->DeleteIfUnreferenced();
		m_tex_rt_buff_0 = nullptr;
	}

	if (m_tex_rt_buff_1) {
		m_tex_rt_buff_1->DecrementReferenceCount();
		m_tex_rt_buff_1->DeleteIfUnreferenced();
		m_tex_rt_buff_1 = nullptr;
	}

	if (m_mat_blur_x) {
		m_mat_blur_x->DecrementReferenceCount();
		m_mat_blur_x->DeleteIfUnreferenced();
		m_mat_blur_x = nullptr;
	}

	if (m_mat_blur_y) {
		m_mat_blur_y->DecrementReferenceCount();
		m_mat_blur_y->DeleteIfUnreferenced();
		m_mat_blur_y = nullptr;
	}

	return true;
}

bool EntityVisualsEffects::onLevelInit()
{
	return onLoad();
}

bool EntityVisualsEffects::onLevelShutdown()
{
	return onUnload();
}

void EntityVisualsEffects::clearDrawn()
{
	m_drawn_ents.clear();
	m_outline_ents.clear();
}

bool EntityVisualsEffects::hasDrawn(const EntityHandle_t ehandle) const
{
	return !m_drawing_outlines && m_drawn_ents.contains(ehandle.GetIndex());
}

void EntityVisualsEffects::drawModelOutline(C_BaseEntity *const ent, const bool model)
{
	if (!ent) {
		return;
	}

	tf_globals::block_mat_override = !model;

	m_drawing_outlines = true;

	if (ent->GetClassId() == class_ids::CTFPlayer)
	{
		if (C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() })
		{
			const float og_invis{ pl->m_flInvisibility() };

			if (og_invis > 0.99f)
			{
				pl->m_flInvisibility() = 0.0f;

				if (model) {
					i::render_view->SetBlend(0.0f);
				}
			}

			pl->DrawModel(model ? STUDIO_RENDER : (STUDIO_RENDER | STUDIO_NOSHADOWS));

			if (og_invis > 0.99f)
			{
				pl->m_flInvisibility() = og_invis;

				if (model) {
					i::render_view->SetBlend(1.0f);
				}
			}
		}
	}

	else {
		ent->DrawModel(model ? STUDIO_RENDER : (STUDIO_RENDER | STUDIO_NOSHADOWS));
	}

	if (model) {
		m_drawn_ents.insert(ent->GetRefEHandle().GetIndex());
	}

	tf_globals::block_mat_override = false;

	m_drawing_outlines = false;
}

bool EntityVisualsEffects::getPaintProcessData(C_BaseEntity *const ent, int effects, PaintProcessData &out) const
{
	if (!ent) {
		return false;
	}

	if (!effects) {
		return false;
	}

	auto has_effect = [&effects](const EVEffect effect)
	{
		return ( effects & static_cast<int>(effect) );
	};

	const size_t class_id{ ent->GetClassId() };

	if (class_id == class_ids::CTFPlayer)
	{
		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

		if (!pl) {
			return false;
		}

		player_info_t pi{};

		if (!i::engine->GetPlayerInfo(pl->entindex(), &pi)) {
			return false;
		}

		if (out.has_name = has_effect(EVEffect::NAME))
		{
			out.name = utils::utf8ToWide(pi.name);
		}

		if (out.has_health = (has_effect(EVEffect::HP) || has_effect(EVEffect::HPBAR)))
		{
			out.health_cur = static_cast<float>(pl->m_iHealth());
			out.health_max = static_cast<float>(pl->GetMaxHealth());
		}

		if (out.has_class = (has_effect(EVEffect::CLASS) || has_effect(EVEffect::CLASSICON)))
		{
			out.class_name = vis_utils->getClassNameWide(pl->m_iClass());
			out.class_icon_texture = vis_utils->getClassIconTextureId(pl->m_iClass());
		}

		PlayerListTag *const tag{ player_list->findPlayer(pl) };

		if (out.has_tags = tag && has_effect(EVEffect::TAGS))
		{
			std::wstring tag_name{ utils::utf8ToWide(tag->name) };
			out.tag = std::format(L"[{}]", tag_name);
			out.tag_clr = tag->color;
		}

		C_TFWeaponBase * const weapon{ pl->m_hActiveWeapon()->cast<C_TFWeaponBase>() };
		if (out.has_weapon = weapon != nullptr && has_effect(EVEffect::WEAPONS)) {
			out.weapon_ptr = weapon;

			for (int i{ 0 }; i < 8; i++) {
				void * econ_item_view{ s::CTFPlayerSharedUtils_GetEconItemViewByLoadoutSlot.call<void *>(pl, i, nullptr) };
				if (econ_item_view) {
                    out.weapon_names.push_back(s::C_EconItemView_GetItemName.call<const wchar_t *>(econ_item_view));
				}
			}

			if (pl->m_iClass() == TF_CLASS_ENGINEER) {
				// Shotgun, Pistol, Wrench, PDA, Construction PDA, Destruction PDA
                out.weapon_names.erase(out.weapon_names.begin() + 3);
			}
			else if (pl->m_iClass() == TF_CLASS_SPY) {
				// Revolver, Knife, Sappder, Disguise Kit, Invis Watch
                std::swap(out.weapon_names[1], out.weapon_names[2]);
			}

			out.active_weapon_idx = weapon->GetSlot();
		}

		C_WeaponMedigun *const medigun{ pl->Weapon_OwnsThisID(TF_WEAPON_MEDIGUN)->cast<C_WeaponMedigun>() };
		if (out.has_uber = medigun != nullptr && (has_effect(EVEffect::UBER) || has_effect(EVEffect::UBERBAR)))
		{
			out.is_vaccinator = medigun->GetMedigunType() == MEDIGUN_RESIST;
			out.uber_cur = medigun->m_flChargeLevel();
		}

		if (out.has_steamid = has_effect(EVEffect::STEAMID))
		{
			CSteamID steam_id(pi.friendsID, k_EUniversePublic, k_EAccountTypeIndividual);
			out.steamid = std::to_wstring(steam_id.ConvertToUint64());
		}

		if (out.has_conditions = has_effect(EVEffect::CONDITIONS))
		{
			if (pl->IsZoomed()) {
				out.conditions.push_back(L"ZOOM");
			}

			if (pl->IsInvisible()) {
				out.conditions.push_back(L"INVIS");
			}

			if (pl->m_bFeignDeathReady()) {
				out.conditions.push_back(L"DR");
			}

			if (pl->IsInvulnerable()) {
				out.conditions.push_back(L"INVULN");
			}

			if (pl->IsCritBoosted()) {
				out.conditions.push_back(L"CRIT");
			}

			if (pl->IsMiniCritBoosted()) {
				out.conditions.push_back(L"MINICRIT");
			}

			if (pl->IsMarked()) {
				out.conditions.push_back(L"MARKED");
			}

			if (pl->InCond(TF_COND_MAD_MILK)) {
				out.conditions.push_back(L"MILK");
			}

			if (pl->InCond(TF_COND_TAUNTING)) {
				out.conditions.push_back(L"TAUNT");
			}

			if (pl->InCond(TF_COND_DISGUISED))
			{
				out.conditions.push_back(L"DISGUISE");

				if (C_TFPlayer *const local{ ec->getLocal() })
				{
					if (!pl->InCond(TF_COND_STEALTHED) && pl->m_iTeamNum() != local->m_iTeamNum() && local->InCond(TF_COND_TEAM_GLOWS)) {
						out.conditions.push_back(L"XRAY");
					}
				}
			}

			if (pl->InCond(TF_COND_BURNING) || pl->InCond(TF_COND_BURNING_PYRO)) {
				out.conditions.push_back(L"BURNING");
			}

			if (pl->InCond(TF_COND_OFFENSEBUFF)) {
				out.conditions.push_back(L"BANNER");
			}

			if (pl->InCond(TF_COND_DEFENSEBUFF)) {
				out.conditions.push_back(L"BACKUP");
			}

			if (pl->InCond(TF_COND_REGENONDAMAGEBUFF)) {
				out.conditions.push_back(L"CONCH");
			}

			if (pl->InCond(TF_COND_MEDIGUN_UBER_BULLET_RESIST)) {
				out.conditions.push_back(L"BULLETRES");
			}

			if (pl->InCond(TF_COND_MEDIGUN_UBER_BLAST_RESIST)) {
				out.conditions.push_back(L"BLASTRES");
			}

			if (pl->InCond(TF_COND_MEDIGUN_UBER_FIRE_RESIST)) {
				out.conditions.push_back(L"FIRERES");
			}

			for (const EntityHandle_t flag : ec->getGroup(ECGroup::CTF_FLAGS))
			{
				C_CaptureFlag *const ctf_flag{ i::ent_list->GetClientEntityFromHandle(flag)->cast<C_CaptureFlag>() };

				if (!ctf_flag) {
					continue;
				}

				if (tf_utils::isEntOwnedBy(ctf_flag, pl)) {
					out.conditions.push_back(L"HAS FLAG");
					break;
				}
			}
		}
	}

	else if (class_id == class_ids::CObjectSentrygun
		|| class_id == class_ids::CObjectDispenser
		|| class_id == class_ids::CObjectTeleporter)
	{
		C_BaseObject *const obj{ ent->cast<C_BaseObject>() };

		if (!obj) {
			return false;
		}

		if (out.has_name = has_effect(EVEffect::NAME))
		{
			if (class_id == class_ids::CObjectSentrygun) {
				out.name = obj->m_bMiniBuilding() ? L"mini sentry" : L"sentry";
			}

			else if (class_id == class_ids::CObjectDispenser) {
				out.name = L"dispenser";
			}

			else if (class_id == class_ids::CObjectTeleporter) {
				out.name = obj->m_iObjectMode() == MODE_TELEPORTER_ENTRANCE ? L"tele in" : L"tele out";
			}
		}

		if (out.has_health = (has_effect(EVEffect::HP) || has_effect(EVEffect::HPBAR)))
		{
			out.health_cur = static_cast<float>(obj->m_iHealth());
			out.health_max = static_cast<float>(obj->m_iMaxHealth());
		}

		if (out.has_conditions = has_effect(EVEffect::CONDITIONS))
		{
			if (obj->IsDisabled()) {
				out.conditions.push_back(L"DISABLED");
			}

			if (obj->m_bBuilding()) {
				out.conditions.push_back(L"BUILDING");
			}

			if (class_id == class_ids::CObjectSentrygun && obj->cast<C_ObjectSentrygun>()->m_bShielded()) {
				out.conditions.push_back(L"WRANGLED");
			}
		}
	}

	else if (class_id == class_ids::CTFProjectile_Rocket || class_id == class_ids::CTFProjectile_SentryRocket
		|| class_id == class_ids::CTFProjectile_Jar || class_id == class_ids::CTFProjectile_JarGas
		|| class_id == class_ids::CTFProjectile_JarMilk || class_id == class_ids::CTFProjectile_Arrow
		|| class_id == class_ids::CTFProjectile_Flare || class_id == class_ids::CTFProjectile_Cleaver
		|| class_id == class_ids::CTFProjectile_HealingBolt || class_id == class_ids::CTFGrenadePipebombProjectile
		|| class_id == class_ids::CTFProjectile_BallOfFire || class_id == class_ids::CTFProjectile_EnergyRing
		|| class_id == class_ids::CTFProjectile_EnergyBall)
	{
		C_BaseProjectile *const proj{ ent->cast<C_BaseProjectile>() };

		if (out.has_name = has_effect(EVEffect::NAME))
		{
			out.name = vis_utils->getProjectileName(proj);
		}

		if (out.has_conditions = has_effect(EVEffect::CONDITIONS))
		{
			if (proj->IsCritical()) {
				out.conditions.push_back(L"CRIT");
			}
		}
	}

	else if (ec->isEntityAmmoPack(ent))
	{
		if (out.has_name = has_effect(EVEffect::NAME))
		{
			out.name = L"ammo pack";
		}

		// TOOD: respawn timer
	}

	else if (ec->isEntityHealthPack(ent))
	{
		if (out.has_name = has_effect(EVEffect::NAME))
		{
			out.name = L"health pack";
		}

		// TOOD: respawn timer
	}

	else if (class_id == class_ids::CHalloweenGiftPickup)
	{
		if (out.has_name = has_effect(EVEffect::NAME))
		{
			out.name = L"halloween gift";
		}
	}

	else if (class_id == class_ids::CCurrencyPack)
	{
		if (out.has_name = has_effect(EVEffect::NAME))
		{
			out.name = L"money";
		}
	}

	else if (class_id == class_ids::CCaptureFlag)
	{
		C_CaptureFlag *const flag{ ent->cast<C_CaptureFlag>() };

		if (flag->m_hOwnerEntity().Get() != nullptr) { // don't draw flag if it's being carried
			return false;
		}

		if (out.has_name = has_effect(EVEffect::NAME))
		{
			out.name = L"flag";
		}
	}

	return true;
}

float EntityVisualsEffects::drawWeaponIcon(C_TFWeaponBase *const weapon, const float x, const float y, const float w, const float h, const Color clr)
{
	if (!weapon) {
		return 0.0f;
	}

	struct CachedTexture
	{
		int id{};
		int w{};
		int h{};
		float left{};
		float right{};
		float top{};
		float bottom{};
	};

	static std::unordered_map<int, CachedTexture> weapon_icons{};

	const int item_def_index{ weapon->m_iItemDefinitionIndex() };

	if (const auto it{ weapon_icons.find(item_def_index) }; it != weapon_icons.end())
	{
		const CachedTexture &cached{ it->second };

		float subrect[4]{ cached.left, cached.right, cached.top, cached.bottom };

		const float width{ static_cast<float>(cached.w) * 0.45f };
		const float height{ static_cast<float>(cached.h) * 0.45f };

		draw->texturedSubrect({ x + (w * 0.5f) - (width * 0.5f), y + h + cfg::esp_spacing_y }, { width, height }, cached.id, subrect, clr);

		return height;
	}

	void *const item_schema{ s::GetItemSchema.call<void *>() };

	if (!item_schema) {
		return 0.0f;
	}

	void *const item_definition{ s::CEconItemSchema_GetItemDefinition.call<void *>(item_schema, item_def_index) };

	if (!item_definition) {
		return 0.0f;
	}

	constexpr size_t offset{ 8 };

	KeyValues *const kv{ *reinterpret_cast<KeyValues **>(reinterpret_cast<uintptr_t>(item_definition) + offset) };

	if (!kv) {
		return 0.0f;
	}

	enum KeyType
	{
		KEYTYPE_ITEM_ICONNAME,
		KEYTYPE_PREFAB,
		KEYTYPE_ITEM_CLASS
	};

	KeyValues *const icon_name_kv{ kv->FindKey("item_iconname", false) };
	KeyValues *const icon_prefab_kv{ kv->FindKey("prefab", false) };
	KeyValues *const icon_class_kv{ kv->FindKey("item_class", false) };

	std::string fmt{};
	KeyType key_type{ KEYTYPE_ITEM_ICONNAME };

	if (icon_name_kv && fmt.empty()) {
		fmt = icon_name_kv->m_sValue;
		key_type = KEYTYPE_ITEM_ICONNAME;
	}

	if (icon_class_kv && fmt.empty()) {
		fmt = icon_class_kv->m_sValue;
		key_type = KEYTYPE_ITEM_CLASS;
	}

	if (icon_prefab_kv && fmt.empty())
	{
		if (std::string_view{ icon_prefab_kv->m_sValue }.compare("valve") != 0) {
			fmt = icon_prefab_kv->m_sValue;
			key_type = KEYTYPE_PREFAB;
		}
	}

	switch (key_type)
	{
		case KEYTYPE_PREFAB:
		{
			/*"weapon_eyelander" - remove the weapon_*/
			if (fmt.find("weapon_") == 0) {
				fmt.erase(0, 7);
			}

			break;
		}
		case KEYTYPE_ITEM_CLASS:
		{
			/*"tf_weapon_sword" - remove the tf_weapon_*/
			if (fmt.find("tf_weapon_") == 0) {
				fmt.erase(0, 10);
			}

			break;
		}

		default: {
			break;
		}
	}

	// edge case time!
	const hash_t hash{ HASH_RT(fmt.c_str()) };

	switch (hash)
	{
		case HASH_CT("shotgun"): {
			fmt = "shotgun_soldier";
			break;
		}

		case HASH_CT("grenadelauncher"): {
			fmt = "tf_projectile_pipe";
			break;
		}

		case HASH_CT("pipebomblauncher"): {
			fmt = "tf_projectile_pipe_remote";
			break;
		}

		default: {
			break;
		}
	}

	fmt = std::format("d_{}", fmt);

	std::transform(fmt.begin(), fmt.end(), fmt.begin(), ::tolower);

	CHudTexture *const icon{ s::CHudBaseDeathNotice_GetIcon.call<CHudTexture *>(nullptr, fmt.c_str(), 0) };

	if (!icon || icon->bRenderUsingFont || icon->Width() <= 0 || icon->Height() <= 0) {
		return 0.0f;
	}

	weapon_icons[item_def_index] = CachedTexture
	{
		icon->textureId,
		icon->Width(),
		icon->Height(),
		icon->texCoords[0],
		icon->texCoords[1],
		icon->texCoords[2],
		icon->texCoords[3]
	};

	return 0.0f;
}

void EntityVisualsEffects::processPaint(C_BaseEntity *const ent, const EVGroup& group)
{
	const int effects{ group.effects };
	if (!effects) {
		return;
	}

	float x{};
	float y{};
	float w{};
	float h{};

	if (!vis_utils->getScreenBounds(ent, x, y, w, h)) {
		return;
	}

	PaintProcessData ppd{};

	if (!getPaintProcessData(ent, effects, ppd)) {
		return;
	}

	const vec2 bbox_pos{ x, y };
	const vec2 bbox_size{ w, h };
	const float text_spacing{ fonts::esp.getTall() + cfg::esp_spacing_y };
	const float cond_spacing{ fonts::esp_conds.getTall() + cfg::esp_spacing_y };
	const Color ent_clr{ vis_utils->getEntColor(ent) };

	auto drawName = [](const vec2 bbox_pos, const vec2 &bbox_size, std::wstring_view name, const Color clr, float *text_offset)
	{
		vec2 pos{ bbox_pos.x + (bbox_size.x * 0.5f), (bbox_pos.y - (fonts::esp.getTall() + cfg::esp_spacing_y)) };

		if (cfg::esp_name_pos == 1 && text_offset) {
			pos = vec2{ bbox_pos.x + (bbox_size.x + cfg::esp_spacing_x), bbox_pos.y + (fonts::esp.getTall() * (*text_offset)) };
			*text_offset += fonts::esp.getTall() + cfg::esp_spacing_y;
		}

		draw->string(pos, fonts::esp.get(), name, clr, cfg::esp_name_pos == 0 ? POS_CENTERX : POS_DEFAULT);
	};

	auto drawHpBar = [](const vec2 &bbox_pos, const vec2 &bbox_size, float hp, const float max_hp, const Color clr, const bool outline)
	{
		if (hp > max_hp) {
			hp = max_hp;
		}

		const float bar_w{ 2.0f };
		const float bar_h{ bbox_size.y };

		const float bar_x{ bbox_pos.x - ((bar_w * 2.0f) + cfg::esp_spacing_x) };
		const float bar_y{ bbox_pos.y };

		const float fill_h{ floorf((hp / max_hp) * bar_h) };

		draw->rectFilled({ bar_x - 1.0f, bar_y - 1.0f }, { bar_w + 2.0f, bar_h + 2.0f }, cfg::color_esp_outline);

		draw->rectFilled({ bar_x, bar_y + (bar_h - fill_h) }, { bar_w, fill_h }, clr);

		if (cfg::esp_healthbar_divisions > 1) {
			for (int n{ 1 }; n < cfg::esp_healthbar_divisions; n++) {
				draw->rectFilled({ bar_x, bar_y + ((bar_h / cfg::esp_healthbar_divisions) * n) }, { bar_w, 1.0f }, cfg::color_esp_outline);
			}
		}
	};

	auto drawUberBar = [](const vec2 &bbox_pos, const vec2 &bbox_size, float uber, bool vaccinator, const Color clr)
	{
		const float bar_w{ bbox_size.x };
		const float bar_h{ 2.0f };

		const float bar_x{ bbox_pos.x };
		const float bar_y{ bbox_pos.y + bbox_size.y + ((bar_h * 2.0f) + cfg::esp_spacing_y) };

		const float fill_w{ floorf(uber * bar_w) };

		draw->rectFilled({ bar_x - 1.0f, bar_y - 1.0f }, { bar_w + 2.0f, bar_h + 2.0f }, cfg::color_esp_outline);
		draw->rectFilled({ bar_x, bar_y }, { fill_w, bar_h }, clr);

		if (vaccinator) {
			for (int n{ 1 }; n < 4; n++) {
				draw->rectFilled({ bar_x + ((bar_w / 4) * n), bar_y }, { 1.0f, bar_h }, cfg::color_esp_outline);
			}
		}
	};

	auto drawBox = [](const vec2 &bbox_pos, const vec2 &bbox_size, const Color clr)
	{
		draw->rect({ bbox_pos.x, bbox_pos.y }, { bbox_size.x, bbox_size.y }, clr);
		draw->rect({ bbox_pos.x - 1, bbox_pos.y - 1 }, { bbox_size.x + 2, bbox_size.y + 2 }, cfg::color_esp_outline);
		draw->rect({ bbox_pos.x + 1, bbox_pos.y + 1 }, { bbox_size.x - 2, bbox_size.y - 2 }, cfg::color_esp_outline);
	};

	float right_offset{};
	float bottom_offset{};

	const Color text_clr{ cfg::esp_text_color == 0 ? ent_clr : cfg::color_esp_text };

	if (ppd.has_name)
	{
		drawName
		(
			bbox_pos,
			bbox_size,
			ppd.name,
			text_clr,
			&right_offset
		);
	}

	if (effects & static_cast<int>(EVEffect::BOX))
	{
		drawBox(bbox_pos, bbox_size, ent_clr);
	}

	if (ppd.has_health) {
		if (effects & static_cast<int>(EVEffect::HP)) {
			draw->string
			(
				{ (bbox_pos.x + bbox_size.x) + cfg::esp_spacing_x, bbox_pos.y + right_offset },
				fonts::esp.get(),
				std::to_string(static_cast<int>(ppd.health_cur)),
				ppd.health_cur > ppd.health_max ? cfg::color_overheal : vis_utils->getHpColor(ppd.health_cur, ppd.health_max),
				POS_DEFAULT
			);

			right_offset += text_spacing;
		}

		if (effects & static_cast<int>(EVEffect::HPBAR)) {
			drawHpBar(bbox_pos, bbox_size, ppd.health_cur, ppd.health_max, vis_utils->getHpColor(ppd.health_cur, ppd.health_max), true);

			if (ppd.health_cur > ppd.health_max) {
				drawHpBar(bbox_pos, bbox_size, ppd.health_cur, ppd.health_max * 0.48f, cfg::color_overheal, false);
			}
		}
	}

	if (ppd.has_class) {
        if (effects & static_cast<int>(EVEffect::CLASS))
		{
			draw->string
			(
				{ (bbox_pos.x + bbox_size.x) + cfg::esp_spacing_x, bbox_pos.y + right_offset },
				fonts::esp.get(),
				ppd.class_name,
				text_clr,
				POS_DEFAULT
			);

			right_offset += text_spacing;
        }

        if (effects & static_cast<int>(EVEffect::CLASSICON))
        {
			const float icon_size{ 18.0f };

			const float draw_y
			{
				(ppd.has_name && cfg::esp_name_pos == 0)
				? (bbox_pos.y - (fonts::esp.getTall() + cfg::esp_spacing_y)) - (icon_size + 1.0f)
				: bbox_pos.y - (icon_size + cfg::esp_spacing_y + 1.0f)
			};

			draw->texture({ bbox_pos.x + (bbox_size.x / 2), draw_y }, { icon_size, icon_size }, ppd.class_icon_texture, POS_CENTERX);
		}
	}

	if (ppd.has_tags) {
		draw->string
		(
			{ bbox_pos.x + (bbox_size.x / 2), bbox_pos.y + bbox_size.y + cfg::esp_spacing_y + bottom_offset },
			fonts::esp.get(),
			ppd.tag,
			ppd.tag_clr,
			POS_CENTERX
		);

		bottom_offset += fonts::esp.getTall() + cfg::esp_spacing_y;
	}

	if (ppd.has_uber) {
        if (effects & static_cast<int>(EVEffect::UBER))
		{
            draw->string
			(
				{ (bbox_pos.x + bbox_size.x) + cfg::esp_spacing_x, bbox_pos.y + right_offset },
				fonts::esp.get(),
				ppd.is_vaccinator ? std::format(L"{}/4", static_cast<int>(ppd.uber_cur * 4.0f)) : std::format(L"{:.0f}%", ppd.uber_cur * 100.0f),
				text_clr,
				POS_DEFAULT
			);

            right_offset += text_spacing;
        }

		if (effects & static_cast<int>(EVEffect::UBERBAR)) {
			drawUberBar(bbox_pos, bbox_size, ppd.uber_cur, ppd.is_vaccinator, cfg::color_uber);

			bottom_offset += 6.0f + cfg::esp_spacing_y;
		}
	}

	if (ppd.has_steamid) {
		draw->string
		(
			{ (bbox_pos.x + bbox_size.x) + cfg::esp_spacing_x, bbox_pos.y + right_offset },
			fonts::esp.get(),
			ppd.steamid,
			text_clr,
			POS_DEFAULT
		);

		right_offset += text_spacing;
	}

	if (ppd.has_conditions) {
		const float draw_x{ bbox_pos.x + bbox_size.x + cfg::esp_spacing_x };
		const float tall{ fonts::esp_conds.getTall() };

		for (const std::wstring &cond : ppd.conditions)
		{
			draw->string
			(
				{ draw_x, bbox_pos.y + right_offset },
				fonts::esp_conds.get(),
				cond,
				cfg::color_conds,
				POS_DEFAULT
			);

			right_offset += cond_spacing;
		}
	}

	if (ppd.has_weapon) {
		if (group.weapon_type == EVWeaponType::ACTIVE_ICON) {
			bottom_offset += drawWeaponIcon(ppd.weapon_ptr, bbox_pos.x, bbox_pos.y + cfg::esp_spacing_y + bottom_offset, bbox_size.x, bbox_size.y, text_clr);
		}

		const float tall{ fonts::esp.getTall() };

		if (group.weapon_type == EVWeaponType::ALL_NAMES) {
			for (int n{ 0 }; n < ppd.weapon_names.size(); n++)
			{
				draw->string
				(
					{ bbox_pos.x + (bbox_size.x / 2), bbox_pos.y + bbox_size.y + cfg::esp_spacing_y + bottom_offset },
					fonts::esp.get(),
					ppd.weapon_names[n],
					n == ppd.active_weapon_idx ? text_clr : text_clr.scaled(0.7f),
					POS_CENTERX
				);

				bottom_offset += tall + cfg::esp_spacing_y;
			}
		}

		if (group.weapon_type == EVWeaponType::ACTIVE_NAME) {
			draw->string
			(
				{ bbox_pos.x + (bbox_size.x / 2), bbox_pos.y + bbox_size.y + cfg::esp_spacing_y + bottom_offset },
				fonts::esp.get(),
				ppd.weapon_names[ppd.active_weapon_idx],
				text_clr,
				POS_CENTERX
			);
		}
	}
}

void EntityVisualsEffects::processModel(const EVModelData *const data)
{
	if (!data || !data->ent) {
		return;
	}

	IMatRenderContext *const rc{ i::mat_sys->GetRenderContext() };

	if (!rc) {
		return;
	}

	if (data->has_outline)
	{
		ShaderStencilState_t sss_models{};

		sss_models.m_bEnable = true;
		sss_models.m_nReferenceValue = 1;
		sss_models.m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		sss_models.m_PassOp = STENCILOPERATION_REPLACE;
		sss_models.m_FailOp = STENCILOPERATION_KEEP;
		sss_models.m_ZFailOp = cfg::outlines_fill_occluded ? STENCILOPERATION_KEEP : STENCILOPERATION_REPLACE;

		sss_models.SetStencilState(rc);
	}

	IMaterial *mat{};
	IMaterial *og_mat{};
	OverrideType_t og_ot{};

	const Color og_clr{ i::render_view->GetColorModulation() };
	const float og_blend{ i::render_view->GetBlend() };

	if (data->has_material)
	{
		switch (data->type)
		{
			case EVMaterialType::ORIGINAL: {
				mat = nullptr;
				break;
			}

			case EVMaterialType::FLAT: {
				mat = m_mat_flat;
				break;
			}

			case EVMaterialType::SHADED: {
				mat = m_mat_shaded;
				break;
			}

			case EVMaterialType::GLOSSY: {
				mat = m_mat_glossy;
				break;
			}

			case EVMaterialType::GLOW: {
				mat = m_mat_glow;		
				m_mat_glow_envmap_tint->SetVecValue(data->ent_clr.r / 255.0f, data->ent_clr.g / 255.0f, data->ent_clr.b / 255.0f);
				break;
			}

			case EVMaterialType::PLASTIC: {
				mat = m_mat_plastic;
				break;
			}
		}

		if (mat) {
			i::model_render->GetMaterialOverride(&og_mat, &og_ot);
			i::model_render->ForcedMaterialOverride(mat);
		}

		if (!data->depth) {
			rc->DepthRange(0.0f, 0.2f);
		}

		i::render_view->SetColorModulation(data->ent_clr);
		i::render_view->SetBlend(data->alpha);
	}

	else {
		i::render_view->SetColorModulation({ 255, 255, 255, 255 });
		i::render_view->SetBlend(1.0f);
	}

	if (data->has_material) {
		tf_globals::block_mat_override = true;
	}

	drawModelOutline(data->ent, true);

	m_drawn_ents.insert(data->ent->GetRefEHandle().GetIndex());

	if (data->has_outline) {
		m_outline_ents.emplace_back(EVOutlineEnt{ data->ent, data->ent_clr, data->outline_alpha });
	}

	for (C_BaseEntity *attach{ data->ent->FirstMoveChild() }; attach; attach = attach->NextMovePeer())
	{
		drawModelOutline(attach, true);

		m_drawn_ents.insert(attach->GetRefEHandle().GetIndex());

		if (data->has_outline) {
			m_outline_ents.emplace_back(EVOutlineEnt{ attach, data->ent_clr, data->outline_alpha });
		}
	}

	if (data->has_material) {
		tf_globals::block_mat_override = false;
	}

	if (data->has_material)
	{
		if (mat) {
			i::model_render->ForcedMaterialOverride(og_mat, og_ot);
		}

		if (!data->depth) {
			rc->DepthRange(0.0f, 1.0f);
		}
	}

	i::render_view->SetColorModulation(og_clr);
	i::render_view->SetBlend(og_blend);

	if (data->has_outline) {
		ShaderStencilState_t{}.SetStencilState(rc);
	}
}

void EntityVisualsEffects::processOutlineEffect()
{
	if (m_outline_ents.empty() || !vis_utils->shouldRunVisuals()) {
		return;
	}

	IMatRenderContext *const rc{ i::mat_sys->GetRenderContext() };

	if (!rc) {
		return;
	}

	if (cfg::outlines_style == 0) {
		m_bloom_amount->SetIntValue(cfg::outlines_bloom_amount);
	}

	const float og_blend{ i::render_view->GetBlend() };
	const Color og_clr{ i::render_view->GetColorModulation() };

	IMaterial *og_mat{};
	OverrideType_t og_mat_override{};

	i::model_render->GetMaterialOverride(&og_mat, &og_mat_override);
	i::model_render->ForcedMaterialOverride(m_mat_glow_color);

	const int w{ i::client->GetScreenWidth() };
	const int h{ i::client->GetScreenHeight() };

	rc->PushRenderTargetAndViewport();
	{
		rc->SetRenderTarget(m_tex_rt_buff_0);
		rc->Viewport(0, 0, w, h);
		rc->ClearColor4ub(0, 0, 0, 0);
		rc->ClearBuffers(true, false, false);

		for (const EVOutlineEnt &ent : m_outline_ents) {
			i::render_view->SetBlend(ent.alpha);
			i::render_view->SetColorModulation(ent.clr);
			drawModelOutline(ent.ent, false);
		}
	}
	rc->PopRenderTargetAndViewport();

	if (cfg::outlines_style == 0)
	{
		rc->PushRenderTargetAndViewport();
		{
			rc->Viewport(0, 0, w, h);

			rc->SetRenderTarget(m_tex_rt_buff_1);
			rc->DrawScreenSpaceRectangle(m_mat_blur_x, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);

			rc->SetRenderTarget(m_tex_rt_buff_0);
			rc->DrawScreenSpaceRectangle(m_mat_blur_y, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		}
		rc->PopRenderTargetAndViewport();
	}

	ShaderStencilState_t sss_outlines{};

	sss_outlines.m_bEnable = true;
	sss_outlines.m_nWriteMask = 0x0;
	sss_outlines.m_nTestMask = 0xFF;
	sss_outlines.m_nReferenceValue = 0;
	sss_outlines.m_CompareFunc = STENCILCOMPARISONFUNCTION_EQUAL;
	sss_outlines.m_PassOp = STENCILOPERATION_KEEP;
	sss_outlines.m_FailOp = STENCILOPERATION_KEEP;
	sss_outlines.m_ZFailOp = STENCILOPERATION_KEEP;

	sss_outlines.SetStencilState(rc);

	if (cfg::outlines_style == 0) {
		rc->DrawScreenSpaceRectangle(m_mat_halo_add_to_screen, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
	}

	else if (cfg::outlines_style == 1) {
		rc->DrawScreenSpaceRectangle(m_mat_halo_add_to_screen, -1, -1, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		rc->DrawScreenSpaceRectangle(m_mat_halo_add_to_screen, -1, 1, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		rc->DrawScreenSpaceRectangle(m_mat_halo_add_to_screen, 1, -1, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
		rc->DrawScreenSpaceRectangle(m_mat_halo_add_to_screen, 1, 1, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
	}

	else if (cfg::outlines_style == 2) {
		rc->DrawScreenSpaceRectangle(m_mat_halo_add_to_screen_fat, 0, 0, w, h, 0.0f, 0.0f, w - 1, h - 1, w, h);
	}

	ShaderStencilState_t{}.SetStencilState(rc);

	i::render_view->SetBlend(og_blend);
	i::render_view->SetColorModulation(og_clr);

	i::model_render->ForcedMaterialOverride(og_mat, og_mat_override);
}