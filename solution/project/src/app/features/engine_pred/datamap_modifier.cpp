#include "datamap_modifier.hpp"

#include "../notifs/notifs.hpp"

const size_t DatamapModifier::getFieldSize(fieldtype_t type)
{
	switch (type)
	{
		case FIELD_INTEGER: 
	{
			return sizeof(int32_t);
		}

		case FIELD_FLOAT: 
		{
			return sizeof(float);
		}
		
		// normie: excuse fucking me returning 0ull doesnt work even though this function is used on only FIELD_FLOAT and FIELD_INTEGER
		// im at a loss of words im going to smoke more
		default: 
		{
			return 4ull;
		}
	}
}

#pragma warning( push )
#pragma warning( disable : 4267 )

bool DatamapModifier::iterateDatamaps(datamap_t *const map)
{
	if (!map) {
		return false;
	}

	//go iterate thru basemaps as well to find the datamap we want

	iterateDatamaps(map->baseMap);

	map->packed_offsets_computed = false;
	map->chains_validated = false;

	if (!map->dataClassName || !map->dataNumFields || !map->dataDesc) {
		return false;
	}

	std::vector<typedescription_t> m_new_descs{};

	auto add_var_to_map = [&](fieldtype_t type, const char *name, const int offset, const int flags, const float tolerance = 0.0f)
	{
		typedescription_t temp{};

		temp.fieldType = type;
		temp.fieldName = name;
		temp.fieldOffset[TD_OFFSET_NORMAL] = offset;
		temp.fieldSize = 1;
		temp.flags = flags;
		temp.fieldSizeInBytes = getFieldSize(temp.fieldType);
		temp.fieldTolerance = tolerance;

		m_new_descs.emplace_back(temp);
	};

	if (HASH_RT(map->dataClassName) == HASH_CT("CTFWeaponBase"))
	{
		for (size_t n{}; n < map->dataNumFields; n++)
		{
			typedescription_t *const field{ &map->dataDesc[n] };

			if (!field || !field->fieldName || !field->fieldSizeInBytes) {
				continue;
			}

			if (HASH_RT(field->fieldName) == HASH_CT("m_flCritTime")) {
				return true;
			}
		}

		//s: xoring these causes some sort of a memory leak that leads to a crash =DDD
		add_var_to_map(FIELD_FLOAT, "m_flCritTime", 4020, FTYPEDESC_NOERRORCHECK);
		add_var_to_map(FIELD_FLOAT, "m_flLastRapidFireCritCheckTIme", 4036, FTYPEDESC_NOERRORCHECK);
		add_var_to_map(FIELD_INTEGER, "m_iLastCritCheckFrame", 4028, FTYPEDESC_NOERRORCHECK);
		add_var_to_map(FIELD_FLOAT, "m_flCritTokenBucket", 3732, FTYPEDESC_NOERRORCHECK);
		add_var_to_map(FIELD_INTEGER, "m_nCritChecks", 3736, FTYPEDESC_NOERRORCHECK);
		add_var_to_map(FIELD_INTEGER, "m_nCritSeedRequests", 3740, FTYPEDESC_NOERRORCHECK);

		typedescription_t *const new_desc{ reinterpret_cast<typedescription_t *>(i::mem_alloc->Alloc((map->dataNumFields + m_new_descs.size()) * sizeof(typedescription_t))) };
		memcpy(new_desc, map->dataDesc, sizeof(typedescription_t) * map->dataNumFields);

		original_datadesc og_desc{ map->dataDesc, HASH_RT(map->dataClassName), m_new_descs.size() };
		addOriginalDataDesc(og_desc);

		for (size_t n{}; n < m_new_descs.size(); n++) {
			memcpy(&new_desc[map->dataNumFields + n], &m_new_descs[n], sizeof(typedescription_t));
		}

		map->dataDesc = new_desc;
		map->dataNumFields += m_new_descs.size();
	}
	else if (HASH_RT(map->dataClassName) == HASH_CT("CWeaponMedigun"))
	{
		for (size_t n{}; n < map->dataNumFields; n++)
		{
			typedescription_t *const field{ &map->dataDesc[n] };

			if (!field || !field->fieldName || !field->fieldSizeInBytes) {
				continue;
			}

			if (HASH_RT(field->fieldName) == HASH_CT("m_nChargeResistType")) {
				return true;
			}
		}

		add_var_to_map(FIELD_INTEGER, "m_nChargeResistType", 4304, FTYPEDESC_INSENDTABLE);

		typedescription_t *const new_desc{ reinterpret_cast<typedescription_t *>(i::mem_alloc->Alloc((map->dataNumFields + m_new_descs.size()) * sizeof(typedescription_t))) };
		memcpy(new_desc, map->dataDesc, sizeof(typedescription_t) * map->dataNumFields);

		original_datadesc og_desc{ map->dataDesc, HASH_RT(map->dataClassName), m_new_descs.size() };
		addOriginalDataDesc(og_desc);

		for (size_t n{}; n < m_new_descs.size(); n++) {
			memcpy(&new_desc[map->dataNumFields + n], &m_new_descs[n], sizeof(typedescription_t));
		}

		map->dataDesc = new_desc;
		map->dataNumFields += m_new_descs.size();
	}

	return true;
}

#pragma warning (pop)

void DatamapModifier::addOriginalDataDesc(original_datadesc &desc)
{
	for (const original_datadesc &original_desc : m_original_descs)
	{
		if (desc.hash == original_desc.hash) {
			return;
		}
	}

	m_original_descs.emplace_back(desc);
}

bool DatamapModifier::restoreOldDataDesc(datamap_t *map)
{
	if (!map) {
		return false;
	}

	restoreOldDataDesc(map->baseMap);

	map->packed_offsets_computed = false;
	map->chains_validated = false;

	for (const original_datadesc &original_desc : m_original_descs)
	{
		if (HASH_RT(map->dataClassName) == original_desc.hash)
		{
			bool modified_map{};

			for (size_t n{}; n < map->dataNumFields; n++)
			{
				typedescription_t *const field{ &map->dataDesc[n] };

				if (!field || !field->fieldName) {
					continue;
				}

				if (HASH_RT(field->fieldName) == HASH_CT("m_flCritTime")
					|| HASH_RT(field->fieldName) == HASH_CT("m_nChargeResistType")) {
					modified_map = true;
					break;
				}
			}

			if (!modified_map) {
				return false;
			}

			if (map->dataDesc) {
				i::mem_alloc->Free(map->dataDesc);
			}

			map->dataNumFields -= static_cast<int>(original_desc.vars_added);
			map->dataDesc = original_desc.desc;

			break;
		}
	}

	return true;
}

bool DatamapModifier::resetAllPredictables()
{
	if (cvars::cl_predict->GetInt() == 0) {
		return false;
	}

	for (int n{ 1 }; n <= i::ent_list->GetHighestEntityIndex(); n++)
	{
		C_BaseEntity *const ent{ i::ent_list->GetClientEntity(n)->cast<C_BaseEntity>() };

		if (!ent || !ent->m_bPredictable())
			continue;

		restoreOldDataDesc(ent->GetPredDescMap());
	}

	i::client_state->ForceFullUpdate();

	return true;
}

bool DatamapModifier::initInGame()
{
	if (m_datamap_init) {
		return true;
	}

	i::client_state->ForceFullUpdate();

	return true;
}

bool DatamapModifier::onUnload()
{
	resetAllPredictables();

	m_original_descs.clear();

	return true;
}

bool DatamapModifier::onLevelShutdown()
{
	return onUnload();
}