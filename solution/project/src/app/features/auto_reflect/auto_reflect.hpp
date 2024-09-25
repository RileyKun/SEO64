#pragma once

#include "../../../game/game.hpp"

class AutoReflect final
{
private:
    bool shouldReflect(const vec3 &airblast_origin, const vec3 &eye_pos, const vec3 &proj_pos, const float airblast_radius) const;
    bool shouldSkipProjectile(C_BaseEntity *const proj, vec3 &proj_position) const;

public:
    void run(CUserCmd *const cmd);
};

MAKE_UNIQUE(AutoReflect, auto_reflect);