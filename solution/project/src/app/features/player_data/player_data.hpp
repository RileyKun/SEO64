#pragma once

#include "../../../game/game.hpp"

class LagRecord final
{
public:
	C_TFPlayer *pl_ptr{};
	float sim_time{};
	float base_sim_time{};
	int tick_count{};
	int flags{};
	vec3 origin{};
	vec3 center{};
	vec3 vel{};
	vec3 render_origin{};
	vec3 abs_origin{};
	vec3 abs_angles{};
	vec3 mins{};
	vec3 maxs{};
	vec3 eye_angs{};
	matrix3x4_t *bones = nullptr;
	bool bones_setup{};
	std::vector<vec3> hitboxes{};
	bool interpolated{};

public:
	LagRecord(C_TFPlayer *const pl, bool is_interp = false);
	~LagRecord();

public:
	const bool isRecordValid(float add_time = 0.f) const;
	const bool isRecordDead() const;
	const bool hasSetupBones() const;
};

class LagRecordMatrixHelper
{
private:
	C_TFPlayer *m_pl{};
	vec3 m_abs_origin{};
	vec3 m_abs_angles{};
	vec3 m_mins{};
	vec3 m_maxs{};
	std::array<matrix3x4_t, 128> m_bones{};

private:
	bool m_allow_bone_setup{};

public:
	void set(C_TFPlayer *const pl, const LagRecord *const lr)
	{
		if (!pl || !lr) {
			return;
		}

		CUtlVector<matrix3x4_t> bones{ pl->m_CachedBoneData() };

		m_pl = pl;
		m_abs_origin = pl->GetAbsOrigin();
		m_abs_angles = pl->GetAbsAngles();

		m_mins = pl->m_vecMinsPreScaled();
		m_maxs = pl->m_vecMaxsPreScaled();

		memcpy(m_bones.data(), bones.Base(), sizeof(matrix3x4_t) * bones.Count());
		memcpy(bones.Base(), lr->bones, sizeof(matrix3x4_t) * bones.Count());

		pl->SetAbsOrigin(lr->abs_origin);
		pl->SetAbsAngles(lr->abs_angles);
		pl->SetCollisionBounds(lr->mins, lr->maxs);
	}

	void reset()
	{
		if (!m_pl) {
			return;
		}

		CUtlVector<matrix3x4_t> bones{ m_pl->m_CachedBoneData() };

		m_pl->SetAbsOrigin(m_abs_origin);
		m_pl->SetAbsAngles(m_abs_angles);
		m_pl->SetCollisionBounds(m_mins, m_maxs);

		memcpy(bones.Base(), m_bones.data(), sizeof(matrix3x4_t) * bones.Count());

		m_pl = nullptr;
		m_abs_origin = {};
		m_abs_angles = {};
		m_mins = {};
		m_maxs = {};

		m_bones = {};
	}

	inline bool isBoneSetupAllowed() const {
		return m_allow_bone_setup;
	}

	inline void allowBoneSetup(bool state) {
		m_allow_bone_setup = state;
	}
};

class PlayerDataVars final
{
private:
	struct InterpHelper final
	{
		float simtime{};
		vec3 origin{};
		vec3 center{};
		vec3 render_origin{};
		vec3 abs_angles{};
		vec3 mins{};
		vec3 maxs{};
		vec3 velocity{};
		InterpHelper(const LagRecord *record);
	};

private:
	void addInterpolatedRecord();

public:
	C_TFPlayer *player{};
	float simtime{};
	int flags{};
	int old_flags{};
	vec3 origin{};
	vec3 eye_angles{};
	vec3 velocity{};
	size_t valid_record_size{};
	std::deque<std::shared_ptr<LagRecord>> records{};

public:
	void update(C_TFPlayer* const pl);
};

class PlayerData final : public HasLoad, public HasUnload, public HasLevelInit, public HasLevelShutdown
{
private:
	std::vector<PlayerDataVars> m_data{};
public:
	void update();
	const size_t getNumRecords(C_TFPlayer *const pl) const;
	const size_t getNumValidRecords(C_TFPlayer *const pl) const;
	LagRecord *const getRecord(C_TFPlayer *const pl, const size_t record_idx) const;
	const PlayerDataVars *const get(C_TFPlayer *const pl) const;

public:
	bool onLoad() override;
	bool onLevelShutdown() override;
	bool onLevelInit() override;
	bool onUnload() override;

private:
	IMaterial *m_model_mat_shaded{};
	IMaterial *m_model_mat_flat{};
	IMaterial *m_model_mat_wireframe{};
	IMaterial *m_model_mat_bg{};

private:
	bool m_drawing_models{};

private:
	void drawLagCompModel(C_TFPlayer *const pl, const LagRecord *const lr);

public:
	void visual();
	bool isDrawingModels() const;
};

MAKE_UNIQUE(PlayerData, player_data);
MAKE_UNIQUE(LagRecordMatrixHelper, lrm);