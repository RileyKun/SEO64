#pragma once

#include "../../../game/game.hpp"

class PartyCommands final : public HasGameEvent, public HasLoad
{
private:
	void onMemberChat(CSteamID steam_id, const std::string &message);

public:
	bool onGameEvent(IGameEvent *const event) override;
};

MAKE_UNIQUE(PartyCommands, party_commands);