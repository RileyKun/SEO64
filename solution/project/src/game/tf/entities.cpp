#include "entities.hpp"
#include "interfaces.hpp"
#include "../helpers/class_ids.hpp"

IHandleEntity *CBaseHandle::Get() const
{
    return i::ent_list->GetClientEntityFromHandle(m_Index);
}

C_TFWeaponBase *const C_BaseCombatCharacter::Weapon_OwnsThisID(const int weaponID)
{
	for (int n{}; n < MAX_WEAPONS; n++)
	{
		C_TFWeaponBase *const weapon{ GetWeapon(n) };

		if (!weapon) {
			continue;
		}

		if (weapon->GetWeaponID() == weaponID) {
			return weapon;
		}
	}

	return nullptr;
}

C_TFWeaponBase *const C_BaseCombatCharacter::FindWeaponByItemDefinitionIndex(const int definition_index)
{
	for (int n{}; n < MAX_WEAPONS; n++)
	{
		C_TFWeaponBase *const weapon{ GetWeapon(n) };

		if (!weapon) {
			continue;
		}

		if (weapon->m_iItemDefinitionIndex() == definition_index) {
			return weapon;
		}
	}

	return nullptr;
}

bool C_TFPlayer::IsCritBoosted()
{
	const bool bAllWeaponCritActive
	{
		InCond(TF_COND_CRITBOOSTED)
		|| InCond(TF_COND_CRITBOOSTED_PUMPKIN)
		|| InCond(TF_COND_CRITBOOSTED_USER_BUFF)
		|| InCond(TF_COND_CRITBOOSTED_FIRST_BLOOD)
		|| InCond(TF_COND_CRITBOOSTED_BONUS_TIME)
		|| InCond(TF_COND_CRITBOOSTED_CTF_CAPTURE)
		|| InCond(TF_COND_CRITBOOSTED_ON_KILL)
		|| InCond(TF_COND_CRITBOOSTED_CARD_EFFECT)
		|| InCond(TF_COND_CRITBOOSTED_RUNE_TEMP)
	};

	if (bAllWeaponCritActive) {
		return true;
	}

	if (C_TFWeaponBase *const pWeapon{ m_hActiveWeapon().Get()->cast<C_TFWeaponBase>() })
	{
		if (InCond(TF_COND_CRITBOOSTED_RAGE_BUFF) && pWeapon->m_pWeaponInfo()->m_iWeaponType == TF_WPN_TYPE_PRIMARY) {
			return true;
		}

		const float flCritHealthPercent{ tf_utils::attribHookValue(1.0f, "mult_crit_when_health_is_below_percent", pWeapon) };

		if (flCritHealthPercent < 1.0f && HealthFraction() < flCritHealthPercent) {
			return true;
		}
	}

	return false;
}

bool C_BaseProjectile::IsCritical()
{
	size_t class_id{ this->GetClassId() };

	if (class_id == class_ids::CTFProjectile_Rocket)
	{
		C_TFProjectile_Rocket *const proj{ this->cast<C_TFProjectile_Rocket>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_SentryRocket)
	{
		C_TFProjectile_SentryRocket *const proj{ this->cast<C_TFProjectile_SentryRocket>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_Jar)
	{
		C_TFProjectile_Jar *const proj{ this->cast<C_TFProjectile_Jar>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_JarGas)
	{
		C_TFProjectile_JarGas *const proj{ this->cast<C_TFProjectile_JarGas>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_JarMilk)
	{
		C_TFProjectile_JarMilk *const proj{ this->cast<C_TFProjectile_JarMilk>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_Arrow)
	{
		C_TFProjectile_Arrow *const proj{ this->cast<C_TFProjectile_Arrow>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_Flare)
	{
		C_TFProjectile_Flare *const proj{ this->cast<C_TFProjectile_Flare>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_Cleaver)
	{
		C_TFProjectile_Cleaver *const proj{ this->cast<C_TFProjectile_Cleaver>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_HealingBolt)
	{
		C_TFProjectile_HealingBolt *const proj{ this->cast<C_TFProjectile_HealingBolt>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFGrenadePipebombProjectile)
	{
		C_TFGrenadePipebombProjectile *const proj{ this->cast<C_TFGrenadePipebombProjectile>() };

		return proj->m_bCritical();
	}
	else if (class_id == class_ids::CTFProjectile_BallOfFire)
	{
		C_TFProjectile_BallOfFire *const proj{ this->cast<C_TFProjectile_BallOfFire>() };

		return proj->m_bCritical();
	}

	return false;
}
