#include "entity_visuals_group.hpp"

static void checkAndIncrement(bool condition, int &num_passed)
{
    if (condition) {
        num_passed++;
    }
}

bool EVGroup::entPass(C_BaseEntity *const ent) const
{
	if (!ent || !ent->ShouldDraw()) {
		return false;
	}

	if (ent->GetClassId() == class_ids::CTFPlayer && ent->cast<C_TFPlayer>()->deadflag()) {
		return false;
	}

	if ((ent->GetClassId() == class_ids::CObjectSentrygun
		|| ent->GetClassId() == class_ids::CObjectDispenser
		|| ent->GetClassId() == class_ids::CObjectTeleporter)) {
        C_BaseObject *const obj{ ent->cast<C_BaseObject>() };

        if (obj->m_iHealth() <= 0) {
            return false;
        }

        if (obj->m_bCarried() || obj->m_bPlacing()) {
            return false;
        }
	}

	auto hasClass = [](C_BaseEntity *const e, int classes)
	{
		if (!e) {
			return false;
		}

        if (!classes) {
            return false;
        }

		const size_t ent_class_id{ e->GetClassId() };

        int num_passed{ 0 };

		if (classes & static_cast<int>(EVClass::PLAYER)) {
            checkAndIncrement(ent_class_id == class_ids::CTFPlayer.get(), num_passed);
		}
		
		if (classes & static_cast<int>(EVClass::SENTRYGUN)) {
            checkAndIncrement(ent_class_id == class_ids::CObjectSentrygun.get(), num_passed);
        }

        if (classes & static_cast<int>(EVClass::DISPENSER)) {
            checkAndIncrement(ent_class_id == class_ids::CObjectDispenser.get(), num_passed);
        }

        if (classes & static_cast<int>(EVClass::TELEPORTER))
        {
            checkAndIncrement(ent_class_id == class_ids::CObjectTeleporter.get(), num_passed);
        }

        if (classes & static_cast<int>(EVClass::PROJECTILE))
        {
            const bool is_projectile = ent_class_id == class_ids::CTFProjectile_Rocket
				|| ent_class_id == class_ids::CTFProjectile_SentryRocket
				|| ent_class_id == class_ids::CTFProjectile_Jar
				|| ent_class_id == class_ids::CTFProjectile_JarGas
				|| ent_class_id == class_ids::CTFProjectile_JarMilk
				|| ent_class_id == class_ids::CTFProjectile_Arrow
				|| ent_class_id == class_ids::CTFProjectile_Flare
				|| ent_class_id == class_ids::CTFProjectile_Cleaver
				|| ent_class_id == class_ids::CTFProjectile_HealingBolt
				|| ent_class_id == class_ids::CTFGrenadePipebombProjectile
				|| ent_class_id == class_ids::CTFProjectile_BallOfFire
				|| ent_class_id == class_ids::CTFProjectile_EnergyRing
				|| ent_class_id == class_ids::CTFProjectile_EnergyBall;

            checkAndIncrement(is_projectile, num_passed);
        }

        if (classes & static_cast<int>(EVClass::HEALTHPACK)) {
            checkAndIncrement(ec->isEntityHealthPack(e), num_passed);
        }
		
		if (classes & static_cast<int>(EVClass::AMMOPACK)) {
            checkAndIncrement(ec->isEntityAmmoPack(e), num_passed);
        }

        if (classes & static_cast<int>(EVClass::HALLOWEEN_GIFTS)) {
            checkAndIncrement(ent_class_id == class_ids::CHalloweenGiftPickup.get(), num_passed);
        }

        if (classes & static_cast<int>(EVClass::MVM_MONEY)) {
            checkAndIncrement(ent_class_id == class_ids::CCurrencyPack.get(), num_passed);
        }

        if (classes & static_cast<int>(EVClass::CTF_FLAGS)) {
            checkAndIncrement(ent_class_id == class_ids::CCaptureFlag.get(), num_passed);
        }

		return num_passed > 0;
	};

	auto hasCond = [](C_BaseEntity *const e, int conditions)
	{
		if (!e) {
			return false;
		}

		C_TFPlayer *const local{ ec->getLocal() };

		if (!local) {
			return false;
		}

        if (conditions == static_cast<int>(EVCond::NONE)) {
			return true;
		}

        int num_passed{ 0 };

        if (conditions & static_cast<int>(EVCond::LOCAL)) {
            checkAndIncrement(e == local || tf_utils::isEntOwnedBy(e, local), num_passed);
        }

        if (conditions & static_cast<int>(EVCond::TEAMMATE)) {
            checkAndIncrement(e->m_iTeamNum() == local->m_iTeamNum() && (e != local && !tf_utils::isEntOwnedBy(e, local)), num_passed);
        }

        if (conditions & static_cast<int>(EVCond::ENEMY)) {
            checkAndIncrement(e->m_iTeamNum() != local->m_iTeamNum(), num_passed);
        }

        if (conditions & static_cast<int>(EVCond::FRIEND)) {
            checkAndIncrement(e->GetClassId() == class_ids::CTFPlayer && e->cast<C_TFPlayer>()->IsPlayerOnSteamFriendsList(), num_passed);
        }

        if (e->GetClassId() == class_ids::CTFPlayer && num_passed > 0) {
            C_TFPlayer *const pl{ e->cast<C_TFPlayer>() };

            if (conditions & static_cast<int>(EVCond::INVISIBLE)) {
                return pl->IsInvisible();
            }

            if (conditions & static_cast<int>(EVCond::INVULNERABLE)) {
                return pl->IsInvulnerable();
            }

            if (conditions & static_cast<int>(EVCond::AIMBOT_TARGET)) {
                return e->GetRefEHandle() == tf_globals::aimbot_target;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_SCOUT)) {
                return pl->m_iClass() == TF_CLASS_SCOUT;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_SOLDIER)) {
                return pl->m_iClass() == TF_CLASS_SOLDIER;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_PYRO)) {
                return pl->m_iClass() == TF_CLASS_PYRO;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_DEMOMAN)) {
                return pl->m_iClass() == TF_CLASS_DEMOMAN;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_HEAVY)) {
                return pl->m_iClass() == TF_CLASS_HEAVYWEAPONS;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_ENGINEER)) {
                return pl->m_iClass() == TF_CLASS_ENGINEER;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_MEDIC)) {
                return pl->m_iClass() == TF_CLASS_MEDIC;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_SNIPER)) {
                return pl->m_iClass() == TF_CLASS_SNIPER;
            }

            if (conditions & static_cast<int>(EVCond::CLASS_SPY)) {
                return pl->m_iClass() == TF_CLASS_SPY;
            }
        }

        // doesn't seem we have any conditions to worry about,
        // fall back to the default behavior
        
		return num_passed > 0;
	};

	if (!hasClass(ent, classes)) {
		return false;
	}

    if (!hasCond(ent, conds)) {
        return false;
    }

	return true;
}