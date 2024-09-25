#pragma once

#include "../../../game/game.hpp"

struct original_datadesc final
{
	typedescription_t *desc{};
	const hash_t hash{};
	const size_t vars_added{};
};

class DatamapModifier final : public HasLoad, public HasLevelInit, public HasUnload, public HasLevelShutdown
{
private:
	std::vector<original_datadesc> m_original_descs{};

private:
	const size_t getFieldSize(fieldtype_t type);

public:
	bool m_datamap_init{};

public:
	bool iterateDatamaps(datamap_t *const map);
	void addOriginalDataDesc(original_datadesc &desc);
	bool restoreOldDataDesc(datamap_t *const map);
	bool resetAllPredictables();
	bool initInGame();

public:
	bool onUnload() override;
	bool onLevelShutdown() override;
};

MAKE_UNIQUE(DatamapModifier, datamap_mod);