#pragma once

#include "../../shared/shared.hpp"

class Iface final : public InstTracker<Iface>
{
private:
	void **m_ptr{};
	std::string m_name{};
	std::function<MemAddr()> m_addr_fn{};

public:
	Iface(void **const ptr, std::string_view name, const std::function<MemAddr()> &addr_fn) : InstTracker(this)
	{
		m_ptr = ptr;
		m_name = name;
		m_addr_fn = addr_fn;
	}

public:
	void *const ptr() const
	{
		if (!m_ptr) {
			return nullptr;
		}

		return *m_ptr;
	}

	std::string name() const
	{
		return m_name;
	}

	bool init()
	{
		return m_ptr && (*m_ptr = m_addr_fn().cast<void *>());
	}
};

#define MAKE_IFACE(type, name, addr_fn) \
namespace i { inline type *name{}; } \
namespace make_iface \
{ \
	inline Iface name{ reinterpret_cast<void **>(&i::name), #name, []() { return addr_fn; } }; \
}