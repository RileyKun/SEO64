#include "trace_utils.hpp"

bool trace_filters::FilterWorld::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
	if (IClientEntity *const ent{ reinterpret_cast<IClientEntity *>(pServerEntity) })
	{
		const size_t class_id{ ent->GetClassId() };

		if (class_id == class_ids::CObjectCartDispenser
			|| class_id == class_ids::CBaseDoor
			|| class_id == class_ids::CPhysicsProp
			|| class_id == class_ids::CDynamicProp
			|| class_id == class_ids::CBaseEntity
			|| class_id == class_ids::CFuncTrackTrain) {
			return true;
		}
	}

	return false;
}

TraceType_t trace_filters::FilterWorld::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

bool trace_filters::FilterHitscanAim::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
	if (!pServerEntity || pServerEntity == m_ignore_ent) {
		return false;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return false;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon) {
		return false;
	}

	IClientEntity *const client_ent{ i::ent_list->GetClientEntityFromHandle(pServerEntity->GetRefEHandle()) };

	if (!client_ent || !client_ent->GetBaseEntity()) {
		return false;
	}

	C_BaseEntity *const ent{ client_ent->cast<C_BaseEntity>() };

	if (!ent) {
		return false;
	}

	const size_t class_id{ ent->GetClassId() };

	if (class_id == class_ids::CFuncAreaPortalWindow
		|| class_id == class_ids::CFuncRespawnRoomVisualizer
		|| class_id == class_ids::CSniperDot
		|| class_id == class_ids::CTFAmmoPack)
	{
		return false;
	}

	if (class_id == class_ids::CTFMedigunShield)
	{
		if (ent->m_iTeamNum() == local->m_iTeamNum()) {
			return false;
		}
	}

	if (class_id == class_ids::CTFPlayer
		|| class_id == class_ids::CObjectSentrygun
		|| class_id == class_ids::CObjectDispenser
		|| class_id == class_ids::CObjectTeleporter)
	{
		const int wep_id{ weapon->GetWeaponID() };

		if ((wep_id == TF_WEAPON_SNIPERRIFLE
			|| wep_id == TF_WEAPON_SNIPERRIFLE_CLASSIC
			|| wep_id == TF_WEAPON_SNIPERRIFLE_DECAP)
			&& ent->m_iTeamNum() == local->m_iTeamNum()) {
			return false;
		}
	}

	return true;
}

TraceType_t trace_filters::FilterHitscanAim::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

bool trace_filters::FilterMeleeAim::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
	if (!pServerEntity || pServerEntity == m_ignore_ent) {
		return false;
	}

	return true;
}

TraceType_t trace_filters::FilterMeleeAim::GetTraceType() const
{
	return TRACE_EVERYTHING;
}