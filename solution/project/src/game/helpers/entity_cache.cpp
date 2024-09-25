#include "entity_cache.hpp"

// myza: for isEntOwnedBy
#include "tf_utils.hpp"

bool EntityCache::isEntityHealthPack(C_BaseEntity *const ent)
{
	if (!ent) {
		return false;
	}

	if (m_health_pack_indexes.contains(ent->m_nModelIndex())) {
		return true;
	}

	return false;
}

bool EntityCache::isEntityAmmoPack(C_BaseEntity *const ent)
{
	if (!ent) {
		return false;
	}

	if (m_ammo_pack_indexes.contains(ent->m_nModelIndex())) {
		return true;
	}

	return false;
}

bool EntityCache::onLoad()
{
	if (!i::engine->IsInGame() || !i::engine->IsConnected()) {
		return true;
	}

	m_health_pack_indexes.clear();
	m_ammo_pack_indexes.clear();

	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/medkit_small.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/medkit_medium.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/medkit_large.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/props_halloween/halloween_medkit_small.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/props_halloween/halloween_medkit_medium.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/props_halloween/halloween_medkit_large.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/medkit_small_bday.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/medkit_medium_bday.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/medkit_large_bday.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/props_medieval/medieval_meat.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/plate.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/plate_sandwich_xmas.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/plate_robo_sandwich.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/workshop/weapons/c_models/c_fishcake/plate_fishcake.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/workshop/weapons/c_models/c_buffalo_steak/plate_buffalo_steak.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/workshop/weapons/c_models/c_chocolate/plate_chocolate.mdl"));
	m_health_pack_indexes.insert(i::model_info->GetModelIndex("models/items/banana/plate_banana.mdl"));

	m_ammo_pack_indexes.insert(i::model_info->GetModelIndex("models/items/ammopack_small.mdl"));
	m_ammo_pack_indexes.insert(i::model_info->GetModelIndex("models/items/ammopack_medium.mdl"));
	m_ammo_pack_indexes.insert(i::model_info->GetModelIndex("models/items/ammopack_large.mdl"));
	m_ammo_pack_indexes.insert(i::model_info->GetModelIndex("models/items/ammopack_small_bday.mdl"));
	m_ammo_pack_indexes.insert(i::model_info->GetModelIndex("models/items/ammopack_medium_bday.mdl"));
	m_ammo_pack_indexes.insert(i::model_info->GetModelIndex("models/items/ammopack_large_bday.mdl"));

	m_sentry_buster_index = 0;

	return true;
}

bool EntityCache::onLevelInit()
{
	return onLoad();
}

bool EntityCache::onLevelShutdown()
{
	m_groups.clear();

	m_health_pack_indexes.clear();
	m_ammo_pack_indexes.clear();

	return true;
}

C_TFPlayer *const EntityCache::getLocal()
{
	IClientEntity *const local{ i::ent_list->GetClientEntity(i::engine->GetLocalPlayer()) };

	if (!local) {
		return nullptr;
	}

	return local->cast<C_TFPlayer>();
}

C_TFWeaponBase *const EntityCache::getWeapon()
{
	C_TFPlayer *const local{ getLocal() };

	if (!local) {
		return nullptr;
	}

	C_BaseEntity *const wep{ local->m_hActiveWeapon().Get() };

	if (!wep) {
		return nullptr;
	}

	return wep->cast<C_TFWeaponBase>();
}

C_TFPlayerResource *const EntityCache::getPlayerResource()
{
	return m_player_resource;
}

C_TFGameRulesProxy *const EntityCache::getGameRules()
{
	// mfed: have to deref here, as dereferencing if this hasnt been initialized gives you a nullptr
	return *s::g_pGameRules.cast<C_TFGameRulesProxy **>();
}

bool EntityCache::isSentryBuster(C_TFPlayer *const ent)
{
	if (!m_sentry_buster_index) {
		m_sentry_buster_index = i::model_info->GetModelIndex("models/bots/demo/bot_sentry_buster.mdl");
	}

	return ent->m_nModelIndex() == m_sentry_buster_index && ent->m_iClass() == TF_CLASS_DEMOMAN;
}

void EntityCache::update()
{
	m_groups.clear();
	m_player_resource = nullptr;

	C_TFPlayer *const local{ getLocal() };

	if (!local || !local->IsInValidTeam()) {
		return;
	}

	for (int n{}; n < i::ent_list->GetHighestEntityIndex(); n++)
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntity(n) };

		if (!ent || ent->IsDormant()) {
			continue;
		}

		const size_t class_id{ ent->GetClassId() };
		const EntityHandle_t ehandle{ ent->GetRefEHandle() };

		if (class_id == class_ids::CTFPlayer)
		{
			C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

			if (!pl || !pl->IsInValidTeam()) {
				continue;
			}

			m_groups[ECGroup::PLAYERS_ALL].push_back(ehandle);
			m_groups[pl->m_iTeamNum() != local->m_iTeamNum() ? ECGroup::PLAYERS_ENEMIES : ECGroup::PLAYERS_TEAMMATES].push_back(ehandle);
		}

		if (class_id == class_ids::CObjectSentrygun
			|| class_id == class_ids::CObjectDispenser
			|| class_id == class_ids::CObjectTeleporter)
		{
			C_BaseObject *const obj{ ent->cast<C_BaseObject>() };

			if (!obj || !obj->IsInValidTeam()) {
				continue;
			}

			bool is_local{ false };

			if (tf_utils::isEntOwnedBy(obj, local)) {
				m_groups[ECGroup::BUILDINGS_LOCAL].push_back(ehandle);
				is_local = true;
			}

			m_groups[ECGroup::BUILDINGS_ALL].push_back(ehandle);

			if (!is_local) {
				m_groups[obj->m_iTeamNum() != local->m_iTeamNum() ? ECGroup::BUILDINGS_ENEMIES : ECGroup::BUILDINGS_TEAMMATES].push_back(ehandle);
			}
		}

		else if (class_id == class_ids::CTFProjectile_Rocket
			|| class_id == class_ids::CTFProjectile_SentryRocket
			|| class_id == class_ids::CTFProjectile_Jar
			|| class_id == class_ids::CTFProjectile_JarGas
			|| class_id == class_ids::CTFProjectile_JarMilk
			|| class_id == class_ids::CTFProjectile_Arrow
			|| class_id == class_ids::CTFProjectile_Flare
			|| class_id == class_ids::CTFProjectile_Cleaver
			|| class_id == class_ids::CTFProjectile_HealingBolt
			|| class_id == class_ids::CTFGrenadePipebombProjectile
			|| class_id == class_ids::CTFProjectile_BallOfFire
			|| class_id == class_ids::CTFProjectile_EnergyRing
			|| class_id == class_ids::CTFProjectile_EnergyBall)
		{
			C_BaseEntity *const proj{ ent->cast<C_BaseEntity>() };

			if (!proj || !proj->IsInValidTeam()) {
				continue;
			}

			bool is_local{ false };

			if (tf_utils::isEntOwnedBy(proj, local)) {
				m_groups[ECGroup::PROJECTILES_LOCAL].push_back(ehandle);
				is_local = true;
			}

			m_groups[ECGroup::PROJECTILES_ALL].push_back(ehandle);

			if (!is_local) {
				m_groups[proj->m_iTeamNum() != local->m_iTeamNum() ? ECGroup::PROJECTILES_ENEMIES : ECGroup::PROJECTILES_TEAMMATES].push_back(ehandle);
			}
		}

		else if (class_id == class_ids::CBaseAnimating)
		{
			C_BaseAnimating *const animating{ ent->cast<C_BaseAnimating>() };

			if (!animating) {
				continue;
			}

			if (m_health_pack_indexes.contains(animating->m_nModelIndex())) {
				m_groups[ECGroup::HEALTH_PACKS].push_back(ehandle);
			}

			if (m_ammo_pack_indexes.contains(animating->m_nModelIndex())) {
				m_groups[ECGroup::AMMO_PACKS].push_back(ehandle);
			}
		}

		else if (class_id == class_ids::CTFAmmoPack)
		{
			C_BaseAnimating *const animating{ ent->cast<C_BaseAnimating>() };

			if (!animating) {
				continue;
			}

			m_groups[ECGroup::AMMO_PACKS].push_back(ehandle);
		}

		else if (class_id == class_ids::CHalloweenGiftPickup)
		{
			C_BaseAnimating *const animating{ ent->cast<C_BaseAnimating>() };

			if (!animating) {
				continue;
			}

			m_groups[ECGroup::HALLOWEEN_GIFTS].push_back(ehandle);
		}

		else if (class_id == class_ids::CCurrencyPack)
		{
			C_CurrencyPack *const curpak{ ent->cast<C_CurrencyPack>() };

			if (!curpak || (local->m_iClass() != TF_CLASS_SCOUT && curpak->m_bDistributed())) {
				continue;
			}

			m_groups[ECGroup::MVM_MONEY].push_back(ehandle);
		}

		else if (class_id == class_ids::CTFPlayerResource) {
			 m_player_resource = ent->cast<C_TFPlayerResource>();
		}

		else if (class_id == class_ids::CCaptureFlag)
		{
			C_CaptureFlag *const flag{ ent->cast<C_CaptureFlag>() };

			if (!flag || flag->m_nType() != TF_FLAGTYPE_CTF) {
				continue;
			}

			m_groups[ECGroup::CTF_FLAGS].push_back(ehandle);
		}

		else if (class_id == class_ids::CFuncTrackTrain)
		{
			C_BaseEntity *const train{ ent->GetBaseEntity() };

			if (!train) {
				continue;
			}

			m_groups[ECGroup::FUNC_TRACK_TRAIN_ALL].push_back(ehandle);
			m_groups[train->m_iTeamNum() != local->m_iTeamNum() ? ECGroup::FUNC_TRACK_TRAIN_ENEMIES : ECGroup::FUNC_TRACK_TRAIN_TEAMMATES].push_back(ehandle);
		}
	}
}

const std::vector<EntityHandle_t> &EntityCache::getGroup(const ECGroup group)
{
	return m_groups[group];
}