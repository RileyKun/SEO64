#include "engine_pred.hpp"

#include "../networking/networking.hpp"
#include "../cfg.hpp"

void EnginePred::update()
{
	if (i::client_state->m_nDeltaTick <= 0) {
		return;
	}

	i::pred->Update
	(
		i::client_state->m_nDeltaTick,
		true,
		i::client_state->last_command_ack,
		networking->getLatestCommandNumber() - 1
	);
}

void EnginePred::suppressEvents(C_BaseEntity *ent)
{
	IPredictionSystem *system{ i::pred_system->g_pPredictionSystems };
	while (system) {
		system->m_bSuppressEvent = ent != nullptr;
		system->m_pSuppressHost = ent;
		
		system->m_nStatusPushed = std::min(0, system->m_nStatusPushed);

		system = system->m_pNextSystem;
	}
}

bool EnginePred::start(CUserCmd *const cmd)
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag() || !cmd || i::client_state->m_nDeltaTick <= 0) {
		return false;
	}

	datamap_t *const map{ local->GetPredDescMap() };

	if (!map) {
		return false;
	}

	if (!m_local_copy.data) {
		m_local_copy.data = reinterpret_cast<uint8_t*>(i::mem_alloc->Alloc(local->GetIntermediateDataSize()));
		m_local_copy.size = local->GetIntermediateDataSize();
	}

	else if (m_local_copy.data && m_local_copy.size != local->GetIntermediateDataSize()) {
		m_local_copy.data = reinterpret_cast<uint8_t *>(i::mem_alloc->Realloc(m_local_copy.data, local->GetIntermediateDataSize()));
		m_local_copy.size = local->GetIntermediateDataSize();
	}

	CPredictionCopy copy{ PC_EVERYTHING, m_local_copy.data, PC_DATA_PACKED, local, PC_DATA_NORMAL };
	copy.TransferData("save_engine_pred", local->entindex(), map);

	m_og_curtime = i::global_vars->curtime;
	m_og_frametime = i::global_vars->frametime;
	m_og_in_prediction = i::pred->m_bInPrediction;
	m_og_first_prediction = i::pred->m_bFirstTimePredicted;

	i::pred->m_bInPrediction = true;
	i::pred->m_bFirstTimePredicted = true;

	runCommand(local, cmd);

	return true;
}

void EnginePred::runCommand(C_TFPlayer *const local, CUserCmd *const cmd)
{
	suppressEvents(local);

	const int og_tickbase{ local->m_nTickBase() };

	//disable all calls to post think since we only want to predict up to our bullet firing which the call to post think will screw up
	m_in_engine_pred = true;

	//calling runcommand is better than rebuilding it as theres only two things that dont need to be called for engine pred
	//plus weapon selection rtti is a bitch and i rather not rebuild that
	i::pred->RunCommand(local, cmd, i::move_helper);

	m_in_engine_pred = false;

	local->m_nTickBase() = og_tickbase;

	suppressEvents(nullptr);
}

void EnginePred::end() const
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	datamap_t *const map{ local->GetPredDescMap() };

	if (!map) {
		return;
	}

	i::global_vars->curtime = m_og_curtime;
	i::global_vars->frametime = m_og_frametime;

	i::pred->m_bInPrediction = m_og_in_prediction;
	i::pred->m_bFirstTimePredicted = m_og_first_prediction;

	//restore all entity states to the last predicted cmd
	CPredictionCopy copy{ PC_EVERYTHING, local, PC_DATA_NORMAL, m_local_copy.data, PC_DATA_PACKED };
	copy.TransferData("restore_engine_pred", local->entindex(), map);
}

void EnginePred::restoreEntityToPredictedFrame(int pred_frame) const
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || cvars::cl_predict->GetInt() == 0) {
		return;
	}

	for (int n{}; n < i::predictables->GetPredictableCount(); n++)
	{
		C_BaseEntity *const ent{ i::predictables->GetPredictable(n) };

		if (!ent || !ent->m_bPredictable()) {
			continue;
		}

		ent->RestoreData("RestoreEntityToPredictedFrame", pred_frame, PC_EVERYTHING);
	}
}

void EnginePred::edgeJump(CUserCmd *const cmd)
{
	if (!cfg::auto_edge_jump_active) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local
		|| local->deadflag()
		|| !cmd
		|| (cmd->buttons & IN_JUMP)
		|| !(local->m_fFlags() & FL_ONGROUND)) {
		return;
	}

	if (start(cmd))
	{
		if (!(local->m_fFlags() & FL_ONGROUND)) {
			cmd->buttons |= IN_JUMP;
		}

		end();
	}
}

bool EnginePred::onUnload()
{
	if (m_local_copy.data) {
		i::mem_alloc->Free(m_local_copy.data);
	}

	if (m_weapon_copy.data) {
		i::mem_alloc->Free(m_weapon_copy.data);
	}

	m_local_copy = {};

	return true;
}

bool EnginePred::onLevelShutdown()
{
	return onUnload();
}