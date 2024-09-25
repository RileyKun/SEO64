#pragma once

#include "tracked/cfgvar.hpp"
#include "tracked/events.hpp"
#include "tracked/hook.hpp"
#include "tracked/iface.hpp"
#include "tracked/sig.hpp"

#define MAKE_UNIQUE(type, name) inline const std::unique_ptr<type> name{ std::make_unique<type>() };