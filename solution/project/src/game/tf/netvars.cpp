#include "netvars.hpp"
#include "interfaces.hpp"

std::string NetVars::propTypeToStr(const SendPropType type) const
{
	switch (type)
	{
		case DPT_Int:
		{
			return "int";
		}

		case DPT_Float:
		{
			return "float";
		}

		case DPT_Vector:
		{
			return "Vector";
		}

		case DPT_VectorXY:
		{
			return "Vector2D";
		}

		case DPT_String:
		{
			return "const char *";
		}

		default:
		{
			return "unknown";
		}
	}
}

bool NetVars::rtNameMatch(std::string_view rt_name)
{
	for (ClientClass *cc{ i::client->GetAllClasses() }; cc; cc = cc->m_pNext)
	{
		if (!cc || std::string{ cc->GetName() }.find(std::string{ rt_name }.erase(0, 3)) == std::string::npos) {
			continue;
		}

		return true;
	}

	return false;
}

void NetVars::dumpRecvTable(RecvTable *const rt, const std::string &network_name, std::ofstream *const file, const size_t offset)
{
	if (!rt) {
		return;
	}

	for (int n{}; n < rt->GetNumProps(); n++)
	{
		RecvProp *const p{ rt->GetProp(n) };

		if (!p || !p->GetNumElements() || p->IsInsideArray() || (!p->GetOffset() && !p->GetDataTable())) {
			continue;
		}

		if (RecvTable *const rpdt{ p->GetDataTable() })
		{
			std::string name{ rpdt->GetName() };

			// mfed: while this check is good, since gamerules is in quite a lot of tables
			if (name.find("Rules") == std::string::npos) {
				if (rtNameMatch(rpdt->GetName())) {
					continue;
				}
			}

			dumpRecvTable(rpdt, network_name, file, offset + p->GetOffset());
		}


		std::string prop_type{ propTypeToStr(p->GetType()) };
		std::string prop_name{ p->GetName() };
		std::string prop_table_name{ rt->GetName() };

		if (p->GetProxyFn() == i::client->GetStandardRecvProxies()->m_Int32ToInt16) {
			prop_type = "short";
		}

		if (p->GetProxyFn() == i::client->GetStandardRecvProxies()->m_Int32ToInt8) {
			prop_type = "byte";
		}

		if (/*prop_type.starts_with("unknown")
			||*/ prop_name.starts_with("m_iRawValue32")
			|| prop_name.starts_with("m_flValue")
			|| prop_name.find('.') != std::string::npos
			|| isdigit(prop_name.front())) {
			continue;
		}

		if (prop_name.ends_with("]"))
		{
			if (prop_name.ends_with("0]")) {
				prop_name.erase(prop_name.end() - 3, prop_name.end());
			}

			else {
				continue;
			}
		}

		if (prop_name.starts_with("m_ang")) {
			prop_type = "QAngle";
		}

		if (prop_name.starts_with("m_h")) {
			prop_type = "EHANDLE";
		}

		if (prop_name.front() == '\"' && prop_name.back() == '\"') {
			prop_name.erase(prop_name.begin());
			prop_name.erase(prop_name.end() - 1);
		}

		if (file && !m_written_netvars[network_name].contains(prop_name)) {
			*file << std::format("\tNETVAR({}, {}, \"{}\"); // + {}\n", prop_type, prop_name, prop_table_name, offset + p->GetOffset());
		}

		m_written_netvars[network_name].insert(prop_name);
		m_offsets[HASH_RT(prop_table_name.c_str())][HASH_RT(prop_name.c_str())] = offset + p->GetOffset();
		m_props[HASH_RT(prop_table_name.c_str())][HASH_RT(prop_name.c_str())] = p;
	}
}

void NetVars::dumpTables(bool write_file)
{
	std::ofstream file{};

	if (write_file)
	{
		file.open(std::filesystem::path(utils::getFilePath() + "netvars.hpp"));

		if (!file.is_open()) {
			file.close();
		}
	}

	for (ClientClass *cc{ i::client->GetAllClasses() }; cc; cc = cc->m_pNext)
	{
		if (!cc || !cc->m_pRecvTable) {
			continue;
		}

		std::string table_name{ cc->GetName() };

		if (table_name.starts_with('C') && isupper(table_name.at(1))) {
			table_name.insert(1, "_");
		}

		if (write_file && file) {
			file << std::format("class {}\n{}\npublic:\n", table_name, '{');
		}

		dumpRecvTable(cc->m_pRecvTable, table_name, write_file ? &file : nullptr, 0);

		if (write_file && file) {
			file << "};\n\n";
		}
	}

	file.close();
}

size_t NetVars::getOffset(const hash_t table, const hash_t var)
{
	return m_offsets[table][var];
}

RecvProp *NetVars::getProp(const hash_t table, const hash_t var)
{
	return m_props[table][var];
}

size_t NetVars::getDatamapVarOffset(const datamap_t *dmap, const hash_t var)
{
	while (dmap)
	{
		for (int n{}; n < dmap->dataNumFields; n++)
		{
			typedescription_t *field = &dmap->dataDesc[n];
			if (!field || !field->fieldName)
				continue;

			if (HASH_RT(field->fieldName) == var){
				return field->fieldOffset[TD_OFFSET_NORMAL];
			}

			if (field->fieldType == FIELD_EMBEDDED)
			{
				if (field->td) {
					size_t ret{};

					if (ret = getDatamapVarOffset(field->td, var))
						return ret;
				}
			}
		}

		dmap = dmap->baseMap;
	}

	return 0;
}

bool NetVars::onLoad()
{
	dumpTables(true);

	for (NetVarHook *const nh : getInsts<NetVarHook>())
	{
		if (!nh || !nh->init()) {
			return false;
		}
	}

	for (NetVarArrayHook *const nah : getInsts<NetVarArrayHook>())
	{
		if (!nah || !nah->init()) {
			return false;
		}
	}

	return true;
}

//#define DEBUG_NETVAR_HOOKS

bool NetVarHook::init()
{
	if (!m_table || !m_var) {
		return false;
	}

	RecvProp *prop{ netvars->getProp(m_table, m_var) };

	if (!prop) {
		return false;
	}

	m_prop = prop;
	m_original_fn = m_prop->GetProxyFn();
	m_prop->SetProxyFn(m_new_fn);

#ifdef DEBUG_NETVAR_HOOKS
	i::cvar->ConsoleColorPrintf({ 0, 255, 0, 255 }, "hooked %s\n", m_name.c_str());
#endif

	return true;
}

bool NetVarHook::onUnload()
{
	if (!m_prop) {
		return false;
	}

	m_prop->SetProxyFn(m_original_fn);

#ifdef DEBUG_NETVAR_HOOKS
	i::cvar->ConsoleColorPrintf({ 255, 0, 0, 255 }, "unhooked %s\n", m_name.c_str());
#endif

	return true;
}

void NetVarHook::callOriginal(const CRecvProxyData *pData, void *pStruct, void *pOut) const
{
	m_original_fn(pData, pStruct, pOut);
}

bool NetVarArrayHook::init()
{
	if (!m_table || !m_var) {
		return false;
	}

	RecvProp *prop{ netvars->getProp(m_table, m_var) };

	if (!prop) {
		return false;
	}

	RecvTable *array{ prop->GetDataTable() };

	if (!array) {
		return false;
	}

	for (int n{}; n < array->GetNumProps(); n++)
	{
		RecvProp *const p{ array->GetProp(n) };

		if (!p) {
			continue;
		}

		HookData data{ p, p->GetProxyFn() };

		m_hooks.push_back(data);
		m_hook_count++;
	}

	for (HookData &data : m_hooks)
	{
		data.m_prop->SetProxyFn(m_new_fn);
	}

	return !m_hooks.empty();
}

bool NetVarArrayHook::onUnload()
{
	for (HookData &data : m_hooks)
	{
		data.m_prop->SetProxyFn(data.m_original_fn);
	}

	return true;
}

void NetVarArrayHook::callOriginal(const CRecvProxyData *pData, void *pStruct, void *pOut) const
{
	size_t index{ GET_ARRAY_INDEX(int) };

	if (index >= m_hook_count) {
		return;
	}

	m_hooks[index].m_original_fn(pData, pStruct, pOut);
}
