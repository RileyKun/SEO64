#pragma once

#include "tf_player_proxy.hpp"

class GameMovement final
{
public:
	TFPlayerProxy *player{};
	CMoveData *mv{};

private:
	int m_iSpeedCropped{};
	Vector m_vecForward{};
	Vector m_vecRight{};
	Vector m_vecUp{};
	int m_nOldWaterLevel{};
	Vector m_vecWaterPoint{};
	float m_flWaterEntryTime{};

public:
	bool IsDead();
	bool InWater();
	Vector GetPlayerMins();
	Vector GetPlayerMaxs();
	float GetCurrentGravity();
	unsigned int PlayerSolidMask(bool brushOnly = false);
	float ComputeConstraintSpeedFactor();
	float CalcRoll(const QAngle &angles, const Vector &velocity, float rollangle, float rollspeed);
	void CheckParameters();
	int GetPointContentsCached(const Vector &point, int slot);
	bool CheckWater();
	void SetGroundEntity(trace_t *pm);
	void TracePlayerBBoxForGround(const Vector &start, const Vector &end, const Vector &minsSrc, const Vector &maxsSrc, IHandleEntity *player, unsigned int fMask, int collisionGroup, trace_t &pm);
	void CategorizePosition();
	void HandleDuckingSpeedCrop();
	void Duck();
	void StartGravity();
	void WaterJump();
	void TracePlayerBBox(const Vector &start, const Vector &end, unsigned int fMask, int collisionGroup, trace_t &pm);
	int ClipVelocity(Vector &in, Vector &normal, Vector &out, float overbounce);
	int TryPlayerMove(Vector *pFirstDest = nullptr, trace_t *pFirstTrace = nullptr);
	void WaterMove();
	void CheckWaterJump();
	void FullWalkMoveUnderwater();
	void CheckVelocity();
	void Friction();
	float CalcWishSpeedThreshold();
	bool CanAccelerate();
	void Accelerate(Vector &wishdir, float wishspeed, float accel);
	void StepMove(Vector &vecDestination, trace_t &trace);
	void WalkMove();
	float GetAirSpeedCap();
	void AirAccelerate(Vector &wishdir, float wishspeed, float accel);
	void AirMove();
	void FinishGravity();
	void FullWalkMove();
	void FullNoClipMove(float factor, float maxacceleration);
	void PlayerMove();
	void FinishMove();
	void ChargeMove();
	void StunMove();
	void TauntMove();
	void GrapplingHookMove();
	void HighMaxSpeedMove();
	void PartialWalkMove();

public:
	void ProcessMovement(TFPlayerProxy *const pl, CMoveData *const move);
};