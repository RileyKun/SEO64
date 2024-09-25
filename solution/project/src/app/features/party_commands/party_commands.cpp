#include "party_commands.hpp"

#include "../notifs/notifs.hpp"

void PartyCommands::onMemberChat(CSteamID steam_id, const std::string &message)
{
    uintptr_t gc_client_system{ s::GTFGCClientSystem.call<uintptr_t>() };

    if (!gc_client_system) {
        return;
    }

    s::CTFGCClientSystem_PreInitGC.call<void>(gc_client_system); // ensure the party client is initialized (CSharedObject::RegisterFactory)

    uintptr_t party_client{ s::GTFPartyClient.call<uintptr_t>() };

    if (!party_client) {
		return; // theoretically shouldnt happen given the above call
	}

    if (message[0] == '!') // command requested
    { 
        std::regex command_regex{ "!(\\w+)\\s*?" };
        std::smatch match;

        if (std::regex_search(message, match, command_regex)) {
			std::string command{ match[1] };

            if (command == "test") {
                s::CTFPartyClient_SendPartyChat.call<void>(party_client, "hello!");
			}
		}
    }
    else if(message[0] == '>') // command output
    {

    }
}

bool PartyCommands::onGameEvent(IGameEvent *const event)
{
    if (!event || HASH_RT(event->GetName()) != HASH_CT("party_chat")) {
        return true;
    }

    ETFPartyChatType type = static_cast<ETFPartyChatType>(event->GetInt("type"));

    if (type == ETFPartyChatType::k_eTFPartyChatType_Invalid) {
		return true;
	}

    const char *steam_id_str = event->GetString("steamid");
    if (!steam_id_str) {
		return true;
	}

    CSteamID steam_id{ steam_id_str };

    const char *message_str = event->GetString("text");
    if (!message_str) {
        return true;
    }
    std::string message { message_str };

    switch (type) {
        case k_eTFPartyChatType_MemberChat:
            onMemberChat(steam_id, message);
            break;
        default: break;
    }

    return true;
}
