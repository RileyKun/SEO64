#include "features/aim_utils/aim_utils.hpp"
#include "features/aimbot/aimbot.hpp"
#include "features/anims/anims.hpp"
#include "features/auto_detonate/auto_detonate.hpp"
#include "features/engine_pred/engine_pred.hpp"
#include "features/player_data/player_data.hpp"
#include "features/mat_mod/mat_mod.hpp"
#include "features/misc/misc.hpp"
#include "features/player_data/player_data.hpp"
#include "features/proj_sim/proj_sim.hpp"
#include "features/trace_utils/trace_utils.hpp"
#include "features/spectator_list/spectator_list.hpp"
#include "features/networking/networking.hpp"
#include "features/notifs/notifs.hpp"
#include "features/crits/crits.hpp"
#include "features/vis_utils/vis_utils.hpp"
#include "features/tick_base/tick_base.hpp"
#include "features/auto_backstab/auto_backstab.hpp"
#include "features/hitboxes/hitboxes.hpp"
#include "features/player_list/player_list.hpp"
#include "features/seed_pred/seed_pred.hpp"
#include "features/engine_pred/datamap_modifier.hpp"
#include "features/engine_pred/compression_handler.hpp"
#include "features/anti_aim/anti_aim.hpp"
#include "features/auto_vacc/auto_vacc.hpp"
#include "features/auto_queue/auto_queue.hpp"
#include "features/entity_visuals/entity_visuals.hpp"
#include "features/game_movement/move_pred.hpp"
#include "features/auto_reflect/auto_reflect.hpp"

#include "features/ui/ui.hpp"
#include "features/ui/menu.hpp"
#include "features/ui/binds.hpp"
#include "features/ui/minimap.hpp"

#include "features/cfg.hpp"

//#define DEBUG_HITBOXES

#ifdef DEBUG_HITBOXES
MAKE_SIG(server_UTIL_EntityByIndex, mem::findBytes("server.dll", "E8 ? ? ? ? 8B 53 40 45 33 C0").fixRip());
MAKE_SIG(server_CBaseAnimating_DrawServerHitboxes, mem::findBytes("server.dll", "44 88 44 24 18 53 48"));
#endif

MAKE_HOOK(
	CBaseWorldView_DrawExecute,
	s::CBaseWorldView_DrawExecute.get(),
	void,
	void *rcx, float waterHeight, view_id_t viewID, float waterZAdjust)
{
	tf_globals::in_CBaseWorldView_DrawExecute = true;
	CALL_ORIGINAL(rcx, waterHeight, viewID, waterZAdjust);
	tf_globals::in_CBaseWorldView_DrawExecute = false;
}

MAKE_HOOK(
	CEngineVGui_Paint,
	s::CEngineVGui_Paint.get(),
	void,
	void *rcx, PaintMode_t mode)
{
	if (mode & PAINT_INGAMEPANELS)
	{
		draw->updateW2S();

		draw->start();
		{
			entity_visuals->runPaint();
			misc->paintFOVCircle();
			crits->paint();
			tick_base->paint();
			seed_pred->paint();
			anti_aim->paint();
		}
		draw->end();
	}

	CALL_ORIGINAL(rcx, mode);

	if (mode & PAINT_UIPANELS)
	{
		draw->start();
		{
			spectator_list->run();
			notifs->run();
		}
		draw->end();

		auto_queue->run();
	}
}

MAKE_HOOK(
	CHLClient_FrameStageNotify,
	s::CHLClient_FrameStageNotify.get(),
	void,
	void *rcx, ClientFrameStage_t curStage)
{
	switch (curStage)
	{
		case FRAME_RENDER_START:
		{
			input_game->processStart();

#ifdef DEBUG_HITBOXES
			if (i::engine->GetNetChannelInfo() && i::engine->GetNetChannelInfo()->IsLoopback()) {
				for (int iter = 1; iter <= i::global_vars->maxClients; iter++)
				{
					uintptr_t server_player{ s::server_UTIL_EntityByIndex.call<uintptr_t>(iter) };
					if (server_player) {
						s::server_CBaseAnimating_DrawServerHitboxes.call<void>(server_player, -1.f, false);
					}
				}
			}
#endif
			break;
		}

		default: {
			break;
		}
	}

	CALL_ORIGINAL(rcx, curStage);

	class_ids::init();

	switch (curStage)
	{
		case FRAME_NET_UPDATE_END: {
			ec->update();
			player_data->update();
			break;
		}

		case FRAME_RENDER_START: {
			mat_mod->run();
			mini_map->cache();
			break;
		}

		case FRAME_RENDER_END: {
			input_game->processEnd();
			break;
		}

		default: {
			break;
		}
	}
}

MAKE_HOOK(
	CHLClient_LevelInitPreEntity,
	s::CHLClient_LevelInitPreEntity.get(),
	void,
	void *rcx, char const *pMapName)
{
	CALL_ORIGINAL(rcx, pMapName);

	for (HasLevelInitPreEntity *const h : getInsts<HasLevelInitPreEntity>())
	{
		if (!h) {
			continue;
		}
		
		h->onLevelInitPreEntity(pMapName);
	}
}

MAKE_HOOK(
	CHLClient_LevelInitPostEntity,
	s::CHLClient_LevelInitPostEntity.get(),
	void,
	void *rcx)
{
	CALL_ORIGINAL(rcx);

	for (HasLevelInit *const h : getInsts<HasLevelInit>())
	{
		if (!h) {
			continue;
		}

		h->onLevelInit();
	}
}

MAKE_HOOK(
	CHLClient_LevelShutdown,
	s::CHLClient_LevelShutdown.get(),
	void,
	void *rcx)
{
	CALL_ORIGINAL(rcx);

	for (HasLevelShutdown *const h : getInsts<HasLevelShutdown>())
	{
		if (!h) {
			continue;
		}

		h->onLevelShutdown();
	}
}

MAKE_HOOK(
	CInput_GetUserCmd, s::CInput_GetUserCmd.get(),
	CUserCmd *,
	void *rcx, int sequence_number)
{
	return &i::input->m_pCommands[sequence_number % MULTIPLAYER_BACKUP];
}

MAKE_HOOK(
	CHLClient_CreateMove,
	mem::findVirtual(i::client, 21).get(),
	void,
	IBaseClientDLL *rcx, int sequence_number, float input_sample_frametime, bool active)
{
	CALL_ORIGINAL(rcx, sequence_number, input_sample_frametime, active);

	CUserCmd *const cmd{ &i::input->m_pCommands[sequence_number % MULTIPLAYER_BACKUP] };

	if (!cmd) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return;
	}

	engine_pred->update();

	datamap_mod->initInGame();

	cmd->random_seed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;

	input_tick->processStart();

	//if we're warping we need to adjust our tickbase immediately to get all of our features to use the correct curtime
	if (tick_base->checkWarp(cmd))
	{
		const int ticks_to_shift{ tick_base->getShiftLimit(true) };
		const int server_tick{ tf_utils::timeToTicks(tf_utils::getServerTime() + tf_utils::getLatency()) - 1 };
		const int adjusted_tick{ tick_base->getAdjustedTick(ticks_to_shift + 1, local->m_nTickBase(), server_tick) };

		local->m_nTickBase() = adjusted_tick;
	}

	misc->tauntSpin(cmd);
	misc->sentryWiggler(cmd);
	misc->bhop(cmd);
	misc->fastStop(cmd);
	misc->autoStrafe(cmd);
	misc->mvmRespawn();
	misc->autoDisguise();
	misc->autoTurn(cmd);

	engine_pred->edgeJump(cmd);

	tf_globals::aimbot_target = EntityHandle_t{};
	tf_globals::aimbot_target_hitbox = -1;

	anti_aim->runPreMove(cmd);

	RUN_PREDICTION(cmd, [&]() 
	{
		aim_utils->blockAttack(cmd);

		aimbot->run(cmd);

		auto_detonate->run(cmd);
		auto_backstab->run(cmd);
		auto_vacc->run(cmd);
		auto_reflect->run(cmd);

		seed_pred->run(cmd);
		crits->run(cmd);

		anti_aim->runFakelag(cmd);

		anti_aim->run(cmd);
	});

	if (tick_base->canDoubleTap(cmd, true) || tick_base->m_should_warp) {
		tick_base->sendMoreMove(cmd);
	}

	input_tick->processEnd();

	tf_globals::last_cmd = *cmd;

	misc->noisemakerSpam();

	if (tf_globals::send_packet) {
		anti_aim->m_jitter_flip = !anti_aim->m_jitter_flip;
	}

	anims->postCreateMove();
}

MAKE_HOOK(
	C_TFPlayer_CreateMove,
	s::C_TFPlayer_CreateMove.get(),
	bool,
	void *rcx, float flInputSampleTime, CUserCmd *cmd)
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		anims->onLoad();
		return CALL_ORIGINAL(rcx, flInputSampleTime, cmd);
	}

	const bool taunting{ local->InCond(TF_COND_TAUNTING) };

	if (cfg::misc_taunts_slide && taunting && local->m_bAllowMoveDuringTaunt()) {
		return false;
	}

	if (cfg::misc_remove_teammate_pushaway && !taunting) {
		return false;
	}

	return CALL_ORIGINAL(rcx, flInputSampleTime, cmd);
}

MAKE_HOOK(
	ClientModeShared_OverrideView,
	s::ClientModeShared_OverrideView.get(),
	void,
	void *rcx, CViewSetup *pSetup)
{
	if (cfg::tp_override_active && pSetup)
	{
		if (C_TFPlayer *const local{ ec->getLocal() })
		{
			bool should_tp{ cfg::tp_override_force || local->InCond(TF_COND_TAUNTING) || menu->aa_hovered };

			if (local->m_iObserverMode() != OBS_MODE_NONE) {
				should_tp = false;
			}

			static bool should_tp_prev{ should_tp };

			const float cur_time{ i::global_vars->curtime };

			static float lerp_start{};
			static float lerp_end{};

			if (should_tp != should_tp_prev) {
				lerp_start = cur_time;
				lerp_end = cur_time + cfg::tp_override_lerp_time;
				should_tp_prev = should_tp;
			}

			if (should_tp)
			{
				if (local->InFirstPersonView()) {
					i::input->CAM_ToThirdPerson();
					local->ThirdPersonSwitch();
				}

				vec3 forward{};
				vec3 right{};
				vec3 up{};

				math::angleVectors(pSetup->angles, &forward, &right, &up);

				const vec3 offset{ (forward * cfg::tp_override_dist_forward) - (right * cfg::tp_override_dist_right) - (up * cfg::tp_override_dist_up) };

				const float lerp_progress{ math::remap(cur_time, lerp_start, lerp_end, 0.25f, 1.0f) };

				float fraction{ 1.0f };

				if (cfg::tp_override_collision)
				{
					trace_t tr{};

					tf_utils::traceHull
					(
						pSetup->origin, pSetup->origin - (offset * lerp_progress),
						{ -10.0f, -10.0f, -10.0f },
						{ 10.0f, 10.0f, 10.0f },
						&trace_filters::world,
						&tr,
						MASK_SOLID
					);

					fraction = tr.fraction;
				}

				pSetup->origin -= (offset * lerp_progress) * fraction;
			}

			else
			{
				if (!local->InFirstPersonView()) {
					i::input->CAM_ToFirstPerson();
					//local->ThirdPersonSwitch(); //seems to be called in CInput::CAM_ToFirstPerson
				}
			}
		}

		return;
	}

	CALL_ORIGINAL(rcx, pSetup);
}

MAKE_HOOK(
	CL_CheckForPureServerWhitelist,
	s::CL_CheckForPureServerWhitelist.get(),
	void,
	void **pFilesToReload)
{
	if (cfg::misc_sv_pure_bypass) {
		return;
	}

	CALL_ORIGINAL(pFilesToReload);
}

MAKE_HOOK(
	CMatSystemSurface_LockCursor,
	s::CMatSystemSurface_LockCursor.get(),
	void,
	void *rcx)
{
	if (ui->isOpen()) {
		i::surface->UnlockCursor();
		return;
	}

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CMatSystemSurface_SetCursor,
	s::CMatSystemSurface_SetCursor.get(),
	void,
	void *rcx, HCursor cursor)
{
	if (ui->isOpen()) {
		cursor = dc_arrow;
	}

	CALL_ORIGINAL(rcx, cursor);
}

MAKE_HOOK(
	CModelRender_DrawModelExecute,
	s::CModelRender_DrawModelExecute.get(),
	void,
	void *rcx, const DrawModelState_t &state, const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld)
{
	if (cfg::force_low_lod_models) {
		*const_cast<int *>(&state.m_lod) = 5;
	}

	if (IClientEntity *const client_ent{ i::ent_list->GetClientEntity(pInfo.entity_index) })
	{
		if (C_BaseEntity *const base_ent{ client_ent->GetBaseEntity() })
		{
			if (ev_effects->hasDrawn(base_ent->GetRefEHandle()) && !player_data->isDrawingModels()) {
				return;
			}
		}

		if (cfg::mat_mod_active && client_ent->GetClassId() == class_ids::CDynamicProp)
		{
			const float night_scale{ math::remap(cfg::mat_mod_night_scale, 0.0f, 1.0f, 1.0f, 0.1f) };

			i::render_view->SetColorModulation(
				cfg::mat_mod_mode == 0 ? Color{ 255, 255, 255, 255 }.scaled(night_scale) : cfg::mat_mod_color_props
			);

			CALL_ORIGINAL(rcx, state, pInfo, pCustomBoneToWorld);

			i::render_view->SetColorModulation({ 255, 255, 255, 255 });

			return;
		}

		if (client_ent->GetClassId() == class_ids::CTFViewModel)
		{
			if (cfg::remove_hands) {
				return;
			}

			else if (C_TFPlayer *player{ ec->getLocal() })
			{
				if (player->m_iObserverMode() == OBS_MODE_IN_EYE 
					&& player->m_hObserverTarget().Get()
					&& player->m_hObserverTarget().Get()->GetClassId() == class_ids::CTFPlayer) {
					player = player->m_hObserverTarget().Get()->cast<C_TFPlayer>();
				}

				if (C_TFWeaponBase *const weapon{ player->m_hActiveWeapon().Get()->cast<C_TFWeaponBase>() })
				{
					if (weapon->m_iItemDefinitionIndex() == Soldier_m_TheOriginal) {
						return;
					}
				}
			}
		}
	}

	CALL_ORIGINAL(rcx, state, pInfo, pCustomBoneToWorld);
}

MAKE_HOOK(
	CModelRender_ForcedMaterialOverride,
	s::CModelRender_ForcedMaterialOverride.get(),
	void,
	void *rcx, IMaterial *mat, OverrideType_t type)
{
	if (tf_globals::block_mat_override) {
		return;
	}

	CALL_ORIGINAL(rcx, mat, type);
}

MAKE_HOOK(
	CModelRender_StudioSetupLighting,
	s::CModelRender_StudioSetupLighting.get(),
	void,
	void *rcx, const DrawModelState_t &state, /*const*/ Vector &absEntCenter,
	LightCacheHandle_t *pLightcache, bool bVertexLit, bool bNeedsEnvCubemap, bool &bStaticLighting,
	DrawModelInfo_t &drawInfo, const ModelRenderInfo_t &pInfo, int drawFlags)
{
	if (cfg::static_player_model_lighting && pInfo.pRenderable)
	{
		if (IClientUnknown *const unknown{ pInfo.pRenderable->GetIClientUnknown() })
		{
			if (C_BaseEntity *const ent{ unknown->GetBaseEntity() })
			{
				if (C_BaseEntity *const owner{ ent->m_hOwnerEntity() })
				{
					if (owner->GetClassId() == class_ids::CTFPlayer) {
						absEntCenter = owner->GetRenderCenter();
					}
				}

				else
				{
					if (ent->GetClassId() == class_ids::CTFPlayer) {
						absEntCenter = ent->GetRenderCenter();
					}
				}
			}
		}
	}

	CALL_ORIGINAL(rcx, state, absEntCenter, pLightcache, bVertexLit, bNeedsEnvCubemap, bStaticLighting, drawInfo, pInfo, drawFlags);
}

MAKE_HOOK(
	CShaderAPIDx8_BindStandardTexture,
	s::CShaderAPIDx8_BindStandardTexture.get(),
	void,
	void *rcx, Sampler_t sampler, StandardTextureId_t id)
{
	if (cfg::spritecard_texture_hack && reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::BindStandardTexture_spritecard_call.get()) {
		CALL_ORIGINAL(rcx, SHADER_SAMPLER0, TEXTURE_WHITE);
		return;
	}

	CALL_ORIGINAL(rcx, sampler, id);
}

MAKE_HOOK(
	CParticleSystemMgr_DrawRenderCache,
	s::CParticleSystemMgr_DrawRenderCache.get(),
	void,
	void *rcx, bool bShadowDepth)
{
	if (tf_globals::in_CBaseWorldView_DrawExecute)
	{
		if (IMatRenderContext *const rc{ i::mat_sys->GetRenderContext() }) {
			rc->ClearBuffers(false, false, true);
		}

		misc->paintPredPath();
		misc->paintSniperSightlines();
		entity_visuals->runModels();
		player_data->visual();
		hitboxes->renderFakeHitboxes();
		hitboxes->render();
		proj_sim->render();
	}

	CALL_ORIGINAL(rcx, bShadowDepth);
}

MAKE_HOOK(
	CRendering3dView_EnableWorldFog,
	s::CRendering3dView_EnableWorldFog.get(),
	void,
	void *rcx)
{
	if (cfg::remove_fog)
	{
		if (IMatRenderContext *const rc{ i::mat_sys->GetRenderContext() })
		{
			rc->FogMode(MATERIAL_FOG_LINEAR);
			rc->FogColor3f(0.0f, 0.0f, 0.0f);
			rc->FogStart(0.0f);
			rc->FogEnd(0.0f);
			rc->FogMaxDensity(0.0f);

			return;
		}
	}

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CSkyboxView_GetSkyboxFogEnable,
	s::CSkyboxView_GetSkyboxFogEnable.get(),
	bool,
	void *rcx)
{
	if (cfg::remove_sky_fog) {
		return false;
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CStudioRender_DrawModelStaticProp,
	s::CStudioRender_DrawModelStaticProp.get(),
	void,
	void *rcx, const DrawModelInfo_t &drawInfo, const matrix3x4_t &modelToWorld, int flags)
{
	if (cfg::mat_mod_active)
	{
		const float night_scale{ math::remap(cfg::mat_mod_night_scale, 0.0f, 1.0f, 1.0f, 0.1f) };

		i::studio_render->SetColorModulation(
			cfg::mat_mod_mode == 0 ? Color{ 255, 255, 255, 255 }.scaled(night_scale) : cfg::mat_mod_color_props
		);
	}

	CALL_ORIGINAL(rcx, drawInfo, modelToWorld, flags);
}

MAKE_HOOK(
	CTFPlayerShared_InCond,
	s::CTFPlayerShared_InCond.get(),
	bool,
	void *rcx, ETFCond eCond)
{
	if (C_TFPlayer *const pl{ *reinterpret_cast<C_TFPlayer **>(reinterpret_cast<uintptr_t>(rcx) + 440 /*CTFPlayerShared::m_pOuter*/) })
	{
		if (cfg::remove_scope && pl->entindex() == i::engine->GetLocalPlayer())
		{
			const uintptr_t ret_address{ reinterpret_cast<uintptr_t>(_ReturnAddress()) };

			if (ret_address == s::CTFPlayerShared_InCond_CHudScope_ShouldDraw_Call.get()) {
				return false;
			}

			if (pl->InFirstPersonView())
			{
				if (ret_address == s::CTFPlayerShared_InCond_ClientModeTFNormal_ShouldDrawViewModel_Call.get()) {
					return false;
				}
			}

			else
			{
				if (ret_address == s::CTFPlayerShared_InCond_C_TFPlayer_ShouldDraw_Call.get()
					|| ret_address == s::CTFPlayerShared_InCond_C_TFWearable_ShouldDraw_Call.get())
				{
					return false;
				}
			}
		}

		if (eCond == TF_COND_DISGUISED)
		{
			if (C_TFPlayer *const local{ ec->getLocal() })
			{
				if (pl->m_iTeamNum() != local->m_iTeamNum()) {
					return false;
				}
			}
		}
	}

	return CALL_ORIGINAL(rcx, eCond);
}

MAKE_HOOK(
	KeyValues_SetInt,
	s::KeyValues_SetInt.get(),
	void,
	void *rcx, const char *keyName, int value)
{
	CALL_ORIGINAL(rcx, keyName, value);

	if (cfg::reveal_scoreboard
		&& reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::KeyValues_SetInt_RS_From.get()
		&& !strcmp(keyName, "nemesis"))
	{
		*reinterpret_cast<uintptr_t *>(_AddressOfReturnAddress()) = s::KeyValues_SetInt_RS_To.get();
	}
}

MAKE_HOOK(
	CTFPlayerShared_IsPlayerDominated,
	s::CTFPlayerShared_IsPlayerDominated.get(),
	bool,
	void *rcx, int index)
{
	const bool result{ CALL_ORIGINAL(rcx, index) };

	if (cfg::reveal_scoreboard
		&& !result
		&& reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::CTFPlayerShared_IsPlayerDominated_RS_From.get())
	{
		*reinterpret_cast<uintptr_t *>(_AddressOfReturnAddress()) = s::CTFPlayerShared_IsPlayerDominated_RS_To.get();
	}

	return result;
}

static int sb_color_player_index{};
static bool sb_color_player_dead{};

MAKE_HOOK(
	C_TFPlayerResource_GetPlayerConnectionState,
	s::C_TFPlayerResource_GetPlayerConnectionState.get(),
	MM_PlayerConnectionState_t,
	void *rcx, int index)
{
	if (!cfg::color_scoreboard) {
		return CALL_ORIGINAL(rcx, index);
	}

	const MM_PlayerConnectionState_t result{ CALL_ORIGINAL(rcx, index) };

	if (result != MM_WAITING_FOR_PLAYER
		&& reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::CTFClientScoreBoardDialog_UpdatePlayerList_C_TFPlayerResource_GetPlayerConnectionState_Call.get())
	{
		if (C_TFGameRulesProxy *const game_rules{ ec->getGameRules() })
		{
			if (game_rules->m_bPlayingMannVsMachine()) {
				sb_color_player_index = -1;
				return result;
			}
		}

		sb_color_player_index = index;

		if (C_TFPlayerResource *const rsrc{ ec->getPlayerResource() })
		{
			if (rsrc->GetAlive(index)) {
				sb_color_player_dead = false;
			}

			else {
				sb_color_player_dead = true;
			}

			rsrc->SetAlive(index, true);
		}
	}

	else {
		sb_color_player_index = -1;
	}

	return result;
}

MAKE_HOOK(
	C_PlayerResource_GetTeamColor,
	s::C_PlayerResource_GetTeamColor.get(),
	const Color &,
	void *rcx, int index) 
{
	if (!cfg::color_scoreboard) {
		return CALL_ORIGINAL(rcx, index);
	}

	if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::CTFClientScoreBoardDialog_UpdatePlayerList_C_PlayerResource_GetTeamColor.get())
	{
		if (sb_color_player_index != -1)
		{
			static Color col{ 255,255,255,255 };

			IClientEntity *const client_ent{ i::ent_list->GetClientEntity(sb_color_player_index) };

			if (!client_ent) {
				return CALL_ORIGINAL(rcx, index);
			}

			C_TFPlayer *const pl{ client_ent->cast<C_TFPlayer>() };

			if (!pl) {
				return CALL_ORIGINAL(rcx, index);
			}

			col = vis_utils->getEntColor(pl);

			// restore old alive state
			if (sb_color_player_dead)
			{
				if (C_TFPlayerResource *const rsrc{ ec->getPlayerResource() }) {
					rsrc->SetAlive(sb_color_player_index, false);
				}

				col = col.lerp({ 0, 0, 0, 255 }, 0.5f);
			}

			return col;
		}
	}

	else
	{
		if (C_TFPlayer *const local{ ec->getLocal() })
		{
			const bool enemy{ index != local->m_iTeamNum() };

			if (enemy) {
				return cfg::color_enemy;
			}

			else {
				return cfg::color_teammate;
			}
		}
	}

	return CALL_ORIGINAL(rcx, index);
}

MAKE_HOOK(
	CUserMessages_DispatchUserMessage,
	s::CUserMessages_DispatchUserMessage.get(),
	bool,
	void *rcx, int msg_type, bf_read &msg_data)
{
	if (msg_type == 10 && cfg::remove_shake) {
		return true;
	}

	if (msg_type == 11 && cfg::remove_fade) {
		return true;
	}

	if (msg_type == 5) {
		seed_pred->recvPlayerPerf(msg_data);
		return true;
	}

	return CALL_ORIGINAL(rcx, msg_type, msg_data);
}

//MAKE_HOOK(
//	ClientModeShared_FireGameEvent,
//	s::ClientModeShared_FireGameEvent.get(),
//	void,
//	void *rcx, IGameEvent *event)
//{
//	CALL_ORIGINAL(rcx, event);
//}

MAKE_HOOK(
	CGameEventManager_FireGameEventIntern,
	s::CGameEventManager_FireGameEventIntern.get(),
	bool,
	void *rcx, IGameEvent *event, bool server_only, bool client_only)
{
	if (!client_only) {
		return CALL_ORIGINAL(rcx, event, server_only, client_only);
	}

	for (HasGameEvent *const h : getInsts<HasGameEvent>())
	{
		if (!h) {
			continue;
		}

		h->onGameEvent(event);
	}

	return CALL_ORIGINAL(rcx, event, server_only, client_only);
}

MAKE_HOOK(
	CViewRender_DrawViewModels,
	s::CViewRender_DrawViewModels.get(),
	void,
	void *rcx, const CViewSetup &viewRender, bool drawViewmodel)
{
	CALL_ORIGINAL(rcx, viewRender, drawViewmodel);

	ev_effects->processOutlineEffect();
}

MAKE_HOOK(
	C_BasePlayer_CalcPlayerView,
	s::C_BasePlayer_CalcPlayerView.get(),
	void,
	void *rcx, Vector &eyeOrigin, QAngle &eyeAngles, float &fov)
{
	if (cfg::remove_punch)
	{
		if (C_BasePlayer *const pl{ reinterpret_cast<C_BasePlayer *>(rcx) })
		{
			const vec3 original_punch_angle{ pl->m_vecPunchAngle() };

			pl->m_vecPunchAngle() = {};
			CALL_ORIGINAL(rcx, eyeOrigin, eyeAngles, fov);
			pl->m_vecPunchAngle() = original_punch_angle;

			return;
		}
	}

	CALL_ORIGINAL(rcx, eyeOrigin, eyeAngles, fov);
}

MAKE_HOOK(
	C_BasePlayer_GetFOV,
	s::C_BasePlayer_GetFOV.get(),
	float,
	void *rcx)
{
	if (cfg::fov_override_active)
	{
		if (C_TFPlayer *const pl{ reinterpret_cast<C_TFPlayer *>(rcx) })
		{
			const int original_fov{ pl->m_iFOV() };
			const int original_fov_start{ pl->m_iFOVStart() };

			pl->m_iFOV() = pl->InCond(TF_COND_ZOOMED) ? cfg::fov_override_value_zoomed : cfg::fov_override_value;

			if (original_fov_start == std::max(75, cvars::fov_desired->GetInt())) {
				pl->m_iFOVStart() = cfg::fov_override_value;
			}

			if (original_fov_start == TF_WEAPON_ZOOM_FOV) {
				pl->m_iFOVStart() = cfg::fov_override_value_zoomed;
			}

			const float result{ CALL_ORIGINAL(rcx) };

			pl->m_iFOV() = original_fov;
			pl->m_iFOVStart() = original_fov_start;

			return result;
		}
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	C_TFPlayer_ThirdPersonSwitch,
	s::C_TFPlayer_ThirdPersonSwitch.get(),
	void,
	void *rcx)
{
	CALL_ORIGINAL(rcx);

	C_TFPlayer *const local{ ec->getLocal() };

	if (!rcx || !local || rcx != local) {
		return;
	}

	C_BaseEntity *const wep{ local->m_hActiveWeapon().Get() };

	if (!wep || wep->cast<C_TFWeaponBase>()->GetWeaponID() != TF_WEAPON_MINIGUN) {
		return;
	}

	C_TFMinigun *const minigun{ wep->cast<C_TFMinigun>() };

	if (!minigun || minigun->m_iState() != AC_STATE_FIRING) {
		return;
	}

	minigun->StartMuzzleEffect();
	minigun->StartBrassEffect();
}

MAKE_HOOK(
	DoEnginePostProcessing,
	s::DoEnginePostProcessing.get(),
	void,
	int x, int y, int w, int h, bool bFlashlightIsOn, bool bPostVGui)
{
	if (cfg::remove_post_processing) {
		return;
	}

	CALL_ORIGINAL(x, y, w, h, bFlashlightIsOn, bPostVGui);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

MAKE_HOOK(
	MatSurfaceWindowProc,
	s::MatSurfaceWindowProc.get(),
	LRESULT,
	HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	input_game->wndproc(uMsg, wParam, lParam);
	input_tick->wndproc(uMsg, wParam, lParam);
	input_dx->wndproc(uMsg, wParam, lParam);

	if (ui->isOpen())
	{
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

		if ((uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST) || (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)) {
			return CALL_ORIGINAL(hwnd, uMsg, 0, 0);
		}
	}

	return CALL_ORIGINAL(hwnd, uMsg, wParam, lParam);
}

MAKE_HOOK(
	CTFMatchSumary_OnTick,
	s::CTFMatchSumary_OnTick.get(),
	void,
	void *rcx)
{
	if (cfg::misc_remove_end_of_match_surveys)
	{
		const uintptr_t gc_client_system{ s::GTFGCClientSystem.call<uintptr_t>() };

		if (!gc_client_system) {
			return;
		}

		const int32_t offset{ *s::CMsgSurveyRequest_match_id_Offset.cast<int32_t *>() };

		uintptr_t *const msg_survey_request{ reinterpret_cast<uintptr_t *>(gc_client_system + offset) };

		if (msg_survey_request && *msg_survey_request & 2) {
			*msg_survey_request &= ~2;
		}
	}

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CMatchInviteNotification_OnTick,
	s::CMatchInviteNotification_OnTick.get(),
	void,
	void *rcx)
{
	if (cfg::auto_accept_active)
	{
		if (cfg::auto_accept_mode == 0 && i::engine->IsInGame()) { // only in menu
			return;
		}

		s::CMatchInviteNotification_AcceptMatch.call<void>(rcx);
	}

	if (cfg::misc_remove_autojoin_countdown)
	{
		const uintptr_t notification{ reinterpret_cast<uintptr_t>(rcx) };
		const int32_t offset{ *s::CMatchInviteNotification_Flags_Offset.cast<int32_t *>() };
		const uint32_t flags{ *reinterpret_cast<uint32_t *>(notification + offset) };

		if ((flags & ~2)) {
			return;
		}
	}

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CEconItemSchema_GetItemDefinition,
	s::CEconItemSchema_GetItemDefinition.get(),
	void *,
	void *rcx, int iItemIndex)
{
	void *const result{ CALL_ORIGINAL(rcx, iItemIndex) };

	const size_t offset{ 524 }; // mfed: todo: sig this offset

	// setting m_unEquipRegionMask and m_unEquipRegionConflictMask to 0
	if (cfg::misc_remove_equip_regions && result) {
		*reinterpret_cast<uint32_t *>(reinterpret_cast<uintptr_t>(result) + offset) = 0;
		*reinterpret_cast<uint32_t *>(reinterpret_cast<uintptr_t>(result) + offset + 4) = 0;
	}

	return result;
}

MAKE_HOOK(
	C_OP_RenderSprites_RenderSpriteCard,
	s::C_OP_RenderSprites_RenderSpriteCard.get(),
	void,
	void *rcx, void *meshBuilder, void *pCtx, SpriteRenderInfo_t &info, int hParticle, void *pSortList, void *pCamera)
{
	if (cfg::particle_mod_active)
	{
		Color color{ cfg::particle_mod_color };

		if (cfg::particle_mod_mode == 1) {
			color = utils::rainbow(i::global_vars->curtime * cfg::particle_mod_rainbow_speed, 255);
		}

		info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 0].m128_f32[hParticle & 0x3] = color.r / 255.0f;
		info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 1].m128_f32[hParticle & 0x3] = color.g / 255.0f;
		info.m_pRGB[((hParticle / 4) * info.m_nRGBStride) + 2].m128_f32[hParticle & 0x3] = color.b / 255.0f;
	}

	CALL_ORIGINAL(rcx, meshBuilder, pCtx, info, hParticle, pSortList, pCamera);
}

MAKE_HOOK(
	NotificationQueue_Add,
	s::NotificationQueue_Add.get(),
	int,
	CEconNotification *pNotification)
{
	if (cfg::misc_remove_item_found_notification && pNotification)
	{
		const std::string_view message{ pNotification->m_pText };

		if (!message.empty() && message.compare("TF_HasNewItems") == 0)
		{
			pNotification->Accept();
			pNotification->Trigger();
			pNotification->UpdateTick();
			pNotification->MarkForDeletion();

			return 0;
		}

		i::cvar->ConsoleColorPrintf({ 255, 255, 255, 255 }, "NotificationQueue_Add: %s\n", message.data());
	}

	return CALL_ORIGINAL(pNotification);
}

MAKE_HOOK(
	CInventoryManager_ShowItemsPickedUp,
	s::CInventoryManager_ShowItemsPickedUp.get(),
	bool,
	void *rcx, bool bForce, bool bReturnToGame, bool bNoPanel)
{
	if (cfg::misc_remove_item_found_notification) {
		CALL_ORIGINAL(rcx, true, true, true);
		return false;
	}

	return CALL_ORIGINAL(rcx, bForce, bReturnToGame, bNoPanel);
}

MAKE_HOOK(
	CClientState_ProcessFixAngle,
	s::CClientState_ProcessFixAngle.get(),
	bool,
	void *rcx, void *msg)
{
	if (cfg::misc_remove_fix_angle) {
		return false;
	}

	return CALL_ORIGINAL(rcx, msg);
}

MAKE_HOOK(
	ISurface_OnScreenSizeChanged,
	mem::findVirtual(i::surface, 111).get(),
	void,
	void *rcx, int nOldWidth, int nOldHeight)
{
	CALL_ORIGINAL(rcx, nOldWidth, nOldHeight);

	for (Font *const f : getInsts<Font>())
	{
		if (!f) {
			continue;
		}

		f->init();
	}

	for (HasScreenSizeChange *const h : getInsts<HasScreenSizeChange>())
	{
		if (!h) {
			continue;
		}

		h->onScreenSizeChange();
	}
}

MAKE_HOOK(
	CL_ReadPackets,
	s::CL_ReadPackets.get(),
	void,
	bool final_tick)
{
	if (networking->shouldReadPackets()) {
		CALL_ORIGINAL(final_tick);
	}
}

MAKE_HOOK(
	CL_Move,
	s::CL_Move.get(),
	void,
	float accumulated_extra_samples, bool final_tick)
{
	networking->fixNetworking(final_tick);
	networking->move(accumulated_extra_samples, final_tick);
}

MAKE_HOOK(
	C_BaseAnimating_Interpolate,
	s::C_BaseAnimating_Interpolate.get(),
	bool,
	void *rcx, float flCurrentTime)
{
	if (C_BaseAnimating *const animating{ reinterpret_cast<C_BaseAnimating *>(rcx) })
	{
		if (animating->m_bPredictable())
		{
			if (tick_base->m_recharge) {
				return true;
			}
		}

		else
		{
			if (cfg::remove_interp) {
				return true;
			}
		}
	}

	return CALL_ORIGINAL(rcx, flCurrentTime);
}

MAKE_HOOK(
	C_BaseEntity_EstimateAbsVelocity,
	s::C_BaseEntity_EstimateAbsVelocity.get(),
	void,
	void *rcx, Vector &vel)
{
	if (C_BaseEntity *const ent{ reinterpret_cast<C_BaseEntity *>(rcx) }) {
		vel = ent->GetAbsVelocity();
	}
}

MAKE_HOOK(
	CSequenceTransitioner_CheckForSequenceChange,
	s::CSequenceTransitioner_CheckForSequenceChange.get(),
	void,
	void *rcx, CStudioHdr *hdr, int nCurSequence, bool bForceNewSequence, bool bInterpolate)
{
	CALL_ORIGINAL(rcx, hdr, nCurSequence, bForceNewSequence, false);
}

MAKE_HOOK(
	C_TFPlayer_UpdateClientSideAnimation,
	s::C_TFPlayer_UpdateClientSideAnimation.get(),
	void,
	void *rcx)
{
	C_TFPlayer *const pl{ reinterpret_cast<C_TFPlayer *>(rcx) };

	if (!pl) {
		return;
	}

	if (pl->entindex() == i::engine->GetLocalPlayer() && !pl->InCond(TF_COND_TAUNTING))
	{
		if (C_TFWeaponBase *const wep{ ec->getWeapon() }) {
			wep->UpdateAllViewmodelAddons();
		}
	}

	if (pl->InCond(TF_COND_TAUNTING)) {
		CALL_ORIGINAL(rcx);
	}

	CStudioHdr *const hdr{ pl->GetModelPtr() };

	if (!hdr) {
		return;
	}

	if (pl->entindex() != i::engine->GetLocalPlayer())
	{
		if (cfg::remove_interp) {
			return;
		}

		const size_t num_records{ player_data->getNumRecords(pl)};

		if (num_records <= 0) {
			return;
		}

		const LagRecord *const first_record{ player_data->getRecord(pl, 0) };

		// in the off chance that we run out of lag records to work with we'll just let the game setup shit bones for rendering so they dont lag behind
		if (!first_record || !first_record->bones_setup) {
			lrm->allowBoneSetup(true);
			pl->InvalidateBoneCache();
			pl->SetupBones(nullptr, 128, BONE_USED_BY_ANYTHING, pl->m_flSimulationTime());
			lrm->allowBoneSetup(false);
			return;
		}

		for (int iter{}; iter < pl->m_CachedBoneData().Count(); iter++)
		{
			const matrix3x4_t &bone{ first_record->bones[iter] };
			const vec3 bone_pos{ bone[0][3], bone[1][3], bone[2][3] };
			const vec3 render_origin{ pl->GetRenderOrigin() + (bone_pos - first_record->origin) };

			matrix3x4_t &cached_bone{ pl->m_CachedBoneData().Element(iter) };

			cached_bone[0][3] = render_origin.x;
			cached_bone[1][3] = render_origin.y;
			cached_bone[2][3] = render_origin.z;
		}

		pl->SetupBones_AttachmentHelper(hdr);
	}

	else //fix for local player
	{
		//shoutouts ucuser0 for helping me out with fixing this
		memcpy(pl->m_CachedBoneData().Base(), anims->m_local_bones.data(), sizeof(matrix3x4_t) * pl->m_CachedBoneData().Count());

		for (int iter{}; iter < pl->m_CachedBoneData().Count(); iter++)
		{
			const matrix3x4_t &bone{ anims->m_local_bones[iter] };
			const vec3 bone_pos{ bone[0][3], bone[1][3], bone[2][3] };
			const vec3 render_origin{ pl->GetRenderOrigin() + (bone_pos - anims->m_bone_origin) };

			matrix3x4_t &cached_bone{ pl->m_CachedBoneData().Element(iter) };

			cached_bone[0][3] = render_origin.x;
			cached_bone[1][3] = render_origin.y;
			cached_bone[2][3] = render_origin.z;
		}

		pl->SetupBones_AttachmentHelper(hdr);
	}
}

MAKE_HOOK(
	C_BaseEntity_ResetLatched,
	s::C_BaseEntity_ResetLatched.get(),
	void,
	void *rcx)
{
	if (C_BaseEntity *const ent{ reinterpret_cast<C_BaseEntity *>(rcx) })
	{
		if (ent->entindex() == i::engine->GetLocalPlayer()) {
			return;
		}
	}

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	C_TFViewModel_CalcViewModelView,
	s::C_TFViewModel_CalcViewModelView.get(),
	void,
	void *rcx, C_BasePlayer *owner, const Vector &eyePosition, const QAngle &eyeAngles)
{
	if (!cfg::vm_override_active) {
		CALL_ORIGINAL(rcx, owner, eyePosition, eyeAngles);
		return;
	}

	const std::string og_tf_viewmodels_offset_override{ cvars::tf_viewmodels_offset_override->GetString() };
	const float og_cl_wpn_sway_interp{ cvars::cl_wpn_sway_interp->GetFloat() };

	cvars::tf_viewmodels_offset_override->SetValue(std::format("{} {} {}", cfg::vm_override_offset_x, cfg::vm_override_offset_y, cfg::vm_override_offset_z).c_str());
	cvars::cl_wpn_sway_interp->SetValue(math::remap(cfg::vm_override_sway, 0.0f, 100.0f, 0.0f, 0.1f));

	CALL_ORIGINAL(rcx, owner, eyePosition, eyeAngles);

	cvars::tf_viewmodels_offset_override->SetValue(og_tf_viewmodels_offset_override.c_str());
	cvars::cl_wpn_sway_interp->SetValue(og_cl_wpn_sway_interp);
}

MAKE_HOOK(
	CHudTournamentSetup_OnTick,
	s::CHudTournamentSetup_OnTick.get(),
	void,
	void *rcx)
{
	CALL_ORIGINAL(rcx);

	misc->autoMvmReadyUp();
}

MAKE_HOOK(
	C_BasePlayer_PostThink,
	s::C_BasePlayer_PostThink.get(),
	void,
	C_BasePlayer *rcx)
{
	if (engine_pred->isInEnginePred()) {
		return;
	}

	return CALL_ORIGINAL(rcx);
}

static bool shot_is_from_local{};
static bool show_tracers{};

MAKE_HOOK(
	C_TFPlayer_FireBullet,
	s::C_TFPlayer_FireBullet.get(),
	void,
	void *rcx, C_TFWeaponBase *weapon,
	FireBulletsInfo_t &info, bool do_effects, int damage_type, int custom_damage_type)
{
	shot_is_from_local = false;

	if (!cfg::tracers_active) {
		return CALL_ORIGINAL(rcx, weapon, info, do_effects, damage_type, custom_damage_type);
	}

	const C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return CALL_ORIGINAL(rcx, weapon, info, do_effects, damage_type, custom_damage_type);
	}

	if (reinterpret_cast<C_TFPlayer *>(rcx) == local) {
		shot_is_from_local = true;
	}

	else {
		return CALL_ORIGINAL(rcx, weapon, info, do_effects, damage_type, custom_damage_type);
	}

	show_tracers = false;

	static int last_tick_count{};
	static int last_bullet_amount{};
	static int shot_count{};

	if (!weapon || weapon != ec->getWeapon() || !i::pred->m_bFirstTimePredicted) {
		shot_count = 0;
		return CALL_ORIGINAL(rcx, weapon, info, do_effects, damage_type, custom_damage_type);
	}

	int bullets_per_shot{ weapon->m_pWeaponInfo()->GetWeaponData(0).m_nBulletsPerShot };

	if (bullets_per_shot >= 1) {
		tf_utils::attribHookValue(bullets_per_shot, "mult_bullets_per_shot", weapon);
	}

	else {
		bullets_per_shot = 1;
	}

	if (bullets_per_shot != last_bullet_amount) {
		shot_count = 0;
		last_bullet_amount = bullets_per_shot;
		return CALL_ORIGINAL(rcx, weapon, info, do_effects, damage_type, custom_damage_type);
	}

	if (last_tick_count == engine_pred->current_prediction_cmd)
	{
		shot_count++;

		if (shot_count >= bullets_per_shot) {
			return CALL_ORIGINAL(rcx, weapon, info, do_effects, damage_type, custom_damage_type);
		}
	}

	shot_count = 0;
	last_tick_count = engine_pred->current_prediction_cmd;

	show_tracers = true;

	info.m_iTracerFreq = 1;

	CALL_ORIGINAL(rcx, weapon, info, do_effects, damage_type, custom_damage_type);
}

MAKE_HOOK(
	UTIL_ParticleTracer,
	s::UTIL_ParticleTracer.get(),
	void,
	const char *tracer_effect_name, const Vector &start_pos, const Vector &end_pos, int ent_index, int attachment_index, bool whiz_effects)
{
	if (!cfg::tracers_active || !shot_is_from_local) {
		return CALL_ORIGINAL(tracer_effect_name, start_pos, end_pos, ent_index, attachment_index, whiz_effects);
	}

	if (!show_tracers) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return;
	}

	if (cfg::tracers_type == 0)
	{
		Color color{ cfg::tracers_color };

		if (cfg::tracers_mode == 1) {
			color = utils::rainbow(i::global_vars->curtime * cfg::tracers_rainbow_speed, 255);
		}

		vis_utils->fireBeam(start_pos, end_pos, color, cfg::tracers_fade);
	}

	else
	{
		auto getTracerName = [&]() -> const char *
		{
			const int team{ local->m_iTeamNum() };

			switch (cfg::tracers_type)
			{
				case 1: {
					return "dxhr_sniper_rail"; // white machina
				}

				case 2: {
					return team == TF_TEAM_RED ? "dxhr_sniper_rail_red" : "dxhr_sniper_rail_blue"; // red/blue
				}

				case 3: {
					return team == TF_TEAM_RED ? "bullet_tracer_raygun_red" : "bullet_tracer_raygun_blue"; // red/blue
				}

				default: {
					break;
				}
			}

			return nullptr;
		};

		const char *const tracer_name{ getTracerName() };

		if (!tracer_name) {
			return;
		}

		CALL_ORIGINAL(tracer_name, start_pos, end_pos, ent_index, attachment_index, whiz_effects);
	}
}

MAKE_HOOK(
	WeaponID_IsSniperRifle,
	s::WeaponID_IsSniperRifle.get(),
	bool,
	int weaponID)
{
	if (cfg::tracers_active && shot_is_from_local) {
		return false;
	}

	return CALL_ORIGINAL(weaponID);
}

MAKE_HOOK(
	CTFPartyClient_GetPartyMemberColor,
	s::CTFPartyClient_GetPartyMemberColor.get(),
	Color *,
	void *rcx, int index)
{
	Color *const result{ CALL_ORIGINAL(rcx, index) };

	if (cfg::misc_rainbow_party && result) {
		*result = utils::rainbow((i::global_vars->curtime * 2.0f) + index * 0.5f, 255);
	}

	return result;
}

MAKE_HOOK(
	CPrediction_RunSimulation,
	s::CPrediction_RunSimulation.get(),
	void,
	CPrediction *rcx, int current_command, float curtime, CUserCmd *cmd, C_BasePlayer *player)
{
	engine_pred->current_prediction_cmd = current_command;

	if (cmd->tick_count >= INT_MAX 
		&& i::client_state->last_command_ack < cmd->command_number) {
		return;
	}

	for (const auto &tick_shifts : tick_base->m_tick_fixes)
	{
		// only place we can do tickbase fix since our command numbers will get screwed up by crithack and we currently dont
		// restore them after we send the cmd off to the server
		if (tick_shifts.command_number == current_command) {
			player->m_nTickBase() = tick_shifts.tickbase;
			curtime = tf_utils::ticksToTime(player->m_nTickBase());
			break;
		}
	}

	CALL_ORIGINAL(rcx, current_command, curtime, cmd, player);

	anims->onRunSimulation(player, cmd, curtime);

	if (current_command == i::client_state->lastoutgoingcommand) {
		anti_aim->m_last_sent_origin = player->m_vecOrigin();
	}

	if (current_command == networking->getLatestCommandNumber() - 1 && anims->m_allow_local_bone_setup)
	{
		anims->m_bone_origin = player->m_vecOrigin();
		lrm->allowBoneSetup(true);
		player->InvalidateBoneCache();
		player->m_fEffects() |= 8; //ef_nointerp
		player->SetupBones(anims->m_local_bones.data(), 128, BONE_USED_BY_ANYTHING, i::global_vars->curtime);
		player->m_fEffects() &= ~8;
		lrm->allowBoneSetup(false);
		anims->m_allow_local_bone_setup = false;
	}
}

MAKE_HOOK(
	C_BaseAnimating_MaintainSequenceTransitions,
	s::C_BaseAnimating_MaintainSequenceTransitions.get(),
	void,
	void* rcx, void* bone_setup, float cycle, vec3 *pos, Vector4D *q)
{
	return;
}

MAKE_HOOK(
	C_BaseAnimating_SetupBones,
	s::C_BaseAnimating_SetupBones.get(),
	bool,
	void *rcx, matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	C_BaseAnimating *const animating{ reinterpret_cast<C_BaseAnimating *>(reinterpret_cast<uintptr_t>(rcx) - 0x8) };

	if (!animating) {
		return CALL_ORIGINAL(rcx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
	}

	C_BaseEntity *const parent{ animating->GetRootMoveParent() };

	if (animating->GetClassId() != class_ids::CTFPlayer || (parent && parent->GetClassId() != class_ids::CTFPlayer)) {
		return CALL_ORIGINAL(rcx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
	}

	if (!lrm->isBoneSetupAllowed())
	{
		if (pBoneToWorldOut) 
		{
			if (nMaxBones >= animating->m_CachedBoneData().Count()) {
				memcpy(pBoneToWorldOut, animating->m_CachedBoneData().Base(), sizeof(matrix3x4_t) * animating->m_CachedBoneData().Count());
			}

			else {
				return false;
			}
		}

		return true;
	}

	return CALL_ORIGINAL(rcx, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}

//MAKE_HOOK(
//	CPrediction_RunCommand,
//	s::CPrediction_RunCommand.get(),
//	void,
//	void *rcx, C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *moveHelper)
//{
//	CALL_ORIGINAL(rcx, player, ucmd, moveHelper);
//}

MAKE_HOOK(
	C_BaseEntity_PostNetworkDataUpdate,
	s::C_BaseEntity_PostNetworkDataReceived.get(),
	bool,
	C_BaseEntity *rcx, int commands_acknowledged)
{
	if (commands_acknowledged <= 0 || rcx->entindex() != i::engine->GetLocalPlayer()) {
		return CALL_ORIGINAL(rcx, commands_acknowledged);
	}

	C_BasePlayer *player{ rcx->cast<C_BasePlayer>() };

	if (!player || player->deadflag()) {
		return CALL_ORIGINAL(rcx, commands_acknowledged);
	}

	compression_handler->onPostNetworkDataReceived(player, commands_acknowledged - 1);

	return CALL_ORIGINAL(rcx, commands_acknowledged);
}

MAKE_HOOK(
	ISteamNetworkingUtils_GetDirectPingToPOP,
	[&]() {
		void *const networking_utils{ s::SteamInternal_FindOrCreateUserInterface.call<void *>(0, "SteamNetworkingUtils004") };

		return mem::findVirtual(networking_utils, 9).get();
	}(),
	int,
	void *rcx, unsigned int pop_id)
{
	if (const int ping{ misc->getRegionPing(pop_id) }) {
		return ping;
	}

	return CALL_ORIGINAL(rcx, pop_id);
}

MAKE_HOOK(
	CTFPartyClient_RequestQueueForMatch,
	s::CTFPartyClient_RequestQueueForMatch.get(),
	void,
	void *rcx, EMatchGroup match_group)
{
	// mfed: updates ping before you queue so you don't have to wait for the ping to update naturally
	constexpr size_t m_bPendingPingRefresh{ 1116 };
	constexpr size_t m_rtLastPingFix{ 1112 };

	const uintptr_t gc_client_system{ s::GTFGCClientSystem.call<uintptr_t>() };

	bool *const m_bPendingPingRefresh_ptr{ reinterpret_cast<bool *>(gc_client_system + m_bPendingPingRefresh) };
	uint32_t *const m_rtLastPingFix_ptr{ reinterpret_cast<uint32_t *>(gc_client_system + m_rtLastPingFix) };

	if (m_bPendingPingRefresh_ptr && m_rtLastPingFix_ptr) {
		*m_bPendingPingRefresh_ptr = true;
		*m_rtLastPingFix_ptr = 0;
	}

	s::CTFGCClientSystem_PingThink.call<void>(gc_client_system);

	if (IGameEvent *const event{ i::event_manager->CreateNewEvent("ping_updated", false) }) {
		i::event_manager->FireEventClientSide(event);
	}

	CALL_ORIGINAL(rcx, match_group);
}

MAKE_HOOK(
	CSniperDot_ClientThink,
	s::CSniperDot_ClientThink.get(),
	void, void *rcx)
{
	if (cfg::sniper_sight_lines != 1) {
		return CALL_ORIGINAL(rcx);
	}

	C_TFGameRulesProxy *const game_rules{ ec->getGameRules() };

	// mfed: if game rules aint there this will crash on original anyway
	if (!game_rules) {
		return;
	}

	C_BaseEntity *const dot_ent{ reinterpret_cast<C_BaseEntity *>(reinterpret_cast<uintptr_t>(rcx) - 24) };

	if (!dot_ent) {
		return;
	}

	C_TFPlayer *const owner{ dot_ent->m_hOwnerEntity().Get()->cast<C_TFPlayer>() };

	if (!owner) {
		return;
	}

	if (owner->m_iClass() != TF_CLASS_SNIPER) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return;
	}

	constexpr size_t m_laserBeamEffect{ 1984 }; // literally 1984.. jorjorwell real?
	const uintptr_t laser_beam_effect{ *reinterpret_cast<uintptr_t *>(reinterpret_cast<uintptr_t>(rcx) + m_laserBeamEffect) };

	if (owner == local
		|| owner->deadflag()
		|| (owner == local->m_hObserverTarget().Get()->cast<C_TFPlayer>() && local->m_iObserverMode() == OBS_MODE_IN_EYE)) {
		return;
	}

	const bool old_pve_mode{ (bool)game_rules->m_bPlayingMannVsMachine() };
	const int old_team_num{ dot_ent->m_iTeamNum() };

	game_rules->m_bPlayingMannVsMachine() = true;
	dot_ent->m_iTeamNum() = TF_TEAM_PVE_INVADERS;

	CALL_ORIGINAL(rcx);

	game_rules->m_bPlayingMannVsMachine() = old_pve_mode;
	dot_ent->m_iTeamNum() = old_team_num;

	if (laser_beam_effect) {
		const vec3 origin{ owner->m_vecOrigin() + vec3{ 0.0f, 0.0f, ((owner->m_fFlags() & FL_DUCKING) ? 45.0f : 75.0f) * owner->m_flModelScale() }};
		s::CNewParticleEffect_SetControlPoint.call<void>(laser_beam_effect, 1, origin);
		const Color color{ vis_utils->getEntColor(owner) };
		s::CNewParticleEffect_SetControlPoint.call<void>(laser_beam_effect, 2, vec3{ (float)color.r, (float)color.g, (float)color.b });
	}
}

MAKE_HOOK(
	CMultiPlayerAnimState_ComputePoseParam_AimYaw,
	s::CMultiPlayerAnimState_ComputePoseParam_AimYaw.get(),
	void,
	void *rcx, CStudioHdr *pStudioHdr)
{
	CALL_ORIGINAL(rcx, pStudioHdr);

//#ifndef CLIENT_DLL
//	QAngle angle = GetBasePlayer()->GetAbsAngles();
//	angle[YAW] = m_flCurrentFeetYaw;
//
//	GetBasePlayer()->SetAbsAngles(angle);
//#endif

	CMultiPlayerAnimState *const anim_state{ reinterpret_cast<CMultiPlayerAnimState *>(rcx) };

	if (!anim_state) {
		return;
	}

	C_BasePlayer *const pl{ anim_state->m_pPlayer };

	if (!pl) {
		return;
	}

	QAngle angle{ pl->GetAbsAngles() };

	angle.y = anim_state->m_flCurrentFeetYaw;

	pl->SetAbsAngles(angle);
}

MAKE_HOOK(
	C_BaseEntity_Teleported,
	s::C_BaseEntity_Teleported.get(),
	bool,
	C_BaseEntity* rcx)
{
	// if we have ik active we should force its targets to clear always
	if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::C_BaseAnimating_SetupBones_C_BaseEntity_Teleported_Call.get()) {
		return true;
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	ClientModeTFNormal_BIsFriendOrPartyMember,
	s::ClientModeTFNormal_BIsFriendOrPartyMember.get(),
	bool,
	void *rcx, C_BaseEntity *entity)
{
	// allows you to inspect teammates who aren't queued with you - can finally kick those knockback rage heavies like herald11 desires
	if (cfg::allow_mvm_inspect 
		&& reinterpret_cast<uintptr_t>(_ReturnAddress()) 
		== s::CHudInspectPanel_UserCmd_InspectTarget_ClientModeTFNormal_BIsFriendOrPartyMember_Call.get()) {
		return true;
	}

	return CALL_ORIGINAL(rcx, entity);
}

MAKE_HOOK(
	CClientState_GetTime,
	s::CClientState_GetTime.get(),
	float,
	void *rcx)
{
	if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::CL_FireEvents_CClientState_GetTime_call.get()) {
		return FLT_MAX;
	}
	
	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	C_TFWeaponBase_GetShootSound,
	s::C_TFWeaponBase_GetShootSound.get(),
	const char *,
	void *rcx, int iIndex)
{
	if (cfg::misc_giant_weapon_sounds)
	{
		if (C_TFWeaponBase *const weapon{ reinterpret_cast<C_TFWeaponBase *>(rcx) })
		{
			C_BaseEntity *const weapon_owner{ weapon->m_hOwnerEntity().Get() };

			if (weapon_owner && weapon_owner == ec->getLocal())
			{
				const int og_team_num{ weapon->m_iTeamNum() };
				weapon->m_iTeamNum() = TF_TEAM_PVE_INVADERS_GIANTS;
				const char *const result{ CALL_ORIGINAL(rcx, iIndex) };
				weapon->m_iTeamNum() = og_team_num;

				switch (HASH_RT(result))
				{
					case HASH_CT("Weapon_FlameThrower.Fire"): {
						return "MVM.GiantPyro_FlameStart";
					}

					case HASH_CT("Weapon_FlameThrower.FireLoop"): {
						return "MVM.GiantPyro_FlameLoop";
					}

					case HASH_CT("Weapon_GrenadeLauncher.Single"): {
						return "MVM.GiantDemoman_GrenadeShoot";
					}

					default: {
						break;
					}
				}

				return result;
			}
		}
	}

	return CALL_ORIGINAL(rcx, iIndex);
}

MAKE_HOOK(
	CTFGameRules_ModifySentChat,
	s::CTFGameRules_ModifySentChat.get(),
	void,
	void *rcx, char *pBuf, int iBufSize)
{
	if (cfg::misc_medieval_chat)
	{
		C_TFGameRulesProxy *const game_rules{ ec->getGameRules() };

		if (!game_rules) {
			return CALL_ORIGINAL(rcx, pBuf, iBufSize);
		}

		const bool old_auto_rp{ cvars::tf_medieval_autorp->GetBool() };
		const bool old_english{ cvars::english->GetBool() };
		const bool old_playing_medieval{ static_cast<bool>(game_rules->m_bPlayingMedieval()) };

		cvars::tf_medieval_autorp->SetValue(true);
		cvars::english->SetValue(true);

		game_rules->m_bPlayingMedieval() = true;

		CALL_ORIGINAL(rcx, pBuf, iBufSize);

		cvars::tf_medieval_autorp->SetValue(old_auto_rp);
		cvars::english->SetValue(old_english);

		game_rules->m_bPlayingMedieval() = old_playing_medieval;

		return;
	}

	CALL_ORIGINAL(rcx, pBuf, iBufSize);
}

MAKE_HOOK(
	IVEngineClient_ClientCmd_Unrestricted,
	mem::findVirtual(i::engine, 106).get(),
	void,
	void *rcx, const char *cmd)
{
	if (cfg::misc_owo_chat)
	{
		const std::string cmd_view{ cmd };

		if (cmd_view.rfind("say", 0) != 0) {
			return CALL_ORIGINAL(rcx, cmd);
		}

		std::smatch match{};

		std::regex_match(cmd_view, match, std::regex{ "(say.*)\"(.*)\"" });

		if (match.size() == 3)
		{
			const std::string owo{ match[2].str() };
			const std::wstring w_owo{ owoify::Owoifier::owoify(owo, static_cast<owoify::Owoifier::OwoifyLevel>(cfg::misc_owo_chat - 1)) };
			const std::string new_owo { utils::wideToUtf8(w_owo) };
			const std::string new_cmd{ match[1].str() + "\"" + new_owo + "\"" };

			return CALL_ORIGINAL(rcx, new_cmd.c_str());
		}
	}

	CALL_ORIGINAL(rcx, cmd);
}

MAKE_HOOK(
	IVEngineClient_IsPaused,
	mem::findVirtual(i::engine, 84).get(),
	bool,
	void *rcx)
{
	if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::IVEngineClient_IsPaused_retaddr.get()) {
		return true;
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	IVEngineClient_GetScreenAspectRatio,
	mem::findVirtual(i::engine, 95).get(),
	float,
	void *rcx)
{
	if (cfg::aspect_ratio_active) {
		return cfg::aspect_ratio_value;
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CNetChan_SendNetMsg,
	s::CNetChan_SendNetMsg.get(),
	bool,
	CNetChan* rcx, INetMessage* msg, bool reliable, bool voice)
{
	if (msg->GetGroup() == INetChannelInfo::VOICE) {
		voice = true;
	}

	if (msg->GetType() == clc_FileCRCCheck)
	{
		if (cfg::misc_sv_pure_bypass) {
			return true;
		}
	}

	return CALL_ORIGINAL(rcx, msg, reliable, voice);
}

MAKE_HOOK(
	CPrediction_FinishMove,
	mem::findVirtual(i::pred, 19).get(),
	void,
	CPrediction *rcx, C_BasePlayer *player, CUserCmd *cmd, CMoveData *move)
{
	// Convert final pitch to body pitch
	float pitch{ move->m_vecAngles.x };

	if (pitch > 180.0f) {
		pitch -= 360.0f;
	}

	pitch = std::clamp(pitch, -90.0f, 90.0f);

	move->m_vecAngles.x = pitch;

	player->SetLocalAngles(move->m_vecAngles);

	CALL_ORIGINAL(rcx, player, cmd, move);
}

MAKE_HOOK(
	CTFPlayerInventory_GetMaxItemCount,
	s::CTFPlayerInventory_GetMaxItemCount.get(),
	int,
	void *rcx)
{
	if (cfg::misc_expanded_inventory) {
		return 3000; // 60 * 50 - max it will even show in hud i think
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CTFGCClientSystem_UpdateAssignedLobby,
	s::CTFGCClientSystem_UpdateAssignedLobby.get(),
	char,
	void *rcx)
{
	if (rcx && cfg::misc_bypass_f2p_chat) {
		const int32_t offset{ *s::GTFGCClientSystem_NonPremiumAccount_Offset.cast<int32_t *>() };
		*reinterpret_cast<char *>(reinterpret_cast<uintptr_t>(rcx) + offset) = 0;
	}

	return CALL_ORIGINAL(rcx);
}

static vec3 new_view{};

MAKE_HOOK(
	MainViewForward,
	s::MainViewForward.get(),
	vec3 *)
{
	if (cfg::misc_damage_feedback_show_through_walls
		&& reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::CDamageAccountPanel_DisplayDamageFeedback_MainViewForward_Call.get()) {
		return &new_view;
	}

	return CALL_ORIGINAL();
}

MAKE_HOOK(
	CDamageAccountPanel_DisplayDamageFeedback,
	s::CDamageAccountPanel_DisplayDamageFeedback.get(),
	void,
	void *rcx, C_TFPlayer *attacker, C_BaseCombatCharacter *victim, int damage, int health, bool crit)
{
	if (!cfg::misc_damage_feedback_active) {
		return CALL_ORIGINAL(rcx, attacker, victim, damage, health, crit);
	}
	
	if (!attacker || !victim) {
		return CALL_ORIGINAL(rcx, attacker, victim, damage, health, crit);
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return CALL_ORIGINAL(rcx, attacker, victim, damage, health, crit);
	}

	if (cfg::misc_damage_feedback_show_through_walls) {
		new_view = victim->GetRenderCenter(); // trace shouldn't fail when we do this i hope
	}

	int old_class{ -1 };

	if (cfg::misc_damage_feedback_show_invisible_spies && victim->GetClassId() == class_ids::CTFPlayer) {
		old_class = victim->cast<C_TFPlayer>()->m_iClass();
		victim->cast<C_TFPlayer>()->m_iClass() = TF_CLASS_SCOUT;
	}

	CALL_ORIGINAL(rcx, attacker, victim, damage, health, crit);

	if (old_class != -1) {
		victim->cast<C_TFPlayer>()->m_iClass() = old_class;
	}
}

MAKE_HOOK(
	C_BaseEntity_PreEntityPacketReceived,
	s::C_BaseEntity_PreEntityPacketReceived.get(),
	void,
	C_BaseEntity* rcx, int commands_acknowledged)
{
	if (rcx->entindex() != i::engine->GetLocalPlayer()) {
		return CALL_ORIGINAL(rcx, commands_acknowledged);
	}

	C_BasePlayer *const player{ rcx->cast<C_BasePlayer>() };
	
	compression_handler->onPreEntityPacketReceived(player, commands_acknowledged);
	
	return CALL_ORIGINAL(rcx, commands_acknowledged);
}

MAKE_HOOK(
	CPrediction_Update2,
	mem::findVirtual(i::pred, 21).get(),
	void,
	CPrediction *rcx, bool received_new_world_update, bool validframe, int incoming_acknowledged, int outgoing_command)
{
	return CALL_ORIGINAL(rcx, true, validframe, incoming_acknowledged, outgoing_command);
}

MAKE_HOOK(
	C_BaseEntity_InitPredictable,
	s::C_BaseEntity_InitPredictable.get(),
	void,
	C_BaseEntity *rcx)
{
	//s: god is dead
	if (C_TFPlayer *const local{ ec->getLocal() })
	{
		if (local->InCond(TF_COND_DISGUISED)) {
			return;
		}
	}

	{
		i::mdl_cache->BeginLock();

		datamap_mod->iterateDatamaps(rcx->GetPredDescMap());
		datamap_mod->m_datamap_init = true;

		i::mdl_cache->EndLock();
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CPrediction_IsFirstTimePredicted,
	mem::findVirtual(i::pred, 15).get(),
	bool,
	CPrediction *rcx)
{
	// force m_nCritChecks to constantly increment as it is now in the datamap
	if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::CPrediction_IsFirstTimePredicted_retaddr.get()) {
		return true;
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	C_BaseEntity_DestroyIntermediateData,
	s::C_BaseEntity_DestroyIntermediateData.get(),
	void,
	C_BaseEntity* rcx)
{
	datamap_mod->restoreOldDataDesc(rcx->GetPredDescMap());

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CNetChan_ProcessPacket,
	s::CNetChan_ProcessPacket.get(),
	void,
	CNetChan *rcx, uint8_t *packet, bool header)
{
	CALL_ORIGINAL(rcx, packet, header);

	i::engine->FireEvents();
}

MAKE_HOOK(
	C_TFBaseRocket_DrawModel,
	s::C_TFBaseRocket_DrawModel.get(),
	int,
	C_TFBaseRocket *rcx, int flags)
{
	if (rcx)
	{
		float &spawn_time{ rcx->m_flSpawnTime() };

		if (i::global_vars->curtime - spawn_time < 0.2f) {
			spawn_time = i::global_vars->curtime - 0.2f;
		}
	}

	return CALL_ORIGINAL(rcx, flags);
}

// mfed: assuming this is what normie had in mind
MAKE_HOOK(
	C_TFWeaponBase_CalcIsAttackCritical,
	s::C_TFWeaponBase_CalcIsAttackCritical.get(),
	void,
	void *rcx)
{
	C_TFWeaponBase *const weapon{ reinterpret_cast<C_TFWeaponBase *>(rcx) };

	if (!weapon) {
		return CALL_ORIGINAL(rcx);
	}

	C_TFPlayer *const player{ weapon->m_hOwner().Get()->cast<C_TFPlayer>() };

	if (!player) {
		return CALL_ORIGINAL(rcx);
	}

	CUserCmd *const cmd{ player->m_pCurrentCommand() };

	if (!cmd) {
		return CALL_ORIGINAL(rcx);
	}

	const int old_framecount{ i::global_vars->framecount };

	i::global_vars->framecount = cmd->command_number;

	CALL_ORIGINAL(rcx);

	i::global_vars->framecount = old_framecount;
}

//MAKE_HOOK(
//	C_TEFireBullets_PostDataUpdate,
//	s::C_TEFireBullets_PostDataUpdate.get(),
//	void,
//	C_TEFireBullets *rcx, DataUpdateType_t update)
//{
//	CALL_ORIGINAL(rcx, update);
//
//	//seed_pred->onPostDataUpdate(rcx);
//}

MAKE_HOOK(
	CMaterialSystem_FindTexture,
	mem::findVirtual(i::mat_sys, 81).get(),
	ITexture *,
	IMaterialSystem *rcx, char const *pTextureName, const char *pTextureGroupName, bool complain, int nAdditionalCreationFlags)
{
	ITexture *result{ CALL_ORIGINAL(rcx, pTextureName, pTextureGroupName, complain, nAdditionalCreationFlags) };

	if (cfg::flat_textures
		&& result
		&& !result->IsTranslucent()
		&& pTextureGroupName
		&& HASH_RT(pTextureGroupName) == HASH_CT(TEXTURE_GROUP_WORLD)
		&& std::string_view{ pTextureName }.find("water") == std::string_view::npos)
	{
		vec3 clr{};

		result->GetLowResColorSample(0.5f, 0.5f, &clr.x);

		unsigned char bits[4]
		{
			static_cast<unsigned char>(clr.x * 255.0f),
			static_cast<unsigned char>(clr.y * 255.0f),
			static_cast<unsigned char>(clr.z * 255.0f),
			255
		};

		result = i::mat_sys->CreateTextureFromBits(1, 1, 1, IMAGE_FORMAT_RGBA8888, 4, bits);
	}

	return result;
}

MAKE_HOOK(CInput_ValidateUserCmd,
	s::CInput_ValidateUserCmd.get(),
	void,
	IInput *rcx, CUserCmd *cmd, int sequence_number)
{
	return;
}

MAKE_HOOK(
	C_WeaponMedigun_WeaponIdle,
	s::C_WeaponMedigun_WeaponIdle.get(),
	void,
	C_WeaponMedigun *const rcx)
{
	if (!rcx) {
		CALL_ORIGINAL(rcx);
		return;
	}

	C_TFPlayer *const owner{ rcx->m_hOwnerEntity()->cast<C_TFPlayer>() };

	if (!owner || owner->entindex() != i::engine->GetLocalPlayer()) {
		CALL_ORIGINAL(rcx);
		return;
	}

	if ((owner->m_afButtonPressed() & IN_RELOAD) && rcx->GetMedigunType() == MEDIGUN_RESIST && !rcx->IsReleasingCharge()) {
		rcx->m_nChargeResistType() += 1;
		rcx->m_nChargeResistType() = rcx->m_nChargeResistType() % MEDIGUN_NUM_RESISTS;
	}

	CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CDirect3DDevice9_Reset,
	mem::findVirtual(i::dxdev, 16).get(),
	HRESULT,
	IDirect3DDevice9 *rcx, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const HRESULT result{ CALL_ORIGINAL(rcx, pPresentationParameters) };

	ImGui_ImplDX9_CreateDeviceObjects();

	return result;
}

MAKE_HOOK(
	CDirect3DDevice9_EndScene,
	mem::findVirtual(i::dxdev, 42).get(),
	HRESULT,
	IDirect3DDevice9 *const rcx)
{
	static void *const return_address{ _ReturnAddress() };

	if (_ReturnAddress() != return_address) {
		return CALL_ORIGINAL(rcx);
	}

	input_dx->processStart();

	ui->start();
	{
		menu->run();
		binds->run();
		mini_map->run();
	}
	ui->end();

	input_dx->processEnd();

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	CHudCrosshair_GetDrawPosition,
	s::CHudCrosshair_GetDrawPosition.get(),
	void,
	float *pX, float *pY, bool *pbBehindCamera, vec3 *angleCrosshairOffset)
{
	if (cfg::crosshair_follows_aimbot)
	{
		C_TFPlayer *const local{ ec->getLocal() };

		if (local && !local->deadflag() && tf_globals::aimbot_target.IsValid())
		{
			vec3 forward{};
			math::angleVectors(tf_globals::last_cmd.viewangles, &forward);

			const vec3 start{ local->GetEyePos() };
			const vec3 end{ start + forward * 8192.0f };

			trace_t tr{};
			tf_utils::trace(start, end, &trace_filters::world, &tr, MASK_SHOT);

			vec2 screen{};

			if (draw->worldToScreen(tr.endpos, screen))
			{
				if (pX && pY && pbBehindCamera)
				{
					*pX = screen.x;
					*pY = screen.y;

					*pbBehindCamera = false;

					return;
				}
			}
		}
	}

	CALL_ORIGINAL(pX, pY, pbBehindCamera, angleCrosshairOffset);
}

MAKE_HOOK(
	C_TFPlayer_OnNewModel,
	s::C_TFPlayer_OnNewModel.get(),
	CStudioHdr *,
	C_TFPlayer *rcx)
{
	if (rcx->m_bPredictable()) 
	{
		if (anims->m_fake_animstate) {
			anims->m_fake_animstate->ClearAnimationState();
			anims->m_fake_animstate->m_PoseParameterData = {};
			anims->m_fake_animstate->m_bPoseParameterInit = false;
		}
	}

	return CALL_ORIGINAL(rcx);
}

MAKE_HOOK(
	R_LoadNamedSkys,
	s::R_LoadNamedSkys.get(),
	bool,
	const char *name)
{
	if (!cfg::skybox_override.empty())
	{
		if (CALL_ORIGINAL(cfg::skybox_override.c_str())) {
			return true;
		}
	}

	return CALL_ORIGINAL(name);
}

MAKE_HOOK(
	C_BasePlayer_CalcDeathCamView,
	s::C_BasePlayer_CalcDeathCamView.get(),
	void,
	void *rcx, Vector &eyeOrigin, QAngle &eyeAngles, float &fov)
{
	CALL_ORIGINAL(rcx, eyeOrigin, eyeAngles, fov);

	if (cfg::fov_override_active) {
		fov = cfg::fov_override_value;
	}
}

MAKE_HOOK(
	C_BasePlayer_CalcFreezeCamView,
	s::C_BasePlayer_CalcFreezeCamView.get(),
	void,
	void *rcx, Vector &eyeOrigin, QAngle &eyeAngles, float &fov)
{
	CALL_ORIGINAL(rcx, eyeOrigin, eyeAngles, fov);

	if (cfg::fov_override_active) {
		fov = cfg::fov_override_value;
	}
}

MAKE_HOOK(
	C_BaseEntity_SetAbsVelocity,
	s::C_BaseEntity_SetAbsVelocity.get(),
	void,
	C_BaseEntity *rcx, const Vector &vecAbsVelocity)
{
	if (reinterpret_cast<uintptr_t>(_ReturnAddress()) == s::C_BaseEntity_SetAbsVelocity_C_BasePlayer_PostDataUpdate_call.get())
	{
		if (rcx && rcx->GetClassId() == class_ids::CTFPlayer)
		{
			if (C_TFPlayer *const pl{ rcx->cast<C_TFPlayer>() })
			{
				if (!(pl->m_fFlags() & FL_ONGROUND) && player_data->getNumRecords(pl->cast<C_TFPlayer>()) >= 1)
				{
					if (const LagRecord *const lr{ player_data->getRecord(pl->cast<C_TFPlayer>(), 0) })
					{
						if (const float dt{ pl->m_flSimulationTime() - lr->sim_time })
						{
							if ((pl->m_fFlags() & FL_DUCKING) != (lr->flags & FL_DUCKING)) {
								const float fixed_z{ lr->origin.z + (pl->m_fFlags() & FL_DUCKING) ? 20.0f : -20.0f };
								CALL_ORIGINAL(rcx, { vecAbsVelocity.x, vecAbsVelocity.y, (pl->m_vecOrigin().z - fixed_z) / dt });
								return;
							}
						}
					}
				}
			}
		}
	}

	CALL_ORIGINAL(rcx, vecAbsVelocity);
}

//================================================================================================================================

//MAKE_HOOK(
//	FX_FireBullets_client,
//	s::FX_FireBullets_client.get(),
//	void,
//	void *pWpn, int iPlayer, const Vector &vecOrigin, const QAngle &vecAngles,
//	int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
//{
//	CALL_ORIGINAL(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
//}

//MAKE_HOOK(
//	ClientModeTFNormal_DoPostScreenSpaceEffects,
//	s::ClientModeTFNormal_DoPostScreenSpaceEffects.get(),
//	bool,
//	void *rcx, const CViewSetup *pSetup)
//{
//	return CALL_ORIGINAL(rcx, pSetup);
//}

//MAKE_SIG(server_FX_FireBullets, mem::findBytes("server.dll", "48 89 5C 24 ? 4C 89 4C 24 ? 55 56 41 54"));
//
//MAKE_HOOK(
//	server_FX_FireBullets,
//	s::server_FX_FireBullets.get(),
//	void,
//	void *pWpn, int iPlayer, const Vector &vecOrigin, const QAngle &vecAngles,
//	int iWeapon, int iMode, int iSeed, float flSpread, float flDamage, bool bCritical)
//{
//	flSpread = 0.0f;
//
//	CALL_ORIGINAL(pWpn, iPlayer, vecOrigin, vecAngles, iWeapon, iMode, iSeed, flSpread, flDamage, bCritical);
//}