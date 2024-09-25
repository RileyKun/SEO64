#include "hitboxes.hpp"

#include "../anims/anims.hpp"
#include "../cfg.hpp"

bool Hitboxes::onLevelShutdown()
{
	m_hitboxes.clear();

	return true;
}

bool Hitboxes::getHitboxes(C_BaseAnimating *const animating, matrix3x4_t *const bones, std::vector<Hitbox> &out, const Color col, const bool no_expire)
{
	if (!animating || !bones) {
		return false;
	}

	const model_t *const model{ animating->GetModel() };

	if (!model) {
		return false;
	}

	studiohdr_t *const studio_model{ i::model_info->GetStudioModel(model) };

	if (!studio_model) {
		return false;
	}

	mstudiohitboxset_t *const set{ studio_model->pHitboxSet(animating->m_nHitboxSet()) };

	if (!set) {
		return false;
	}

	for (int n{}; n < set->numhitboxes; n++)
	{
		mstudiobbox_t *const box{ set->pHitbox(n) };

		if (!box) {
			continue;
		}

		matrix3x4_t bone_to_world{};
		memcpy(&bone_to_world, &bones[box->bone], sizeof(matrix3x4_t));

		Hitbox hbox{};

		math::matrixGetColumn(bone_to_world, 3, hbox.pos);
		math::matrixAngles(bone_to_world, hbox.ang);

		hbox.min = box->bbmin;
		hbox.max = box->bbmax;
		hbox.col = col;
		hbox.target = false;

		if (cfg::hitboxes_show_target_hitbox && n == tf_globals::aimbot_target_hitbox)  {
			hbox.col = cfg::hitboxes_target_hitbox_color;
			hbox.target = true;
		}

		if (no_expire) {
			hbox.expire_time = FLT_MAX;
		}

		else
		{
			hbox.expire_time = i::global_vars->curtime + cfg::hitboxes_duration;

			if (cfg::hitboxes_timeout_style == 1) {
				hbox.expire_time += cfg::hitboxes_fade_out_duration;
			}
		}

		out.push_back(hbox);
	}

	return !out.empty();
}

void Hitboxes::renderHitbox(const Hitbox &hbox)
{
	Color clr{ hbox.col };

	if (cfg::hitboxes_timeout_style == 1) {
		uint8_t alpha { hbox.target ? cfg::hitboxes_target_hitbox_color.a : cfg::hitboxes_color.a };
		const float start_alpha{ static_cast<float>(alpha) / 255.0f };
		const float fade_t{ math::remap(hbox.expire_time - i::global_vars->curtime, 0.0f, cfg::hitboxes_fade_out_duration, 0.0f, start_alpha) };
		clr.a = static_cast<uint8_t>(fade_t * 255.0f);
	}

	if (cfg::hitboxes_style == 0) {
		render_utils::RenderWireframeBox(hbox.pos, hbox.ang, hbox.min, hbox.max, clr, !cfg::hitboxes_no_depth);
	}

	else if (cfg::hitboxes_style == 1) {
		render_utils::RenderBox(hbox.pos, hbox.ang, hbox.min, hbox.max, clr, !cfg::hitboxes_no_depth);
	}

	else if (cfg::hitboxes_style == 2) {
		render_utils::RenderBox(hbox.pos, hbox.ang, hbox.min, hbox.max, clr, !cfg::hitboxes_no_depth);
		render_utils::RenderWireframeBox(hbox.pos, hbox.ang, hbox.min, hbox.max, clr, !cfg::hitboxes_no_depth);
	}
}

void Hitboxes::renderHitboxesLR(const LagRecord *const lr)
{
	if (!lr || !lr->pl_ptr || !lr->bones_setup || !lr->bones) {
		return;
	}

	C_BaseAnimating *const animating{ lr->pl_ptr->cast<C_BaseAnimating>() };

	if (!animating) {
		return;
	}

	std::vector<Hitbox> hitboxes{};

	if (!getHitboxes(animating, lr->bones, hitboxes, cfg::lag_comp_visuals_clr, true)) {
		return;
	}

	for (const Hitbox &hbox : hitboxes) {
		renderHitbox(hbox);
	}
}

void Hitboxes::renderFakeHitboxes()
{
	C_TFPlayer *const local{ ec->getLocal() };

	if (!local || local->deadflag()) {
		return;
	}

	if ((!cfg::fakelag_active && (cfg::antiaim_fake_type == 0 || !cfg::antiaim_active)) || !cfg::fakelag_render_hitboxes) {
		return;
	}

	const bool in_fp{ local->InFirstPersonView() };

	if (!cfg::fakelag_render_hitboxes_firstperson && in_fp) {
		return;
	}

	matrix3x4_t *const bones{ anims->m_fake_bones };
	//matrix3x4_t *const bones{ anims->m_local_bones.data() };

	if (!bones) {
		return;
	}
	
	std::vector<Hitbox> hitboxes{};

	if (!getHitboxes(local, bones, hitboxes, cfg::fakelag_clr, true)) {
		return;
	}

	if (in_fp)
	{
		const Hitbox &closest{ *std::min_element(hitboxes.begin(), hitboxes.end(), [&](const Hitbox &a, const Hitbox &b) {
			return a.pos.distTo(local->GetEyePos()) < b.pos.distTo(local->GetEyePos());
		}) };

		if (closest.pos.distTo(local->GetEyePos()) < 25.0f) {
			return;
		}
	}

	for (const Hitbox &hbox : hitboxes) {
		renderHitbox(hbox);
	}
}

void Hitboxes::add(C_BaseAnimating *const animating, matrix3x4_t *const bones)
{
	if (!cfg::hitboxes_active || !animating || !bones) {
		return;
	}

	if (cfg::hitboxes_clear_on_shot) {
		m_hitboxes.clear();
		i::debug_overlay->ClearAllOverlays(); // for sv_showlagcompensation
	}

	getHitboxes(animating, bones, m_hitboxes, cfg::hitboxes_color);
}

void Hitboxes::render()
{
	for (size_t n{}; n < m_hitboxes.size(); n++)
	{
		const Hitbox &hbox{ m_hitboxes[n] };

		if (i::global_vars->curtime >= hbox.expire_time) {
			m_hitboxes.erase(m_hitboxes.begin() + n);
			n--;
			continue;
		}

		renderHitbox(hbox);
	}
}