#include "entity_visuals.hpp"

#include "../vis_utils/vis_utils.hpp"

void EntityVisuals::pushGroup(const EVGroup &group)
{
	if (std::any_of(m_groups.begin(), m_groups.end(), [&group](const EVGroup &g) { return g.name == group.name; })) {
		return;
	}

	m_groups.push_back(group);
}

void EntityVisuals::deleteGroup(const std::string &name)
{
	m_groups.erase(std::remove_if(m_groups.begin(), m_groups.end(), [&name](const EVGroup &g) { return g.name == name; }), m_groups.end());

	save(utils::getFilePath() + "entity_visuals.json");
}

void EntityVisuals::save(const std::string &path)
{
	if (m_groups.empty()) {
		return;
	}

	nlohmann::json j{};
	
	for (EVGroup &g : m_groups)
	{
		nlohmann::json j_group{};

		j_group["name"] = g.name;
		j_group["classes"] = g.classes;
		j_group["conds"] = g.conds;
		j_group["effects"] = g.effects;
		j_group["esp_alpha"] = g.esp_alpha;
		j_group["mat_type"] = static_cast<int>(g.mat_type);
		j_group["weapon_type"] = static_cast<int>(g.weapon_type);
		j_group["mat_alpha"] = g.mat_alpha;
		j_group["mat_depth"] = g.mat_depth;
		j_group["outline_alpha"] = g.outline_alpha;

		j.push_back(j_group);
	}

	std::ofstream file{ path };

	if (!file.is_open()) {
		return;
	}

	file << j.dump(4);
	file.close();
}

template<typename t>
t loadValue(const nlohmann::json &j, const std::string &name)
{
    if (j.find(name) == j.end() || j[name].is_null()) {
        return t{0};
    }

    return j[name].get<t>();
}

void EntityVisuals::load(const std::string &path)
{
	std::ifstream file{ path };

	if (!file.is_open()) {
		return;
	}

	nlohmann::json j{};
	file >> j;
	file.close();

	for (const nlohmann::json &j_group : j)
	{
		EVGroup group{};

		group.name = loadValue<std::string>(j_group, "name");
        group.classes = loadValue<int>(j_group, "classes");
        group.conds = loadValue<int>(j_group, "conds");
        group.effects = loadValue<int>(j_group, "effects");
		group.esp_alpha = loadValue<float>(j_group, "esp_alpha");
		group.mat_type = static_cast<EVMaterialType>(loadValue<int>(j_group, "mat_type"));
		group.weapon_type = static_cast<EVWeaponType>(loadValue<int>(j_group, "weapon_type"));
		group.mat_alpha = loadValue<float>(j_group, "mat_alpha");
		group.mat_depth = loadValue<bool>(j_group, "mat_depth");
		group.outline_alpha = loadValue<float>(j_group, "outline_alpha");

		pushGroup(group);
	}
}

bool EntityVisuals::onLoad()
{
	load(utils::getFilePath() + "entity_visuals.json");

	return true;
}

void EntityVisuals::runPaint()
{
	const std::shared_lock<std::shared_mutex> lock(getMutex());

	if (m_groups.empty() || !vis_utils->shouldRunVisuals()) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return;
	}

	draw->updateW2S();

	std::unordered_set<C_BaseEntity *> processed_ents{};

	for (int n{}; n < i::ent_list->GetHighestEntityIndex(); n++)
	{
		IClientEntity *const client_ent{ i::ent_list->GetClientEntity(n) };

		if (!client_ent || client_ent->IsDormant()) {
			continue;
		}

		C_BaseEntity *const base_ent{ client_ent->GetBaseEntity() };

		if (!base_ent) {
			continue;
		}

		for (const EVGroup &g : m_groups)
		{
			if (processed_ents.contains(base_ent) || !g.entPass(base_ent)) {
				continue;
			}

			const float old_alpha{ i::surface->DrawGetAlphaMultiplier() };

			i::surface->DrawSetAlphaMultiplier(g.esp_alpha);
			ev_effects->processPaint(base_ent, g);
			i::surface->DrawSetAlphaMultiplier(old_alpha);


			processed_ents.insert(base_ent);
		}
	}
}

void EntityVisuals::runModels()
{
	const std::shared_lock<std::shared_mutex> lock(getMutex());

	ev_effects->clearDrawn();

	if (m_groups.empty() || !vis_utils->shouldRunVisuals()) {
		return;
	}

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return;
	}

	std::unordered_set<C_BaseEntity *> processed_ents{};

	for (int n{}; n < i::ent_list->GetHighestEntityIndex(); n++)
	{
		IClientEntity *const client_ent{ i::ent_list->GetClientEntity(n) };

		if (!client_ent || client_ent->IsDormant()) {
			continue;
		}

		C_BaseEntity *const base_ent{ client_ent->GetBaseEntity() };

		if (!base_ent) {
			continue;
		}

		for (const EVGroup &g : m_groups)
		{
			if (processed_ents.contains(base_ent) || !g.entPass(base_ent)) {
				continue;
			}

			if (!vis_utils->isEntOnScreen(local, base_ent)) {
				continue;
			}

			bool has_material{};
			bool has_outline{};

			if (g.effects & static_cast<int>(EVEffect::MATERIAL)) {
                has_material = true;
			}

            if (g.effects & static_cast<int>(EVEffect::OUTLINE)) {
                has_outline = true;
            }

			if (!has_material && !has_outline) {
				continue;
			}

			ev_effects->processModel
			(
				std::make_shared<EVModelData>
				(
					base_ent,
					vis_utils->getEntColor(base_ent),
					g.mat_type,
					g.mat_alpha,
					g.mat_depth,
					has_material,
					has_outline,
					g.outline_alpha
				).get()
			);

			processed_ents.insert(base_ent);
		}
	}
}