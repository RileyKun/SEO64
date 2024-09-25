#include "player_data.hpp"

#include "../anims/anims.hpp"
#include "../anti_aim/anti_aim.hpp"
#include "../vis_utils/vis_utils.hpp"
#include "../hitboxes/hitboxes.hpp"
#include "../engine_pred/compression_handler.hpp"
#include "../cfg.hpp"

LagRecord::LagRecord(C_TFPlayer *const pl, bool is_interp)
{
	if (!pl || pl->GetClassId() != class_ids::CTFPlayer) {
		return;
	}

	pl_ptr = pl;
	sim_time = base_sim_time = pl->m_flSimulationTime();
	tick_count = i::global_vars->tickcount;
	flags = pl->m_fFlags();
	origin = pl->m_vecOrigin();
	center = pl->GetCenter();
	vel = pl->m_vecVelocity();
	render_origin = pl->GetRenderOrigin();
	abs_origin = pl->GetAbsOrigin();
	abs_angles = pl->GetAbsAngles();
	mins = pl->m_vecMinsPreScaled();
	maxs = pl->m_vecMaxsPreScaled();
	eye_angs = pl->m_angEyeAngles();

	bones_setup = false;

	if (!bones) {
		bones = reinterpret_cast<matrix3x4_t *>(i::mem_alloc->Alloc(sizeof(matrix3x4_t) * 128));
	}

	if (bones && !is_interp)
	{
		lrm->allowBoneSetup(true);

		pl->InvalidateBoneCache();
		pl->m_fEffects() |= 8; //ef_nointerp

		bones_setup = pl->SetupBones(bones, 128, BONE_USED_BY_ANYTHING, pl->m_flSimulationTime());

		pl->m_fEffects() &= ~8;

		lrm->allowBoneSetup(false);

		for (int n{}; n < tf_utils::getHitboxCount(pl); n++)
		{
			vec3 hbox{};

			if (!tf_utils::getHitbox(pl, bones, n, hbox)) {
				continue;
			}

			hitboxes.push_back(hbox);
		}
	}
}

LagRecord::~LagRecord()
{
	if (bones) {
		i::mem_alloc->Free(bones);
		bones = nullptr;
	}
}

const bool LagRecord::isRecordValid(float add_time) const
{
	if (anti_aim->m_fakewalk_active) {
		add_time += tf_utils::ticksToTime(cfg::antiaim_fakewalk_ticks - i::client_state->chokedcommands);
	}

	if (!hasSetupBones()) {
		return false;
	}

	const float correct{ std::clamp(tf_utils::getLatency() + tf_utils::getLerp(), 0.0f, cvars::sv_maxunlag->GetFloat()) };
	const float delta_time{ std::fabs(correct - ((tf_utils::getServerTime() + tf_utils::getLatency() + add_time) - base_sim_time)) };

	return delta_time <= (0.2f - i::global_vars->interval_per_tick) && delta_time <= (0.2f + i::global_vars->interval_per_tick);
}

const bool LagRecord::isRecordDead() const
{
	C_TFPlayer *const pl{ this->pl_ptr };

	if (!pl || pl->deadflag()) {
		return true;
	}

	const float cur_time{ tf_utils::getServerTime() + tf_utils::getLatency() };

	// yes this is an int
	// blame the game coders not me
	// https://github.com/lua9520/source-engine-2018-hl2_src/blob/3bf9df6b2785fa6d951086978a3e66f49427166a/game/server/player_lagcompensation.cpp#L246
	const int dead_time{ static_cast<int>(cur_time - cvars::sv_maxunlag->GetFloat()) };

	return this->base_sim_time < dead_time;
}

const bool LagRecord::hasSetupBones() const
{
	return bones != nullptr && bones_setup;
}

void PlayerDataVars::update(C_TFPlayer *const pl)
{
	if (player != pl) {
		memset(this, 0, sizeof(PlayerDataVars));
	}

	player = pl;

	old_flags = flags;

	simtime = pl->m_flSimulationTime();
	origin = pl->m_vecOrigin();
	flags = pl->m_fFlags();
	eye_angles = pl->m_angEyeAngles();

	// straight outta C_BaseEntity::PostDataUpdate
	const bool origin_changed
	{
		!compression_handler->CloseEnough(pl->m_vecOrigin().x, pl->m_vecOldOrigin().x, DIST_EPSILON)
		|| !compression_handler->CloseEnough(pl->m_vecOrigin().y, pl->m_vecOldOrigin().y, DIST_EPSILON)
		|| !compression_handler->CloseEnough(pl->m_vecOrigin().z, pl->m_vecOldOrigin().z, DIST_EPSILON)
	};

	const bool simulation_changed{ origin_changed || !compression_handler->CloseEnough(pl->m_flSimulationTime(), pl->m_flOldSimulationTime()) };

	if (simulation_changed && pl->entindex() != i::engine->GetLocalPlayer()) {
		anims->onNetworkUpdate(pl);
	}

	// should also check for deadtime here but i heavily doubt anyone can shift tickbase that far back in normal gameplay
	float front_time{};

	if (records.size() > 0)
	{
		LagRecord *const front{ records[0].get() };

		if (front) {
			front_time = front->base_sim_time;
		}
	}

	if (pl->m_flSimulationTime() > front_time) {
		records.emplace_front(std::make_shared<LagRecord>(pl));
	}

	records.erase(
		std::ranges::remove_if(records,
			[&](const std::shared_ptr<LagRecord>& record) {
			return record->isRecordDead();
			}
		).begin(), records.end());

	const int max_records{ static_cast<int>(1.0f / i::global_vars->interval_per_tick) };

	while (records.size() > max_records) {
		records.pop_back();
	}

	const float max_teleport_dist{ cvars::sv_lagcompensation_teleport_dist->GetFloat() * cvars::sv_lagcompensation_teleport_dist->GetFloat() };

	for (valid_record_size = 0; valid_record_size < records.size(); valid_record_size++) 
	{
		LagRecord *const record{ records[valid_record_size].get() };

		if (!record) {
			continue;
		}

		if (vec3(pl->m_vecOrigin() - record->origin).length() > max_teleport_dist) {
			break;
		}
	}
}

PlayerDataVars::InterpHelper::InterpHelper(const LagRecord *record)
{
	simtime = record->sim_time;
	origin = record->origin;
	center = record->center;
	render_origin = record->render_origin;
	abs_angles = record->abs_angles;
	mins = record->mins;
	maxs = record->maxs;
	velocity = record->vel;
}

void PlayerDataVars::addInterpolatedRecord()
{
	C_TFPlayer *const pl{ player };

	if (!pl
		|| pl->deadflag()
		|| pl->GetClassId() != class_ids::CTFPlayer
		|| pl->entindex() == i::engine->GetLocalPlayer()) {
		return;
	}

	// we need atleast two records to be able to do this
	if (records.size() < 2) {
		return;
	}

	const LagRecord *const first{ records[0].get() };
	const LagRecord *const second{ records[1].get()};

	if (!first || !second) {
		return;
	}

	const int tick_delta{ tf_utils::timeToTicks(first->sim_time - second->sim_time) };

	// we need the lagrecords to be apart by two ticks worth to be able to do this
	if (tick_delta < 2) {
		return;
	}

	// pointer location isnt guaranteed
	// yes this is aids but if we construct a lagrecord then the bones will be unallocated (BAD!)
	const InterpHelper first_helper{ first };
	const InterpHelper second_helper{ second };

	std::deque<std::shared_ptr<LagRecord>> &deque{ records };

	for (size_t n{ 1 }; n < tick_delta; n++)
	{
		const float frac{ static_cast<float>(n) / tick_delta };

		deque.insert(deque.begin() + 1, std::make_shared<LagRecord>(player, true));

		LagRecord *const current{ deque[1].get()};

		if (!current) {
			continue;
		}

		current->origin = current->abs_origin = math::lerp(first_helper.origin, second_helper.origin, frac);
		current->render_origin = math::lerp(first_helper.render_origin, second_helper.render_origin, frac);

		current->sim_time -= i::global_vars->interval_per_tick * n;

		current->mins = math::lerp(first_helper.mins, second_helper.mins, frac);
		current->maxs = math::lerp(first_helper.maxs, second_helper.maxs, frac);

		current->abs_angles = math::lerp(first_helper.abs_angles, second_helper.abs_angles, frac);

		current->center = math::lerp(first_helper.center, second_helper.center, frac);

		current->vel = math::lerp(first_helper.velocity, second_helper.velocity, frac);

		// normie: figure out maths to rotate angles and set origin for bones instead of forcing a setupbones call
		if (current->bones)
		{
			const vec3 og_origin{ pl->GetAbsOrigin() };
			const vec3 og_angles{ pl->GetAbsAngles() };

			pl->SetAbsOrigin(current->abs_origin);
			pl->SetAbsAngles(current->abs_angles);

			lrm->allowBoneSetup(true);

			pl->InvalidateBoneCache();
			pl->m_fEffects() |= 8; //ef_nointerp

			current->bones_setup = pl->SetupBones(current->bones, 128, BONE_USED_BY_ANYTHING, pl->m_flSimulationTime());

			pl->m_fEffects() &= ~8;

			lrm->allowBoneSetup(false);

			pl->SetAbsOrigin(og_origin);
			pl->SetAbsAngles(og_angles);

			if (current->bones_setup)
			{
				for (int n{}; n < tf_utils::getHitboxCount(pl); n++)
				{
					vec3 hbox{};

					if (!tf_utils::getHitbox(pl, current->bones, n, hbox)) {
						continue;
					}

					current->hitboxes.push_back(hbox);
				}
			}
		}

		current->interpolated = true;
	}
}

void PlayerData::update()
{
	const int max_players{ i::global_vars->maxClients };

	if (m_data.size() != max_players) {
		m_data.resize(max_players);
	}

	for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PLAYERS_ALL))
	{
		C_TFPlayer *const pl{ i::ent_list->GetClientEntityFromHandle(ehandle)->cast<C_TFPlayer>()};

		if (!pl || pl->IsDormant()) {
			continue;
		}

		PlayerDataVars *const data{ &m_data[static_cast<size_t>(pl->entindex() - 1)] };

		if (!data) {
			continue;
		}

		data->update(pl);
	}
}

const size_t PlayerData::getNumRecords(C_TFPlayer *const pl) const
{
	if (!pl) {
		return 0ull;
	}

	// space for this player hasn't been allocated yet probably
	const int max_players{ i::global_vars->maxClients };

	if (m_data.size() != max_players) {
		return 0ull;
	}

	return m_data[static_cast<size_t>(pl->entindex() - 1)].records.size();
}

const size_t PlayerData::getNumValidRecords(C_TFPlayer *const pl) const
{
	if (!pl) {
		return 0ull;
	}

	// space for this player hasn't been allocated yet probably
	const int max_players{ i::global_vars->maxClients };

	if (m_data.size() != max_players) {
		return 0ull;
	}

	return m_data[static_cast<size_t>(pl->entindex() - 1)].valid_record_size;
}

LagRecord *const PlayerData::getRecord(C_TFPlayer *const pl, const size_t record_idx) const
{
	if (!pl || record_idx >= getNumRecords(pl)) {
		return nullptr;
	}

	return m_data[static_cast<size_t>(pl->entindex() - 1)].records[record_idx].get();
}

const PlayerDataVars *const PlayerData::get(C_TFPlayer *const pl) const
{
	if (!pl) {
		return nullptr;
	}

	// space for this player hasn't been allocated yet probably
	const int max_players{ i::global_vars->maxClients };

	if (m_data.size() != max_players) {
		return 0ull;
	}

	return &m_data[static_cast<size_t>(pl->entindex() - 1)];
}

bool PlayerData::onLoad()
{
	if (!i::engine->IsInGame() || !i::engine->IsConnected()) {
		return true;
	}

	if (!m_model_mat_shaded)
	{
		KeyValues *const kv{ new KeyValues("VertexLitGeneric") }; {
			kv->SetString("$basetexture", "white");
		}

		m_model_mat_shaded = i::mat_sys->CreateMaterial("model_mat", kv);
	}

	if (!m_model_mat_flat)
	{
		KeyValues *const kv{ new KeyValues("UnlitGeneric") }; {
			kv->SetString("$basetexture", "white");
		}

		m_model_mat_flat = i::mat_sys->CreateMaterial("model_mat_flat", kv);
	}

	if (!m_model_mat_wireframe)
	{
		KeyValues *const kv{ new KeyValues("UnlitGeneric") }; {
			kv->SetString("$basetexture", "white");
			kv->SetString("$wireframe", "1");
		}

		m_model_mat_wireframe = i::mat_sys->CreateMaterial("model_mat_wireframe", kv);
	}

	if (!m_model_mat_bg)
	{
		KeyValues *const kv{ new KeyValues("UnlitGeneric") }; {
			kv->SetString("$basetexture", "white");
			kv->SetString("$alpha", "0");
		}

		m_model_mat_bg = i::mat_sys->CreateMaterial("model_mat_bg", kv);
	}

	return true;
}

bool PlayerData::onLevelShutdown()
{
	m_data.clear();

	if (m_model_mat_shaded) {
		m_model_mat_shaded->DecrementReferenceCount();
		m_model_mat_shaded->DeleteIfUnreferenced();
		m_model_mat_shaded = nullptr;
	}

	if (m_model_mat_flat) {
		m_model_mat_flat->DecrementReferenceCount();
		m_model_mat_flat->DeleteIfUnreferenced();
		m_model_mat_flat = nullptr;
	}

	if (m_model_mat_wireframe) {
		m_model_mat_wireframe->DecrementReferenceCount();
		m_model_mat_wireframe->DeleteIfUnreferenced();
		m_model_mat_wireframe = nullptr;
	}

	if (m_model_mat_bg) {
		m_model_mat_bg->DecrementReferenceCount();
		m_model_mat_bg->DeleteIfUnreferenced();
		m_model_mat_bg = nullptr;
	}

	return true;
}

bool PlayerData::onLevelInit()
{
	return onLoad();
}

bool PlayerData::onUnload()
{
	return onLevelShutdown();
}

void PlayerData::drawLagCompModel(C_TFPlayer *const pl, const LagRecord *const lr)
{
	if (!pl || !lr) {
		return;
	}

	m_drawing_models = true;
	lrm->set(pl, lr);
	pl->DrawModel(STUDIO_RENDER | STUDIO_NOSHADOWS);
	lrm->reset();
	m_drawing_models = false;
}

void PlayerData::visual()
{
	if (!cfg::lag_comp_visuals_active) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	C_TFWeaponBase *const weapon{ ec->getWeapon() };

	if (!weapon || (!tf_utils::isWeaponHitscan(weapon) && !tf_utils::isWeaponMelee(weapon))) {
		return;
	}

	IMatRenderContext *const rc{ i::mat_sys->GetRenderContext() };

	if (!rc) {
		return;
	}

	switch (cfg::lag_comp_visuals_mat)
	{
		case 0: {
			i::model_render->ForcedMaterialOverride(m_model_mat_shaded);
			break;
		}

		case 1: {
			i::model_render->ForcedMaterialOverride(m_model_mat_flat);
			break;
		}

		default: {
			break;
		}
	}

	i::render_view->SetColorModulation(cfg::lag_comp_visuals_clr);
	i::render_view->SetBlend(static_cast<float>(cfg::lag_comp_visuals_clr.a) / 255.0f);

	if (cfg::lag_comp_visuals_no_depth) {
		rc->DepthRange(0.0f, 0.2f);
	}

	const bool weapon_supports_friendly_fire{ tf_utils::attribHookValue(0.0f, "speed_buff_ally", weapon) > 0.0f };
	const bool is_using_medigun{ weapon->GetWeaponID() == TF_WEAPON_MEDIGUN };

	for (const CBaseHandle ehandle : ec->getGroup(ECGroup::PLAYERS_ALL))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

		if (!pl || pl == local || pl->deadflag() || getNumRecords(pl) <= 0) {
			continue;
		}

		const auto local_team{ local->m_iTeamNum() };
		const auto pl_team{ pl->m_iTeamNum() };

		if (!(weapon_supports_friendly_fire || is_using_medigun) && pl_team == local_team) {
			continue;
		}

		if (is_using_medigun && pl_team != local_team) {
			continue;
		}

		const int max_records{ static_cast<int>(getNumValidRecords(pl)) - 1 };

		for (int n{ max_records }; n >= 0; n--)
		{
			const LagRecord *const lr{ getRecord(pl, n) };

			if (!lr
				|| !vis_utils->isPosOnScreen(local, lr->render_origin)
				|| lr->hitboxes.empty()
				|| !lr->isRecordValid()) {
				continue;
			}

			if (lr->interpolated && !cfg::aimbot_fakelag_fix) {
				continue;
			}

			vec3 head{};

			if (!tf_utils::getHitbox(pl, pl->m_CachedBoneData().Base(), HITBOX_HEAD, head) || head.distToSqr(lr->hitboxes[0]) < (2.0f * 2.0f)) {
				continue;
			}

			if (cfg::lag_comp_visuals_mat == 3) {
				hitboxes->renderHitboxesLR(lr);
			}

			else
			{
				if (cfg::lag_comp_visuals_mat == 2) {
					i::model_render->ForcedMaterialOverride(m_model_mat_wireframe);
					i::render_view->SetColorModulation(cfg::lag_comp_visuals_clr);
					rc->CullMode(MATERIAL_CULLMODE_CW);
					drawLagCompModel(pl, lr);
					rc->CullMode(MATERIAL_CULLMODE_CCW);
					i::model_render->ForcedMaterialOverride(m_model_mat_bg); // mfed: TODO: find some way to make this material invisible but not really invisible?
					i::render_view->SetColorModulation({ 0, 0, 0, 255 });
				}

				drawLagCompModel(pl, lr);
			}

			if (cfg::lag_comp_visuals_last_only) {
				break;
			}
		}
	}

	i::model_render->ForcedMaterialOverride(nullptr);

	i::render_view->SetColorModulation({ 255, 255, 255, 255 });
	i::render_view->SetBlend(1.0f);

	if (cfg::lag_comp_visuals_no_depth) {
		rc->DepthRange(0.0f, 1.0f);
	}
}

bool PlayerData::isDrawingModels() const
{
	return m_drawing_models;
}