#include "cfgvar.hpp"

#include "../../app/features/notifs/notifs.hpp"

void cfg::save(std::string_view name)
{
	std::ofstream file{ name.data() };

	if (!file.is_open()) {
		return;
	}

	nlohmann::json j{};

	for (ConfigVar *const var : getInsts<ConfigVar>())
	{
		try
		{
			const size_t type{ var->hash() };

			if (type == typeid(bool).hash_code()) {
				j[var->name()] = *static_cast<bool *>(var->ptr());
			}

			else if (type == typeid(int).hash_code()) {
				j[var->name()] = *static_cast<int *>(var->ptr());
			}

			else if (type == typeid(float).hash_code()) {
				j[var->name()] = *static_cast<float *>(var->ptr());
			}

			else if (type == typeid(std::string).hash_code()) {
				j[var->name()] = *static_cast<std::string *>(var->ptr());
			}

			else if (type == typeid(Color).hash_code()) {
				const Color clr{ *static_cast<Color *>(var->ptr()) };
				j[var->name()] = { clr.r, clr.g, clr.b, clr.a };
			}

			else {
				notifs->warning(std::format("failed to save config var: {} (unknown type)", var->name()));
			}
		}

		catch (const std::exception &e) {
			notifs->warning(std::format("failed to save config var: {} ({})", var->name(), e.what()));
		}
	}

	file << std::setw(4) << j;
}

void cfg::load(std::string_view name)
{
	std::ifstream file{ name.data() };

	if (!file.is_open()) {
		return;
	}

	nlohmann::json j{};

	file >> j;

	for (ConfigVar *const var : getInsts<ConfigVar>())
	{
		try
		{
			if (j.contains(var->name()))
			{
				const size_t type{ var->hash() };

				if (type == typeid(bool).hash_code()) {
					*static_cast<bool *>(var->ptr()) = j[var->name()];
				}

				else if (type == typeid(int).hash_code()) {
					*static_cast<int *>(var->ptr()) = j[var->name()];
				}

				else if (type == typeid(float).hash_code()) {
					*static_cast<float *>(var->ptr()) = j[var->name()];
				}

				else if (type == typeid(std::string).hash_code()) {
					*static_cast<std::string *>(var->ptr()) = j[var->name()];
				}

				else if (type == typeid(Color).hash_code()) {
					*static_cast<Color *>(var->ptr()) = Color{ j[var->name()][0], j[var->name()][1], j[var->name()][2], j[var->name()][3] };
				}

				else {
					notifs->warning(std::format("failed to load config var: {} (unknown type)", var->name()));
				}
			}
			else {
				notifs->warning(std::format("failed to load config var: {} (not found)", var->name()));
			}
		}

		catch (const std::exception &e) {
			notifs->warning(std::format("failed to load config var: {} ({})", var->name(), e.what()));
		}
	}
}