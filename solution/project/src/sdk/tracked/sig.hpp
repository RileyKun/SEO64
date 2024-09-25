#pragma once

#include "../../shared/shared.hpp"

class Sig final : public InstTracker<Sig>
{
private:
	MemAddr *m_ptr{};
	std::string m_name{};
	std::function<MemAddr()> m_addr_fn{};

public:
	Sig(MemAddr *const ptr, std::string_view name, const std::function<MemAddr()> &addr_fn) : InstTracker(this)
	{
		m_ptr = ptr;
		m_name = name;
		m_addr_fn = addr_fn;
	}

public:
	MemAddr addr() const
	{
		if (!m_ptr) {
			return 0;
		}

		return *m_ptr;
	}

	std::string name() const
	{
		return m_name;
	}

	bool init()
	{
		return m_ptr && (*m_ptr = m_addr_fn());
	}
};

#define MAKE_SIG(name, addr_fn) \
namespace s { inline MemAddr name{}; } \
namespace make_sig \
{ \
	inline Sig name{ &s::name, #name, []() { return addr_fn; } }; \
}