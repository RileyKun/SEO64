#pragma once

#include "../../shared/shared.hpp"

#define MAKE_EVENT(name, ...) \
class Has##name : public InstTracker<Has##name> \
{ \
public: \
	Has##name() : InstTracker(this) {} \
 \
public: \
	virtual bool on##name(__VA_ARGS__) { return true; } \
};

class IGameEvent;

MAKE_EVENT(Load);
MAKE_EVENT(Unload);
MAKE_EVENT(LevelInit);
MAKE_EVENT(LevelInitPreEntity, char const *const pMapName);
MAKE_EVENT(LevelShutdown);
MAKE_EVENT(ScreenSizeChange);
MAKE_EVENT(GameEvent, IGameEvent *const event);