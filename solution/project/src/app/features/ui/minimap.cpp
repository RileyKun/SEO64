#include "minimap.hpp"
#include "ui.hpp"

#include "../vis_utils/vis_utils.hpp"
#include "../trace_utils/trace_utils.hpp"
#include "../cfg.hpp"

bool MiniMap::loadNavMap(std::string_view map_name)
{
	// developer.valvesoftware.com/wiki/NAV_(file_format) \\

	const std::string nav_file_dir
	{
		std::string{ i::engine->GetGameDirectory() }
		+ "\\maps\\"
		+ std::string{ map_name }
		+ ".nav"
	};

	std::ifstream fstream{ nav_file_dir, std::ios::binary };

	if (!fstream.is_open()) {
		return false;
	}

	unsigned int magic_number{};
	fstream.read(reinterpret_cast<char *>(&magic_number), sizeof(magic_number));

	unsigned int version{};
	fstream.read(reinterpret_cast<char *>(&version), sizeof(version));

	if (version >= 10) {
		unsigned int sub_version{};
		fstream.read(reinterpret_cast<char *>(&sub_version), sizeof(sub_version));
	}

	if (version >= 4) {
		unsigned int save_bsp_size{};
		fstream.read(reinterpret_cast<char *>(&save_bsp_size), sizeof(save_bsp_size));
	}

	if (version >= 14) {
		unsigned char is_analyzed{};
		fstream.read(reinterpret_cast<char *>(&is_analyzed), sizeof(is_analyzed));
	}

	if (version >= 5)
	{
		unsigned short count{};
		fstream.read(reinterpret_cast<char *>(&count), sizeof(count));

		for (int n{}; n < count; n++)
		{
			unsigned short len{};
			fstream.read(reinterpret_cast<char *>(&len), sizeof(len));

			char name[256]{ '\0' };
			fstream.read(reinterpret_cast<char *>(&name), len);
		}

		if (version > 11) {
			unsigned char has_unnamed_areas{};
			fstream.read(reinterpret_cast<char *>(&has_unnamed_areas), sizeof(has_unnamed_areas));
		}
	}

	unsigned int area_count{};
	fstream.read(reinterpret_cast<char *>(&area_count), sizeof(area_count));

	for (int n{}; n < static_cast<int>(area_count); n++)
	{
		unsigned int id{};
		fstream.read(reinterpret_cast<char *>(&id), sizeof(id));

		unsigned int attribute{};
		fstream.read(reinterpret_cast<char *>(&attribute), sizeof(attribute));

		float nw_corner[3]{};
		fstream.read(reinterpret_cast<char *>(&nw_corner), sizeof(nw_corner));

		float se_corner[3]{};
		fstream.read(reinterpret_cast<char *>(&se_corner), sizeof(se_corner));

		float ne_z{};
		fstream.read(reinterpret_cast<char *>(&ne_z), sizeof(ne_z));

		float sw_z{};
		fstream.read(reinterpret_cast<char *>(&sw_z), sizeof(sw_z));

		for (int n{}; n < 4; n++)
		{
			unsigned int count{};
			fstream.read(reinterpret_cast<char *>(&count), sizeof(count));

			for (int n{}; n < static_cast<int>(count); n++) {
				unsigned int area_id{};
				fstream.read(reinterpret_cast<char *>(&area_id), sizeof(area_id));
			}
		}

		unsigned char hiding_spot_count{};
		fstream.read(reinterpret_cast<char *>(&hiding_spot_count), sizeof(hiding_spot_count));

		for (int n{}; n < static_cast<int>(hiding_spot_count); n++)
		{
			unsigned int id{};
			fstream.read(reinterpret_cast<char *>(&id), sizeof(id));

			float position[3]{};
			fstream.read(reinterpret_cast<char *>(&position), sizeof(position));

			unsigned char attributes{};
			fstream.read(reinterpret_cast<char *>(&attributes), sizeof(attributes));
		}

		unsigned int encounter_path_count{};
		fstream.read(reinterpret_cast<char *>(&encounter_path_count), sizeof(encounter_path_count));

		for (int n{}; n < static_cast<int>(encounter_path_count); n++)
		{
			unsigned int entry_area_id{};
			fstream.read(reinterpret_cast<char *>(&entry_area_id), sizeof(entry_area_id));

			unsigned char entry_direction{};
			fstream.read(reinterpret_cast<char *>(&entry_direction), sizeof(entry_direction));

			unsigned int dest_area_id{};
			fstream.read(reinterpret_cast<char *>(&dest_area_id), sizeof(dest_area_id));

			unsigned char dest_direction{};
			fstream.read(reinterpret_cast<char *>(&dest_direction), sizeof(dest_direction));

			unsigned char encounter_spot_count{};
			fstream.read(reinterpret_cast<char *>(&encounter_spot_count), sizeof(encounter_spot_count));

			for (int n{}; n < static_cast<int>(encounter_spot_count); n++)
			{
				unsigned int area_id{};
				fstream.read(reinterpret_cast<char *>(&area_id), sizeof(area_id));

				unsigned char parametric_distance{};
				fstream.read(reinterpret_cast<char *>(&parametric_distance), sizeof(parametric_distance));
			}
		}

		unsigned short place_id{};
		fstream.read(reinterpret_cast<char *>(&place_id), sizeof(place_id));

		for (int n{}; n < 2; n++)
		{
			unsigned int ladder_count{};
			fstream.read(reinterpret_cast<char *>(&ladder_count), sizeof(ladder_count));

			for (int n{}; n < static_cast<int>(ladder_count); n++) {
				unsigned int ladder_id{};
				fstream.read(reinterpret_cast<char *>(&ladder_id), sizeof(ladder_id));
			}
		}

		float earliest_occupy_times[2]{};
		fstream.read(reinterpret_cast<char *>(&earliest_occupy_times), sizeof(earliest_occupy_times));

		float light_intensity[4]{};
		fstream.read(reinterpret_cast<char *>(&light_intensity), sizeof(light_intensity));

		unsigned int area_bind_count{};
		fstream.read(reinterpret_cast<char *>(&area_bind_count), sizeof(area_bind_count));

		for (int n{}; n < static_cast<int>(area_bind_count); n++)
		{
			unsigned int target_area_id{};
			fstream.read(reinterpret_cast<char *>(&target_area_id), sizeof(target_area_id));

			unsigned char attributes{};
			fstream.read(reinterpret_cast<char *>(&attributes), sizeof(attributes));
		}

		unsigned int inherit_visibility_from_area_id{};
		fstream.read(reinterpret_cast<char *>(&inherit_visibility_from_area_id), sizeof(inherit_visibility_from_area_id));

		unsigned int custom_data{};
		fstream.read(reinterpret_cast<char *>(&custom_data), sizeof(custom_data));

		//not perfect for 3D!
		const vec3 nw{ nw_corner[0], nw_corner[1], nw_corner[2] };
		const vec3 ne{ se_corner[0], nw_corner[1], nw_corner[2] };
		const vec3 sw{ nw_corner[0], se_corner[1], se_corner[2] };
		const vec3 se{ se_corner[0], se_corner[1], se_corner[2] };

		const float min_x{ (std::min({ nw.x, ne.x, sw.x, se.x })) };
		const float max_x{ (std::max({ nw.x, ne.x, sw.x, se.x })) };
		const float min_y{ (std::min({ nw.y, ne.y, sw.y, se.y })) };
		const float max_y{ (std::max({ nw.y, ne.y, sw.y, se.y })) };

		const float height{ (nw.z + ne.z) * 0.5f };

		m_areas.push_back({ vec2{ min_x, min_y }, vec2{ (max_x - min_x), (max_y - min_y) }, height });
	}

	std::sort(m_areas.begin(), m_areas.end(), [](const NavArea2D &a, const NavArea2D &b) { return a.height < b.height; });

	return true;
}

bool MiniMap::onLoad()
{
	loadNavMap(i::client_state->m_szLevelBaseName);

	return true;
}

bool MiniMap::onLevelInitPreEntity(const char *const map_name)
{
	loadNavMap(map_name);

	return true;
}

bool MiniMap::onLevelShutdown()
{
	m_areas.clear();

	return true;
}

void MiniMap::paint()
{
	if (!m_data.has_data) {
		return;
	}

	auto transformPoint = [&](const vec2 &point_in)
	{
		//2D point on the screen
		const vec2 map_origin{
			ImGui::GetWindowPos().x + (ImGui::GetWindowSize().x * 0.5f),
			ImGui::GetWindowPos().y + (ImGui::GetWindowSize().y * cfg::map_origin)
		};

		//3D point in the world
		const vec2 map_follow_target{
			m_data.local_pos.x,
			m_data.local_pos.y
		};

		vec2 point_out{ map_origin + ((point_in - map_follow_target) * cfg::map_zoom) };

		//rotation
		{
			const ImVec2 translated{ point_out.x - map_origin.x, point_out.y - map_origin.y };

			const float angle{ math::degToRad(m_data.local_angs.y - 90.0f) * -1.0f };

			const float rotated_x{ translated.x * cosf(angle) - translated.y * sinf(angle) };
			const float rotated_y{ translated.x * sinf(angle) + translated.y * cosf(angle) };

			point_out.x = rotated_x + map_origin.x;
			point_out.y = rotated_y + map_origin.y;
		}

		//perspective projection
		{
			const float dist_from_center{ fmaxf((point_out.y - ImGui::GetWindowPos().y) / 350.0f, 0.01f) };
			const float perspective_scale{ fmaxf(1.0f / (1.0f + fabsf(dist_from_center) * cfg::map_perspective), 0.01f) };

			point_out.x = map_origin.x + (point_out.x - map_origin.x) * perspective_scale;
			point_out.y = map_origin.y - (point_out.y - map_origin.y) * perspective_scale;
		}

		return ImVec2{ point_out.x, point_out.y };
	};

	auto isSameHeight = [&](const float h0, const float h1) {
		return h1 <= (h0 + 150.0f) && h0 <= (h1 + 150.0f);
	};

	if (cfg::map_draw_nav)
	{
		for (NavArea2D &area : m_areas)
		{
			if (cfg::map_nav_highlight_height)
			{
				if (isSameHeight(m_data.local_pos.z, area.height)) {
					area.alpha_scale += 5.0f * m_data.frame_time;
				}

				else {
					area.alpha_scale -= 5.0f * m_data.frame_time;
				}

				area.alpha_scale = std::clamp(area.alpha_scale, 0.1f, 1.0f);
			}

			else {
				area.alpha_scale = 0.1f;
			}

			const ImVec2 points[4]
			{
				transformPoint({ area.pos.x, area.pos.y }),
				transformPoint({ area.pos.x + area.size.x, area.pos.y }),
				transformPoint({ area.pos.x + area.size.x, area.pos.y + area.size.y }),
				transformPoint({ area.pos.x, area.pos.y + area.size.y }),
			};

			const Color clr{ cfg::map_nav_color };

			ImGui::GetWindowDrawList()->AddConvexPolyFilled(
				points, 4,
				IM_COL32(clr.r, clr.g, clr.b, static_cast<uint8_t>(255.0f * area.alpha_scale))
			);
		}
	}

	if (cfg::map_players_active)
	{
		for (const EntityData &data : m_data.players)
		{
			const ImVec2 point{ transformPoint({ data.pos.x, data.pos.y }) };

			const Color clr{ data.clr.scaled(isSameHeight(m_data.local_pos_cur.z, data.pos.z) ? 1.0f : 0.8f) };

			if (cfg::map_players_style == 0)
			{
				const float rad{ math::remap(cfg::map_zoom, 0.1f, 1.0f, 2.0f, 5.0f) };

				ImGui::GetWindowDrawList()->AddCircleFilled(point, rad * 1.2f, IM_COL32(0, 0, 0, 100));
				ImGui::GetWindowDrawList()->AddCircleFilled(point, rad, IM_COL32(clr.r, clr.g, clr.b, 255));
			}

			else if (cfg::map_players_style == 1)
			{
				const float arrow_scale{ math::remap(cfg::map_zoom, 0.1f, 1.0f, 0.5f, 1.5f) };
				const float arrow_length{ 10.0f * arrow_scale };
				const float arrow_width{ 10.0f * arrow_scale };
				const float outline_scale{ 1.1f };

				ImVec2 arrow_points[4]{
					ImVec2(point.x, point.y - arrow_length),
					ImVec2(point.x + arrow_width * 0.5f, point.y + arrow_length * 0.25f),
					ImVec2(point.x, point.y),
					ImVec2(point.x - arrow_width * 0.5f, point.y + arrow_length * 0.25f)
				};

				ImVec2 arrow_outline[4]{
					ImVec2(point.x, point.y - arrow_length * outline_scale),
					ImVec2(point.x + arrow_width * 0.5f * outline_scale, point.y + arrow_length * 0.25f * outline_scale),
					ImVec2(point.x, point.y + 1.0f * outline_scale),
					ImVec2(point.x - arrow_width * 0.5f * outline_scale, point.y + arrow_length * 0.25f * outline_scale)
				};

				ImVec2 arrow_points_shade[3]{
					ImVec2(point.x, point.y - arrow_length),
					ImVec2(point.x + arrow_width * 0.5f, point.y + arrow_length * 0.25f),
					ImVec2(point.x, point.y)
				};

				auto rotate = [&](ImVec2 *points, const float scale)
				{
					if (!points) {
						return;
					}

					const float minimap_rotation{ math::degToRad(m_data.local_angs.y - 90.0f) };
					const float player_rotation{ math::degToRad(data.angle.y - 90.0f) };
					const float angle{ -1.0f * (player_rotation - minimap_rotation) };

					for (int n{}; n < 4; n++)
					{
						const float translated_x{ (points[n].x - point.x) * scale };
						const float translated_y{ (points[n].y - point.y) * scale };

						points[n].x = translated_x * cosf(angle) - translated_y * sinf(angle) + point.x;
						points[n].y = translated_x * sinf(angle) + translated_y * cosf(angle) + point.y;
					}
				};

				rotate(arrow_outline, outline_scale);

				ImGui::GetWindowDrawList()->AddConvexPolyFilled(
					arrow_outline, 4, IM_COL32(0, 0, 0, 100)
				);

				rotate(arrow_points, 1.0f);

				ImGui::GetWindowDrawList()->AddConvexPolyFilled(
					arrow_points, 4, IM_COL32(clr.r, clr.g, clr.b, 255)
				);

				rotate(arrow_points_shade, 1.0f);

				ImGui::GetWindowDrawList()->AddConvexPolyFilled(
					arrow_points_shade, 3, IM_COL32(0, 0, 0, 60)
				);
			}
		}
	}
}

void MiniMap::cache()
{
	const std::unique_lock<std::shared_mutex> lock(m_data_mutex);

	m_data = {};

	C_TFPlayer *const local{ ec->getLocal() };

	if (!local) {
		return;
	}

	m_data.should_run_visuals = vis_utils->shouldRunVisuals();
	m_data.frame_time = i::global_vars->frametime;

	m_data.local_pos = local->GetRenderCenter();
	m_data.local_pos_cur = local->GetRenderCenter();
	m_data.local_angs = i::engine->GetViewAngles();

	if (local->deadflag() && (local->m_iObserverMode() == OBS_MODE_CHASE || local->m_iObserverMode() == OBS_MODE_IN_EYE))
	{
		if (C_BaseEntity *const obs_target{ local->m_hObserverTarget() }) {
			m_data.local_pos = obs_target->GetRenderCenter();
		}
	}

	trace_t trace{};

	tf_utils::trace(m_data.local_pos, m_data.local_pos - vec3{ 0.0f, 0.0f, 8192.0f }, &trace_filters::world, &trace, MASK_SOLID);

	m_data.local_pos.z = trace.endpos.z;

	for (const EntityHandle_t ehandle : ec->getGroup(ECGroup::PLAYERS_ALL))
	{
		IClientEntity *const ent{ i::ent_list->GetClientEntityFromHandle(ehandle) };

		if (!ent) {
			continue;
		}

		C_TFPlayer *const pl{ ent->cast<C_TFPlayer>() };

		if (!pl || pl->deadflag()) {
			continue;
		}

		if (vis_utils->shouldSkipPlayer(
			local, pl,
			cfg::map_players_ignore_local,
			cfg::map_players_ignore_friends,
			cfg::map_players_ignore_enemies,
			cfg::map_players_ignore_teammates,
			true)) {
			continue;
		}

		m_data.players.push_back({ pl->GetRenderOrigin(), 
								 pl == local ? m_data.local_angs : pl->m_angEyeAngles(),
								 vis_utils->getEntColor(pl)});
	}

	m_data.has_data = true;
}

void MiniMap::run()
{
	const std::shared_lock<std::shared_mutex> lock(m_data_mutex);

	if (!cfg::map_active || (!ui->isOpen() && !m_data.should_run_visuals)) {
		return;
	}

	if (ImGui::Begin("minimap", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		ImGui::SetWindowSize({ cfg::map_width, cfg::map_height }, ImGuiCond_Always);

		paint();

		if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
			ImGui::SetWindowPos({ cfg::ui_pos_minimap_x, cfg::ui_pos_minimap_y });
		}

		cfg::ui_pos_minimap_x = ImGui::GetWindowPos().x;
		cfg::ui_pos_minimap_y = ImGui::GetWindowPos().y;

		ImGui::End();
	}
}