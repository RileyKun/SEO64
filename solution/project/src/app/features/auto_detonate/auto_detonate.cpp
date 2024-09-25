#include "auto_detonate.hpp"

#include "../trace_utils/trace_utils.hpp"
#include "../aim_utils/aim_utils.hpp"
#include "../cfg.hpp"

bool AutoDetonate::canSee(C_BaseEntity *const target, C_BaseEntity *const projectile, const float radius) const
{
    const vec3 target_center{ target->GetCenter() };
    const vec3 projectile_center{ projectile->GetCenter() };

    if (projectile_center.distTo(target_center) > radius) {
        return false;
    }

    trace_t trace{};

    tf_utils::trace(projectile_center, target_center, &trace_filters::world, &trace, MASK_SOLID & ~CONTENTS_GRATE);

    return trace.fraction > 0.99f;
}

void AutoDetonate::detonate(CUserCmd *const cmd, C_TFPlayer *const local, C_TFGrenadePipebombProjectile *const sticky)
{
    cmd->buttons |= IN_ATTACK2;

    if (!sticky || !sticky->m_bDefensiveBomb()) {
        return;
    }

    const vec3 target_pos{ math::calcAngle(ec->getLocal()->GetEyePos(), sticky->GetCenter()) };
    tf_utils::fixMovement(cmd, target_pos);
    cmd->viewangles = target_pos;
}

void AutoDetonate::handleStickies(CUserCmd *const cmd, C_TFPlayer *const local)
{
    if (!cfg::auto_detonate_active ) {
        return;
    }

    C_TFPipebombLauncher *const weapon{ local->Weapon_OwnsThisID(TF_WEAPON_PIPEBOMBLAUNCHER)->cast<C_TFPipebombLauncher>() };

    if (!weapon) {
        return;
    }

    float live_time{ tf_utils::attribHookValue(cvars::tf_grenadelauncher_livetime->GetFloat(), "sticky_arm_time", weapon) };

    if (local->GetCarryingRuneType() == RUNE_HASTE) {
        live_time *= 0.5f;
    }

    else if (local->GetCarryingRuneType() == RUNE_KING || local->InCond(TF_COND_KING_BUFFED)) {
        live_time *= 0.75f;
    }

    m_live_time = live_time;

    std::vector<std::pair<C_TFGrenadePipebombProjectile *, float>> active_bombs{};

    for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PROJECTILES_LOCAL))
    {
        IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

        if (!ent) {
            continue;
        }

        C_BaseEntity *const proj{ ent->cast<C_BaseEntity>() };

        if (!proj || proj->IsDormant() || proj->GetClassId() != class_ids::CTFGrenadePipebombProjectile) {
            continue;
        }

        C_TFGrenadePipebombProjectile *const sticky{ proj->cast<C_TFGrenadePipebombProjectile>() };

        if (!sticky || sticky->m_iType() != TF_GL_MODE_REMOTE_DETONATE) {
            continue;
        }

        if (i::global_vars->curtime < sticky->m_flCreationTime() + m_live_time) {
            continue;
        }

        active_bombs.push_back(std::make_pair(sticky, sticky->m_bTouched() ? 170.0f : 150.0f));
    }

    if (active_bombs.empty()) {
        return;
    }

    for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PLAYERS_ENEMIES))
    {
        IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

        if (!ent) {
            continue;
        }

        C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

        if (!pl || pl->deadflag()) {
            continue;
        }

        if (cfg::auto_detonate_ignore_friends && pl->IsPlayerOnSteamFriendsList()) {
            continue;
        }

        if (cfg::auto_detonate_ignore_invisible && pl->IsInvisible()) {
            continue;
        }

        if (cfg::auto_detonate_ignore_invulnerable && pl->IsInvulnerable()) {
            continue;
        }

        for (auto& pair : active_bombs)
        {
            if (!canSee(pl, pair.first, pair.second)) {
                continue;
            }

            detonate(cmd, local, pair.first);

            return;
        }
    }

    for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::BUILDINGS_ENEMIES))
    {
        IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

        if (!ent) {
            continue;
        }

        C_BaseObject *const obj{ ent->cast<C_BaseObject>() };

        if (!obj || (obj->m_iHealth() <= 0) || obj->m_bCarried()) {
            continue;
        }

        if (cfg::auto_detonate_ignore_sentries && obj->GetClassId() == class_ids::CObjectSentrygun) {
            continue;
        }

        if (cfg::auto_detonate_ignore_dispensers && obj->GetClassId() == class_ids::CObjectDispenser) {
            continue;
        }

        if (cfg::aimbot_ignore_teleporters && obj->GetClassId() == class_ids::CObjectTeleporter) {
            continue;
        }

        for (auto &pair : active_bombs)
        {
            if (!canSee(obj, pair.first, pair.second)) {
                continue;
            }

            detonate(cmd, local, pair.first);

            return;
        }
    }
}

void AutoDetonate::handleFlares(CUserCmd *const cmd, C_TFPlayer *const local)
{
    constexpr float radius{ 100.0f };

    C_TFWeaponBase *const weapon{ ec->getWeapon() };
    if (!weapon || weapon->GetWeaponID() != TF_WEAPON_FLAREGUN) {
        return;
    }

    C_TFFlareGun *const flare_gun{ weapon->cast<C_TFFlareGun>() };
    if (!flare_gun || flare_gun->GetFlareGunType() != FLAREGUN_DETONATE) {
        return;
    }

    C_TFProjectile_Flare *flare{};

    for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PROJECTILES_LOCAL))
    {
        IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

        if (!ent) {
            continue;
        }

        C_BaseEntity *const proj{ ent->cast<C_BaseEntity>() };

        if (!proj || proj->IsDormant() || proj->GetClassId() != class_ids::CTFProjectile_Flare) {
            continue;
        }

        flare = proj->cast<C_TFProjectile_Flare>();
    }

    if (!flare) {
        return;
    }

    for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PLAYERS_ENEMIES))
    {
        IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };
        if (!ent) {
            continue;
        }

        C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

        if (!pl || pl->deadflag()) {
            continue;
        }

        if (cfg::auto_detonate_ignore_friends && pl->IsPlayerOnSteamFriendsList()) {
            continue;
        }

        if (cfg::auto_detonate_ignore_invisible && pl->IsInvisible()) {
            continue;
        }

        if (cfg::auto_detonate_ignore_invulnerable && pl->IsInvulnerable()) {
            continue;
        }

        if (!canSee(pl, flare, radius)) {
            continue;
        }

        cmd->buttons |= IN_ATTACK2;

        return;
    }

    // if you aim at buildings with detonator, 
    // fuck you.
}

void AutoDetonate::run(CUserCmd *const cmd)
{
    if (!cfg::auto_detonate_active
        || !cmd) {
        return;
    }

    C_TFPlayer *const local{ ec->getLocal() };

    if (!local || local->deadflag()) {
        return;
    }

    const int cur_class{ local->m_iClass() };

    if (cur_class == TF_CLASS_DEMOMAN) {
        handleStickies(cmd, local);
    }

    if (cur_class == TF_CLASS_PYRO) {
        handleFlares(cmd, local);
    }
}