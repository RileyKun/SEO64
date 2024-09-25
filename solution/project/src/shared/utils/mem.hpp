#pragma once

#include "memaddr.hpp"

#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <string_view>

namespace mem
{
	MemAddr findBytes(std::string_view dll, std::string_view bytes);
	MemAddr findExport(std::string_view dll, std::string_view fn);
	MemAddr findInterface(std::string_view mod, std::string_view iface);
	MemAddr findVirtual(void *const vtable, const size_t vidx);

	template <size_t idx, typename T, typename... Args>
	inline T callVirtual(void *const base, Args... args)
	{
		return reinterpret_cast<T(__fastcall *)(void *, Args...)>(findVirtual(base, idx).get())(base, args...);
	}
}