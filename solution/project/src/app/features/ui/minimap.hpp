#pragma once

#include "../../../game/game.hpp"

class MiniMap final : public HasLoad, public HasLevelInitPreEntity, public HasLevelShutdown
{
private:
	bool loadNavMap(std::string_view map_name);

private:
	struct NavArea2D final
	{
		vec2 pos{};
		vec2 size{};
		float height{};
		float alpha_scale{ 1.0f };
	};

private:
	std::vector<NavArea2D> m_areas{};

public:
	bool onLoad() override;
	bool onLevelInitPreEntity(const char *const map_name) override;
	bool onLevelShutdown() override;

private:
	void paint();

private:
	struct EntityData final
	{
		vec3 pos{};
		vec3 angle{};
		Color clr{};
	};

	struct CachedData final
	{
		bool should_run_visuals{};
		float frame_time{};

		vec3 local_pos{};
		vec3 local_pos_cur{};
		vec3 local_angs{};

		std::vector<EntityData> players{};

		bool has_data{};
	};

private:
	std::shared_mutex m_data_mutex{};
	CachedData m_data{};

public:
	void cache();
	void run();
};

MAKE_UNIQUE(MiniMap, mini_map);