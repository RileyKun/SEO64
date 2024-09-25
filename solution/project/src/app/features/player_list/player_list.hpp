#pragma once

#include "../../../game/game.hpp"

struct PlayerListTag
{
	PlayerListTag() = default;

	PlayerListTag(std::string_view name, Color color, int priority = 0)
		: name_hash{ HASH_RT(name.data()) }, name{ name }, color{ color }, priority{ priority }
	{
	}

	hash_t name_hash{};
	std::string name{};
	Color color{};
	int priority{}; // -1 = ignore, 0 = no effect, 1+ = increase order they get scanned in aimbot etc

	bool operator==(const PlayerListTag &other) const
	{
		return name_hash == other.name_hash;
	}
};

class PlayerList final : public HasLoad
{
private:
	std::vector<PlayerListTag> m_tags{};
	std::vector<PlayerListTag> m_old_tags{};
	std::unordered_map<uint32_t, hash_t> m_players{};

public:
	PlayerListTag *findTagByName(const std::string &name);
	PlayerListTag *findTagByHash(hash_t hash);

	const std::vector<std::pair<std::string, int>> getTags() const;

public:
	void setPlayerTag(uint32_t player, hash_t tag);

public:
	void add(const PlayerListTag &tag);
	void remove(const PlayerListTag &tag);

public:
	void load(const std::string &path);
	void save(const std::string &path);

private:
	bool onLoad() override;

public:
	PlayerListTag *findPlayer(uint32_t player);
	PlayerListTag *findPlayer(C_TFPlayer *const player);
};

MAKE_UNIQUE(PlayerList, player_list);