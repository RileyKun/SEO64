#pragma once

#include "../tf/interfaces.hpp"

class CvarInit final : public InstTracker<CvarInit>
{
private:
	std::string m_name{};
	ConVar **m_cvar{};

public:
	CvarInit(std::string_view name, ConVar **const cvar) : InstTracker(this)
	{
		m_name = name;
		m_cvar = cvar;
	}

public:
	bool init()
	{
		if (!m_cvar) {
			return false;
		}

		return (*m_cvar = i::cvar->FindVar(m_name.c_str())) != nullptr;
	}
};

#define MAKE_CVAR(name) \
namespace cvars { inline ConVar *name{}; } \
namespace make_cvars { inline CvarInit name##_init{ #name, &cvars::name }; }

MAKE_CVAR(sv_gravity);
MAKE_CVAR(cl_interp);
MAKE_CVAR(cl_interp_ratio);
MAKE_CVAR(cl_updaterate);
MAKE_CVAR(cl_flipviewmodels);
MAKE_CVAR(tf_grenadelauncher_livetime);
MAKE_CVAR(tf_weapon_criticals_bucket_cap);
MAKE_CVAR(tf_weapon_criticals_bucket_bottom);
MAKE_CVAR(tf_weapon_criticals_bucket_default);
MAKE_CVAR(cl_predict);
MAKE_CVAR(fov_desired);
MAKE_CVAR(tf_viewmodels_offset_override);
MAKE_CVAR(cl_wpn_sway_interp);
MAKE_CVAR(sv_maxunlag);
MAKE_CVAR(host_limitlocal);
MAKE_CVAR(engine_no_focus_sleep);
MAKE_CVAR(cl_cmdrate);
MAKE_CVAR(sv_clockcorrection_msecs);
MAKE_CVAR(sv_lagcompensation_teleport_dist);
MAKE_CVAR(tf_medieval_autorp);
MAKE_CVAR(english);
MAKE_CVAR(sv_friction);
MAKE_CVAR(sv_stopspeed);
MAKE_CVAR(tf_use_fixed_weaponspreads);
MAKE_CVAR(mat_hdr_level);
MAKE_CVAR(sv_rollangle);
MAKE_CVAR(sv_rollspeed);
MAKE_CVAR(sv_optimizedmovement);
MAKE_CVAR(tf_parachute_maxspeed_xy);
MAKE_CVAR(tf_parachute_maxspeed_z);
MAKE_CVAR(sv_bounce);
MAKE_CVAR(sv_maxvelocity);
MAKE_CVAR(sv_accelerate);
MAKE_CVAR(tf_clamp_back_speed);
MAKE_CVAR(tf_clamp_back_speed_min);
MAKE_CVAR(sv_airaccelerate);
MAKE_CVAR(tf_max_charge_speed);
MAKE_CVAR(cl_forwardspeed);
MAKE_CVAR(cl_backspeed);
MAKE_CVAR(cl_sidespeed);
MAKE_CVAR(tf_ghost_xy_speed);
MAKE_CVAR(sv_noclipspeed);
MAKE_CVAR(sv_noclipaccelerate);
MAKE_CVAR(sv_maxspeed);