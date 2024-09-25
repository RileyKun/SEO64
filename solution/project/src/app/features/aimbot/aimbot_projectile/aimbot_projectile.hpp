#pragma once

#include "../../../../game/game.hpp"

class AimbotProjectile final
{
	struct Target final
	{
		C_BaseEntity *m_ent{};
		vec3 m_ang_to{};
		vec3 m_init_pos{};
		float m_fov_to{};
		float m_dist_to{};
		float m_time_to{};
	};

	enum BBoxPoint
	{
		BBOX_FEET,
		BBOX_BODY,
		BBOX_HEAD,
		BBOX_POINT_COUNT
	};

private:
	std::vector<vec3> m_pred_path{};

public:
	struct ProjectileInfo final
	{
		float m_speed{};
		float m_gravity_mod{};
		float m_drag_mod{};
		float m_life_time{};
		float m_splash_radius{};
		vec3 m_offset{};
	};

	bool calcProjAngle(const vec3 &from, const vec3 &to, const ProjectileInfo &proj_info, vec3 &ang_out, float &t_out);
	BBoxPoint getBBoxPoint(C_TFWeaponBase *const weapon, C_TFPlayer *const pl);
	bool getBBoxOffset(C_TFPlayer *const target_pl, const BBoxPoint point, const bool dynamic_head, vec3 &offset_out);
	std::vector<vec3> getPointsAroundPos(const vec3 &pos, const float radius);
	bool canSee(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const ProjectileInfo &proj_info, Target &target, const vec3 &target_pos);
	bool solveTarget(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const ProjectileInfo &proj_info, Target &target);
	bool getTarget(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon, const ProjectileInfo &proj_info, Target &out);
	void setAttack(CUserCmd *const cmd, C_TFWeaponBase *const weapon);
	void setAngles(CUserCmd *const cmd, C_TFWeaponBase *const weapon, const ProjectileInfo &proj_info, const Target &target);

public:
	bool getProjInfo(C_TFPlayer *const local, C_TFWeaponBase *const weapon, ProjectileInfo &out);
	void run(CUserCmd *const cmd, C_TFPlayer *const local, C_TFWeaponBase *const weapon);
};

MAKE_UNIQUE(AimbotProjectile, aimbot_projectile);