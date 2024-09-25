#include "tf_player_proxy.hpp"

template <typename tIntType>
class CConditionVars
{
public:
	CConditionVars(tIntType &nPlayerCond, tIntType &nPlayerCondEx, tIntType &nPlayerCondEx2, tIntType &nPlayerCondEx3, ETFCond eCond)
	{
		if (eCond >= 96) {
			m_pnCondVar = &nPlayerCondEx3;
			m_nCondBit = eCond - 96;
		}

		else if (eCond >= 64) {
			m_pnCondVar = &nPlayerCondEx2;
			m_nCondBit = eCond - 64;
		}

		else if (eCond >= 32) {
			m_pnCondVar = &nPlayerCondEx;
			m_nCondBit = eCond - 32;
		}

		else {
			m_pnCondVar = &nPlayerCond;
			m_nCondBit = eCond;
		}
	}

public:
	tIntType &CondVar() const {
		return *m_pnCondVar;
	}

	int CondBit() const {
		return 1 << m_nCondBit;
	}

private:
	tIntType *m_pnCondVar{};
	int m_nCondBit{};
};

TFPlayerProxy::TFPlayerProxy(C_TFPlayer *const pl)
{
	if (!pl) {
		return;
	}
	
	m_move_type = static_cast<int>(pl->m_MoveType());
	m_flags = pl->m_fFlags();
	m_health = pl->m_iHealth();
	m_deadflag = pl->deadflag();
	m_surface_friction = pl->m_surfaceFriction();
	m_water_level = static_cast<int>(pl->m_nWaterLevel());
	m_observer_mode = pl->m_iObserverMode();
	m_ground_ent = nullptr;
	m_abs_vel = pl->GetAbsVelocity();
	m_base_vel = pl->m_vecBaseVelocity();
	m_water_jump_time = 0.0f;
	m_mins = pl->m_vecMins() * (pl->m_flModelScale() - DIST_EPSILON);
	m_maxs = pl->m_vecMaxs() * (pl->m_flModelScale() - DIST_EPSILON);
	m_player_cond = pl->m_nPlayerCond();
	m_player_cond_ex = pl->m_nPlayerCondEx();
	m_player_cond_ex2 = pl->m_nPlayerCondEx2();
	m_player_cond_ex3 = pl->m_nPlayerCondEx3();
	m_gravity = 1.0f; //s: ?
	m_allow_auto_movement = pl->m_bAllowAutoMovement();
	m_step_size = pl->m_flStepSize();
	m_class = pl->m_iClass();
	m_active_weapon = pl->m_hActiveWeapon()->cast<C_TFWeaponBase>();
	m_max_speed = pl->m_flMaxspeed();
	m_view_offset = pl->m_vecViewOffset();
	m_water_jump_vel = {};
	m_water_type = 0;
	m_team_num = pl->m_iTeamNum();
}

int TFPlayerProxy::GetMoveType() const
{
	return m_move_type;
}

int TFPlayerProxy::GetFlags() const
{
	return m_flags;
}

int TFPlayerProxy::GetHealth() const
{
	return m_health;
}

bool TFPlayerProxy::IsAlive() const
{
	return !m_deadflag;
}

float TFPlayerProxy::GetSurfaceFriction() const
{
	return m_surface_friction;
}

int TFPlayerProxy::GetWaterLevel() const
{
	return m_water_level;
}

int TFPlayerProxy::GetObserverMode() const
{
	return m_observer_mode;
}

bool TFPlayerProxy::IsObserver() const
{
	return GetObserverMode() != OBS_MODE_NONE;
}

C_BaseEntity *TFPlayerProxy::GetGroundEntity() const
{
	return m_ground_ent;
}

Vector TFPlayerProxy::GetAbsVelocity() const
{
	return m_abs_vel;
}

Vector TFPlayerProxy::GetBaseVelocity() const
{
	return m_base_vel;
}

float TFPlayerProxy::GetWaterJumpTime() const
{
	return m_water_jump_time;
}

Vector TFPlayerProxy::GetMins() const
{
	return m_mins;
}

Vector TFPlayerProxy::GetMaxs() const
{
	return m_maxs;
}

float TFPlayerProxy::GetGravity() const
{
	return m_gravity;
}

bool TFPlayerProxy::AllowAutoMovement() const
{
	return m_allow_auto_movement;
}

float TFPlayerProxy::GetStepSize() const
{
	return m_step_size;
}

int TFPlayerProxy::GetClass() const
{
	return m_class;
}

C_TFWeaponBase *const TFPlayerProxy::GetActiveTFWeapon() const
{
	return m_active_weapon;
}

float TFPlayerProxy::MaxSpeed() const
{
	return m_max_speed;
}

Vector TFPlayerProxy::GetViewOffset() const
{
	return m_view_offset;
}

Vector &TFPlayerProxy::GetWaterJumpVel()
{
	return m_water_jump_vel;
}

int TFPlayerProxy::GetWaterType() const
{
	return m_water_type;
}

int TFPlayerProxy::GetTeamNumber() const
{
	return m_team_num;
}

void TFPlayerProxy::SetSurfaceFriction(const float surface_friction)
{
	m_surface_friction = surface_friction;
}

void TFPlayerProxy::SetGroundEntity(C_BaseEntity *const ground_ent)
{
	m_ground_ent = ground_ent;
}

void TFPlayerProxy::SetAbsVelocity(const Vector &abs_vel)
{
	m_abs_vel = abs_vel;
}

void TFPlayerProxy::SetBaseVelocity(const Vector &base_vel)
{
	m_base_vel = base_vel;
}

void TFPlayerProxy::SetWaterJumpTime(const float water_jump_time)
{
	m_water_jump_time = water_jump_time;
}

void TFPlayerProxy::SetWaterLevel(const int water_level)
{
	m_water_level = water_level;
}

void TFPlayerProxy::SetWaterType(const int water_type)
{
	m_water_type = water_type;
}

void TFPlayerProxy::AddCond(const ETFCond cond)
{
	CConditionVars<int> player_cond{ m_player_cond, m_player_cond_ex, m_player_cond_ex2, m_player_cond_ex3, cond };

	player_cond.CondVar() |= player_cond.CondBit();
}

void TFPlayerProxy::RemoveCond(const ETFCond cond)
{
	if (!InCond(cond)) {
		return;
	}

	CConditionVars<int> player_cond{ m_player_cond, m_player_cond_ex, m_player_cond_ex2, m_player_cond_ex3, cond };

	player_cond.CondVar() &= ~player_cond.CondBit();
}

bool TFPlayerProxy::InCond(const ETFCond cond)
{
	CConditionVars<int> player_cond{ m_player_cond, m_player_cond_ex, m_player_cond_ex2, m_player_cond_ex3, cond };

	return (player_cond.CondVar() & player_cond.CondBit()) != 0;
}