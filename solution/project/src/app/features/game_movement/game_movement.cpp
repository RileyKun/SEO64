#include "game_movement.hpp"

//===================================================================================================================================================\\

class CTraceFilterSimple
{
public:
	CTraceFilterSimple(const IHandleEntity *passentity, int collisionGroup, ShouldHitFunc_t pExtraShouldHitCheckFn = nullptr) {
		s::CTraceFilterSimple_Ctor.call<void>(this, passentity, collisionGroup, pExtraShouldHitCheckFn);
	}

private:
	void *thisptr{};
	const IHandleEntity *m_pPassEnt{};
	int m_collisionGroup{};
	ShouldHitFunc_t m_pExtraShouldHitCheckFunction{};
};

//===================================================================================================================================================\\

const bool g_bMovementOptimizations{ true };
const Vector vec3_origin{ 0.0f, 0.0f, 0.0f };
const int nanmask{ 255 << 23 };

//===================================================================================================================================================\\

#define M_PI 3.14159265358979323846
#define M_PI_F ((float)(M_PI))

#define RAD2DEG(x) ((float)(x) * (float)(180.0f / M_PI_F))
#define DEG2RAD(x) ((float)(x) * (float)(M_PI_F / 180.0f))

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

#define	COORD_INTEGER_BITS 14
#define COORD_FRACTIONAL_BITS 5
#define COORD_DENOMINATOR (1 << (COORD_FRACTIONAL_BITS))
#define COORD_RESOLUTION (1.0f / (COORD_DENOMINATOR))

//===================================================================================================================================================\\

FORCEINLINE void VectorMultiply(const Vector &a, vec_t b, Vector &c)
{
	c.x = a.x * b;
	c.y = a.y * b;
	c.z = a.z * b;
}

FORCEINLINE void VectorMAInline(const Vector &start, float scale, const Vector &direction, Vector &dest)
{
	dest.x = start.x + direction.x * scale;
	dest.y = start.y + direction.y * scale;
	dest.z = start.z + direction.z * scale;
}

FORCEINLINE void VectorMAInline(const float *start, float scale, const float *direction, float *dest)
{
	dest[0] = start[0] + direction[0] * scale;
	dest[1] = start[1] + direction[1] * scale;
	dest[2] = start[2] + direction[2] * scale;
}

FORCEINLINE void VectorMA(const Vector &start, float scale, const Vector &direction, Vector &dest)
{
	VectorMAInline(start, scale, direction, dest);
}

FORCEINLINE void VectorMA(const float *start, float scale, const float *direction, float *dest)
{
	VectorMAInline(start, scale, direction, dest);
}

FORCEINLINE void VectorSubtract(const Vector &a, const Vector &b, Vector &c)
{
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
}

FORCEINLINE void _SSE_RSqrtInline(float a, float *out)
{
	__m128 xx{ _mm_load_ss(&a) };
	__m128 xr{ _mm_rsqrt_ss(xx) };
	__m128 xt{};

	xt = _mm_mul_ss(xr, xr);
	xt = _mm_mul_ss(xt, xx);
	xt = _mm_sub_ss(_mm_set_ss(3.0f), xt);
	xt = _mm_mul_ss(xt, _mm_set_ss(0.5f));
	xr = _mm_mul_ss(xr, xt);

	_mm_store_ss(out, xr);
}

FORCEINLINE float VectorNormalize(Vector &vec)
{
	float sqrlen{ vec.lengthSqr() + 1.0e-10f };
	float invlen{};

	_SSE_RSqrtInline(sqrlen, &invlen);

	vec.x *= invlen;
	vec.y *= invlen;
	vec.z *= invlen;

	return sqrlen * invlen;
}

FORCEINLINE vec_t DotProduct(const Vector &a, const Vector &b)
{
	return(a.x * b.x + a.y * b.y + a.z * b.z);
}

template <class T>
FORCEINLINE T Lerp(float flPercent, T const &A, T const &B)
{
	return A + (B - A) * flPercent;
}

template< class T >
FORCEINLINE T Clamp(T const &val, T const &minVal, T const &maxVal)
{
	if (val < minVal) {
		return minVal;
	}

	else if (val > maxVal) {
		return maxVal;
	}

	else {
		return val;
	}
}

template< class T >
FORCEINLINE T Min(T const &val1, T const &val2)
{
	return val1 < val2 ? val1 : val2;
}

template< class T >
FORCEINLINE T Max(T const &val1, T const &val2)
{
	return val1 > val2 ? val1 : val2;
}

FORCEINLINE void SinCos(float radians, float *sine, float *cosine)
{
	/*_asm
	{
		fld DWORD PTR[radians]
		fsincos

		mov edx, DWORD PTR[cosine]
		mov eax, DWORD PTR[sine]

		fstp DWORD PTR[edx]
		fstp DWORD PTR[eax]
	}*/

	*sine = sinf(radians);
	*cosine = cosf(radians);
}

FORCEINLINE void VectorCopy(const Vector &src, Vector &dst)
{
	dst.x = src.x;
	dst.y = src.y;
	dst.z = src.z;
}

FORCEINLINE void CrossProduct(const Vector &a, const Vector &b, Vector &result)
{
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
}

FORCEINLINE void VectorScale(const Vector &in, vec_t scale, Vector &result)
{
	VectorMultiply(in, scale, result);
}

FORCEINLINE vec_t VectorLength(const Vector &v)
{
	return (vec_t)sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

FORCEINLINE void VectorAdd(const Vector &a, const Vector &b, Vector &c)
{
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
}

FORCEINLINE int VectorCompare(const Vector &v1, const Vector &v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

//===================================================================================================================================================\\

FORCEINLINE bool AE_IsInfinite(const float a)
{
	return (*(int *)&a & 0x7FFFFFFF) == 0x7F800000;
}

FORCEINLINE bool AE_IsNan(const float a)
{
	const int exp{ *(int *)&a & 0x7F800000 };
	const int mantissa{ *(int *)&a & 0x007FFFFF };

	return exp == 0x7F800000 && mantissa != 0;
}

FORCEINLINE int AE_Sign(const float a)
{
	return (*(int *)&a) & 0x80000000;
}

FORCEINLINE bool AlmostEqual(const float a, const float b, const int maxUlps = 10)
{
	if (AE_IsInfinite(a) || AE_IsInfinite(b)) {
		return a == b;
	}

	if (AE_IsNan(a) || AE_IsNan(b)) {
		return false;
	}

	if (AE_Sign(a) != AE_Sign(b)) {
		return a == b;
	}

	int aInt{ *(int *)&a };

	if (aInt < 0) {
		aInt = 0x80000000 - aInt;
	}

	int bInt{ *(int *)&b };

	if (bInt < 0) {
		bInt = 0x80000000 - bInt;
	}

	return abs(aInt - bInt) <= maxUlps;
}

//===================================================================================================================================================\\

void AngleVectors(const QAngle &angles, Vector *forward, Vector *right, Vector *up)
{
	if (!forward && !right && !up) {
		return;
	}

	float sr{};
	float sp{};
	float sy{};
	float cr{};
	float cp{};
	float cy{};

	SinCos(DEG2RAD(angles[YAW]), &sy, &cy);
	SinCos(DEG2RAD(angles[PITCH]), &sp, &cp);
	SinCos(DEG2RAD(angles[ROLL]), &sr, &cr);

	if (forward) {
		forward->x = cp * cy;
		forward->y = cp * sy;
		forward->z = -sp;
	}

	if (right) {
		right->x = (-1.0f * sr * sp * cy + -1.0f * cr * -sy);
		right->y = (-1.0f * sr * sp * sy + -1.0f * cr * cy);
		right->z = -1.0f * sr * cp;
	}

	if (up) {
		up->x = (cr * sp * cy + -sr * -sy);
		up->y = (cr * sp * sy + -sr * cy);
		up->z = cr * cp;
	}
}

//===================================================================================================================================================\\

void UTIL_TraceRay(const Ray_t &ray, unsigned int mask, const IHandleEntity *ignore, int collisionGroup, trace_t *ptr, ShouldHitFunc_t pExtraShouldHitCheckFn = nullptr)
{
	CTraceFilterSimple traceFilter{ ignore, collisionGroup, pExtraShouldHitCheckFn };

	i::trace->TraceRay(ray, mask, reinterpret_cast<ITraceFilter *>(&traceFilter), ptr);
}

//===================================================================================================================================================\\

bool GameMovement::IsDead()
{
	return (player->GetHealth() <= 0 && !player->IsAlive());
}

bool GameMovement::InWater()
{
	return player->GetWaterLevel() > WL_Feet;
}

Vector GameMovement::GetPlayerMins()
{
	return player->GetMins();
}

Vector GameMovement::GetPlayerMaxs()
{
	return player->GetMaxs();
}

float GameMovement::GetCurrentGravity()
{
	return cvars::sv_gravity->GetFloat();
}

unsigned int GameMovement::PlayerSolidMask(bool brushOnly)
{
	unsigned int uMask{};

	if (player->InCond(TF_COND_HALLOWEEN_GHOST_MODE)) {
		return MASK_PLAYERSOLID_BRUSHONLY;
	}

	switch (player->GetTeamNumber())
	{
		case TF_TEAM_RED: {
			uMask = CONTENTS_BLUETEAM;
			break;
		}

		case TF_TEAM_BLU: {
			uMask = CONTENTS_REDTEAM;
			break;
		}

		default: {
			break;
		}
	}

	return uMask | (brushOnly ? MASK_PLAYERSOLID_BRUSHONLY : MASK_PLAYERSOLID);
}

float GameMovement::ComputeConstraintSpeedFactor()
{
	if (!mv || mv->m_flConstraintRadius == 0.0f) {
		return 1.0f;
	}

	const float flDistSq{ mv->GetAbsOrigin().distToSqr(mv->m_vecConstraintCenter) };
	const float flOuterRadiusSq{ mv->m_flConstraintRadius * mv->m_flConstraintRadius };

	float flInnerRadiusSq{ mv->m_flConstraintRadius - mv->m_flConstraintWidth };

	flInnerRadiusSq *= flInnerRadiusSq;

	if ((flDistSq <= flInnerRadiusSq) || (flDistSq >= flOuterRadiusSq)) {
		return 1.0f;
	}

	Vector vecDesired{};

	VectorMultiply(m_vecForward, mv->m_flForwardMove, vecDesired);
	VectorMA(vecDesired, mv->m_flSideMove, m_vecRight, vecDesired);
	VectorMA(vecDesired, mv->m_flUpMove, m_vecUp, vecDesired);

	Vector vecDelta{};

	VectorSubtract(mv->GetAbsOrigin(), mv->m_vecConstraintCenter, vecDelta);
	VectorNormalize(vecDelta);
	VectorNormalize(vecDesired);

	if (DotProduct(vecDelta, vecDesired) < 0.0f) {
		return 1.0f;
	}

	const float flFrac{ (sqrtf(flDistSq) - (mv->m_flConstraintRadius - mv->m_flConstraintWidth)) / mv->m_flConstraintWidth };

	return Lerp(flFrac, 1.0f, mv->m_flConstraintSpeedFactor);
}

float GameMovement::CalcRoll(const QAngle &angles, const Vector &velocity, float rollangle, float rollspeed)
{
	float sign{};
	float side{};
	float value{};

	Vector forward{};
	Vector right{};
	Vector up{};

	AngleVectors(angles, &forward, &right, &up);

	side = DotProduct(velocity, right);
	sign = side < 0.0f ? -1.0f : 1.0f;
	side = fabsf(side);
	value = rollangle;

	if (side < rollspeed) {
		side = side * value / rollspeed;
	}

	else {
		side = value;
	}

	return side * sign;
}

void GameMovement::CheckParameters()
{
	QAngle v_angle{};

	if (player->GetMoveType() != MOVETYPE_ISOMETRIC && player->GetMoveType() != MOVETYPE_NOCLIP && player->GetMoveType() != MOVETYPE_OBSERVER)
	{
		float spd{ (mv->m_flForwardMove * mv->m_flForwardMove) + (mv->m_flSideMove * mv->m_flSideMove) + (mv->m_flUpMove * mv->m_flUpMove) };

		const float maxspeed{ mv->m_flClientMaxSpeed };

		if (maxspeed != 0.0f) {
			mv->m_flMaxSpeed = fminf(maxspeed, mv->m_flMaxSpeed);
		}

		float flSpeedFactor{ 1.0f };

		/*if (player->m_pSurfaceData) {
			flSpeedFactor = player->m_pSurfaceData->game.maxSpeedFactor;
		}*/

		const float flConstraintSpeedFactor{ ComputeConstraintSpeedFactor() };

		if (flConstraintSpeedFactor < flSpeedFactor) {
			flSpeedFactor = flConstraintSpeedFactor;
		}

		mv->m_flMaxSpeed *= flSpeedFactor;

		if (g_bMovementOptimizations)
		{
			if ((spd != 0.0f) && (spd > mv->m_flMaxSpeed * mv->m_flMaxSpeed))
			{
				const float fRatio{ mv->m_flMaxSpeed / sqrtf(spd) };

				mv->m_flForwardMove *= fRatio;
				mv->m_flSideMove *= fRatio;
				mv->m_flUpMove *= fRatio;
			}
		}

		else
		{
			spd = sqrtf(spd);

			if ((spd != 0.0f) && (spd > mv->m_flMaxSpeed))
			{
				const float fRatio{ mv->m_flMaxSpeed / spd };

				mv->m_flForwardMove *= fRatio;
				mv->m_flSideMove *= fRatio;
				mv->m_flUpMove *= fRatio;
			}
		}
	}

	if ((player->GetFlags() & FL_FROZEN) || (player->GetFlags() & FL_ONTRAIN) || IsDead()) {
		mv->m_flForwardMove = 0.0f;
		mv->m_flSideMove = 0.0f;
		mv->m_flUpMove = 0.0f;
	}

	//DecayPunchAngle();

	if (!IsDead())
	{
		v_angle = mv->m_vecAngles;
		v_angle = v_angle /*+ player->m_Local.m_vecPunchAngle*/;

		if ((player->GetMoveType() != MOVETYPE_ISOMETRIC) && (player->GetMoveType() != MOVETYPE_NOCLIP)) {
			mv->m_vecAngles[ROLL] = CalcRoll(v_angle, mv->m_vecVelocity, cvars::sv_rollangle->GetFloat(), cvars::sv_rollspeed->GetFloat());
		}

		else {
			mv->m_vecAngles[ROLL] = 0.0f;
		}

		mv->m_vecAngles[PITCH] = v_angle[PITCH];
		mv->m_vecAngles[YAW] = v_angle[YAW];
	}

	else {
		mv->m_vecAngles = mv->m_vecOldAngles;
	}
}

int GameMovement::GetPointContentsCached(const Vector &point, int slot)
{
	return i::trace->GetPointContents(point, nullptr);
}

bool GameMovement::CheckWater()
{
	const Vector vecPlayerMin{ GetPlayerMins() };
	const Vector vecPlayerMax{ GetPlayerMaxs() };

	Vector vecPoint
	{
		(mv->GetAbsOrigin().x + (vecPlayerMin.x + vecPlayerMax.x) * 0.5f),
		(mv->GetAbsOrigin().y + (vecPlayerMin.y + vecPlayerMax.y) * 0.5f),
		(mv->GetAbsOrigin().z + vecPlayerMin.z + 1.0f)
	};

	int wl{ WL_NotInWater };
	int wt{ CONTENTS_EMPTY };

	int nContents{ GetPointContentsCached(vecPoint, 0) };

	if (nContents & MASK_WATER)
	{
		wt = nContents;
		wl = WL_Feet;

		const float flWaistZ{ mv->GetAbsOrigin().z + (vecPlayerMin.z + vecPlayerMax.z) * 0.5f + 12.0f };

		vecPoint.z = mv->GetAbsOrigin().z + player->GetViewOffset()[2];

		nContents = GetPointContentsCached(vecPoint, 1);

		if (nContents & MASK_WATER) {
			wl = WL_Eyes;
			VectorCopy(vecPoint, m_vecWaterPoint);
			m_vecWaterPoint.z = flWaistZ;
		}

		else
		{
			vecPoint.z = flWaistZ;
			nContents = GetPointContentsCached(vecPoint, 2);

			if (nContents & MASK_WATER) {
				wl = WL_Waist;
				VectorCopy(vecPoint, m_vecWaterPoint);
			}
		}
	}

	if (player->InCond(TF_COND_SWIMMING_CURSE)) {
		wl = WL_Eyes;
	}

	player->SetWaterLevel(wl);
	player->SetWaterType(wt);

	if ((WL_NotInWater == m_nOldWaterLevel) && (wl > WL_NotInWater)) {
		m_flWaterEntryTime = i::global_vars->curtime;
	}

	/*if (m_nOldWaterLevel != wl) {
		m_pTFPlayer->TeamFortress_SetSpeed();
	}*/

	return wl > WL_Feet;
}

void GameMovement::SetGroundEntity(trace_t *pm)
{
	C_BaseEntity *const newGround{ pm ? pm->m_pEnt : nullptr };
	C_BaseEntity *const oldGround{ player->GetGroundEntity() };

	Vector vecBaseVelocity{ player->GetBaseVelocity() };

	if (!oldGround && newGround) {
		vecBaseVelocity -= newGround->GetAbsVelocity();
		vecBaseVelocity.z = newGround->GetAbsVelocity().z;
	}

	else if (oldGround && !newGround) {
		vecBaseVelocity += oldGround->GetAbsVelocity();
		vecBaseVelocity.z = oldGround->GetAbsVelocity().z;
	}

	player->SetBaseVelocity(vecBaseVelocity);
	player->SetGroundEntity(newGround);

	if (newGround)
	{
		//CategorizeGroundSurface(*pm);

		player->SetWaterJumpTime(0.0f);

		/*if (!pm->DidHitWorld()) {
			MoveHelper()->AddToTouched(*pm, mv->m_vecVelocity);
		}*/

		mv->m_vecVelocity.z = 0.0f;
	}
}

void GameMovement::TracePlayerBBoxForGround(const Vector &start, const Vector &end, const Vector &minsSrc, const Vector &maxsSrc, IHandleEntity *player, unsigned int fMask, int collisionGroup, trace_t &pm)
{
	Ray_t ray{};

	Vector mins{};
	Vector maxs{};

	float fraction{ pm.fraction };
	Vector endpos{ pm.endpos };

	mins = minsSrc;
	maxs.set(fminf(0, maxsSrc.x), fminf(0, maxsSrc.y), maxsSrc.z);

	ray.Init(start, end, mins, maxs);

	UTIL_TraceRay(ray, fMask, player, collisionGroup, &pm);

	if (pm.m_pEnt && pm.plane.normal[2] >= 0.7f) {
		pm.fraction = fraction;
		pm.endpos = endpos;
		return;
	}

	mins.set(fmaxf(0, minsSrc.x), fmaxf(0, minsSrc.y), minsSrc.z);
	maxs = maxsSrc;

	ray.Init(start, end, mins, maxs);

	UTIL_TraceRay(ray, fMask, player, collisionGroup, &pm);

	if (pm.m_pEnt && pm.plane.normal[2] >= 0.7f) {
		pm.fraction = fraction;
		pm.endpos = endpos;
		return;
	}

	mins.set(minsSrc.x, fmaxf(0, minsSrc.y), minsSrc.z);
	maxs.set(fminf(0, maxsSrc.x), maxsSrc.y, maxsSrc.z);

	ray.Init(start, end, mins, maxs);

	UTIL_TraceRay(ray, fMask, player, collisionGroup, &pm);

	if (pm.m_pEnt && pm.plane.normal[2] >= 0.7f) {
		pm.fraction = fraction;
		pm.endpos = endpos;
		return;
	}

	mins.set(fmaxf(0, minsSrc.x), minsSrc.y, minsSrc.z);
	maxs.set(maxsSrc.x, fminf(0, maxsSrc.y), maxsSrc.z);

	ray.Init(start, end, mins, maxs);

	UTIL_TraceRay(ray, fMask, player, collisionGroup, &pm);

	if (pm.m_pEnt && pm.plane.normal[2] >= 0.7f) {
		pm.fraction = fraction;
		pm.endpos = endpos;
		return;
	}

	pm.fraction = fraction;
	pm.endpos = endpos;
}

void GameMovement::CategorizePosition()
{
	if (player->IsObserver()) {
		return;
	}

	player->SetSurfaceFriction(1.0f);

	CheckWater();

	if (player->GetMoveType() == MOVETYPE_LADDER || mv->m_vecVelocity.z > 250.0f) {
		SetGroundEntity(nullptr);
		return;
	}

	const Vector vecStartPos{ mv->GetAbsOrigin() };

	Vector vecEndPos{ mv->GetAbsOrigin().x, mv->GetAbsOrigin().y, mv->GetAbsOrigin().z - 2.0f };

	const bool bUnderwater{ player->GetWaterLevel() >= WL_Eyes };

	bool bMoveToEndPos{};

	if (player->GetMoveType() == MOVETYPE_WALK && player->GetGroundEntity() != nullptr && !bUnderwater) {
		vecEndPos.z -= player->GetStepSize();
		bMoveToEndPos = true;
	}

	trace_t trace{};

	TracePlayerBBox(vecStartPos, vecEndPos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace);

	if (trace.plane.normal.z < 0.7f)
	{
		TracePlayerBBoxForGround(vecStartPos, vecEndPos, GetPlayerMins(), GetPlayerMaxs(), mv->m_nPlayerHandle.Get(), PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace);

		if (trace.plane.normal[2] < 0.7f)
		{
			SetGroundEntity(nullptr);

			if ((mv->m_vecVelocity.z > 0.0f) && (player->GetMoveType() != MOVETYPE_NOCLIP)) {
				player->SetSurfaceFriction(0.25f);
			}
		}

		else {
			SetGroundEntity(&trace);
		}
	}

	else
	{
		if (bMoveToEndPos && !trace.startsolid && trace.fraction > 0.0f && trace.fraction < 1.0f)
		{
			const float flDelta{ fabsf(mv->GetAbsOrigin().z - trace.endpos.z) };
			
			if (flDelta > 0.5f * COORD_RESOLUTION) {
				Vector org{ mv->GetAbsOrigin() };
				org.z = trace.endpos.z;
				mv->SetAbsOrigin(org);
			}
		}

		SetGroundEntity(&trace);
	}
}

void GameMovement::HandleDuckingSpeedCrop()
{
	if (!(m_iSpeedCropped & SPEED_CROPPED_DUCK) && (player->GetFlags() & FL_DUCKING) && (player->GetGroundEntity() != nullptr))
	{
		const float frac{ 0.33333333f };

		mv->m_flForwardMove *= frac;
		mv->m_flSideMove *= frac;
		mv->m_flUpMove *= frac;
	
		m_iSpeedCropped |= SPEED_CROPPED_DUCK;
	}
}

void GameMovement::Duck()
{
	if (mv->m_nButtons & IN_DUCK) {
		mv->m_nOldButtons |= IN_DUCK;
	}

	else {
		mv->m_nOldButtons &= ~IN_DUCK;
	}

	if (IsDead()) {
		return;
	}

	HandleDuckingSpeedCrop();
}

void GameMovement::StartGravity()
{
	float ent_gravity{};

	if (player->GetGravity()) {
		ent_gravity = player->GetGravity();
	}

	else {
		ent_gravity = 1.0f;
	}

	mv->m_vecVelocity[2] -= (ent_gravity * GetCurrentGravity() * 0.5f * i::global_vars->interval_per_tick);
	mv->m_vecVelocity[2] += player->GetBaseVelocity()[2] * i::global_vars->interval_per_tick;

	Vector temp{ player->GetBaseVelocity() };

	temp[2] = 0.0f;

	player->SetBaseVelocity(temp);
}

void GameMovement::WaterJump()
{
	if (player->GetWaterJumpTime() > 10000.0f) {
		player->SetWaterJumpTime(10000.0f);
	}

	if (!player->GetWaterJumpTime()) {
		return;
	}

	player->SetWaterJumpTime(player->GetWaterJumpTime() - 1000.0f * i::global_vars->interval_per_tick);

	if (player->GetWaterJumpTime() <= 0.0f || !player->GetWaterLevel()) {
		player->SetWaterJumpTime(0.0f);
		//player->RemoveFlag(FL_WATERJUMP);
	}

	mv->m_vecVelocity[0] = player->GetWaterJumpVel()[0];
	mv->m_vecVelocity[1] = player->GetWaterJumpVel()[1];
}

void GameMovement::TracePlayerBBox(const Vector &start, const Vector &end, unsigned int fMask, int collisionGroup, trace_t &pm)
{
	Ray_t ray{};

	ray.Init(start, end, GetPlayerMins(), GetPlayerMaxs());

	UTIL_TraceRay(ray, fMask, mv->m_nPlayerHandle.Get(), collisionGroup, &pm);
}

int GameMovement::ClipVelocity(Vector &in, Vector &normal, Vector &out, float overbounce)
{
	float backoff{};
	float change{};
	float angle{};
	int i{};
	int blocked{};

	angle = normal[2];

	blocked = 0x00;

	if (angle > 0.0f) {
		blocked |= 0x01;
	}

	if (!angle) {
		blocked |= 0x02;
	}

	backoff = DotProduct(in, normal) * overbounce;

	for (i = 0; i < 3; i++) {
		change = normal[i] * backoff;
		out[i] = in[i] - change;
	}

	const float adjust{ DotProduct(out, normal) };

	if (adjust < 0.0f) {
		out -= (normal * adjust);
	}

	return blocked;
}

int GameMovement::TryPlayerMove(Vector *pFirstDest, trace_t *pFirstTrace)
{
	int bumpcount{};
	int numbumps{};
	Vector dir{};
	float d{};
	int numplanes{};
	Vector planes[MAX_CLIP_PLANES]{};
	Vector primal_velocity{};
	Vector original_velocity{};
	Vector new_velocity{};
	int i{};
	int j{};
	trace_t	pm{};
	Vector end{};
	float time_left{};
	float allFraction{};
	int blocked{};

	numbumps = 4;
	blocked = 0;
	numplanes = 0;

	VectorCopy(mv->m_vecVelocity, original_velocity);
	VectorCopy(mv->m_vecVelocity, primal_velocity);

	allFraction = 0.0f;
	time_left = i::global_vars->interval_per_tick;

	new_velocity.set();

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		if (mv->m_vecVelocity.length() == 0.0f) {
			break;
		}

		VectorMA(mv->GetAbsOrigin(), time_left, mv->m_vecVelocity, end);

		if (g_bMovementOptimizations)
		{
			if (pFirstDest && (end.x == pFirstDest->x && end.y == pFirstDest->y && end.z == pFirstDest->z)) {
				pm = *pFirstTrace;
			}

			else {
				TracePlayerBBox(mv->GetAbsOrigin(), end, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);
			}
		}

		else {
			TracePlayerBBox(mv->GetAbsOrigin(), end, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);
		}

		allFraction += pm.fraction;

		if (pm.allsolid) {
			VectorCopy(vec3_origin, mv->m_vecVelocity);
			return 4;
		}

		if (pm.fraction > 0.0f)
		{
			if (numbumps > 0 && pm.fraction == 1.0f)
			{
				trace_t stuck{};

				TracePlayerBBox(pm.endpos, pm.endpos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, stuck);

				if (stuck.startsolid || (stuck.fraction != 1.0f)) {
					VectorCopy(vec3_origin, mv->m_vecVelocity);
					break;
				}
			}

			mv->SetAbsOrigin(pm.endpos);

			VectorCopy(mv->m_vecVelocity, original_velocity);

			numplanes = 0;
		}

		if (pm.fraction == 1.0f) {
			break;
		}

		//MoveHelper()->AddToTouched(pm, mv->m_vecVelocity);

		if (pm.plane.normal[2] > 0.7f) {
			blocked |= 1;
		}

		if (!pm.plane.normal[2]) {
			blocked |= 2;
		}

		time_left -= time_left * pm.fraction;

		if (numplanes >= MAX_CLIP_PLANES) {
			VectorCopy(vec3_origin, mv->m_vecVelocity);
			break;
		}

		VectorCopy(pm.plane.normal, planes[numplanes]);

		numplanes++;

		if (numplanes == 1 && (player->GetMoveType() == MOVETYPE_WALK) && (player->GetGroundEntity() == nullptr))
		{
			for (i = 0; i < numplanes; i++)
			{
				if (planes[i][2] > 0.7f) {
					ClipVelocity(original_velocity, planes[i], new_velocity, 1.0f);
					VectorCopy(new_velocity, original_velocity);
				}

				else {
					ClipVelocity(original_velocity, planes[i], new_velocity, 1.0f + cvars::sv_bounce->GetFloat() * (1.0f - player->GetSurfaceFriction()));
				}
			}

			VectorCopy(new_velocity, mv->m_vecVelocity);
			VectorCopy(new_velocity, original_velocity);
		}

		else
		{
			for (i = 0; i < numplanes; i++)
			{
				ClipVelocity(original_velocity, planes[i], mv->m_vecVelocity, 1.0f);

				for (j = 0; j < numplanes; j++)
				{
					if (j != i)
					{
						if (mv->m_vecVelocity.dot(planes[j]) < 0.0f) {
							break;
						}
					}
				}

				if (j == numplanes) {
					break;
				}
			}

			if (i != numplanes) {
				;
			}

			else
			{
				if (numplanes != 2) {
					VectorCopy(vec3_origin, mv->m_vecVelocity);
					break;
				}

				CrossProduct(planes[0], planes[1], dir);
				VectorNormalize(dir); //dir.normalizeInPlace();
				d = dir.dot(mv->m_vecVelocity);
				VectorScale(dir, d, mv->m_vecVelocity);
			}

			d = mv->m_vecVelocity.dot(primal_velocity);

			if (d <= 0.0f) {
				VectorCopy(vec3_origin, mv->m_vecVelocity);
				break;
			}
		}
	}

	if (allFraction == 0.0f) {
		VectorCopy(vec3_origin, mv->m_vecVelocity);
	}

	return blocked;
}

void GameMovement::WaterMove()
{
	float wishspeed{};
	Vector wishdir{};
	Vector start{};
	Vector dest{};
	Vector temp{};
	trace_t	pm{};
	float speed{};
	float newspeed{};
	float addspeed{};
	float accelspeed{};

	Vector vecForward{};
	Vector vecRight{};
	Vector vecUp{};

	AngleVectors(mv->m_vecViewAngles, &vecForward, &vecRight, &vecUp);

	Vector vecWishVelocity{};

	for (int iAxis{}; iAxis < 3; iAxis++) {
		vecWishVelocity[iAxis] = (vecForward[iAxis] * mv->m_flForwardMove) + (vecRight[iAxis] * mv->m_flSideMove);
	}

	int iCannotSwim{};

	//CALL_ATTRIB_HOOK_INT_ON_OTHER(m_pTFPlayer, iCannotSwim, cannot_swim);

	if (iCannotSwim) {
		vecWishVelocity[0] *= 0.1f;
		vecWishVelocity[1] *= 0.1f;
		vecWishVelocity[2] = -60.0f;
	}
	
	else if (mv->m_nButtons & IN_JUMP)
	{
		if (player->GetWaterLevel() == WL_Eyes) {
			vecWishVelocity[2] += mv->m_flClientMaxSpeed;
		}
	}
	
	else if (!mv->m_flForwardMove && !mv->m_flSideMove && !mv->m_flUpMove) {
		vecWishVelocity[2] -= 60.0f;
	}
	
	else {
		vecWishVelocity[2] += mv->m_flUpMove;
	}

	VectorCopy(vecWishVelocity, wishdir);

	wishspeed = VectorNormalize(wishdir);

	if (wishspeed > mv->m_flMaxSpeed) {
		VectorScale(vecWishVelocity, mv->m_flMaxSpeed / wishspeed, vecWishVelocity);
		wishspeed = mv->m_flMaxSpeed;
	}

	int iSwimmingMastery{};

	//CALL_ATTRIB_HOOK_INT_ON_OTHER(m_pTFPlayer, iSwimmingMastery, swimming_mastery);

	if (iSwimmingMastery == 0) {
		wishspeed *= 0.8f;
	}

	VectorCopy(mv->m_vecVelocity, temp);

	speed = VectorNormalize(temp);

	if (speed)
	{
		newspeed = speed - i::global_vars->interval_per_tick * speed * cvars::sv_friction->GetFloat() * player->GetSurfaceFriction();

		if (newspeed < 0.1f) {
			newspeed = 0.0f;
		}

		VectorScale(mv->m_vecVelocity, newspeed / speed, mv->m_vecVelocity);
	}

	else {
		newspeed = 0.0f;
	}

	if (player->InCond(TF_COND_HALLOWEEN_GHOST_MODE))
	{
		VectorNormalize(vecWishVelocity);

		accelspeed = cvars::sv_accelerate->GetFloat() * wishspeed * i::global_vars->interval_per_tick * player->GetSurfaceFriction();

		for (int i{}; i < 3; i++)
		{
			const float deltaSpeed{ accelspeed * vecWishVelocity[i] };

			mv->m_vecVelocity[i] += deltaSpeed;
			mv->m_outWishVel[i] += deltaSpeed;
		}

		const float flGhostXYSpeed{ mv->m_vecVelocity.length2D() };

		if (flGhostXYSpeed > cvars::tf_ghost_xy_speed->GetFloat())
		{
			const float flGhostXYSpeedScale{ cvars::tf_ghost_xy_speed->GetFloat() / flGhostXYSpeed };

			mv->m_vecVelocity.x *= flGhostXYSpeedScale;
			mv->m_vecVelocity.y *= flGhostXYSpeedScale;
		}
	}

	else if (wishspeed >= 0.1f)
	{
		addspeed = wishspeed - newspeed;

		if (addspeed > 0.0f)
		{
			VectorNormalize(vecWishVelocity);

			accelspeed = cvars::sv_accelerate->GetFloat() * wishspeed * i::global_vars->interval_per_tick * player->GetSurfaceFriction();

			if (accelspeed > addspeed) {
				accelspeed = addspeed;
			}

			for (int i{}; i < 3; i++)
			{
				const float deltaSpeed{ accelspeed * vecWishVelocity[i] };

				mv->m_vecVelocity[i] += deltaSpeed;
				mv->m_outWishVel[i] += deltaSpeed;
			}
		}
	}

	VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
	VectorMA(mv->GetAbsOrigin(), i::global_vars->interval_per_tick, mv->m_vecVelocity, dest);

	TracePlayerBBox(mv->GetAbsOrigin(), dest, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);

	if (pm.fraction == 1.0f)
	{
		VectorCopy(dest, start);

		if (player->AllowAutoMovement()) {
			start[2] += player->GetStepSize() + 1.0f;
		}

		TracePlayerBBox(start, dest, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, pm);

		if (!pm.startsolid && !pm.allsolid)
		{
			const float stepDist{ pm.endpos.z - mv->GetAbsOrigin().z };

			mv->m_outStepHeight += stepDist;	

			mv->SetAbsOrigin(pm.endpos);

			VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

			return;
		}

		TryPlayerMove();
	}

	else
	{
		if (!player->GetGroundEntity()) {
			TryPlayerMove();
			VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
			return;
		}

		StepMove(dest, pm);
	}

	VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
}

void GameMovement::CheckWaterJump()
{
	Vector flatforward{};
	Vector flatvelocity{};

	float curspeed{};

	bool bJump{ ((mv->m_nButtons & IN_JUMP) != 0) };

	Vector forward{};
	Vector right{};

	AngleVectors(mv->m_vecViewAngles, &forward, &right, nullptr);

	if (player->GetWaterJumpTime() || mv->m_vecVelocity[2] < -180.0f) {
		return;
	}

	flatvelocity[0] = mv->m_vecVelocity[0];
	flatvelocity[1] = mv->m_vecVelocity[1];
	flatvelocity[2] = 0.0f;

	curspeed = VectorNormalize(flatvelocity);

	for (int iAxis{}; iAxis < 2; iAxis++) {
		flatforward[iAxis] = forward[iAxis] * mv->m_flForwardMove + right[iAxis] * mv->m_flSideMove;
	}

	flatforward[2] = 0.0f;

	VectorNormalize(flatforward);

	if (curspeed != 0.0f && (DotProduct(flatvelocity, flatforward) < 0.0f) && !bJump) {
		return;
	}

	Vector vecStart{};
	vecStart = mv->GetAbsOrigin() + (GetPlayerMins() + GetPlayerMaxs()) * 0.5f;

	Vector vecEnd{};
	VectorMA(vecStart, TF_WATERJUMP_FORWARD, flatforward, vecEnd);

	trace_t tr{};
	TracePlayerBBox(vecStart, vecEnd, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, tr);

	if (tr.fraction < 1.0f)
	{
		/*IPhysicsObject *pPhysObj{ tr.m_pEnt->VPhysicsGetObject() };

		if (pPhysObj && (pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD)) {
			return;
		}*/

		vecStart.z = mv->GetAbsOrigin().z + player->GetViewOffset().z + WATERJUMP_HEIGHT;

		VectorMA(vecStart, TF_WATERJUMP_FORWARD, flatforward, vecEnd);
		VectorMA(vec3_origin, -50.0f, tr.plane.normal, player->GetWaterJumpVel());

		TracePlayerBBox(vecStart, vecEnd, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, tr);

		if (tr.fraction == 1.0)
		{
			VectorCopy(vecEnd, vecStart);

			vecEnd.z -= 1024.0f;

			TracePlayerBBox(vecStart, vecEnd, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, tr);

			if ((tr.fraction < 1.0f) && (tr.plane.normal.z >= 0.7f)) {
				mv->m_vecVelocity[2] = TF_WATERJUMP_UP;
				mv->m_nOldButtons |= IN_JUMP;
				//player->AddFlag(FL_WATERJUMP);
				player->SetWaterJumpTime(2000.0f);
			}
		}
	}
}

void GameMovement::FullWalkMoveUnderwater()
{
	if (player->GetWaterLevel() == WL_Waist) {
		CheckWaterJump();
	}

	if ((mv->m_vecVelocity.z < 0.0f) && player->GetWaterJumpTime()) {
		player->SetWaterJumpTime(0.0f);
	}

	if (mv->m_nButtons & IN_JUMP) {
		//CheckJumpButton();
	}

	else {
		mv->m_nOldButtons &= ~IN_JUMP;
	}

	WaterMove();

	CategorizePosition();

	if (player->GetGroundEntity() != nullptr) {
		mv->m_vecVelocity[2] = 0.0f;
	}
}

void GameMovement::CheckVelocity()
{
	int i{};

	Vector org{ mv->GetAbsOrigin() };

	for (i = 0; i < 3; i++)
	{
		if (IS_NAN(mv->m_vecVelocity[i])) {
			mv->m_vecVelocity[i] = 0.0f;
		}

		if (IS_NAN(org[i])) {
			org[i] = 0.0f;
			mv->SetAbsOrigin(org);
		}

		if (mv->m_vecVelocity[i] > cvars::sv_maxvelocity->GetFloat()) {
			mv->m_vecVelocity[i] = cvars::sv_maxvelocity->GetFloat();
		}

		else if (mv->m_vecVelocity[i] < -cvars::sv_maxvelocity->GetFloat()) {
			mv->m_vecVelocity[i] = -cvars::sv_maxvelocity->GetFloat();
		}
	}
}

void GameMovement::Friction()
{
	float speed{};
	float newspeed{};
	float control{};
	float friction{};
	float drop{};

	if (player->GetWaterJumpTime()) {
		return;
	}

	speed = VectorLength(mv->m_vecVelocity);

	if (speed < 0.1f) {
		return;
	}

	drop = 0.0f;

	if (player->GetGroundEntity() != nullptr) {
		friction = cvars::sv_friction->GetFloat() * player->GetSurfaceFriction();
		control = (speed < cvars::sv_stopspeed->GetFloat()) ? cvars::sv_stopspeed->GetFloat() : speed;
		drop += control * friction * i::global_vars->interval_per_tick;
	}

	newspeed = speed - drop;

	if (newspeed < 0.0f) {
		newspeed = 0.0f;
	}

	if (newspeed != speed) {
		newspeed /= speed;
		VectorScale(mv->m_vecVelocity, newspeed, mv->m_vecVelocity);
	}

	mv->m_outWishVel -= mv->m_vecVelocity * (1.0f - newspeed);
}

float GameMovement::CalcWishSpeedThreshold()
{
	return 100.0f * cvars::sv_friction->GetFloat() / (cvars::sv_accelerate->GetFloat());
}

bool GameMovement::CanAccelerate()
{
	if (!player->IsAlive() || player->GetWaterJumpTime()) {
		return false;
	}

	return true;
}

void GameMovement::Accelerate(Vector &wishdir, float wishspeed, float accel)
{
	int i{};

	float addspeed{};
	float accelspeed{};
	float currentspeed{};

	if (!CanAccelerate()) {
		return;
	}

	currentspeed = mv->m_vecVelocity.dot(wishdir);
	addspeed = wishspeed - currentspeed;

	if (addspeed <= 0.0f) {
		return;
	}

	accelspeed = accel * i::global_vars->interval_per_tick * wishspeed * player->GetSurfaceFriction();

	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}

	for (i = 0; i < 3; i++) {
		mv->m_vecVelocity[i] += accelspeed * wishdir[i];
	}
}

void GameMovement::StepMove(Vector &vecDestination, trace_t &trace)
{
	trace_t saveTrace{};

	saveTrace = trace;

	Vector vecEndPos{};
	VectorCopy(vecDestination, vecEndPos);

	Vector vecPos{};
	Vector vecVel{};

	VectorCopy(mv->GetAbsOrigin(), vecPos);
	VectorCopy(mv->m_vecVelocity, vecVel);

	bool bLowRoad{ false };
	bool bUpRoad{ true };

	if (player->AllowAutoMovement())
	{
		VectorCopy(mv->GetAbsOrigin(), vecEndPos);

		vecEndPos.z += player->GetStepSize() + DIST_EPSILON;

		TracePlayerBBox(mv->GetAbsOrigin(), vecEndPos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace);

		if (!trace.startsolid && !trace.allsolid) {
			mv->SetAbsOrigin(trace.endpos);
		}

		TryPlayerMove();

		VectorCopy(mv->GetAbsOrigin(), vecEndPos);

		vecEndPos.z -= player->GetStepSize() + DIST_EPSILON;

		TracePlayerBBox(mv->GetAbsOrigin(), vecEndPos, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace);
		
		if (!trace.startsolid && !trace.allsolid) {
			mv->SetAbsOrigin(trace.endpos);
		}

		if ((trace.fraction != 1.0f && (trace.plane.normal[2] < 0.7f)) || VectorCompare(mv->GetAbsOrigin(), vecPos)) {
			bLowRoad = true;
			bUpRoad = false;
		}
	}

	else {
		bLowRoad = true;
		bUpRoad = false;
	}

	if (bLowRoad)
	{
		Vector vecUpPos{};
		Vector vecUpVel{};

		if (bUpRoad) {
			VectorCopy(mv->GetAbsOrigin(), vecUpPos);
			VectorCopy(mv->m_vecVelocity, vecUpVel);
		}

		mv->SetAbsOrigin(vecPos);

		VectorCopy(vecVel, mv->m_vecVelocity);
		VectorCopy(vecDestination, vecEndPos);

		TryPlayerMove(&vecEndPos, &saveTrace);

		Vector vecDownPos{};
		Vector vecDownVel{};

		VectorCopy(mv->GetAbsOrigin(), vecDownPos);
		VectorCopy(mv->m_vecVelocity, vecDownVel);

		if (bUpRoad)
		{
			const float flUpDist{ (vecUpPos.x - vecPos.x) * (vecUpPos.x - vecPos.x) + (vecUpPos.y - vecPos.y) * (vecUpPos.y - vecPos.y) };
			const float flDownDist{ (vecDownPos.x - vecPos.x) * (vecDownPos.x - vecPos.x) + (vecDownPos.y - vecPos.y) * (vecDownPos.y - vecPos.y) };

			if (flUpDist >= flDownDist) {
				mv->SetAbsOrigin(vecUpPos);
				VectorCopy(vecUpVel, mv->m_vecVelocity);
				mv->m_vecVelocity.z = vecDownVel.z;
			}
		}
	}

	const float flStepDist{ mv->GetAbsOrigin().z - vecPos.z };

	if (flStepDist > 0.0f) {
		mv->m_outStepHeight += flStepDist;
	}
}

void GameMovement::WalkMove()
{
	Vector vecForward{};
	Vector vecRight{};
	Vector vecUp{};

	AngleVectors(mv->m_vecViewAngles, &vecForward, &vecRight, &vecUp);

	vecForward.z = 0.0f;
	vecRight.z = 0.0f;

	VectorNormalize(vecForward);
	VectorNormalize(vecRight);

	float flForwardMove{ mv->m_flForwardMove };
	float flSideMove{ mv->m_flSideMove };

	Vector vecWishDirection{ ((vecForward.x * flForwardMove) + (vecRight.x * flSideMove)), ((vecForward.y * flForwardMove) + (vecRight.y * flSideMove)), 0.0f };

	float flWishSpeed{ VectorNormalize(vecWishDirection) };

	flWishSpeed = Clamp(flWishSpeed, 0.0f, mv->m_flMaxSpeed);

	mv->m_vecVelocity.z = 0.0f;

	float flAccelerate{ cvars::sv_accelerate->GetFloat() };

	if (flWishSpeed > 0.0f && flWishSpeed < CalcWishSpeedThreshold())
	{
		const float flSpeed{ VectorLength(mv->m_vecVelocity) };
		const float flControl{ (flSpeed < cvars::sv_stopspeed->GetFloat()) ? cvars::sv_stopspeed->GetFloat() : flSpeed };

		flAccelerate = (flControl * cvars::sv_friction->GetFloat()) / flWishSpeed + 1.0f;
	}

	Accelerate(vecWishDirection, flWishSpeed, flAccelerate);

	float flNewSpeed{ VectorLength(mv->m_vecVelocity) };

	if (flNewSpeed > mv->m_flMaxSpeed)
	{
		float flScale{ (mv->m_flMaxSpeed / flNewSpeed) };

		mv->m_vecVelocity.x *= flScale;
		mv->m_vecVelocity.y *= flScale;
	}

	const float flForwardPull{}; //m_pTFPlayer->GetMovementForwardPull()

	if (flForwardPull > 0.0f)
	{
		mv->m_vecVelocity += vecForward * flForwardPull;

		if (mv->m_vecVelocity.length2D() > mv->m_flMaxSpeed) {
			VectorNormalize(mv->m_vecVelocity);
			mv->m_vecVelocity *= mv->m_flMaxSpeed;
		}
	}

	if (cvars::tf_clamp_back_speed->GetFloat() < 1.0f && VectorLength(mv->m_vecVelocity) > cvars::tf_clamp_back_speed_min->GetFloat())
	{
		const float flDot{ DotProduct(vecForward, mv->m_vecVelocity) };

		if (flDot < 0.0f)
		{
			Vector vecBackMove{ vecForward * flDot };
			Vector vecRightMove{ vecRight * DotProduct(vecRight, mv->m_vecVelocity) };

			const float flBackSpeed{ VectorLength(vecBackMove) };
			const float flMaxBackSpeed{ (mv->m_flMaxSpeed * cvars::tf_clamp_back_speed->GetFloat()) };

			if (flBackSpeed > flMaxBackSpeed) {
				vecBackMove *= flMaxBackSpeed / flBackSpeed;
			}

			mv->m_vecVelocity = vecBackMove + vecRightMove;

			flNewSpeed = VectorLength(mv->m_vecVelocity);

			if (flNewSpeed > mv->m_flMaxSpeed)
			{
				const float flScale{ (mv->m_flMaxSpeed / flNewSpeed) };

				mv->m_vecVelocity.x *= flScale;
				mv->m_vecVelocity.y *= flScale;
			}
		}
	}

	VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

	const float flSpeed = VectorLength(mv->m_vecVelocity);

	if (flSpeed < 1.0f) {
		mv->m_vecVelocity.set();
		return;
	}

	Vector vecDestination{};

	vecDestination.x = mv->GetAbsOrigin().x + (mv->m_vecVelocity.x * i::global_vars->interval_per_tick);
	vecDestination.y = mv->GetAbsOrigin().y + (mv->m_vecVelocity.y * i::global_vars->interval_per_tick);
	vecDestination.z = mv->GetAbsOrigin().z;

	trace_t trace{};

	TracePlayerBBox(mv->GetAbsOrigin(), vecDestination, PlayerSolidMask(), COLLISION_GROUP_PLAYER_MOVEMENT, trace);

	if (trace.fraction == 1.0f) {
		mv->SetAbsOrigin(trace.endpos);
		VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
		mv->m_outWishVel += (vecWishDirection * flWishSpeed);
		return;
	}

	StepMove(vecDestination, trace);

	const Vector baseVelocity{ player->GetBaseVelocity() };

	VectorSubtract(mv->m_vecVelocity, baseVelocity, mv->m_vecVelocity);

	//CheckKartWallBumping();

	mv->m_outWishVel += (vecWishDirection * flWishSpeed);
}

float GameMovement::GetAirSpeedCap()
{
	return 30.0f;
}

void GameMovement::AirAccelerate(Vector &wishdir, float wishspeed, float accel)
{
	int i{};

	float addspeed{};
	float accelspeed{};
	float currentspeed{};
	float wishspd{};

	wishspd = wishspeed;

	if (!player->IsAlive() || player->GetWaterJumpTime()) {
		return;
	}

	if (wishspd > GetAirSpeedCap()) {
		wishspd = GetAirSpeedCap();
	}

	currentspeed = mv->m_vecVelocity.dot(wishdir);
	addspeed = wishspd - currentspeed;

	if (addspeed <= 0.0f) {
		return;
	}

	accelspeed = accel * wishspeed * i::global_vars->interval_per_tick * player->GetSurfaceFriction();

	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}

	for (i = 0; i < 3; i++) {
		mv->m_vecVelocity[i] += accelspeed * wishdir[i];
		mv->m_outWishVel[i] += accelspeed * wishdir[i];
	}
}

void GameMovement::AirMove()
{
	//s: some grappling hook shit i skipped here:
	//...

	int i{};
	Vector wishvel{};
	float fmove{};
	float smove{};
	Vector wishdir{};
	float wishspeed{};
	Vector forward{};
	Vector right{};
	Vector up{};

	AngleVectors(mv->m_vecViewAngles, &forward, &right, &up);

	fmove = mv->m_flForwardMove;
	smove = mv->m_flSideMove;

	forward[2] = 0.0f;
	right[2] = 0.0f;

	VectorNormalize(forward);
	VectorNormalize(right);

	for (i = 0; i < 2; i++) {
		wishvel[i] = forward[i] * fmove + right[i] * smove;
	}

	wishvel[2] = 0.0f;

	VectorCopy(wishvel, wishdir);

	wishspeed = VectorNormalize(wishdir);

	if (wishspeed != 0.0f && (wishspeed > mv->m_flMaxSpeed)) {
		VectorScale(wishvel, mv->m_flMaxSpeed / wishspeed, wishvel);
		wishspeed = mv->m_flMaxSpeed;
	}

	AirAccelerate(wishdir, wishspeed, cvars::sv_airaccelerate->GetFloat());

	float flForwardPull{}; //m_pTFPlayer->GetMovementForwardPull()

	if (flForwardPull > 0.0f)
	{
		mv->m_vecVelocity += forward * flForwardPull;

		if (mv->m_vecVelocity.length2D() > mv->m_flMaxSpeed)
		{
			const float flZ{ mv->m_vecVelocity.z };
			mv->m_vecVelocity.z = 0.0f;
			VectorNormalize(mv->m_vecVelocity);
			mv->m_vecVelocity *= mv->m_flMaxSpeed;
			mv->m_vecVelocity.z = flZ;
		}
	}

	VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

	const int iBlocked{ TryPlayerMove() };

	if (iBlocked & 2) {
		//CheckKartWallBumping();
	}

	VectorSubtract(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);
}

void GameMovement::FinishGravity()
{
	float ent_gravity{};

	if (player->GetWaterJumpTime()) {
		return;
	}

	if (player->GetGravity()) {
		ent_gravity = player->GetGravity();
	}

	else {
		ent_gravity = 1.0f;
	}

	mv->m_vecVelocity[2] -= (ent_gravity * GetCurrentGravity() * i::global_vars->interval_per_tick * 0.5f);

	CheckVelocity();
}

void GameMovement::FullWalkMove()
{
	if (!InWater())
	{
		if (player->InCond(TF_COND_PARACHUTE_DEPLOYED) && mv->m_vecVelocity[2] < 0.0f)
		{
			mv->m_vecVelocity[2] = Max(mv->m_vecVelocity[2], cvars::tf_parachute_maxspeed_z->GetFloat());

			const float flDrag{ cvars::tf_parachute_maxspeed_xy->GetFloat() };
			const float flSpeedX{ fabsf(mv->m_vecVelocity[0]) };
			const float flSpeedY{ fabsf(mv->m_vecVelocity[1]) };
			const float flReductionX{ flSpeedX > flDrag ? (flSpeedX - flDrag) / 3.0f - 10.0f : 0.0f };
			const float flReductionY{ flSpeedY > flDrag ? (flSpeedY - flDrag) / 3.0f - 10.0f : 0.0f };

			mv->m_vecVelocity[0] = Clamp(mv->m_vecVelocity[0], -flDrag - flReductionX, flDrag + flReductionX);
			mv->m_vecVelocity[1] = Clamp(mv->m_vecVelocity[1], -flDrag - flReductionY, flDrag + flReductionY);
		}

		StartGravity();
	}

	if (player->GetWaterJumpTime()) {
		WaterJump();
		TryPlayerMove();
		CheckWater();
		return;
	}

	if (InWater() || (player && (player->InCond(TF_COND_HALLOWEEN_GHOST_MODE) || player->InCond(TF_COND_SWIMMING_NO_EFFECTS)))) {
		FullWalkMoveUnderwater();
		return;
	}

	if (mv->m_nButtons & IN_JUMP) {
		//CheckJumpButton();
	}

	else {
		mv->m_nOldButtons &= ~IN_JUMP;
	}

	CheckVelocity();

	if (player->GetGroundEntity() != nullptr) {
		mv->m_vecVelocity[2] = 0.0f;
		Friction();
		WalkMove();
	}

	else {
		AirMove();
	}

	CategorizePosition();

	if (!InWater()) {
		FinishGravity();
	}

	if (player->GetGroundEntity() != nullptr) {
		mv->m_vecVelocity[2] = 0.0f;
	}

	//CheckFalling();

	CheckVelocity();
}

void GameMovement::FullNoClipMove(float factor, float maxacceleration)
{
	Vector wishvel{};
	Vector forward{};
	Vector right{};
	Vector up{};
	Vector wishdir{};
	float wishspeed{};

	const float maxspeed{ cvars::sv_maxspeed->GetFloat() * factor };

	AngleVectors(mv->m_vecViewAngles, &forward, &right, &up);

	if (mv->m_nButtons & IN_SPEED) {
		factor /= 2.0f;
	}

	const float fmove{ mv->m_flForwardMove * factor };
	const float smove{ mv->m_flSideMove * factor };

	VectorNormalize(forward);
	VectorNormalize(right);

	for (int i{}; i < 3; i++) {
		wishvel[i] = forward[i] * fmove + right[i] * smove;
	}

	wishvel[2] += mv->m_flUpMove * factor;

	VectorCopy(wishvel, wishdir);

	wishspeed = VectorNormalize(wishdir);

	if (wishspeed > maxspeed) {
		VectorScale(wishvel, maxspeed / wishspeed, wishvel);
		wishspeed = maxspeed;
	}

	if (maxacceleration > 0.0)
	{
		Accelerate(wishdir, wishspeed, maxacceleration);

		const float spd{ VectorLength(mv->m_vecVelocity) };

		if (spd < 1.0f) {
			mv->m_vecVelocity.set();
			return;
		}

		const float control{ (spd < maxspeed / 4.0f) ? maxspeed / 4.0f : spd };
		const float friction{ cvars::sv_friction->GetFloat() * player->GetSurfaceFriction() };
		const float drop{ control * friction * i::global_vars->interval_per_tick };

		float newspeed{ spd - drop };

		if (newspeed < 0.0f) {
			newspeed = 0.0f;
		}

		newspeed /= spd;

		VectorScale(mv->m_vecVelocity, newspeed, mv->m_vecVelocity);
	}

	else {
		VectorCopy(wishvel, mv->m_vecVelocity);
	}

	Vector out{};

	VectorMA(mv->GetAbsOrigin(), i::global_vars->interval_per_tick, mv->m_vecVelocity, out);

	mv->SetAbsOrigin(out);

	if (maxacceleration < 0.0f) {
		mv->m_vecVelocity.set();
	}
}

void GameMovement::PlayerMove()
{
	CheckParameters();

	mv->m_outWishVel.set();
	mv->m_outJumpVel.set();

	//MoveHelper()->ResetTouchList();

	//ReduceTimers();

	AngleVectors(mv->m_vecViewAngles, &m_vecForward, &m_vecRight, &m_vecUp);

	/*if ((player->GetMoveType() != MOVETYPE_NOCLIP)
		&& (player->GetMoveType() != MOVETYPE_NONE)
		&& (player->GetMoveType() != MOVETYPE_ISOMETRIC)
		&& (player->GetMoveType() != MOVETYPE_OBSERVER)
		&& player->IsAlive())
	{
		if (CheckInterval(STUCK) && CheckStuck()) {
			return;
		}
	}*/

	//normie: idgaf if this shit "faithful" to game code m_bGameCodeMovedPlayer will always be fucking false!
	if ((player->GetMoveType() != MOVETYPE_WALK) || !cvars::sv_optimizedmovement->GetBool() /*|| mv->m_bGameCodeMovedPlayer*/) {
		CategorizePosition();
	}

	else
	{
		if (mv->m_vecVelocity.z > 250.0f) {
			SetGroundEntity(nullptr);
		}
	}

	m_nOldWaterLevel = player->GetWaterLevel();

	/*if (player->GetGroundEntity() == nullptr) {
		player->m_Local.m_flFallVelocity = -mv->m_vecVelocity[2];
	}*/

	//m_nOnLadder = 0;

	//player->UpdateStepSound(player->m_pSurfaceData, mv->GetAbsOrigin(), mv->m_vecVelocity);

	//UpdateDuckJumpEyeOffset();

	Duck();
	
	//s: some fucking ladder code i skipped:
	//...

	switch (player->GetMoveType())
	{
		case MOVETYPE_WALK: {
			FullWalkMove();
			break;
		}

		case MOVETYPE_NOCLIP: {
			FullNoClipMove(cvars::sv_noclipspeed->GetFloat(), cvars::sv_noclipaccelerate->GetFloat());
			break;
		}

		default: {
			break;
		}
	}
}

void GameMovement::FinishMove()
{
	mv->m_nOldButtons = mv->m_nButtons;
	mv->m_flOldForwardMove = mv->m_flForwardMove;
}

void GameMovement::ChargeMove()
{
	if (!player->InCond(TF_COND_SHIELD_CHARGE))
	{
		if (player->GetClass() != TF_CLASS_MEDIC) {
			return;
		}

		C_TFWeaponBase *const pTFWeapon{ player->GetActiveTFWeapon() };

		if (!pTFWeapon || pTFWeapon->GetWeaponID() != TF_WEAPON_MEDIGUN) {
			return;
		}

		C_WeaponMedigun *const pMedigun{ pTFWeapon->cast<C_WeaponMedigun>() };

		if (!pMedigun || pMedigun->GetMedigunType() != MEDIGUN_QUICKFIX) {
			return;
		}

		C_TFPlayer *const pHealTarget{ pMedigun->m_hHealingTarget().Get()->cast<C_TFPlayer>() };

		if (!pHealTarget || !pHealTarget->InCond(TF_COND_SHIELD_CHARGE)) {
			return;
		}
	}

	mv->m_flMaxSpeed = cvars::tf_max_charge_speed->GetFloat();

	const int oldbuttons{ mv->m_nButtons };

	mv->m_flForwardMove = cvars::tf_max_charge_speed->GetFloat();
	mv->m_flSideMove = 0.0f;
	mv->m_flUpMove = 0.0f;

	if (mv->m_nButtons & IN_ATTACK2) {
		mv->m_nButtons = IN_ATTACK2;
	}

	else {
		mv->m_nButtons = 0;
	}

	if (oldbuttons & IN_ATTACK) {
		mv->m_nButtons |= IN_ATTACK;
	}
}

void GameMovement::StunMove()
{
	//TODO:
}

void GameMovement::TauntMove()
{
	//TODO:
}

void GameMovement::GrapplingHookMove()
{
	//TODO:
}

void GameMovement::HighMaxSpeedMove()
{
	if (fabsf(mv->m_flForwardMove) < player->MaxSpeed())
	{
		if (AlmostEqual(mv->m_flForwardMove, cvars::cl_forwardspeed->GetFloat())) {
			mv->m_flForwardMove = player->MaxSpeed();
		}

		else if (AlmostEqual(mv->m_flForwardMove, -cvars::cl_backspeed->GetFloat())) {
			mv->m_flForwardMove = -player->MaxSpeed();
		}
	}

	if (fabsf(mv->m_flSideMove) < player->MaxSpeed())
	{
		if (AlmostEqual(mv->m_flSideMove, cvars::cl_sidespeed->GetFloat())) {
			mv->m_flSideMove = player->MaxSpeed();
		}

		else if (AlmostEqual(mv->m_flSideMove, -cvars::cl_sidespeed->GetFloat())) {
			mv->m_flSideMove = -player->MaxSpeed();
		}
	}
}

void GameMovement::PartialWalkMove()
{
	Vector vecForward{};
	Vector vecRight{};
	Vector vecUp{};

	AngleVectors(mv->m_vecViewAngles, &vecForward, &vecRight, &vecUp);

	vecForward.z = 0.0f;
	vecRight.z = 0.0f;

	VectorNormalize(vecForward);
	VectorNormalize(vecRight);

	float flForwardMove{ mv->m_flForwardMove };
	float flSideMove{ mv->m_flSideMove };

	Vector vecWishDirection{ ((vecForward.x * flForwardMove) + (vecRight.x * flSideMove)), ((vecForward.y * flForwardMove) + (vecRight.y * flSideMove)), 0.0f };

	float flWishSpeed{ VectorNormalize(vecWishDirection) };

	flWishSpeed = Clamp(flWishSpeed, 0.0f, mv->m_flMaxSpeed);

	mv->m_vecVelocity.z = 0.0f;

	float flAccelerate{ cvars::sv_accelerate->GetFloat() };

	if (flWishSpeed > 0.0f && flWishSpeed < CalcWishSpeedThreshold())
	{
		const float flSpeed{ VectorLength(mv->m_vecVelocity) };
		const float flControl{ (flSpeed < cvars::sv_stopspeed->GetFloat()) ? cvars::sv_stopspeed->GetFloat() : flSpeed };

		flAccelerate = (flControl * cvars::sv_friction->GetFloat()) / flWishSpeed + 1.0f;
	}

	Accelerate(vecWishDirection, flWishSpeed, flAccelerate);

	const float flNewSpeed{ VectorLength(mv->m_vecVelocity) };

	if (flNewSpeed > mv->m_flMaxSpeed)
	{
		const float flScale{ (mv->m_flMaxSpeed / flNewSpeed) };

		mv->m_vecVelocity.x *= flScale;
		mv->m_vecVelocity.y *= flScale;
	}

	VectorAdd(mv->m_vecVelocity, player->GetBaseVelocity(), mv->m_vecVelocity);

	const float flSpeed = VectorLength(mv->m_vecVelocity);

	if (flSpeed < 1.0f) {
		mv->m_vecVelocity.set();
		return;
	}
}

void GameMovement::ProcessMovement(TFPlayerProxy *const pl, CMoveData *const move)
{
	if (!pl || !move) {
		return;
	}

	m_iSpeedCropped = SPEED_CROPPED_RESET;

	mv->m_flMaxSpeed = TF_MAX_SPEED;

	//s: HandleDuckingSpeedCrop mults these by 0.33333333, if we sim for more than a tick it becomes an issue
	//store these off and restore at the end!
	const float fmove{ mv->m_flForwardMove };
	const float smove{ mv->m_flSideMove };
	const float umove{ mv->m_flUpMove };

	ChargeMove();
	StunMove();
	TauntMove();
	GrapplingHookMove();
	HighMaxSpeedMove();

	PlayerMove();
	FinishMove();

	move->m_flForwardMove = fmove;
	move->m_flSideMove = smove;
	move->m_flUpMove = umove;
}