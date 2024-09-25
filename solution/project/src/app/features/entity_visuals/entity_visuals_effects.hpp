#pragma once

#include "../../../game/game.hpp"

class EVGroup;

enum struct EVEffect : int
{
	NONE = 0,
	NAME = 1 << 0,
	BOX = 1 << 1,
	HP = 1 << 2,
	HPBAR = 1 << 3,
	MATERIAL = 1 << 4,
	OUTLINE = 1 << 5,
	CLASS = 1 << 6,
	CLASSICON = 1 << 7,
	UBER = 1 << 8,
	UBERBAR = 1 << 9,
	TAGS = 1 << 10,
	STEAMID = 1 << 11,
	CONDITIONS = 1 << 12,
	WEAPONS = 1 << 13,
};

enum struct EVWeaponType
{
	ACTIVE_NAME,
	ACTIVE_ICON,
	ALL_NAMES,

	COUNT
};

enum struct EVMaterialType
{
	ORIGINAL,
	FLAT,
	SHADED,
	GLOSSY,
	GLOW,
	PLASTIC,

	COUNT
};

class EVEffectData
{
public:
	C_BaseEntity *ent{};
	Color ent_clr{};

public:
	EVEffectData(C_BaseEntity *const e, const Color c)
		: ent(e), ent_clr(c) {}
};

class EVPaintData final : public EVEffectData
{
public:
	EVEffect effect{};
	vec2 bbox_pos{};
	vec2 bbox_size{};

public:
	EVPaintData(C_BaseEntity *const e, const Color c, const EVEffect f, const vec2 &bp, const vec2 &bs)
		: EVEffectData(e, c), effect(f), bbox_pos(bp), bbox_size(bs) {}
};

class EVModelData final : public EVEffectData
{
public:
	EVMaterialType type{};
	float alpha{};
	bool depth{};
	bool has_material{};
	bool has_outline{};
	float outline_alpha{};

public:
	EVModelData(C_BaseEntity *const e, const Color c, const EVMaterialType t, const float a, const bool d, const bool hm, const bool ho, const float oa)
		: EVEffectData(e, c), type(t), alpha(a), depth(d), has_material(hm), has_outline(ho), outline_alpha(oa) {}
};

class EntityVisualsEffects final : public HasLoad, public HasUnload, public HasLevelInit, public HasLevelShutdown
{
private:
	std::unordered_set<unsigned long> m_drawn_ents{};

private:
	IMaterial *m_mat_flat{};
	IMaterial *m_mat_shaded{};
	IMaterial *m_mat_glossy{};
	IMaterial *m_mat_glow{};
	IMaterial *m_mat_plastic{};

private:
	IMaterialVar *m_mat_glow_envmap_tint{};
	IMaterialVar *m_mat_glow_selfillum_tint{};

private:
	IMaterial *m_mat_glow_color{};
	IMaterial *m_mat_halo_add_to_screen{};
	IMaterial *m_mat_halo_add_to_screen_fat{};
	IMaterial *m_mat_blur_x{};
	IMaterial *m_mat_blur_y{};

private:
	IMaterialVar *m_bloom_amount{};

private:
	ITexture *m_tex_rt_buff_0{};
	ITexture *m_tex_rt_buff_1{};

private:
	struct EVOutlineEnt final
	{
		C_BaseEntity *ent{};
		Color clr{};
		float alpha{};
	};

private:
	std::vector<EVOutlineEnt> m_outline_ents{};
	bool m_drawing_outlines{};

public:
	bool onLoad() override;
	bool onUnload() override;
	bool onLevelInit() override;
	bool onLevelShutdown() override;

public:
	void clearDrawn();
	bool hasDrawn(const EntityHandle_t ehandle) const;

private:
	void drawModelOutline(C_BaseEntity *const ent, const bool model);

private:
	class PaintProcessData final
	{
	public:
		bool has_name{};
		std::wstring name{};

	public:
		bool has_health{};
		float health_cur{};
		float health_max{};

	public:
		bool has_class{};
		std::wstring class_name{};
		int class_icon_texture{};

	public:
		bool has_tags{};
		std::wstring tag{};
		Color tag_clr{};

	public:
		bool has_weapon{};
		C_TFWeaponBase *weapon_ptr{};
		std::vector<std::wstring> weapon_names{};
		int active_weapon_idx{ };

	public:
		bool has_uber{};
		bool is_vaccinator{};
		float uber_cur{};

	public:
		bool has_conditions{};
		std::vector<std::wstring> conditions{};

	public:
		bool has_steamid{};
		std::wstring steamid{};
	};

private:
	bool getPaintProcessData(C_BaseEntity *const ent, int effects, PaintProcessData &out) const;
	float drawWeaponIcon(C_TFWeaponBase *const weapon, const float x, const float y, const float w, const float h, const Color clr);

public:
	void processPaint(C_BaseEntity *const ent, const EVGroup& group);
	void processModel(const EVModelData *const data);

public:
	void processOutlineEffect();
};

MAKE_UNIQUE(EntityVisualsEffects, ev_effects);