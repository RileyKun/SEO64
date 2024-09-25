#include "player_list.hpp"

#include "../notifs/notifs.hpp"
#include "../cfg.hpp"

bool PlayerList::onLoad()
{
	load(utils::getFilePath() + "player_list.json");

	return true;
}

PlayerListTag *PlayerList::findTagByName(const std::string &name)
{
	if (name.empty()) {
		return nullptr;
	}

	const hash_t name_hash = HASH_RT(name.c_str());

	for (auto &tag : m_tags)
	{
		if (tag.name_hash == name_hash) {
			return &tag;
		}
	}

	return nullptr;
}

PlayerListTag *PlayerList::findTagByHash(hash_t hash)
{
	for (auto &tag : m_tags)
	{
		if (tag.name_hash == hash) {
			return &tag;
		}
	}

	return nullptr;
}

const std::vector<std::pair<std::string, int>> PlayerList::getTags() const
{
	std::vector<std::pair<std::string, int>> tags{};

	tags.push_back({ "none", 0 });

	for (const auto &tag : m_tags) {
		tags.push_back({ tag.name, tag.name_hash });
	}

	return tags;
}

void PlayerList::setPlayerTag(uint32_t player, hash_t tag)
{
	if (tag == 0) {
		m_players.erase(player);
	}
	else {
		m_players[player] = tag;
	}

	save(utils::getFilePath() + "player_list.json");
}

void PlayerList::add(const PlayerListTag &tag)
{
	if (tag.name.empty()) {
		return;
	}

	if (auto * existing{ findTagByName(tag.name) }) {
		*existing = tag;
	}
	else {
		m_tags.push_back(tag);
	}

	save(utils::getFilePath() + "player_list.json");
}

void PlayerList::remove(const PlayerListTag &tag)
{
	if (tag.name.empty()) {
		return;
	}

	if (auto * existing{ findTagByName(tag.name) }) {
		m_tags.erase(std::remove(m_tags.begin(), m_tags.end(), *existing), m_tags.end());

		save(utils::getFilePath() + "player_list.json");
	}
}

void PlayerList::load(const std::string &path)
{
	std::ifstream file{ path.data() };

	if (!file.is_open()) {
		notifs->warning(std::format("failed to load player list: file not found ({}))", path));
		return;
	}

	m_players.clear();
	m_tags.clear();

	nlohmann::json j{};
	file >> j;

	try
	{
		const auto &tags = j.at("tags");

		for (const auto &tag : tags) {
			PlayerListTag t{};

			t.name_hash = HASH_RT(tag.at("name").get<std::string>().c_str());
			t.name = tag.at("name").get<std::string>();
			t.color = Color{ tag.at("color")[0], tag.at("color")[1], tag.at("color")[2], tag.at("color")[3] };
			t.priority = tag.at("priority");

			m_tags.push_back(t);
		}
	}

	catch (const std::exception &e) {
		notifs->warning(std::format("failed to load player list tags: {}", e.what()));
	}

	try
	{
		const auto &players = j.at("players");

		for (const auto &player : players) {
			m_players[player.at("player").get<uint32_t>()] = HASH_RT(player.at("tag").get<std::string>().c_str());
		}
	}

	catch (const std::exception &e) {
		notifs->warning(std::format("failed to load player list players: {}", e.what()));
	}
}

void PlayerList::save(const std::string &path)
{
	nlohmann::json j{};

	{
		nlohmann::json tags{};

		for (const auto &tag : m_tags) {
			nlohmann::json t{};

			t["name"] = tag.name;
			t["color"] = { tag.color.r, tag.color.g, tag.color.b, tag.color.a };
			t["priority"] = tag.priority;

			tags.push_back(t);
		}

		j["tags"] = tags;
	}

	{
		nlohmann::json players{};

		for (const auto &player : m_players) {
			nlohmann::json p{};

			PlayerListTag *tag = findTagByHash(player.second);

			if (!tag) {
				continue;
			}

			p["player"] = player.first;
			p["tag"] = tag->name;

			players.push_back(p);
		}

		j["players"] = players;
	}

	std::ofstream file{ path.data() };

	file << std::setw(4) << j;
}

PlayerListTag *PlayerList::findPlayer(uint32_t player)
{
	for (size_t i{}; i < m_tags.size(); i++)
	{
		if (m_players[player] == m_tags[i].name_hash) {
			return &m_tags[i];
		}
	}

	return nullptr;
}

PlayerListTag *PlayerList::findPlayer(C_TFPlayer *const player)
{
	if (!player) {
		return nullptr;
	}

	player_info_t info{};

	if (!i::engine->GetPlayerInfo(player->entindex(), &info)) {
		return nullptr;
	}

	if (info.fakeplayer) {
		return nullptr;
	}

	return findPlayer(info.friendsID);
}