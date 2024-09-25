#include "tf_utils.hpp"

int tf_utils::timeToTicks(const float time)
{
	return static_cast<int>(0.5f + time / i::global_vars->interval_per_tick);
}

float tf_utils::ticksToTime(const int tick)
{
	return i::global_vars->interval_per_tick * static_cast<float>(tick);
}

bool tf_utils::isEntOwnedBy(C_BaseEntity *const ent, C_BaseEntity *const who)
{
	if (!ent || !who) {
		return false;
	}

	const size_t classId{ ent->GetClassId() };

	if (classId == class_ids::CTFGrenadePipebombProjectile
		|| classId == class_ids::CTFProjectile_Jar
		|| classId == class_ids::CTFProjectile_JarGas
		|| classId == class_ids::CTFProjectile_JarMilk
		|| classId == class_ids::CTFProjectile_Cleaver)
	{
		return ent->cast<C_BaseGrenade>()->m_hThrower().Get() == who;
	}

	if (classId == class_ids::CTFProjectile_SentryRocket)
	{
		if (C_BaseEntity *const owner{ ent->m_hOwnerEntity().Get() }) {
			return owner->cast<C_BaseObject>()->m_hBuilder() == who;
		}
	}

	if (classId == class_ids::CObjectSentrygun
		|| classId == class_ids::CObjectDispenser
		|| classId == class_ids::CObjectTeleporter)
	{
		return ent->cast<C_BaseObject>()->m_hBuilder() == who;
	}

	return ent->m_hOwnerEntity().Get() == who;
}

void tf_utils::trace(const vec3 &start, const vec3 &end, ITraceFilter *const filter, trace_t *const tr, const int mask)
{
	Ray_t ray{};

	ray.Init(start, end);

	i::trace->TraceRay(ray, mask, filter, tr);
}

void tf_utils::traceHull(const vec3 &start, const vec3 &end, const vec3 &mins, const vec3 &maxs, ITraceFilter *const filter, trace_t *const tr, const int mask)
{
	Ray_t ray{};

	ray.Init(start, end, mins, maxs);

	i::trace->TraceRay(ray, mask, filter, tr);
}

bool tf_utils::getHitbox(C_BaseAnimating *const animating,const matrix3x4_t *const matrix, const int hitbox, vec3 &out)
{
	if (!animating || !matrix) {
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

	mstudiobbox_t *const bbox{ studio_model->pHitbox(hitbox, animating->m_nHitboxSet()) };

	if (!bbox) {
		return false;
	}

	math::vectorTransform((bbox->bbmin + bbox->bbmax) * 0.5f, matrix[bbox->bone], out);

	return true;
}

int tf_utils::getHitboxCount(C_BaseAnimating *const animating)
{
	if (!animating) {
		return 0;
	}

	const model_t *const model{ animating->GetModel() };

	if (!model) {
		return 0;
	}

	studiohdr_t *const studio_model{ i::model_info->GetStudioModel(model) };

	if (!studio_model) {
		return 0;
	}

	return studio_model->iHitboxCount(animating->m_nHitboxSet());
}

float tf_utils::getLerp()
{
	return std::max(cvars::cl_interp->GetFloat(), cvars::cl_interp_ratio->GetFloat() / cvars::cl_updaterate->GetFloat());
}

float tf_utils::getLatency()
{
	INetChannelInfo *const net{ i::engine->GetNetChannelInfo() };

	if (!net) {
		return 0.0f;
	}

	return net->GetLatency(FLOW_INCOMING) + net->GetLatency(FLOW_OUTGOING);
}

//thank u ucuser0
void tf_utils::fixMovement(CUserCmd *const cmd, const Vector &target_angle)
{
	Vector forward, right;
	math::angleVectors(Vector(0, target_angle.y, target_angle.z), &forward, &right, nullptr);

	forward.normalize();
	right.normalize();

	const float denom = (forward.y - (right.y / right.x) * forward.x);
	if (right.x != 0.f && denom != 0.f)
	{
		Vector wish_forward, wish_right;
		math::angleVectors(QAngle(0, cmd->viewangles.y, cmd->viewangles.z), &wish_forward, &wish_right, nullptr);

		wish_forward.normalize();
		wish_right.normalize();

		Vector wish_vel;
		for (int32_t i = 0; i < 2; i++)
			wish_vel[i] = wish_forward[i] * cmd->forwardmove + wish_right[i] * cmd->sidemove;

		wish_vel.z = 0.f;

		cmd->forwardmove = (wish_vel.y - (right.y / right.x) * wish_vel.x) / denom;
		cmd->sidemove = (wish_vel.x - forward.x * cmd->forwardmove) / right.x;
	}
}

void tf_utils::friction(vec3 &velocity, const float surface_friction)
{
	const float speed{ velocity.length() };

	if (speed < 0.1f) {
		return;
	}

	const float friction{ cvars::sv_friction->GetFloat() * surface_friction };
	const float control{ (speed < cvars::sv_stopspeed->GetFloat()) ? cvars::sv_stopspeed->GetFloat() : speed };

	const float drop{ control * friction * i::global_vars->interval_per_tick };

	float new_speed{ std::max(0.0f, speed - drop) };

	if (new_speed != speed) {
		new_speed /= speed;
		velocity *= new_speed;
	}
}

void tf_utils::walkTo(CUserCmd *const cmd, const vec3 &from, const vec3 &to, const float speed)
{
	const vec3 difference{ to - from };

	if (difference.length() == 0.0f) {
		cmd->forwardmove = 0.0f;
		cmd->sidemove = 0.0f;
		return;
	}

	vec3 direction{};
	math::vectorAngles({ difference.x, difference.y, 0.0f }, direction);

	const float yaw{ math::degToRad(direction.y - cmd->viewangles.y) };

	constexpr float move_speed{ 450.0f };

	cmd->forwardmove = cosf(yaw) * (speed * move_speed);
	cmd->sidemove = -sinf(yaw) * (speed * move_speed);
}

bool tf_utils::isWeaponHitscan(C_TFWeaponBase *const wep)
{
	if (!wep
		|| wep->GetWeaponID() == TF_WEAPON_MEDIGUN
		|| isWeaponProjectile(wep)
		|| isWeaponMelee(wep)) {
		return false;
	}

	return (wep->GetDamageType() & DMG_BULLET) || (wep->GetDamageType() & DMG_BUCKSHOT);
}

bool tf_utils::isWeaponMelee(C_TFWeaponBase *const wep)
{
	if (!wep) {
		return false;
	}

	return wep->GetSlot() == 2;
}

bool tf_utils::isWeaponProjectile(C_TFWeaponBase *const wep)
{
	if (!wep) {
		return false;
	}

	switch (wep->GetWeaponID())
	{
		case TF_WEAPON_ROCKETLAUNCHER:
		{
			if (wep->m_iItemDefinitionIndex() == Soldier_m_RocketJumper) {
				return false;
			}

			return true;
		}

		case TF_WEAPON_PIPEBOMBLAUNCHER:
		{
			if (wep->m_iItemDefinitionIndex() == Demoman_s_StickyJumper) {
				return false;
			}

			return true;
		}

		case TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT:
		case TF_WEAPON_GRENADELAUNCHER:
		case TF_WEAPON_FLAREGUN:
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_CROSSBOW:
		case TF_WEAPON_PARTICLE_CANNON:
		case TF_WEAPON_DRG_POMSON:
		case TF_WEAPON_RAYGUN:
		case TF_WEAPON_FLAREGUN_REVENGE:
		case TF_WEAPON_CANNON:
		case TF_WEAPON_SYRINGEGUN_MEDIC:
		case TF_WEAPON_FLAME_BALL:
		case TF_WEAPON_FLAMETHROWER:
		case TF_WEAPON_SHOTGUN_BUILDING_RESCUE: {
			return true;
		}

		default: {
			return false;
		}
	}
}

bool tf_utils::isWeaponChargeable(C_TFWeaponBase* const wep)
{
	if (!wep) {
		return false;
	}

	switch (wep->GetWeaponID())
	{
		case TF_WEAPON_PIPEBOMBLAUNCHER:
		case TF_WEAPON_COMPOUND_BOW:
		case TF_WEAPON_CANNON:
		case TF_WEAPON_SNIPERRIFLE_CLASSIC: {
			return true;
		}

		default: {
			return false;
		}
	}
}

float tf_utils::getGravity()
{
	return cvars::sv_gravity->GetFloat();
}

void tf_utils::getProjectileFireSetup(const vec3 &angs, vec3 offset, vec3 &pos_out)
{
	if (cvars::cl_flipviewmodels->GetInt()) {
		offset.y *= -1.0f;
	}

	vec3 forward{};
	vec3 right{};
	vec3 up{};

	math::angleVectors(angs, &forward, &right, &up);

	pos_out += (forward * offset.x) + (right * offset.y) + (up * offset.z);
}

float tf_utils::getServerTime()
{
	return ticksToTime(i::client_state->m_ClockDriftMgr.m_nServerTick + 1) /*+ i::client_state->m_NetChannel->GetLatency(FLOW_OUTGOING)*/;
}

float tf_utils::randomFloat(const float min, const float max)
{
	static const MemAddr addr{ mem::findExport("vstdlib.dll", "RandomFloat") };

	if (!addr) {
		return 0.0f;
	}

	return addr.call<float>(min, max);
}

int tf_utils::randomInt(const int min, const int max)
{
	static const MemAddr addr{ mem::findExport("vstdlib.dll", "RandomInt") };

	if (!addr) {
		return 0;
	}

	return addr.call<int>(min, max);
}

void tf_utils::randomSeed(const unsigned int seed)
{
	static const MemAddr addr{ mem::findExport("vstdlib.dll", "RandomSeed") };

	if (!addr) {
		return;
	}

	addr.call<void>(seed);
}

double tf_utils::Plat_FloatTime()
{
	static const MemAddr addr{ mem::findExport("tier0.dll", "Plat_FloatTime") };

	if (!addr) {
		return -1.0f;
	}

	return addr.call<double>();
}

bool tf_utils::isLoopback()
{
	INetChannelInfo *const net{ i::engine->GetNetChannelInfo() };

	return net && net->IsLoopback();
}

CUtlVector<int> *tf_utils::getValidPaintKits(C_TFWeaponBase *const wep)
{
	if (!wep) {
		return nullptr;
	}

	void *const item_schema{ s::GetItemSchema.call<void *>() };

	if (!item_schema) {
		return nullptr;
	}

	void *const item_def{ s::CEconItemSchema_GetItemDefinition.call<void *>(item_schema, wep->m_iItemDefinitionIndex())};

	if (!item_def) {
		return nullptr;
	}

	CUtlVector<int> *paint_kits{ reinterpret_cast<CUtlVector<int> *>(s::CTFItemDefinition_GetValidPaintkits.call<void *>(item_def)) };

	if (!paint_kits) {
		return nullptr;
	}

	return paint_kits;
}

std::string tf_utils::getPaintKitName(const int kit_id)
{
	static std::unordered_map<int, std::string> paint_kit_names{};

	if (paint_kit_names.empty()) 
	{
		KeyValues *const kv{ new KeyValues("lang") };

		if (!i::file_system->LoadKeyValues(*kv, TYPE_VMT, "resource/tf_proto_obj_defs_english.txt", "GAME")) {
			return "";
		}

		const std::regex id_pattern{ "9_(\\d+)_field.*" };

		KeyValues *tokens { kv->FindKey("Tokens", false) };

		if (!tokens) {
			return "";
		}

		for (KeyValues *sub_key{ tokens->m_pSub }; sub_key; sub_key = sub_key->m_pPeer)
		{
			const std::string name{ sub_key->GetName() };

			std::smatch matches{};

			if (!std::regex_search(name, matches, id_pattern)) {
				continue;
			}

			const int id{ std::stoi(matches[1].str()) };

			if (id == 0) {
				continue;
			}

			if (id == 97 || id == 98 || id == 99) {
				continue;
			}

			paint_kit_names[id] = std::string(sub_key->m_sValue);
		}
	}

	const auto it{ paint_kit_names.find(kit_id) };

	if (it == paint_kit_names.end()) {
		return "";
	}

	return it->second;
}

void recursiveDump(KeyValues *const kv, std::stringstream &ss, int recursion)
{
	auto indent = [&ss](int times)
	{
		for (int i{}; i < times; ++i) {
			ss << "\t";
		}
	};

	for (KeyValues *dat{ kv->m_pSub }; dat != nullptr; dat = dat->m_pPeer)
	{
		if (dat->m_pSub) {
			recursiveDump(dat, ss, recursion + 1);
		}

		else
		{
			switch (dat->m_iDataType)
			{
				case TYPE_STRING:
				{
					if (dat->m_sValue && *(dat->m_sValue))
					{
						indent(recursion + 1);
						ss << "\"" << dat->GetName() << "\"\t\t\"" << dat->m_sValue << "\"\n";
					}
					break;
				}

				case TYPE_WSTRING:
				{
					if (dat->m_wsValue && *(dat->m_wsValue))
					{
						const std::string str{ utils::wideToUtf8(std::wstring(dat->m_wsValue)) };

						if (!str.empty()) {
							indent(recursion + 1);
							ss << "\"" << dat->GetName() << "\"\t\t\"" << str << "\"\n";
						}
					}
					break;
				}

				case TYPE_FLOAT: {
					indent(recursion + 1);
					ss << "\"" << dat->GetName() << "\"\t\t\"" << std::to_string(dat->m_flValue) << "\"\n";
					break;
				}

				case TYPE_INT: {
					indent(recursion + 1);
					ss << "\"" << dat->GetName() << "\"\t\t\"" << std::to_string(dat->m_iValue) << "\"\n";
					break;
				}

				case TYPE_UINT64:
				{
					indent(recursion + 1);
					char buf[32];
					sprintf_s(buf, sizeof(buf), "0x%016llX", *((uint64_t *)dat->m_sValue));
					ss << "\"" << dat->GetName() << "\"\t\t\"" << buf << "\"\n";
					break;
				}

				default: {
					break;
				}
			}
		}
	}

	indent(recursion);

	ss << "}\n";
}

std::string tf_utils::dumpKeyValues(KeyValues *const kv)
{
	int recursion{};
	std::stringstream ss{};

	ss << "\"" << kv->GetName() << "\"\n";
	ss << "{\n";

	recursiveDump(kv, ss, recursion);

	return ss.str();
}

float tf_utils::airburstDamageForce(const vec3 &size, float damage, float scale)
{
	float force{ damage * (static_cast<float>((48 * 48 * 82.0)) / (size.x * size.y * size.z)) * scale };

	force = std::min(force, 1000.0f);

	return force;
}