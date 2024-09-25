#pragma once

#include "entity_visuals_group.hpp"

#include <mutex>

class EntityVisuals final : public HasLoad
{
private:
	std::vector<EVGroup> m_groups{};

private:
	std::shared_mutex m_mutex{};

public:
	std::shared_mutex &getMutex() { return m_mutex; }
	std::vector<EVGroup> &getGroups() { return m_groups; }

public:
	void pushGroup(const EVGroup &group);
	void deleteGroup(const std::string &name);

public:
	void save(const std::string &path);
	void load(const std::string &path);

public:
	bool onLoad() override;

public:
	void runPaint();
	void runModels();
};

MAKE_UNIQUE(EntityVisuals, entity_visuals);