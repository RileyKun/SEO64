#include "auto_queue.hpp"

#include "../cfg.hpp"

/*MAKE_CFGVAR(auto_queue_active, false);
MAKE_CFGVAR(auto_queue_mode, 0); // (0 = only in menu) (1 = always)
MAKE_CFGVAR(auto_queue_auto_accept, 0); // (0 = only in menu) (1 = always)*/

void AutoQueue::queueForCasual()
{
	std::uintptr_t party_client { s::GTFPartyClient.call<std::uintptr_t>() };

	if (!party_client) {
		return;
	}

	if (!m_loaded) { // mfed: prevents a crash if you try to queue without having any criteria loaded (before you open the casual queue menu at least once)
		s::CTFPartyClient_LoadSavedCasualCriteria.call<void>(party_client);
		m_loaded = true;
	}

	if (!s::CTFPartyClient_BInQueueForMatchGroup.call<bool>(party_client, k_nMatchGroup_Casual_12v12)) {
		s::CTFPartyClient_RequestQueueForMatch.call<void>(party_client, k_nMatchGroup_Casual_12v12);
	}
}

void AutoQueue::run()
{
	if (!cfg::auto_queue_active) {
		return;
	}

	const float current_time{ i::engine->Time() };

	if (current_time - m_last_check < 1.0f) {
		return;
	}

	m_last_check = current_time;

	if (cfg::auto_queue_mode == 0 && i::engine->IsInGame()) {
		return;
	}

	queueForCasual();
}
