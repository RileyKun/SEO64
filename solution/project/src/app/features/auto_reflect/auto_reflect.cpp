#include "auto_reflect.hpp"

#include "../cfg.hpp"
#include "../trace_utils/trace_utils.hpp"

bool AutoReflect::shouldReflect(const vec3 &airblast_origin, const vec3 &eye_pos, const vec3 &proj_pos, const float airblast_radius) const
{
    if (airblast_origin.distTo(proj_pos) > airblast_radius) {
        return false;
    }

    trace_t trace{};

    tf_utils::trace(eye_pos, proj_pos, &trace_filters::world, &trace, MASK_SOLID);

    return trace.fraction >= 1.0f;
};

bool AutoReflect::shouldSkipProjectile(C_BaseEntity *const proj, vec3 &proj_position) const
{
    const auto class_id{ proj->GetClassId() };

    if (class_id == class_ids::CTFProjectile_EnergyRing
        || class_id == class_ids::CTFProjectile_BallOfFire) {
        return true;
    }

    if (class_id == class_ids::CTFProjectile_Cleaver
        || class_id == class_ids::CTFProjectile_Jar
        || class_id == class_ids::CTFProjectile_JarGas
        || class_id == class_ids::CTFProjectile_Flare
        || class_id == class_ids::CTFProjectile_Arrow
        || class_id == class_ids::CTFProjectile_HealingBolt) {
        if (proj->m_MoveType() == MOVETYPE_NONE) {
            return true;
        }
    }

    vec3 velocity{ proj->GetAbsVelocity() };

    if (class_id == class_ids::CTFGrenadePipebombProjectile
        || class_id == class_ids::CTFStunBall) {
        C_TFGrenadePipebombProjectile *const sticky{ proj->cast<C_TFGrenadePipebombProjectile>() };
        if (sticky->m_bTouched() || sticky->m_iType() == TF_PROJECTILE_PIPEBOMB_PRACTICE) {
            return true;
        }
    }

    if (class_id == class_ids::CTFProjectile_Rocket
        || class_id == class_ids::CTFProjectile_EnergyBall) {
        if (proj->cast<C_TFProjectile_Rocket>()->isRocketJumper()) {
            return true;
        }
    }

    if (class_id == class_ids::CTFProjectile_Arrow) {
        velocity = proj->cast<C_TFProjectile_Arrow>()->m_vInitialVelocity();
    }

    // not sure if we need to worry about latency here, but did anyway
    proj_position = proj->GetCenter() + (velocity * tf_utils::getLatency());

    return false;
}

void AutoReflect::run(CUserCmd *const cmd)
{
    if (!cfg::auto_reflect
        || !cmd) {
        return;
    }

    C_TFPlayer *const local{ ec->getLocal() };
    if (!local || local->deadflag()) {
        return;
    }

    C_TFWeaponBase *const weapon{ ec->getWeapon() };
    if (!weapon
        || weapon->GetWeaponID() != TF_WEAPON_FLAMETHROWER
        || weapon->m_iItemDefinitionIndex() == Pyro_m_ThePhlogistinator) {
        return;
    }

    const vec3 angles{ i::engine->GetViewAngles() };
    const vec3 eye_pos{ local->GetEyePos() };

    vec3 forward{};
    math::angleVectors(angles, &forward);

    const auto airblast_radius{ tf_utils::attribHookValue(1.0f, "deflection_size_multiplier", weapon) * 128.f };
    const vec3 airblast_origin{ eye_pos + (forward * airblast_radius) };

    for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PROJECTILES_ENEMIES))
    {
        IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

        if (!ent) {
            continue;
        }

        C_BaseEntity *const proj{ ent->cast<C_BaseEntity>() };

        if (!proj) {
            continue;
        }

        vec3 proj_position{};

        if (shouldSkipProjectile(proj, proj_position)) {
            continue;
        }

        if (!shouldReflect(airblast_origin, eye_pos, proj_position, airblast_radius)) {
            continue;
        }

        cmd->buttons |= IN_ATTACK2;

        break;
    }
}