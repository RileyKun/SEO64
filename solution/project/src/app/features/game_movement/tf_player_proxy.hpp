#pragma once

#include "../../../game/game.hpp"

class TFPlayerProxy final
{
public:
	TFPlayerProxy() = default;
	TFPlayerProxy(C_TFPlayer *const pl);

private:
	int m_move_type{};
	int m_flags{};
	int m_health{};
	bool m_deadflag{};
	float m_surface_friction{};
	int m_water_level{};
	int m_observer_mode{};
	EHANDLE m_ground_ent{};
	Vector m_abs_vel{};
	Vector m_base_vel{};
	float m_water_jump_time{};
	Vector m_mins{};
	Vector m_maxs{};
	int m_player_cond{};
	int m_player_cond_ex{};
	int m_player_cond_ex2{};
	int m_player_cond_ex3{};
	float m_gravity{};
	bool m_allow_auto_movement{};
	float m_step_size{};
	int m_class{};
	C_TFWeaponBase *m_active_weapon{};
	float m_max_speed{};
	Vector m_view_offset{};
	Vector m_water_jump_vel{};
	int m_water_type{};
	int m_team_num{};

public:
	int GetMoveType() const;
	int GetFlags() const;
	int GetHealth() const;
	bool IsAlive() const;
	float GetSurfaceFriction() const;
	int GetWaterLevel() const;
	int GetObserverMode() const;
	bool IsObserver() const;
	C_BaseEntity *GetGroundEntity() const;
	Vector GetAbsVelocity() const;
	Vector GetBaseVelocity() const;
	float GetWaterJumpTime() const;
	Vector GetMins() const;
	Vector GetMaxs() const;
	float GetGravity() const;
	bool AllowAutoMovement() const;
	float GetStepSize() const;
	int GetClass() const;
	C_TFWeaponBase *const GetActiveTFWeapon() const;
	float MaxSpeed() const;
	Vector GetViewOffset() const;
	Vector &GetWaterJumpVel();
	int GetWaterType() const;
	int GetTeamNumber() const;

public:
	void SetSurfaceFriction(const float surface_friction);
	void SetGroundEntity(C_BaseEntity *const ground_ent);
	void SetAbsVelocity(const Vector &abs_vel);
	void SetBaseVelocity(const Vector &base_vel);
	void SetWaterJumpTime(const float water_jump_time);
	void SetWaterLevel(const int water_level);
	void SetWaterType(const int water_type);

public:
	void AddCond(const ETFCond cond);
	void RemoveCond(const ETFCond cond);
	bool InCond(const ETFCond cond);
};