#include "mat_mod.hpp"

#include "../cfg.hpp"

void MatMod::modMats(const Color clr, const bool mod_world, const bool mod_sky)
{
	if (!mod_world && !mod_sky) {
		return;
	}

	for (MaterialHandle_t h{ i::mat_sys->FirstMaterial() }; h != i::mat_sys->InvalidMaterial(); h = i::mat_sys->NextMaterial(h))
	{
		IMaterial *const mat{ i::mat_sys->GetMaterial(h) };

		if (!mat || mat->IsErrorMaterial() || !mat->IsPrecached()) {
			continue;
		}

		const bool world_found{ (std::string_view(mat->GetTextureGroupName()).starts_with(TEXTURE_GROUP_WORLD)
			|| (std::string_view(mat->GetTextureGroupName()).starts_with(TEXTURE_GROUP_OTHER)
			&& std::string_view(mat->GetName()).starts_with("detail/")))
			&& !std::string_view(mat->GetName()).starts_with("skybox/cloud")
		};

		const bool sky_found{ std::string_view(mat->GetName()).find("sky") != std::string_view::npos };

		if (!world_found && !sky_found) {
			continue;
		}

		const vec3 clr_vec{ static_cast<float>(clr.r) / 255.0f, static_cast<float>(clr.g) / 255.0f, static_cast<float>(clr.b) / 255.0f };

		if (mod_world && world_found && !sky_found)
		{
			if (IMaterialVar *const var{ mat->FindVar("$color2", nullptr) }) {
				var->SetVecValue(clr_vec.x, clr_vec.y, clr_vec.z);
			}

			else {
				mat->ColorModulate(clr_vec.x, clr_vec.y, clr_vec.z);
			}
		}

		if (mod_sky && sky_found && !world_found) {

			mat->ColorModulate(clr_vec.x, clr_vec.y, clr_vec.z);
		}
	}
}

bool MatMod::onUnload()
{
	if (m_mod_applied) {
		modMats({ 255, 255, 255, 255 }, true, true);
		m_mod_applied = false;
	}

	return true;
}

void MatMod::run()
{
	if (!i::engine->IsInGame()) {
		m_mod_applied = false;
		return;
	}

	if (!cfg::mat_mod_active) {
		onUnload();
		return;
	}

	auto modeChanged = []()
	{
		const int val{ cfg::mat_mod_mode };

		static int prev_val{ val };

		if (val != prev_val) {
			prev_val = val;
			return true;
		}

		return false;
	};

	if (m_mod_applied && !modeChanged())
	{
		bool clr_changed{};

		if (cfg::mat_mod_mode == 0)
		{
			auto nightScaleChanged = []()
			{
				const float val{ cfg::mat_mod_night_scale };

				static float prev_val{ val };

				if (val != prev_val) {
					prev_val = val;
					return true;
				}

				return false;
			};

			clr_changed = nightScaleChanged();
		}

		if (cfg::mat_mod_mode == 1)
		{
			auto worldClrChanged = []()
			{
				const int val{ cfg::mat_mod_color_world.r + cfg::mat_mod_color_world.g + cfg::mat_mod_color_world.b };

				static int prev_val{ val };

				if (val != prev_val) {
					prev_val = val;
					return true;
				}

				return false;
			};

			auto skyClrChanged = []()
			{
				const int val{ cfg::mat_mod_color_sky.r + cfg::mat_mod_color_sky.g + cfg::mat_mod_color_sky.b };

				static int prev_val{ val };

				if (val != prev_val) {
					prev_val = val;
					return true;
				}

				return false;
			};

			clr_changed = worldClrChanged() || skyClrChanged();
		}

		if (!clr_changed) {
			return;
		}
	}

	const Color mod_clr_night{ Color{ 255, 255, 255, 255 }.scaled(math::remap(cfg::mat_mod_night_scale, 0.0f, 1.0f, 1.0f, 0.01f)) };

	modMats(cfg::mat_mod_mode == 0 ? mod_clr_night : cfg::mat_mod_color_world, true, false);
	modMats(cfg::mat_mod_mode == 0 ? mod_clr_night : cfg::mat_mod_color_sky, false, true);

	m_mod_applied = true;
}