#include "move_pred.hpp"

#include "../player_data/player_data.hpp"
#include "../strafe_pred/strafe_pred.hpp"
#include "../cfg.hpp"

bool MovePred::setupMoveData(C_TFPlayer *const pl, CMoveData *const move_data, const bool ignore_special_ability)
{
	if (!pl || !move_data) {
		return false;
	}

	move_data->m_bFirstRunOfFunctions = false;
	move_data->m_bGameCodeMovedPlayer = false;
	move_data->m_nPlayerHandle = pl->GetRefEHandle().GetIndex();
	move_data->m_vecVelocity = pl->m_vecVelocity();
	move_data->m_vecAbsOrigin = pl->m_vecOrigin();
	move_data->m_flClientMaxSpeed = pl->TeamFortress_CalculateMaxSpeed(ignore_special_ability);

	//s: go up a bit to not get stuck in the ground
	move_data->m_vecAbsOrigin.z += DIST_EPSILON * 3.0f;

	math::vectorAngles(move_data->m_vecVelocity, move_data->m_vecViewAngles);

	move_data->m_vecAngles = move_data->m_vecViewAngles;
	move_data->m_vecAbsViewAngles = move_data->m_vecViewAngles;
	move_data->m_vecOldAngles = move_data->m_vecViewAngles;

	move_data->m_flForwardMove = 450.0f;
	move_data->m_flSideMove = 0.0f;

	const float speed{ move_data->m_vecVelocity.length2D() };
	const float max_speed{ move_data->m_flClientMaxSpeed * ((pl->m_fFlags() & FL_DUCKING) ? 0.3333333f : 1.0f) };

	if (speed < max_speed * 0.5f)
	{
		vec3 forward{};
		vec3 right{};

		math::angleVectors(move_data->m_vecViewAngles, &forward, &right);

		move_data->m_flForwardMove = (move_data->m_vecVelocity.y - right.y / right.x * move_data->m_vecVelocity.x) / (forward.y - right.y / right.x * forward.x);
		move_data->m_flSideMove = (move_data->m_vecVelocity.x - forward.x * move_data->m_flForwardMove) / right.x;
	}

	if (speed < max_speed * 0.1f) {
		move_data->m_flForwardMove = 0.0f;
		move_data->m_flSideMove = 0.0f;
	}

	if (pl->m_hConstraintEntity()) {
		move_data->m_vecConstraintCenter = pl->m_hConstraintEntity()->GetAbsOrigin();
	}

	else {
		move_data->m_vecConstraintCenter = pl->m_vecConstraintCenter();
	}

	move_data->m_flConstraintRadius = pl->m_flConstraintRadius();
	move_data->m_flConstraintWidth = pl->m_flConstraintWidth();
	move_data->m_flConstraintSpeedFactor = pl->m_flConstraintSpeedFactor();

	m_yaw_per_tick = 0.0f;
	m_air_strafing = false;

	if (cfg::aimbot_strafe_pred)
	{
		const size_t num_recs{ 5ull };

		if (player_data->getNumValidRecords(pl) >= num_recs)
		{
			std::vector<vec3> points{};

			for (size_t n{}; n < num_recs; n++)
			{
				const LagRecord *const lr{ player_data->getRecord(pl, n) };

				if (lr && lr->isRecordValid()) {
					points.push_back(lr->origin);
				}
			}

			if (strafe_pred->calc(points, move_data->m_vecVelocity.length2D(), m_yaw_per_tick))
			{
				if (!(pl->m_fFlags() & FL_ONGROUND) && pl->m_nWaterLevel() <= WL_Feet) {
					m_air_strafing = true;
					move_data->m_flForwardMove = 0.0f;
					move_data->m_flSideMove = (m_yaw_per_tick > 0.0f) ? -450.0f : 450.0f;
				}
			}
		}
	}

	game_movement.player = &player;
	game_movement.mv = move_data;

	return true;
}

bool MovePred::setupMoveData(C_TFPlayer *const pl, CUserCmd *const cmd, CMoveData *const move_data, const bool ignore_special_ability)
{
	i::pred->SetupMove(pl, cmd, i::move_helper, move_data);

	m_yaw_per_tick = 0.0f;
	m_air_strafing = false;

	if (cfg::aimbot_strafe_pred)
	{
		const size_t num_recs{ 5ull };

		if (player_data->getNumValidRecords(pl) >= num_recs)
		{
			std::vector<vec3> points{};

			for (size_t n{}; n < num_recs; n++)
			{
				const LagRecord *const lr{ player_data->getRecord(pl, n) };

				if (lr && lr->isRecordValid()) {
					points.push_back(lr->origin);
				}
			}

			if (strafe_pred->calc(points, move_data->m_vecVelocity.length2D(), m_yaw_per_tick))
			{
				if (!(pl->m_fFlags() & FL_ONGROUND) && pl->m_nWaterLevel() <= WL_Feet) {
					m_air_strafing = true;
					move_data->m_flForwardMove = 0.0f;
					move_data->m_flSideMove = (m_yaw_per_tick > 0.0f) ? -450.0f : 450.0f;
				}
			}
		}
	}

	game_movement.player = &player;
	game_movement.mv = move_data;

	return true;
}

bool MovePred::init(C_TFPlayer *const pl, const bool ignore_special_ability)
{
	if (!pl || pl->deadflag()) {
		return false;
	}

	player = TFPlayerProxy{ pl };

	return setupMoveData(pl, &move_data, ignore_special_ability);
}

void MovePred::tick()
{
	if (m_air_strafing)
	{
		if (!player.GetGroundEntity()) {
			move_data.m_vecViewAngles.y += m_yaw_per_tick;
		}
	}

	else {
		move_data.m_vecViewAngles.y += m_yaw_per_tick;
	}

	game_movement.ProcessMovement(&player, &move_data);
}

vec3 MovePred::origin() const
{
	return move_data.m_vecAbsOrigin;
}