#pragma once

#include "../../../game/game.hpp"

class VisualUtils final
{
public:
	bool shouldSkipPlayer(
		C_TFPlayer *const local,
		C_TFPlayer *const pl,
		const bool ignore_local,
		const bool ignore_friends,
		const bool ignore_enemies,
		const bool ignore_teammates,
		const bool no_fp_check = false);

public:
	bool shouldSkipBuilding(
		C_TFPlayer *const local,
		C_BaseObject *const obj,
		const bool ignore_local,
		const bool ignore_enemies,
		const bool ignore_teammates);

public:
	bool shouldSkipProjectile(
		C_TFPlayer *const local,
		C_BaseEntity *const proj,
		const bool ignore_local,
		const bool ignore_enemies,
		const bool ignore_teammates);

public:
	bool shouldRunVisuals();

public:
	bool getScreenBounds(C_BaseEntity *const ent, float &x, float &y, float &w, float &h);
	Color getHpColor(float hp, const float max_hp);
	Color getEntColor(C_BaseEntity *const ent);
	std::string getClassName(const int class_num);
	std::wstring getClassNameWide(const int class_num);
	std::wstring getProjectileName(C_BaseEntity *const projectile);
	int createTextureRGBA(const unsigned char *rgba, int w, int h);
	int getClassIconTextureId(const int class_num);
	bool isEntOnScreen(C_TFPlayer *const local, IClientEntity *const ent);
	bool isPosOnScreen(C_TFPlayer *const local, const vec3 &pos);
	void fireBeam(const vec3 &start, const vec3 &end, const Color &color, bool fade);
};

MAKE_UNIQUE(VisualUtils, vis_utils);