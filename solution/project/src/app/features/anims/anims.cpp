#include "anims.hpp"

#include "../player_data/player_data.hpp"
#include "../networking/networking.hpp"
#include "../engine_pred/engine_pred.hpp"
#include "../anti_aim/anti_aim.hpp"
#include "../cfg.hpp"

void Anims::onNetworkUpdate(C_TFPlayer *const pl)
{
	if (!pl
		|| pl->GetClassId() != class_ids::CTFPlayer
		|| pl->m_nSequence() == -1
		|| pl->entindex() == i::engine->GetLocalPlayer()
		|| pl->InCond(TF_COND_TAUNTING)) {
		return;
	}

	CTFPlayerAnimState *const anim_state{ pl->m_PlayerAnimState() };

	if (!anim_state) {
		return;
	}

	const float og_curtime{ i::global_vars->curtime };
	const float og_frametime{ i::global_vars->frametime };

	i::global_vars->curtime = pl->m_flOldSimulationTime();
	i::global_vars->frametime = i::global_vars->interval_per_tick;

	const int update_ticks{ std::clamp(tf_utils::timeToTicks(pl->m_flSimulationTime() - pl->m_flOldSimulationTime()), 1, 24)};

	for (int n{}; n < update_ticks; n++) {
		anim_state->Update(pl->m_angEyeAngles().y, pl->m_angEyeAngles().x);
		pl->FrameAdvance(0.0f);
		i::global_vars->curtime += i::global_vars->interval_per_tick; //set any animation related timers in the right state
	}

	i::global_vars->curtime = og_curtime;
	i::global_vars->frametime = og_frametime;

	pl->SetAbsOrigin(pl->m_vecOrigin()); // unnecessary?
}

void Anims::onRunSimulation(C_BasePlayer *const base_pl, CUserCmd *const cmd, float curtime)
{
	if (!base_pl
		|| base_pl->GetClassId() != class_ids::CTFPlayer
		|| base_pl->m_nSequence() == -1
		|| base_pl->cast<C_TFPlayer>()->InCond(TF_COND_TAUNTING)) {
		return;
	}

	C_TFPlayer *const pl{ base_pl->cast<C_TFPlayer>() };

	if (!pl) {
		return;
	}

	CTFPlayerAnimState *const anim_state{ pl->m_PlayerAnimState() };

	if (!anim_state) {
		return;
	}

	// only update anims on fresh cmds
	if (cmd->hasbeenpredicted) {
		return;
	}

	const float og_curtime{ i::global_vars->curtime };
	const float og_frametime{ i::global_vars->frametime };

	i::global_vars->curtime = curtime;
	i::global_vars->frametime = i::global_vars->interval_per_tick;

	pl->FrameAdvance(0.0f);

	anim_state->Update(cmd->viewangles.y, cmd->viewangles.x);

	i::global_vars->curtime = og_curtime;
	i::global_vars->frametime = og_frametime;
}

void Anims::postCreateMove()
{
	if (i::client_state->m_nDeltaTick == -1) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local
		|| local->deadflag()
		|| local->m_nSequence() == -1) {
		return;
	}

	CTFPlayerAnimState *const og_animstate{ local->m_PlayerAnimState() };

	if (!og_animstate || !anims->m_fake_animstate) {
		return;
	}

	CUserCmd *const latest_cmd{ &i::input->m_pCommands[networking->getLatestCommandNumber() % MULTIPLAYER_BACKUP] };
	CUserCmd *const last_networked_cmd{ &i::input->m_pCommands[i::client_state->lastoutgoingcommand % MULTIPLAYER_BACKUP] };


	if (!latest_cmd || !last_networked_cmd) {
		return;
	}

	std::array<float, 24> og_poseparams{};
	memcpy(og_poseparams.data(), local->m_flPoseParameter(), sizeof(og_poseparams));

	vec3 server_angles{ last_networked_cmd->viewangles };

	server_angles.x = std::clamp(server_angles.x, -90.0f, 90.0f);
	server_angles.y = std::clamp(math::normalizeAngle(server_angles.y), -180.0f, 180.0f);

	local->m_PlayerAnimState() = m_fake_animstate;

	RUN_PREDICTION(latest_cmd, [&]()
	{
		const vec3 velocity{ (local->m_vecOrigin() - m_fake_origin) / tf_utils::ticksToTime((local->m_nTickBase() - m_last_sent_tickbase) + 1) };

		local->SetAbsVelocity(velocity);

		memcpy(local->m_flPoseParameter(), m_fake_poses.data(), sizeof(float) * 24);

		m_fake_animstate->Update(server_angles.y, server_angles.x);

		if (m_fake_bones && tf_globals::send_packet)
		{
			lrm->allowBoneSetup(true);

			local->InvalidateBoneCache();
			local->m_fEffects() |= 8; //ef_nointerp
			local->SetupBones(m_fake_bones, 128, BONE_USED_BY_ANYTHING, 0.f);
			local->m_fEffects() &= ~8;

			lrm->allowBoneSetup(false);

			m_last_sent_tickbase = local->m_nTickBase() + 1;
			m_fake_origin = local->m_vecOrigin();
		}
	});

	memcpy(local->m_flPoseParameter(), og_poseparams.data(), sizeof(og_poseparams));

	local->m_PlayerAnimState() = og_animstate;
}

bool Anims::onLoad()
{
	m_bone_origin = {};
	m_fake_origin = {};
	m_fake_poses = {};

	memset(m_local_bones.data(), 0, sizeof(m_local_bones));

	if (!m_fake_bones) {
		m_fake_bones = reinterpret_cast<matrix3x4_t *>(i::mem_alloc->Alloc(sizeof(matrix3x4_t) * 128));
	}
	else {
		memset(m_fake_bones, 0, sizeof(matrix3x4_t) * 128);
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!m_fake_animstate)
	{
		if (local) {
			m_fake_animstate = s::CreateTFPlayerAnimState.call<CTFPlayerAnimState*>(local);
		}
	}

	else if (m_fake_animstate->m_pTFPlayer != local || m_fake_animstate->m_pPlayer != local) {
		m_fake_animstate->m_pPlayer = m_fake_animstate->m_pTFPlayer = local;
	}

	else {
		m_fake_animstate->ClearAnimationState();
	}

	return true;
}

bool Anims::onLevelInit()
{
	return onLoad();
}

bool Anims::onUnload()
{
	i::mdl_cache->BeginLock();

	if (m_fake_animstate) {
		i::mem_alloc->Free(m_fake_animstate);
		m_fake_animstate = nullptr;
	}

	if (m_fake_bones) {
		i::mem_alloc->Free(m_fake_bones);
		m_fake_bones = nullptr;
	}

	i::mdl_cache->EndLock();

	return true;
}

bool Anims::onLevelShutdown()
{
	return onUnload();
}