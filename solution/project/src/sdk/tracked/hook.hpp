#pragma once

#include "../../shared/shared.hpp"

class Hook final : public InstTracker<Hook>
{
private:
	std::string m_name{};
	std::function<bool()> m_init_fn{};
	void *m_og_fn{};

public:
	Hook(std::string_view name, const std::function<bool()> &init_fn) : InstTracker(this)
	{
		m_name = name;
		m_init_fn = init_fn;
	}

public:
	bool create(void *const src, void *const dst)
	{
		return MH_CreateHook(src, dst, &m_og_fn) == MH_OK;
	}

public:
	std::string name() const
	{
		return m_name;
	}

	bool init() const
	{
		return m_init_fn();
	}

public:
	template <typename T>
	inline T call() const
	{
		return reinterpret_cast<T>(m_og_fn);
	}
};

#define MAKE_HOOK(name, address, type, ...) \
namespace hooks::name \
{\
	bool init(); \
	inline Hook hook{ #name, init }; \
	using fn = type(__fastcall*)(__VA_ARGS__); \
	type __fastcall func(__VA_ARGS__); \
}\
bool hooks::name::init() { return hook.create(reinterpret_cast<void *>(address), func); } \
type __fastcall hooks::name::func(__VA_ARGS__)

#define CALL_ORIGINAL hook.call<fn>()