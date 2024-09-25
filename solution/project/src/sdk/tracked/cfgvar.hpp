#pragma once

#include "../../shared/shared.hpp"

class ConfigVar final : public InstTracker<ConfigVar>
{
private:
	std::string m_name{};
	void *m_ptr{};
	size_t m_type_hash{};

public:
	ConfigVar(std::string_view name, void *const ptr, const size_t type_hash) : InstTracker(this)
	{
		m_name = name;
		m_ptr = ptr;
		m_type_hash = type_hash;
	}

public:
	std::string name() const
	{
		return m_name;
	}

	void *const ptr() const
	{
		return m_ptr;
	}

	size_t hash() const
	{
		return m_type_hash;
	}
};

#define MAKE_CFGVAR(name, val) \
namespace cfg { inline auto name{ val }; } \
namespace make_cfgvar { inline ConfigVar name{ #name, &cfg::name, typeid(cfg::name).hash_code() }; }

namespace cfg
{
	void save(std::string_view name);
	void load(std::string_view name);
}