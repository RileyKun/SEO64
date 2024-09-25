#pragma once

#include "shared.hpp"

class NetVars final : public HasLoad
{
private:
	std::unordered_map<std::string, std::unordered_set<std::string>> m_written_netvars{};
	std::unordered_map<hash_t, std::unordered_map<hash_t, size_t>> m_offsets{};
	std::unordered_map<hash_t, std::unordered_map<hash_t, RecvProp *>> m_props{};

private:
	std::string propTypeToStr(const SendPropType type) const;
	bool rtNameMatch(std::string_view rt_name);
	void dumpRecvTable(RecvTable *const rt, const std::string &network_name, std::ofstream *const file, const size_t offset);
	void dumpTables(bool write_file);

public:
	size_t getOffset(const hash_t table, const hash_t var);
	RecvProp *getProp(const hash_t table, const hash_t var);
	size_t getDatamapVarOffset(const datamap_t *dmap, const hash_t var);
public:
	bool onLoad() override;
};

MAKE_UNIQUE(NetVars, netvars);

#define NETVAR(type, var, table) type &var() \
{ \
	static uintptr_t offset{ netvars->getOffset(HASH_CT(table), HASH_CT(#var)) }; \
	return *reinterpret_cast<type *>(reinterpret_cast<uintptr_t>(this) + offset); \
}

#define NETVAR_PTR(type, var, table) type* var() \
{ \
	static uintptr_t offset{ netvars->getOffset(HASH_CT(table), HASH_CT(#var)) }; \
	return reinterpret_cast<type *>(reinterpret_cast<uintptr_t>(this) + offset); \
}

#define DATAMAP_VAR(type, var) type &var() \
{ \
	static uintptr_t offset{ netvars->getDatamapVarOffset(this->GetPredDescMap(), HASH_CT(#var)) }; \
	return *reinterpret_cast<type *>(reinterpret_cast<uintptr_t>(this) + offset); \
}

#define NETVAR_OFFSET(type, name, table, var, off) type &name() \
{ \
	static uintptr_t offset{ netvars->getOffset(HASH_CT(table), HASH_CT(var)) + off }; \
	return *reinterpret_cast<type *>(reinterpret_cast<uintptr_t>(this) + offset); \
}

#define CLASSVAR(type, var, off) type &var() \
{ \
	return *reinterpret_cast<type *>(reinterpret_cast<uintptr_t>(this) + off); \
}

class NetVarHook final : public InstTracker<NetVarHook>, public HasUnload
{
private:
	RecvProp *m_prop{};
	RecvVarProxyFn m_original_fn{};
	RecvVarProxyFn m_new_fn{};

private:
	std::string m_name{};

private:
	hash_t m_table{};
	hash_t m_var{};

public:
	NetVarHook(std::string_view name, hash_t m_table, hash_t m_var, RecvVarProxyFn new_fn) : m_name(name), m_table(m_table), m_var(m_var), m_new_fn(new_fn), InstTracker<NetVarHook>(this) {}

public:
	bool init();
	bool onUnload() override;

public:
	void callOriginal(const CRecvProxyData *pData, void *pStruct, void *pOut) const;
};

class NetVarArrayHook final : public InstTracker<NetVarArrayHook>, public HasUnload
{
private:
	//RecvProp *m_prop{};
	//RecvVarProxyFn m_original_fn{};

	struct HookData
	{
		RecvProp *m_prop{};
		RecvVarProxyFn m_original_fn{};
	};

	std::vector<HookData> m_hooks{};
	std::size_t m_hook_count{};

	RecvVarProxyFn m_new_fn{};

private:
	std::string m_name{};

private:
	hash_t m_table{};
	hash_t m_var{};

public:
	NetVarArrayHook(std::string_view name, hash_t m_table, hash_t m_var, RecvVarProxyFn new_fn) : m_name(name), m_table(m_table), m_var(m_var), m_new_fn(new_fn), InstTracker<NetVarArrayHook>(this) {}

public:
	bool init();
	bool onUnload() override;

public:
	void callOriginal(const CRecvProxyData *pData, void *pStruct, void *pOut) const;
};

#define MAKE_NETVAR_HOOK(name, table, var) \
namespace netvar_hooks::name \
{\
	void func(const CRecvProxyData *pData, void *pStruct, void *pOut); \
	inline NetVarHook hook{ #name, HASH_CT(table), HASH_CT(var), func }; \
}\
void netvar_hooks::name::func(const CRecvProxyData *pData, void *pStruct, void *pOut)

#define MAKE_NETVAR_ARRAY_HOOK(name, table, var) \
namespace netvar_hooks::name \
{\
	void func(const CRecvProxyData *pData, void *pStruct, void *pOut); \
	inline NetVarArrayHook hook{ #name, HASH_CT(table), HASH_CT(var), func }; \
}\
void netvar_hooks::name::func(const CRecvProxyData *pData, void *pStruct, void *pOut)

#define CALL_ORIGINAL_PROXY hook.callOriginal

#define GET_ARRAY_INDEX(t) [&]() -> size_t { size_t index { reinterpret_cast<uintptr_t>(pOut) - reinterpret_cast<uintptr_t>(pStruct) }; if (index > 0) { return index / sizeof(t); } return index; }()