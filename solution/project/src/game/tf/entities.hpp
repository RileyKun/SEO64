#pragma once

#include "netvars.hpp"

class IClientUnknown : public IHandleEntity
{
public:
	virtual ICollideable *GetCollideable() = 0;
	virtual IClientNetworkable *GetClientNetworkable() = 0;
	virtual IClientRenderable *GetClientRenderable() = 0;
	virtual IClientEntity *GetIClientEntity() = 0;
	virtual C_BaseEntity *GetBaseEntity() = 0;
	virtual IClientThinkable *GetClientThinkable() = 0;
};

class IClientRenderable
{
public:
	virtual IClientUnknown *GetIClientUnknown() = 0;
	virtual Vector const &GetRenderOrigin(void) = 0;
	virtual QAngle const &GetRenderAngles(void) = 0;
	virtual bool ShouldDraw(void) = 0;
	virtual bool IsTransparent(void) = 0;
	virtual bool UsesPowerOfTwoFrameBufferTexture() = 0;
	virtual bool UsesFullFrameBufferTexture() = 0;
	virtual ClientShadowHandle_t GetShadowHandle() const = 0;
	virtual ClientRenderHandle_t &RenderHandle() = 0;
	virtual const model_t *GetModel() const = 0;
	virtual int DrawModel(int flags) = 0;
	virtual int GetBody() = 0;
	virtual void ComputeFxBlend() = 0;
	virtual int GetFxBlend(void) = 0;
	virtual void GetColorModulation(float *color) = 0;
	virtual bool LODTest() = 0;
	virtual bool SetupBones(matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) = 0;
	virtual void SetupWeights(const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights) = 0;
	virtual void DoAnimationEvents(void) = 0;
	virtual IPVSNotify *GetPVSNotifyInterface() = 0;
	virtual void GetRenderBounds(Vector &mins, Vector &maxs) = 0;
	virtual void GetRenderBoundsWorldspace(Vector &mins, Vector &maxs) = 0;
	virtual void GetShadowRenderBounds(Vector &mins, Vector &maxs, ShadowType_t shadowType) = 0;
	virtual bool ShouldReceiveProjectedTextures(int flags) = 0;
	virtual bool GetShadowCastDistance(float *pDist, ShadowType_t shadowType) const = 0;
	virtual bool GetShadowCastDirection(Vector *pDirection, ShadowType_t shadowType) const = 0;
	virtual bool IsShadowDirty() = 0;
	virtual void MarkShadowDirty(bool bDirty) = 0;
	virtual IClientRenderable *GetShadowParent() = 0;
	virtual IClientRenderable *FirstShadowChild() = 0;
	virtual IClientRenderable *NextShadowPeer() = 0;
	virtual ShadowType_t ShadowCastType() = 0;
	virtual void CreateModelInstance() = 0;
	virtual ModelInstanceHandle_t GetModelInstance() = 0;
	virtual const matrix3x4_t &RenderableToWorldTransform() = 0;
	virtual int LookupAttachment(const char *pAttachmentName) = 0;
	virtual bool GetAttachment(int number, Vector &origin, QAngle &angles) = 0;
	virtual bool GetAttachment(int number, matrix3x4_t &matrix) = 0;
	virtual float *GetRenderClipPlane(void) = 0;
	virtual int GetSkin() = 0;
	virtual bool IsTwoPass(void) = 0;
	virtual void OnThreadedDrawSetup() = 0;
	virtual bool UsesFlexDelayedWeights() = 0;
	virtual void RecordToolMessage() = 0;
	virtual bool IgnoresZBuffer(void) const = 0;
};

class IClientNetworkable
{
public:
	virtual IClientUnknown *GetIClientUnknown() = 0;
	virtual void Release() = 0;
	virtual ClientClass *GetClientClass() = 0;
	virtual void NotifyShouldTransmit(ShouldTransmitState_t state) = 0;
	virtual void OnPreDataChanged(DataUpdateType_t updateType) = 0;
	virtual void OnDataChanged(DataUpdateType_t updateType) = 0;
	virtual void PreDataUpdate(DataUpdateType_t updateType) = 0;
	virtual void PostDataUpdate(DataUpdateType_t updateType) = 0;
	virtual bool IsDormant(void) = 0;
	virtual int entindex(void) const = 0;
	virtual void ReceiveMessage(int classID, bf_read &msg) = 0;
	virtual void *GetDataTableBasePtr() = 0;
	virtual void SetDestroyedOnRecreateEntities(void) = 0;
	virtual void OnDataUnchangedInPVS() = 0;

public:
	size_t GetClassId()
	{
		ClientClass *const cc{ GetClientClass() };

		if (!cc) {
			return 0;
		}

		return static_cast<size_t>(cc->m_ClassID);
	}
};

class IClientThinkable
{
public:
	virtual IClientUnknown *GetIClientUnknown() = 0;
	virtual void ClientThink() = 0;
	virtual ClientThinkHandle_t	GetThinkHandle() = 0;
	virtual void SetThinkHandle(ClientThinkHandle_t hThink) = 0;
	virtual void Release() = 0;
};

class IClientEntity : public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable
{
public:
	virtual void Release(void) = 0;
	virtual const Vector &GetAbsOrigin(void) const = 0;
	virtual const QAngle &GetAbsAngles(void) const = 0;
	virtual CMouthInfo *GetMouth(void) = 0;
	virtual bool GetSoundSpatialization(SpatializationInfo_t &info) = 0;

public:
	template <typename T>
	inline T *cast()
	{
		return static_cast<T *>(this);
	}
};

class C_TEFireBullets
{
public:
	uintptr_t vtable;
	char _pad0[16];
	int		m_iPlayer;
	Vector	m_vecOrigin;
	QAngle	m_vecAngles;
	int		m_iWeaponID;
	int		m_iMode;
	int		m_iSeed;
	float	m_flSpread;
	bool	m_bCritical;
};

class C_BaseEntity : public IClientEntity
{
public:
	NETVAR(float, m_flAnimTime, "DT_AnimTimeMustBeFirst");
	NETVAR(float, m_flSimulationTime, "DT_BaseEntity");
	NETVAR(byte, m_ubInterpolationFrame, "DT_BaseEntity");
	NETVAR(Vector, m_vecOrigin, "DT_BaseEntity");
	NETVAR(QAngle, m_angRotation, "DT_BaseEntity");
	NETVAR(int, m_nModelIndex, "DT_BaseEntity");
	NETVAR(int, m_fEffects, "DT_BaseEntity");
	NETVAR(byte, m_nRenderMode, "DT_BaseEntity");
	NETVAR(byte, m_nRenderFX, "DT_BaseEntity");
	NETVAR(int, m_clrRender, "DT_BaseEntity");
	NETVAR(int, m_iTeamNum, "DT_BaseEntity");
	NETVAR(int, m_CollisionGroup, "DT_BaseEntity");
	NETVAR(float, m_flElasticity, "DT_BaseEntity");
	NETVAR(float, m_flShadowCastDistance, "DT_BaseEntity");
	NETVAR(EHANDLE, m_hOwnerEntity, "DT_BaseEntity");
	NETVAR(EHANDLE, m_hEffectEntity, "DT_BaseEntity");
	NETVAR(int, moveparent, "DT_BaseEntity");
	NETVAR(byte, m_iParentAttachment, "DT_BaseEntity");
	NETVAR(Vector, m_vecMinsPreScaled, "DT_CollisionProperty");
	NETVAR(Vector, m_vecMaxsPreScaled, "DT_CollisionProperty");
	NETVAR(Vector, m_vecMins, "DT_CollisionProperty");
	NETVAR(Vector, m_vecMaxs, "DT_CollisionProperty");
	NETVAR(int, m_nSolidType, "DT_CollisionProperty");
	NETVAR(int, m_usSolidFlags, "DT_CollisionProperty");
	NETVAR(int, m_nSurroundType, "DT_CollisionProperty");
	NETVAR(int, m_triggerBloat, "DT_CollisionProperty");
	NETVAR(bool, m_bUniformTriggerBloat, "DT_CollisionProperty");
	NETVAR(Vector, m_vecSpecifiedSurroundingMinsPreScaled, "DT_CollisionProperty");
	NETVAR(Vector, m_vecSpecifiedSurroundingMaxsPreScaled, "DT_CollisionProperty");
	NETVAR(Vector, m_vecSpecifiedSurroundingMins, "DT_CollisionProperty");
	NETVAR(Vector, m_vecSpecifiedSurroundingMaxs, "DT_CollisionProperty");
	NETVAR(byte, m_iTextureFrameIndex, "DT_BaseEntity");
	NETVAR(int, m_PredictableID, "DT_PredictableId");
	NETVAR(bool, m_bIsPlayerSimulated, "DT_PredictableId");
	NETVAR(int, m_bSimulatedEveryTick, "DT_BaseEntity");
	NETVAR(int, m_bAnimatedEveryTick, "DT_BaseEntity");
	NETVAR(bool, m_bAlternateSorting, "DT_BaseEntity");

public:
	NETVAR_OFFSET(EHANDLE, GetMoveParent, "DT_BaseEntity", "moveparent", -16);
	NETVAR_OFFSET(EHANDLE, FirstMoveChild, "DT_BaseEntity", "moveparent", -12);
	NETVAR_OFFSET(EHANDLE, NextMovePeer, "DT_BaseEntity", "moveparent", -8);
	NETVAR_OFFSET(float, m_flOldSimulationTime, "DT_BaseEntity", "m_flSimulationTime", 4);
	NETVAR_OFFSET(bool, m_bPredictable, "DT_BaseEntity", "m_iParentAttachment", 5); // 550 + 5 = 555
	NETVAR_OFFSET(uint8_t *, m_pOriginalData, "DT_PredictableId", "m_bIsPlayerSimulated", -12); // 1876 - 12 = 1864
	NETVAR_OFFSET(uint8_t **, m_pIntermediateData, "DT_BaseEntity", "m_vecOrigin", 32); // 1112 + 32 = 1144

public: //uhh
	NETVAR_OFFSET(unsigned char, m_MoveType, "DT_TFPlayer", "m_nWaterLevel", -4);
	NETVAR_OFFSET(unsigned char, m_MoveCollide, "DT_TFPlayer", "m_nWaterLevel", -3);

public:
	CLASSVAR(Vector, m_vecAbsVelocity, 472);
	CLASSVAR(Vector, m_vecOldOrigin, 824);
	CLASSVAR(Vector, m_vecNetworkOrigin, 1096);

public:
	bool IsInValidTeam()
	{
		return m_iTeamNum() == TF_TEAM_RED || m_iTeamNum() == TF_TEAM_BLU;
	}

	C_BaseEntity *const GetRootMoveParent()
	{
		C_BaseEntity *ent{ this };
		C_BaseEntity *parent{ GetMoveParent() };

		while (parent)
		{
			ent = parent;
			parent = ent->GetMoveParent();
		}

		return ent;
	}

	int SaveData(const char *context, int slot, int type)
	{
		return s::C_BaseEntity_SaveData.call<int>(this, context, slot, type);
	}

	int RestoreData(const char *context, int slot, int type)
	{
		return s::C_BaseEntity_RestoreData.call<int>(this, context, slot, type);
	}

	Vector GetRenderCenter()
	{
		Vector mins{};
		Vector maxs{};

		GetRenderBounds(mins, maxs);

		return GetRenderOrigin() + Vector(0.0f, 0.0f, (mins.z + maxs.z) * 0.5f);
	}

	Vector GetCenter()
	{
		vec3 origin{ m_vecOrigin() };
		origin.z += (m_vecMins().z + m_vecMaxs().z) * 0.5f;

		return origin;
	}

	Vector OBBSize()
	{
		vec3 temp{ m_vecMaxs() - m_vecMins() };
		return temp;
	}

	void SetAbsAngles(const QAngle &absAngles)
	{
		s::C_BaseEntity_SetAbsAngles.call<void>(this, absAngles);
	}

	void SetAbsOrigin(const Vector &absOrigin)
	{
		s::C_BaseEntity_SetAbsOrigin.call<void>(this, absOrigin);
	}

	void SetAbsVelocity(const Vector &absVel)
	{
		s::C_BaseEntity_SetAbsVelocity.call<void>(this, absVel);
	}

	static int GetPredictionRandomSeed()
	{
		return *s::C_BaseEntity_m_nPredictionRandomSeed.cast<int *>();
	}

	static void SetPredictionRandomSeed(int seed)
	{
		*s::C_BaseEntity_m_nPredictionRandomSeed.cast<int *>() = seed;
	}

	bool PhysicsRunThink(thinkmethods_t thinkMethod = THINK_FIRE_ALL_FUNCTIONS)
	{
		return s::C_BaseEntity_PhysicsRunThink.call<bool>(this, thinkMethod);
	}

	int GetNextThinkTick(const char *szContext = nullptr)
	{
		return s::C_BaseEntity_GetNextThinkTick.call<int>(this, szContext);
	}

	void SetNextThink(float thinkTime, const char *szContext = nullptr)
	{
		s::C_BaseEntity_SetNextThink.call<void>(this, thinkTime, szContext);
	}

	void Think()
	{
		mem::callVirtual<121, void>(this);
	}

	void *GetOriginalNetworkDataObject()
	{
		if (m_pOriginalData())
			return m_pOriginalData();

		return nullptr;
	}

	void *GetPredictedFrame(int framenumber)
	{
		return s::C_BaseEntity_GetPredictedFrame.call<void *>(this, framenumber);
	}

	datamap_t *GetPredDescMap()
	{
		return mem::callVirtual<15, datamap_t *>(this);
	}

	void CalcAbsoluteVelocity()
	{
		return s::C_BaseEntity_CalcAbsoluteVelocity.call<void>(this);
	}

	const Vector &GetAbsVelocity()
	{
		const_cast<C_BaseEntity *>(this)->CalcAbsoluteVelocity();
		return m_vecAbsVelocity();
	}

	void SetLocalAngles(vec3 &ang)
	{
		return s::C_BaseEntity_SetLocalAngles.call<void>(this, ang);
	}

	bool BlocksLOS()
	{
		return s::C_BaseEntity_BlocksLOS.call<bool>(this);
	}

	// will always atleast return a size greater or equal to 4 bytes
	size_t GetIntermediateDataSize()
	{
		datamap_t *map = GetPredDescMap();
		if (!map)
			return sizeof(int32_t);

		return std::max(4, map->packed_size);
	}

	void SetCollisionBounds(const vec3 &mins, const vec3 &maxs)
	{
		uintptr_t collideable{ reinterpret_cast<uintptr_t>(this) + 592 };
		return s::CCollisionProperty_SetCollisionBounds.call<void>(collideable, mins, maxs);
	}
};

class C_BaseAnimating : public C_BaseEntity
{
public:
	NETVAR(int, m_nSequence, "DT_BaseAnimating");
	NETVAR(int, m_nForceBone, "DT_BaseAnimating");
	NETVAR(Vector, m_vecForce, "DT_BaseAnimating");
	NETVAR(int, m_nSkin, "DT_BaseAnimating");
	NETVAR(int, m_nBody, "DT_BaseAnimating");
	NETVAR(int, m_nHitboxSet, "DT_BaseAnimating");
	NETVAR(float, m_flModelScale, "DT_BaseAnimating");
	NETVAR(float, m_flModelWidthScale, "DT_BaseAnimating");
	NETVAR(float, m_flPlaybackRate, "DT_BaseAnimating");
	NETVAR(bool, m_bClientSideAnimation, "DT_BaseAnimating");
	NETVAR(bool, m_bClientSideFrameReset, "DT_BaseAnimating");
	NETVAR(int, m_nNewSequenceParity, "DT_BaseAnimating");
	NETVAR(int, m_nResetEventsParity, "DT_BaseAnimating");
	NETVAR(byte, m_nMuzzleFlashParity, "DT_BaseAnimating");
	NETVAR(EHANDLE, m_hLightingOrigin, "DT_BaseAnimating");
	NETVAR(EHANDLE, m_hLightingOriginRelative, "DT_BaseAnimating");
	NETVAR(float, m_flCycle, "DT_ServerAnimationData");
	NETVAR(float, m_fadeMinDist, "DT_BaseAnimating");
	NETVAR(float, m_fadeMaxDist, "DT_BaseAnimating");
	NETVAR(float, m_flFadeScale, "DT_BaseAnimating");
	NETVAR_PTR(float, m_flPoseParameter, "DT_BaseAnimating");
public:
	NETVAR_OFFSET(CUtlVector<matrix3x4_t>, m_CachedBoneData, "DT_BaseAnimating", "m_hLightingOrigin", -0x58);
public:
	float FrameAdvance(float flInterval)
	{
		return s::C_BaseAnimating_FrameAdvance.call<float>(this, flInterval);
	}

	CStudioHdr *GetModelPtr()
	{
		return s::C_BaseAnimating_GetModelPtr.call<CStudioHdr *>(this);
	}

	bool SetupBones_AttachmentHelper(CStudioHdr *hdr)
	{
		return s::C_BaseAnimating_SetupBones_AttachmentHelper.call<bool>(this, hdr);
	}

	void InvalidateBoneCache()
	{
		return s::C_BaseAnimating_InvalidateBoneCache.call<void>(this);
	}
};

class C_BaseAnimatingOverlay : public C_BaseAnimating
{
public:
	NETVAR(float, m_flCycle, "DT_Animationlayer");
	NETVAR(float, m_flPrevCycle, "DT_Animationlayer");
	NETVAR(float, m_flWeight, "DT_Animationlayer");
	NETVAR(int, m_nOrder, "DT_Animationlayer");
};

class C_BaseFlex : public C_BaseAnimatingOverlay
{
public:
	NETVAR(int, m_blinktoggle, "DT_BaseFlex");
	NETVAR(Vector, m_viewtarget, "DT_BaseFlex");
};

class C_TFWeaponBase;

class C_BaseCombatCharacter : public C_BaseFlex
{
public:
	NETVAR(float, m_flNextAttack, "DT_BCCLocalPlayerExclusive");
	NETVAR(EHANDLE, m_hActiveWeapon, "DT_BaseCombatCharacter");
	//NETVAR(EHANDLE, m_hMyWeapons, "DT_BaseCombatCharacter");
	NETVAR(bool, m_bGlowEnabled, "DT_BaseCombatCharacter");

public:
	CHandle<C_TFWeaponBase>(&m_hMyWeapons())[MAX_WEAPONS]
		{
			static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_BaseCombatCharacter"), HASH_CT("m_hMyWeapons")) };

			return *reinterpret_cast<CHandle<C_TFWeaponBase>(*)[MAX_WEAPONS]>(reinterpret_cast<uintptr_t>(this) + offset);
		}

		C_TFWeaponBase *const GetWeapon(const int slot)
		{
			if (slot < 0 || slot >= MAX_WEAPONS)
			{
				return nullptr;
			}

			return m_hMyWeapons()[slot].Get();
		}

		C_TFWeaponBase *const Weapon_OwnsThisID(const int weaponID);
		C_TFWeaponBase *const FindWeaponByItemDefinitionIndex(const int definition_index);
};

class C_BasePlayer : public C_BaseCombatCharacter
{
public:
	NETVAR(int, m_iHideHUD, "DT_Local");
	NETVAR(float, m_flFOVRate, "DT_Local");
	NETVAR(bool, m_bDucked, "DT_Local");
	NETVAR(bool, m_bDucking, "DT_Local");
	NETVAR(bool, m_bInDuckJump, "DT_Local");
	NETVAR(float, m_flDucktime, "DT_Local");
	NETVAR(float, m_flDuckJumpTime, "DT_Local");
	NETVAR(float, m_flJumpTime, "DT_Local");
	NETVAR(float, m_flFallVelocity, "DT_Local");
	NETVAR(Vector, m_vecPunchAngle, "DT_Local");
	NETVAR(Vector, m_vecPunchAngleVel, "DT_Local");
	NETVAR(bool, m_bDrawViewmodel, "DT_Local");
	NETVAR(bool, m_bWearingSuit, "DT_Local");
	NETVAR(bool, m_bPoisoned, "DT_Local");
	NETVAR(bool, m_bForceLocalPlayerDraw, "DT_Local");
	NETVAR(float, m_flStepSize, "DT_Local");
	NETVAR(bool, m_bAllowAutoMovement, "DT_Local");
	NETVAR(const char *, m_szScriptOverlayMaterial, "DT_Local");
	NETVAR(Vector, m_vecViewOffset, "DT_LocalPlayerExclusive");
	NETVAR(float, m_flFriction, "DT_LocalPlayerExclusive");
	NETVAR(byte, m_fOnTarget, "DT_LocalPlayerExclusive");
	NETVAR(int, m_nTickBase, "DT_LocalPlayerExclusive");
	NETVAR(int, m_nNextThinkTick, "DT_LocalPlayerExclusive");
	NETVAR(EHANDLE, m_hLastWeapon, "DT_LocalPlayerExclusive");
	NETVAR(EHANDLE, m_hGroundEntity, "DT_LocalPlayerExclusive");
	NETVAR(Vector, m_vecVelocity, "DT_LocalPlayerExclusive");
	NETVAR(Vector, m_vecBaseVelocity, "DT_LocalPlayerExclusive");
	NETVAR(EHANDLE, m_hConstraintEntity, "DT_LocalPlayerExclusive");
	NETVAR(Vector, m_vecConstraintCenter, "DT_LocalPlayerExclusive");
	NETVAR(float, m_flConstraintRadius, "DT_LocalPlayerExclusive");
	NETVAR(float, m_flConstraintWidth, "DT_LocalPlayerExclusive");
	NETVAR(float, m_flConstraintSpeedFactor, "DT_LocalPlayerExclusive");
	NETVAR(float, m_flDeathTime, "DT_LocalPlayerExclusive");
	NETVAR(byte, m_nWaterLevel, "DT_LocalPlayerExclusive");
	NETVAR(float, m_flLaggedMovementValue, "DT_LocalPlayerExclusive");
	NETVAR(short, m_iAttributeDefinitionIndex, "DT_ScriptCreatedAttribute");
	NETVAR(int, m_nRefundableCurrency, "DT_ScriptCreatedAttribute");
	NETVAR(byte, deadflag, "DT_PlayerState");
	NETVAR(int, m_iFOV, "DT_BasePlayer");
	NETVAR(int, m_iFOVStart, "DT_BasePlayer");
	NETVAR(float, m_flFOVTime, "DT_BasePlayer");
	NETVAR(int, m_iDefaultFOV, "DT_BasePlayer");
	NETVAR(EHANDLE, m_hZoomOwner, "DT_BasePlayer");
	NETVAR(EHANDLE, m_hVehicle, "DT_BasePlayer");
	NETVAR(EHANDLE, m_hUseEntity, "DT_BasePlayer");
	NETVAR(int, m_iHealth, "DT_BasePlayer");
	NETVAR(byte, m_lifeState, "DT_BasePlayer");
	NETVAR(int, m_iBonusProgress, "DT_BasePlayer");
	NETVAR(int, m_iBonusChallenge, "DT_BasePlayer");
	NETVAR(float, m_flMaxspeed, "DT_BasePlayer");
	NETVAR(int, m_fFlags, "DT_BasePlayer");
	NETVAR(int, m_iObserverMode, "DT_BasePlayer");
	NETVAR(EHANDLE, m_hObserverTarget, "DT_BasePlayer");
	NETVAR(const char *, m_szLastPlaceName, "DT_BasePlayer");
	NETVAR(EHANDLE, m_hMyWearables, "DT_BasePlayer");

public:
	NETVAR_OFFSET(int, m_afButtonLast, "DT_LocalPlayerExclusive", "m_hConstraintEntity", -24);
	NETVAR_OFFSET(int, m_afButtonPressed, "DT_LocalPlayerExclusive", "m_hConstraintEntity", -20);
	NETVAR_OFFSET(int, m_afButtonReleased, "DT_LocalPlayerExclusive", "m_hConstraintEntity", -16);
	NETVAR_OFFSET(int, m_nButtons, "DT_LocalPlayerExclusive", "m_hConstraintEntity", -12);
	NETVAR_OFFSET(CUserCmd *, m_pCurrentCommand, "DT_LocalPlayerExclusive", "m_hConstraintEntity", -8);

public:
	DATAMAP_VAR(int, m_flPhysics); //what the fuck?
	DATAMAP_VAR(float, m_surfaceFriction);

public:
	float GetFOV()
	{
		return s::C_BasePlayer_GetFOV.call<float>(this);
	}

	bool InFirstPersonView()
	{
		return s::C_BasePlayer_InFirstPersonView.call<bool>(this);
	}

	void Weapon_ShootPosition(Vector &out)
	{
		out = m_vecOrigin() + m_vecViewOffset();
	}

	int GetAmmoCount(int iAmmoType)
	{
		return s::C_BaseCombatCharacter_GetAmmoCount.call<int>(this, iAmmoType);
	}

	void PreThink()
	{
		mem::callVirtual<261, void>(this);
	}

	void PostThink()
	{
		mem::callVirtual<262, void>(this);
	}

	void ItemPostFrame()
	{
		mem::callVirtual<264, void>(this);
	}

	void UpdateButtonState(uint32_t buttons)
	{
		return s::C_BasePlayer_UpdateButtonState.call<void>(this, buttons);
	}
};

class C_TFPlayer : public C_BasePlayer
{
public:
	NETVAR(bool, m_bSaveMeParity, "DT_TFPlayer");
	NETVAR(bool, m_bIsMiniBoss, "DT_TFPlayer");
	NETVAR(bool, m_bIsABot, "DT_TFPlayer");
	NETVAR(int, m_nBotSkill, "DT_TFPlayer");
	NETVAR(byte, m_nWaterLevel, "DT_TFPlayer");
	NETVAR(EHANDLE, m_hRagdoll, "DT_TFPlayer");
	NETVAR(int, m_iClass, "DT_TFPlayerClassShared");
	NETVAR(const char *, m_iszClassIcon, "DT_TFPlayerClassShared");
	NETVAR(const char *, m_iszCustomModel, "DT_TFPlayerClassShared");
	NETVAR(Vector, m_vecCustomModelOffset, "DT_TFPlayerClassShared");
	NETVAR(QAngle, m_angCustomModelRotation, "DT_TFPlayerClassShared");
	NETVAR(bool, m_bCustomModelRotates, "DT_TFPlayerClassShared");
	NETVAR(bool, m_bCustomModelRotationSet, "DT_TFPlayerClassShared");
	NETVAR(bool, m_bCustomModelVisibleToSelf, "DT_TFPlayerClassShared");
	NETVAR(bool, m_bUseClassAnimations, "DT_TFPlayerClassShared");
	NETVAR(int, m_iClassModelParity, "DT_TFPlayerClassShared");
	NETVAR(int, m_nPlayerCond, "DT_TFPlayerShared");
	NETVAR(bool, m_bJumping, "DT_TFPlayerShared");
	NETVAR(int, m_nNumHealers, "DT_TFPlayerShared");
	NETVAR(int, m_iCritMult, "DT_TFPlayerShared");
	NETVAR(int, m_iAirDash, "DT_TFPlayerShared");
	NETVAR(int, m_nAirDucked, "DT_TFPlayerShared");
	NETVAR(float, m_flDuckTimer, "DT_TFPlayerShared");
	NETVAR(int, m_nPlayerState, "DT_TFPlayerShared");
	NETVAR(int, m_iDesiredPlayerClass, "DT_TFPlayerShared");
	NETVAR(float, m_flMovementStunTime, "DT_TFPlayerShared");
	NETVAR(int, m_iMovementStunAmount, "DT_TFPlayerShared");
	NETVAR(byte, m_iMovementStunParity, "DT_TFPlayerShared");
	NETVAR(EHANDLE, m_hStunner, "DT_TFPlayerShared");
	NETVAR(int, m_iStunFlags, "DT_TFPlayerShared");
	NETVAR(int, m_nArenaNumChanges, "DT_TFPlayerShared");
	NETVAR(bool, m_bArenaFirstBloodBoost, "DT_TFPlayerShared");
	NETVAR(int, m_iWeaponKnockbackID, "DT_TFPlayerShared");
	NETVAR(bool, m_bLoadoutUnavailable, "DT_TFPlayerShared");
	NETVAR(int, m_iItemFindBonus, "DT_TFPlayerShared");
	NETVAR(bool, m_bShieldEquipped, "DT_TFPlayerShared");
	NETVAR(bool, m_bParachuteEquipped, "DT_TFPlayerShared");
	NETVAR(int, m_iNextMeleeCrit, "DT_TFPlayerShared");
	NETVAR(int, m_iDecapitations, "DT_TFPlayerShared");
	NETVAR(int, m_iRevengeCrits, "DT_TFPlayerShared");
	NETVAR(int, m_iDisguiseBody, "DT_TFPlayerShared");
	NETVAR(EHANDLE, m_hCarriedObject, "DT_TFPlayerShared");
	NETVAR(bool, m_bCarryingObject, "DT_TFPlayerShared");
	NETVAR(float, m_flNextNoiseMakerTime, "DT_TFPlayerShared");
	NETVAR(int, m_iSpawnRoomTouchCount, "DT_TFPlayerShared");
	NETVAR(int, m_iKillCountSinceLastDeploy, "DT_TFPlayerShared");
	NETVAR(float, m_flFirstPrimaryAttack, "DT_TFPlayerShared");
	NETVAR(float, m_flEnergyDrinkMeter, "DT_TFPlayerShared");
	NETVAR(float, m_flHypeMeter, "DT_TFPlayerShared");
	NETVAR(float, m_flChargeMeter, "DT_TFPlayerShared");
	NETVAR(float, m_flInvisChangeCompleteTime, "DT_TFPlayerShared");
	NETVAR(int, m_nDisguiseTeam, "DT_TFPlayerShared");
	NETVAR(int, m_nDisguiseClass, "DT_TFPlayerShared");
	NETVAR(int, m_nDisguiseSkinOverride, "DT_TFPlayerShared");
	NETVAR(int, m_nMaskClass, "DT_TFPlayerShared");
	NETVAR(EHANDLE, m_hDisguiseTarget, "DT_TFPlayerShared");
	NETVAR(int, m_iDisguiseHealth, "DT_TFPlayerShared");
	NETVAR(bool, m_bFeignDeathReady, "DT_TFPlayerShared");
	NETVAR(EHANDLE, m_hDisguiseWeapon, "DT_TFPlayerShared");
	NETVAR(int, m_nTeamTeleporterUsed, "DT_TFPlayerShared");
	NETVAR(float, m_flCloakMeter, "DT_TFPlayerShared");
	NETVAR(float, m_flSpyTranqBuffDuration, "DT_TFPlayerShared");
	NETVAR(int, m_nDesiredDisguiseTeam, "DT_TFPlayerSharedLocal");
	NETVAR(int, m_nDesiredDisguiseClass, "DT_TFPlayerSharedLocal");
	NETVAR(float, m_flStealthNoAttackExpire, "DT_TFPlayerSharedLocal");
	NETVAR(float, m_flStealthNextChangeTime, "DT_TFPlayerSharedLocal");
	NETVAR(bool, m_bLastDisguisedAsOwnTeam, "DT_TFPlayerSharedLocal");
	NETVAR(float, m_flRageMeter, "DT_TFPlayerSharedLocal");
	NETVAR(bool, m_bRageDraining, "DT_TFPlayerSharedLocal");
	NETVAR(float, m_flNextRageEarnTime, "DT_TFPlayerSharedLocal");
	NETVAR(bool, m_bInUpgradeZone, "DT_TFPlayerSharedLocal");
	NETVAR(int, m_iCaptures, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iDefenses, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iKills, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iDeaths, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iSuicides, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iDominations, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iRevenge, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iBuildingsBuilt, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iBuildingsDestroyed, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iHeadshots, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iBackstabs, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iHealPoints, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iInvulns, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iTeleports, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iResupplyPoints, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iKillAssists, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iPoints, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iBonusPoints, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iDamageDone, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, m_iCrits, "DT_TFPlayerScoringDataExclusive");
	NETVAR(int, _condition_bits, "DT_TFPlayerConditionListExclusive");
	NETVAR(int, m_iTauntIndex, "DT_TFPlayerShared");
	NETVAR(int, m_iTauntConcept, "DT_TFPlayerShared");
	NETVAR(int, m_nPlayerCondEx, "DT_TFPlayerShared");
	NETVAR(int, m_iStunIndex, "DT_TFPlayerShared");
	NETVAR(int, m_nHalloweenBombHeadStage, "DT_TFPlayerShared");
	NETVAR(int, m_nPlayerCondEx2, "DT_TFPlayerShared");
	NETVAR(int, m_nPlayerCondEx3, "DT_TFPlayerShared");
	NETVAR(int, m_unTauntSourceItemID_Low, "DT_TFPlayerShared");
	NETVAR(int, m_unTauntSourceItemID_High, "DT_TFPlayerShared");
	NETVAR(float, m_flRuneCharge, "DT_TFPlayerShared");
	NETVAR(bool, m_bHasPasstimeBall, "DT_TFPlayerShared");
	NETVAR(bool, m_bIsTargetedForPasstimePass, "DT_TFPlayerShared");
	NETVAR(EHANDLE, m_hPasstimePassTarget, "DT_TFPlayerShared");
	NETVAR(float, m_askForBallTime, "DT_TFPlayerShared");
	NETVAR(bool, m_bKingRuneBuffActive, "DT_TFPlayerShared");
	NETVAR(int, m_pProvider, "DT_TFPlayerConditionSource");
	NETVAR(int, m_nPlayerCondEx4, "DT_TFPlayerShared");
	NETVAR(float, m_flHolsterAnimTime, "DT_TFPlayerShared");
	NETVAR(EHANDLE, m_hSwitchTo, "DT_TFPlayerShared");
	NETVAR(EHANDLE, m_hItem, "DT_TFPlayer");
	NETVAR(QAngle, m_angEyeAngles, "DT_TFLocalPlayerExclusive");
	NETVAR(bool, m_bIsCoaching, "DT_TFLocalPlayerExclusive");
	NETVAR(EHANDLE, m_hCoach, "DT_TFLocalPlayerExclusive");
	NETVAR(EHANDLE, m_hStudent, "DT_TFLocalPlayerExclusive");
	NETVAR(int, m_nCurrency, "DT_TFLocalPlayerExclusive");
	NETVAR(int, m_nExperienceLevel, "DT_TFLocalPlayerExclusive");
	NETVAR(int, m_nExperienceLevelProgress, "DT_TFLocalPlayerExclusive");
	NETVAR(bool, m_bMatchSafeToLeave, "DT_TFLocalPlayerExclusive");
	NETVAR(bool, m_bAllowMoveDuringTaunt, "DT_TFPlayer");
	NETVAR(bool, m_bIsReadyToHighFive, "DT_TFPlayer");
	NETVAR(EHANDLE, m_hHighFivePartner, "DT_TFPlayer");
	NETVAR(int, m_nForceTauntCam, "DT_TFPlayer");
	NETVAR(float, m_flTauntYaw, "DT_TFPlayer");
	NETVAR(int, m_nActiveTauntSlot, "DT_TFPlayer");
	NETVAR(short, m_iTauntItemDefIndex, "DT_TFPlayer");
	NETVAR(float, m_flCurrentTauntMoveSpeed, "DT_TFPlayer");
	NETVAR(float, m_flVehicleReverseTime, "DT_TFPlayer");
	NETVAR(float, m_flMvMLastDamageTime, "DT_TFPlayer");
	NETVAR(float, m_flLastDamageTime, "DT_TFPlayer");
	NETVAR(bool, m_bInPowerPlay, "DT_TFPlayer");
	NETVAR(int, m_iSpawnCounter, "DT_TFPlayer");
	NETVAR(bool, m_bArenaSpectator, "DT_TFPlayer");
	NETVAR(EHANDLE, m_hOuter, "DT_AttributeManager");
	NETVAR(int, m_ProviderType, "DT_AttributeManager");
	NETVAR(int, m_iReapplyProvisionParity, "DT_AttributeManager");
	NETVAR(float, m_flHeadScale, "DT_TFPlayer");
	NETVAR(float, m_flTorsoScale, "DT_TFPlayer");
	NETVAR(float, m_flHandScale, "DT_TFPlayer");
	NETVAR(bool, m_bUseBossHealthBar, "DT_TFPlayer");
	NETVAR(bool, m_bUsingVRHeadset, "DT_TFPlayer");
	NETVAR(bool, m_bForcedSkin, "DT_TFPlayer");
	NETVAR(int, m_nForcedSkin, "DT_TFPlayer");
	NETVAR(bool, m_bGlowEnabled, "DT_TFPlayer");
	NETVAR(short, m_nActiveWpnClip, "DT_TFSendHealersDataTable");
	NETVAR(float, m_flKartNextAvailableBoost, "DT_TFPlayer");
	NETVAR(int, m_iKartHealth, "DT_TFPlayer");
	NETVAR(int, m_iKartState, "DT_TFPlayer");
	NETVAR(EHANDLE, m_hGrapplingHookTarget, "DT_TFPlayer");
	NETVAR(EHANDLE, m_hSecondaryLastWeapon, "DT_TFPlayer");
	NETVAR(bool, m_bUsingActionSlot, "DT_TFPlayer");
	NETVAR(float, m_flInspectTime, "DT_TFPlayer");
	NETVAR(float, m_flHelpmeButtonPressTime, "DT_TFPlayer");
	NETVAR(int, m_iCampaignMedals, "DT_TFPlayer");
	NETVAR(int, m_iPlayerSkinOverride, "DT_TFPlayer");
	NETVAR(bool, m_bViewingCYOAPDA, "DT_TFPlayer");
	NETVAR(bool, m_bRegenerating, "DT_TFPlayer");
	NETVAR(float, m_flItemChargeMeter, "DT_TFPlayerSharedLocal");

public:
	NETVAR_OFFSET(float, m_flInvisibility, "DT_TFPlayerShared", "m_flInvisChangeCompleteTime", -8);
	NETVAR_OFFSET(CTFPlayerAnimState *, m_PlayerAnimState, "DT_TFPlayer", "m_hItem", -88);
	NETVAR_OFFSET(bool, m_bTauntForceMoveForward, "DT_TFPlayer", "m_bAllowMoveDuringTaunt", 1);

public:
	int GetMaxHealth()
	{
		return mem::callVirtual<107, int>(this);
	}

	float HealthFraction()
	{
		if (GetMaxHealth() == 0) {
			return 1.0f;
		}

		return std::clamp(static_cast<float>(m_iHealth()) / static_cast<float>(GetMaxHealth()), 0.0f, 1.0f);
	}

	bool InCond(const ETFCond tfcond)
	{
		const int cond{ static_cast<int>(tfcond) };

		switch (cond / 32)
		{
			case 0:
			{
				const int bit{ (1 << cond) };

				if ((m_nPlayerCond() & bit) == bit || (_condition_bits() & bit) == bit) {
					return true;
				}

				break;
			}

			case 1:
			{
				const int bit{ 1 << (cond - 32) };

				if ((m_nPlayerCondEx() & bit) == bit) {
					return true;
				}

				break;
			}

			case 2:
			{
				const int bit{ 1 << (cond - 64) };

				if ((m_nPlayerCondEx2() & bit) == bit) {
					return true;
				}

				break;
			}

			case 3:
			{
				const int bit{ 1 << (cond - 96) };

				if ((m_nPlayerCondEx3() & bit) == bit) {
					return true;
				}

				break;
			}

			case 4:
			{
				const int bit{ 1 << (cond - 128) };

				if ((m_nPlayerCondEx4() & bit) == bit) {
					return true;
				}

				break;
			}

			default:
			{
				break;
			}
		}

		return false;
	}

	void ThirdPersonSwitch()
	{
		s::C_TFPlayer_ThirdPersonSwitch.call<void>(this);
	}

	Vector GetEyePos()
	{
		return m_vecOrigin() + m_vecViewOffset();
	}

	bool IsPlayerOnSteamFriendsList()
	{
		return s::C_TFPlayer_IsPlayerOnSteamFriendsList.call<bool>(this, this);
	}

	bool IsInvisible()
	{
		if (InCond(TF_COND_BURNING)
			|| InCond(TF_COND_BURNING_PYRO)
			|| InCond(TF_COND_MAD_MILK)
			|| InCond(TF_COND_URINE))
		{
			return false;
		}

		return m_flInvisibility() >= 1.0f;
	}

	bool IsZoomed()
	{
		return InCond(TF_COND_ZOOMED);
	}

	bool IsInvulnerable()
	{
		return InCond(TF_COND_INVULNERABLE)
			|| InCond(TF_COND_INVULNERABLE_CARD_EFFECT)
			|| InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE)
			|| InCond(TF_COND_INVULNERABLE_USER_BUFF)
			|| InCond(TF_COND_PHASE);
	}

	bool IsUbered()
	{
		return InCond(TF_COND_INVULNERABLE)
			|| InCond(TF_COND_INVULNERABLE_CARD_EFFECT)
			|| InCond(TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE)
			|| InCond(TF_COND_INVULNERABLE_USER_BUFF);
	}

	bool IsCritBoosted();

	bool IsMiniCritBoosted()
	{
		return InCond(TF_COND_MINICRITBOOSTED_ON_KILL)
			|| InCond(TF_COND_NOHEALINGDAMAGEBUFF)
			|| InCond(TF_COND_ENERGY_BUFF)
			/*|| InCond(TF_COND_CRITBOOSTED_DEMO_CHARGE)*/;
	}

	bool IsMarked()
	{
		return InCond(TF_COND_URINE)
			|| InCond(TF_COND_MARKEDFORDEATH)
			|| InCond(TF_COND_MARKEDFORDEATH_SILENT);
	}

	bool IsStealthed()
	{
		return InCond(TF_COND_STEALTHED)
			|| InCond(TF_COND_STEALTHED_USER_BUFF)
			|| InCond(TF_COND_STEALTHED_USER_BUFF_FADING);
	}

	float TeamFortress_CalculateMaxSpeed(bool bIgnoreSpecialAbility = false)
	{
		return s::C_TFPlayer_TeamFortress_CalculateMaxSpeed.call<float>(this, bIgnoreSpecialAbility);
	}

	void UpdateClientSideAnimation()
	{
		s::C_TFPlayer_UpdateClientSideAnimation.call<void>(this);
	}

	ETFCond GetConditionFromRuneType(RuneTypes_t rt)
	{
		switch (rt)
		{
			case RUNE_NONE: return TF_COND_INVALID;
			case RUNE_STRENGTH: return TF_COND_RUNE_STRENGTH;
			case RUNE_HASTE: return TF_COND_RUNE_HASTE;
			case RUNE_REGEN: return TF_COND_RUNE_REGEN;
			case RUNE_RESIST: return TF_COND_RUNE_RESIST;
			case RUNE_VAMPIRE: return TF_COND_RUNE_VAMPIRE;
			case RUNE_REFLECT: return TF_COND_RUNE_REFLECT;
			case RUNE_PRECISION: return TF_COND_RUNE_PRECISION;
			case RUNE_AGILITY: return TF_COND_RUNE_AGILITY;
			case RUNE_KNOCKOUT: return TF_COND_RUNE_KNOCKOUT;
			case RUNE_KING: return TF_COND_RUNE_KING;
			case RUNE_PLAGUE: return TF_COND_RUNE_PLAGUE;
			case RUNE_SUPERNOVA: return TF_COND_RUNE_SUPERNOVA;
		}

		return TF_COND_INVALID;
	}

	RuneTypes_t GetCarryingRuneType()
	{
		RuneTypes_t result{ RUNE_NONE };

		for (int n{}; n < RUNE_TYPES_MAX; n++)
		{
			const RuneTypes_t rune_type{ static_cast<RuneTypes_t>(n) };

			if (InCond(GetConditionFromRuneType(rune_type))) {
				result = rune_type;
				break;
			}
		}

		return result;
	}

	float GetCritMult()
	{
		return math::remap(static_cast<float>(m_iCritMult()), 0.0f, 255.0f, 1.0f, 4.0f);
	}
};

class C_BaseCombatWeapon : public C_BaseAnimating
{
public:
	NETVAR(int, m_iClip1, "DT_LocalWeaponData");
	NETVAR(int, m_iClip2, "DT_LocalWeaponData");
	NETVAR(int, m_iPrimaryAmmoType, "DT_LocalWeaponData");
	NETVAR(int, m_iSecondaryAmmoType, "DT_LocalWeaponData");
	NETVAR(int, m_nViewModelIndex, "DT_LocalWeaponData");
	NETVAR(short, m_nCustomViewmodelModelIndex, "DT_LocalWeaponData");
	NETVAR(bool, m_bFlipViewModel, "DT_LocalWeaponData");
	NETVAR(float, m_flNextPrimaryAttack, "DT_LocalActiveWeaponData");
	NETVAR(float, m_flNextSecondaryAttack, "DT_LocalActiveWeaponData");
	NETVAR(int, m_nNextThinkTick, "DT_LocalActiveWeaponData");
	NETVAR(float, m_flTimeWeaponIdle, "DT_LocalActiveWeaponData");
	NETVAR(int, m_iViewModelIndex, "DT_BaseCombatWeapon");
	NETVAR(int, m_iWorldModelIndex, "DT_BaseCombatWeapon");
	NETVAR(int, m_iState, "DT_BaseCombatWeapon");
	NETVAR(EHANDLE, m_hOwner, "DT_BaseCombatWeapon");

public:
	CLASSVAR(float, m_flCritTokenBucket, 3732);
	CLASSVAR(int, m_nCritChecks, 3736);
	CLASSVAR(int, m_nCritSeedRequests, 3740);
	CLASSVAR(float, m_flLastRapidFireCritCheckTime, 4036);
	CLASSVAR(float, m_flCritTime, 4020);

	const char *GetPrintName() {
		return mem::callVirtual<333, const char*>(this);
	}
};

class C_TFWeaponBase : public C_BaseCombatWeapon
{
public:
	NETVAR(bool, m_bLowered, "DT_TFWeaponBase");
	NETVAR(int, m_iReloadMode, "DT_TFWeaponBase");
	NETVAR(bool, m_bResetParity, "DT_TFWeaponBase");
	NETVAR(bool, m_bReloadedThroughAnimEvent, "DT_TFWeaponBase");
	NETVAR(bool, m_bDisguiseWeapon, "DT_TFWeaponBase");
	NETVAR(float, m_flLastCritCheckTime, "DT_LocalTFWeaponData");
	NETVAR(float, m_flReloadPriorNextFire, "DT_LocalTFWeaponData");
	NETVAR(float, m_flLastFireTime, "DT_LocalTFWeaponData");
	NETVAR(float, m_flEffectBarRegenTime, "DT_LocalTFWeaponData");
	NETVAR(float, m_flObservedCritChance, "DT_LocalTFWeaponData");
	NETVAR(float, m_flEnergy, "DT_TFWeaponBase");
	NETVAR(EHANDLE, m_hExtraWearable, "DT_TFWeaponBase");
	NETVAR(EHANDLE, m_hExtraWearableViewModel, "DT_TFWeaponBase");
	NETVAR(bool, m_bBeingRepurposedForTaunt, "DT_TFWeaponBase");
	NETVAR(int, m_nKillComboClass, "DT_TFWeaponBase");
	NETVAR(int, m_nKillComboCount, "DT_TFWeaponBase");
	NETVAR(float, m_flInspectAnimEndTime, "DT_TFWeaponBase");
	NETVAR(int, m_nInspectStage, "DT_TFWeaponBase");
	NETVAR(int, m_iConsecutiveShots, "DT_TFWeaponBase");

public:
	NETVAR(int, m_iItemDefinitionIndex, "DT_ScriptCreatedItem");

public:
	NETVAR_OFFSET(int, m_iWeaponMode, "DT_TFWeaponBase", "m_iReloadMode", -4);
	NETVAR_OFFSET(CTFWeaponInfo *, m_pWeaponInfo, "DT_LocalTFWeaponData", "m_flReloadPriorNextFire", 4);
	NETVAR_OFFSET(float, m_flSmackTime, "DT_TFWeaponBase", "m_nInspectStage", 28);

	// reloadmode = 3976, curatkiscrit = 3994
	NETVAR_OFFSET(bool, m_bCurrentAttackIsCrit, "DT_TFWeaponBase", "m_iReloadMode", 18);

public:
	int GetSlot()
	{
		return mem::callVirtual<330, int>(this);
	}

	int GetWeaponID()
	{
		return mem::callVirtual<381, int>(this);
	}

	int GetDamageType()
	{
		return mem::callVirtual<382, int>(this);
	}

	float GetSwingRange()
	{
		return GetWeaponID() == TF_WEAPON_SWORD ? 72.0f : 48.0f;
	}

	float GetWeaponSpread()
	{
		return mem::callVirtual< 467, float >(this);
	}

	bool IsEnergyWeapon()
	{
		return mem::callVirtual<432, bool>(this);
	}

	bool HasPrimaryAmmoForShot()
	{
		if (IsEnergyWeapon()) {
			return m_flEnergy() > 0.0f;
		}

		const int clip{ m_iClip1() };

		if (clip == -1)
		{
			if (C_BaseEntity *const owner{ m_hOwnerEntity().Get() })
			{
				const int ammo_count{ owner->cast<C_BasePlayer>()->GetAmmoCount(m_iPrimaryAmmoType()) };

				if (m_iItemDefinitionIndex() == Engi_m_TheWidowmaker) {
					return ammo_count >= 30;
				}

				return ammo_count > 0;
			}
		}

		return clip > 0;
	}

	bool AreRandomCritsEnabled()
	{
		return s::C_TFWeaponBase_AreRandomCritsEnabled.call<bool>(this);
	}

	bool CanFireCriticalShot(bool bIsHeadshot)
	{
		return mem::callVirtual<425, bool, bool, C_BaseEntity *>(this, bIsHeadshot, nullptr);
		//(*(unsigned __int8 (__fastcall **)(__int64, _QWORD, _QWORD))(*(_QWORD *)a1 + 3400i64))(a1, 0i64, 0i64)
	}

	void UpdateAllViewmodelAddons()
	{
		return s::C_TFWeaponBase_UpdateAllViewmodelAddons.call<void>(this);
	}

	bool CanOverload()
	{
		return static_cast<int>(tf_utils::attribHookValue(0.0f, "can_overload", this)) != 0;
	}

	float Energy_GetShotCost()
	{
		return mem::callVirtual<434, float>(this);
	}

	float Energy_GetMaxEnergy()
	{
		const int num_shots{ static_cast<int>(20.f / Energy_GetShotCost()) };

		tf_utils::attribHookValue(num_shots, "mult_clipsize_upgrade", this);

		return num_shots * Energy_GetShotCost();
	}

	bool IsBlastImpactWeapon()
	{
		return mem::callVirtual<433, bool>(this); // 3464 / 8 = 433
	}

	int GetMaxClip1()
	{
		if (IsEnergyWeapon()) {
			return Energy_GetMaxEnergy();
		}

		const int mod_max_clip_override{ static_cast<int>(tf_utils::attribHookValue(0, "mod_max_primary_clip_override", this)) };

		int max_clip1{ mod_max_clip_override ? mod_max_clip_override : m_pWeaponInfo()->iMaxClip1 };

		if (max_clip1 >= 0) {
			max_clip1 = tf_utils::attribHookValue(max_clip1, "mult_clipsize", this);
		}

		if (max_clip1 < 0) {
			return max_clip1;
		}

		C_TFPlayer *const owner{ m_hOwnerEntity().Get()->cast<C_TFPlayer>() };

		if (!owner) {
			return max_clip1;
		}

		if (IsBlastImpactWeapon())
		{
			int projectiles{ static_cast<int>(tf_utils::attribHookValue(0, "mult_clipsize_upgrade_atomic", this)) };

			const int clip_size_on_kills{ static_cast<int>(tf_utils::attribHookValue(0, "clipsize_increase_on_kill", this)) };

			if (clip_size_on_kills) {
				projectiles += std::min(owner->m_iDecapitations(), clip_size_on_kills);
			}

			if (owner->GetCarryingRuneType() == RUNE_HASTE) {
				max_clip1 *= 2;
			}

			if (owner->GetCarryingRuneType() == RUNE_PRECISION) {
				max_clip1 *= 1.5f;
			}

			return (max_clip1 + projectiles);
		}
		else {
			max_clip1 = tf_utils::attribHookValue(max_clip1, "mult_clipsize_upgrade", this);

			if (owner->GetCarryingRuneType() == RUNE_HASTE) {
				max_clip1 *= 2;
			}
		}

		return max_clip1;
	}

	bool IsReloading()
	{
		return m_iReloadMode() != TF_RELOAD_START;
	}
};

class C_TFWeaponBaseGun : public C_TFWeaponBase
{
public:
	bool HasKnockBack()
	{
		return static_cast<bool>(tf_utils::attribHookValue(0, "set_scattergun_has_knockback", this));
	}
};

class C_TFWeaponBaseMelee : public C_TFWeaponBase
{
public:
	NETVAR_OFFSET(float, m_flSmackTime, "DT_TFWeaponBase", "m_nInspectStage", 28);
};

class C_TFBottle : public C_TFWeaponBaseMelee
{
public:

};

class C_TFSMG : public C_TFWeaponBaseGun
{
public:

};

class C_TFRevolver : public C_TFWeaponBaseGun
{
public:
	bool CanHeadshot()
	{
		return static_cast<int>(tf_utils::attribHookValue(0, "set_weapon_mode", this)) == 1;
	}

	float GetWeaponSpread()
	{
		return s::C_TFRevolver_GetWeaponSpread.call<float>(this);
	}
};

class C_TFChargedSMG : public C_TFSMG
{
public:
	NETVAR(float, m_flMinicritCharge, "DT_WeaponChargedSMG");
};

class C_TFPipebombLauncher : public C_TFWeaponBaseGun
{
public:
	NETVAR(int, m_iPipebombCount, "DT_PipebombLauncherLocalData");
	NETVAR(float, m_flChargeBeginTime, "DT_PipebombLauncherLocalData");
};

class C_TFCompoundBow : public C_TFPipebombLauncher
{
public:
	NETVAR(bool, m_bArrowAlight, "DT_WeaponCompoundBow");
	NETVAR(bool, m_bNoFire, "DT_WeaponCompoundBow");
};

class C_TFRocketLauncher : public C_TFWeaponBaseGun
{
public:

};

class C_TFCrossbow : public C_TFRocketLauncher
{
public:
	NETVAR(float, m_flRegenerateDuration, "DT_Crossbow");
	NETVAR(float, m_flLastUsedTimestamp, "DT_Crossbow");
};

class C_TFFlameThrower : public C_TFWeaponBaseGun
{
public:
	NETVAR(int, m_iWeaponState, "DT_WeaponFlameThrower");
	NETVAR(bool, m_bCritFire, "DT_WeaponFlameThrower");
	NETVAR(bool, m_bHitTarget, "DT_WeaponFlameThrower");
	NETVAR(float, m_flChargeBeginTime, "DT_WeaponFlameThrower");
	NETVAR(int, m_iActiveFlames, "DT_LocalFlameThrower");
	NETVAR(int, m_iDamagingFlames, "DT_LocalFlameThrower");
	NETVAR(EHANDLE, m_hFlameManager, "DT_LocalFlameThrower");
	NETVAR(bool, m_bHasHalloweenSpell, "DT_LocalFlameThrower");
};

class C_TFFlareGun : public C_TFWeaponBaseGun
{
public:
	NETVAR(float, m_flChargeBeginTime, "DT_WeaponFlareGun");

	int GetFlareGunType()
	{
		int iTmp{ 0 };
		iTmp = static_cast<int>(tf_utils::attribHookValue(static_cast<float>(iTmp), "set_weapon_mode", this));
		return iTmp;
	};
};

class C_TFFlareGun_Revenge : public C_TFFlareGun
{
public:
	NETVAR(float, m_fLastExtinguishTime, "DT_WeaponFlareGun_Revenge");
};

class C_TFGrapplingHook : public C_TFRocketLauncher
{
public:
	NETVAR(EHANDLE, m_hProjectile, "DT_GrapplingHook");
};

class C_TFGrenadeLauncher : public C_TFWeaponBaseGun
{
public:
	NETVAR(float, m_flDetonateTime, "DT_WeaponGrenadeLauncher");
	NETVAR(int, m_iCurrentTube, "DT_WeaponGrenadeLauncher");
	NETVAR(int, m_iGoalTube, "DT_WeaponGrenadeLauncher");
};

class C_TFJar : public C_TFWeaponBaseGun
{
public:

};

class C_TFThrowable : public C_TFJar
{
public:
	NETVAR(float, m_flChargeBeginTime, "DT_TFWeaponThrowable");
};

class C_TFKnife : public C_TFWeaponBaseMelee
{
public:
	NETVAR(bool, m_bReadyToBackstab, "DT_TFWeaponKnife");
	NETVAR(bool, m_bKnifeExists, "DT_TFWeaponKnife");
	NETVAR(float, m_flKnifeRegenerateDuration, "DT_TFWeaponKnife");
	NETVAR(float, m_flKnifeMeltTimestamp, "DT_TFWeaponKnife");
};

class C_TFLunchBox : public C_TFWeaponBase
{
public:
	NETVAR(int, m_bBroken, "DT_WeaponLunchBox");
};

class C_TFMinigun : public C_TFWeaponBaseGun
{
public:
	NETVAR(int, m_iWeaponState, "DT_WeaponMinigun");
	NETVAR(bool, m_bCritShot, "DT_WeaponMinigun");

public:
	void StartMuzzleEffect()
	{
		s::C_TFMinigun_StartMuzzleEffect.call<void>(this);
	}

	void StartBrassEffect()
	{
		s::C_TFMinigun_StartBrassEffect.call<void>(this);
	}
};

class C_TFParticleCannon : public C_TFRocketLauncher
{
public:
	NETVAR(float, m_flChargeBeginTime, "DT_ParticleCannon");
	NETVAR(int, m_iChargeEffect, "DT_ParticleCannon");
};

class C_TFRaygun : public C_TFRocketLauncher
{
public:
	NETVAR(bool, m_bUseNewProjectileCode, "DT_WeaponRaygun");
};

class C_TFRocketPack : public C_TFWeaponBaseMelee
{
public:
	NETVAR(float, m_flInitLaunchTime, "DT_TFWeaponRocketPack");
	NETVAR(float, m_flLaunchTime, "DT_TFWeaponRocketPack");
	NETVAR(float, m_flToggleEndTime, "DT_TFWeaponRocketPack");
	NETVAR(bool, m_bEnabled, "DT_TFWeaponRocketPack");
};

class C_TFSniperRifle : public C_TFWeaponBaseGun
{
public:
	NETVAR(float, m_flChargedDamage, "DT_SniperRifleLocalData");

public:
	int GetRifleType()
	{
		return static_cast<int>(tf_utils::attribHookValue(0, "set_weapon_mode", this));
	};

	bool IsFullyCharged()
	{
		return m_flChargedDamage() >= TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX;
	}
};

class C_TFSniperRifleClassic : public C_TFSniperRifle
{
public:
	NETVAR(bool, m_bCharging, "DT_TFSniperRifleClassic");
};

class C_TFSpellBook : public C_TFThrowable
{
public:
	NETVAR(int, m_iSelectedSpellIndex, "DT_TFWeaponSpellBook");
	NETVAR(int, m_iSpellCharges, "DT_TFWeaponSpellBook");
	NETVAR(float, m_flTimeNextSpell, "DT_TFWeaponSpellBook");
	NETVAR(bool, m_bFiredAttack, "DT_TFWeaponSpellBook");
};

class C_TFStickBomb : public C_TFBottle
{
public:
	NETVAR(int, m_iDetonated, "DT_TFWeaponStickBomb");
};

class C_TFWeaponBuilder : public C_TFWeaponBase
{
public:
	NETVAR(int, m_iBuildState, "DT_TFWeaponBuilder");
	NETVAR(int, m_iObjectMode, "DT_TFWeaponBuilder");
	NETVAR(float, m_flWheatleyTalkingUntil, "DT_TFWeaponBuilder");
	NETVAR(int, m_iObjectType, "DT_BuilderLocalData");
	NETVAR(EHANDLE, m_hObjectBeingBuilt, "DT_BuilderLocalData");
	NETVAR(void *, m_aBuildableObjectTypes, "DT_BuilderLocalData");
};

class C_TFWeaponFlameBall : public C_TFFlameThrower
{
public:
	NETVAR(float, m_flRechargeScale, "DT_WeaponFlameBall");
};

class C_TFWeaponSlap : public C_TFWeaponBaseMelee
{
public:
	NETVAR(bool, m_bFirstHit, "DT_TFWeaponSlap");
	NETVAR(int, m_nNumKills, "DT_TFWeaponSlap");
};

class C_WeaponMedigun : public C_TFWeaponBaseGun
{
public:
	NETVAR(EHANDLE, m_hHealingTarget, "DT_WeaponMedigun");
	NETVAR(bool, m_bHealing, "DT_WeaponMedigun");
	NETVAR(bool, m_bAttacking, "DT_WeaponMedigun");
	NETVAR(bool, m_bChargeRelease, "DT_WeaponMedigun");
	NETVAR(bool, m_bHolstered, "DT_WeaponMedigun");
	NETVAR(int, m_nChargeResistType, "DT_WeaponMedigun");
	NETVAR(EHANDLE, m_hLastHealingTarget, "DT_WeaponMedigun");
	NETVAR(float, m_flChargeLevel, "DT_LocalTFWeaponMedigunData");

public:
	NETVAR_OFFSET(bool, m_bReloadDown, "DT_WeaponMedigun", "m_nChargeResistType", 11);

public:
	int GetMedigunType()
	{
		int iMode{ 0 };

		iMode = static_cast<int>(tf_utils::attribHookValue(static_cast<float>(iMode), "set_weapon_mode", this));

		return iMode;
	}

	medigun_charge_types GetChargeType()
	{
		int iTmp{ MEDIGUN_CHARGE_INVULN };

		iTmp = static_cast<int>(tf_utils::attribHookValue(static_cast<float>(iTmp), "set_charge_type", this));

		if (GetMedigunType() == MEDIGUN_RESIST) {
			iTmp += m_nChargeResistType();
		}

		return static_cast<medigun_charge_types>(iTmp);
	}

	medigun_resist_types_t GetResistType()
	{
		int nCurrentActiveResist{ GetChargeType() - MEDIGUN_CHARGE_BULLET_RESIST };

		nCurrentActiveResist = nCurrentActiveResist % MEDIGUN_NUM_RESISTS;

		return static_cast<medigun_resist_types_t>(nCurrentActiveResist);
	}

	bool IsReleasingCharge()
	{
		return m_bChargeRelease() && !m_bHolstered();
	}
};

class C_PasstimeGun : public C_TFWeaponBase
{
public:
	NETVAR(int, m_eThrowState, "DT_PasstimeGun");
	NETVAR(float, m_fChargeBeginTime, "DT_PasstimeGun");
};

class C_BaseProjectile : public C_BaseAnimating
{
public:
	bool IsCritical();
};

class C_TFProjectile_EnergyRing : public C_BaseProjectile
{
public:
};

class C_BaseGrenade : public C_BaseProjectile
{
public:
	NETVAR(float, m_flDamage, "DT_BaseGrenade");
	NETVAR(float, m_DmgRadius, "DT_BaseGrenade");
	NETVAR(bool, m_bIsLive, "DT_BaseGrenade");
	NETVAR(EHANDLE, m_hThrower, "DT_BaseGrenade");
	NETVAR(Vector, m_vecVelocity, "DT_BaseGrenade");
	NETVAR(int, m_fFlags, "DT_BaseGrenade");
};

class C_TFBaseProjectile : public C_BaseProjectile
{
public:
	NETVAR(Vector, m_vInitialVelocity, "DT_TFBaseProjectile");
	NETVAR(EHANDLE, m_hLauncher, "DT_TFBaseProjectile");
};

class C_TFBaseRocket : public C_BaseProjectile
{
public:
	NETVAR(Vector, m_vInitialVelocity, "DT_TFBaseRocket");
	NETVAR(Vector, m_vecOrigin, "DT_TFBaseRocket");
	NETVAR(QAngle, m_angRotation, "DT_TFBaseRocket");
	NETVAR(int, m_iDeflected, "DT_TFBaseRocket");
	NETVAR(EHANDLE, m_hLauncher, "DT_TFBaseRocket");

	// 3128 - 3112 = 16
	CLASSVAR(float, m_flSpawnTime, 3112);
};

class C_TFWeaponBaseGrenadeProj : public C_BaseGrenade
{
public:
	NETVAR(Vector, m_vInitialVelocity, "DT_TFWeaponBaseGrenadeProj");
	NETVAR(bool, m_bCritical, "DT_TFWeaponBaseGrenadeProj");
	NETVAR(int, m_iDeflected, "DT_TFWeaponBaseGrenadeProj");
	NETVAR(Vector, m_vecOrigin, "DT_TFWeaponBaseGrenadeProj");
	NETVAR(QAngle, m_angRotation, "DT_TFWeaponBaseGrenadeProj");
	NETVAR(EHANDLE, m_hDeflectOwner, "DT_TFWeaponBaseGrenadeProj");
};

class C_TFGrenadePipebombProjectile : public C_TFWeaponBaseGrenadeProj
{
public:
	NETVAR(bool, m_bTouched, "DT_TFProjectile_Pipebomb");
	NETVAR(int, m_iType, "DT_TFProjectile_Pipebomb");
	NETVAR(EHANDLE, m_hLauncher, "DT_TFProjectile_Pipebomb");
	NETVAR(bool, m_bDefensiveBomb, "DT_TFProjectile_Pipebomb");

public:
	NETVAR_OFFSET(float, m_flCreationTime, "DT_TFProjectile_Pipebomb", "m_iType", 4);

public:
	bool HasStickyEffects()
	{
		return m_iType() == TF_GL_MODE_REMOTE_DETONATE || m_iType() == TF_GL_MODE_REMOTE_DETONATE_PRACTICE;
	}
};

class C_TFProjectile_Arrow : public C_TFBaseRocket
{
public:
	NETVAR(bool, m_bArrowAlight, "DT_TFProjectile_Arrow");
	NETVAR(bool, m_bCritical, "DT_TFProjectile_Arrow");
	NETVAR(int, m_iProjectileType, "DT_TFProjectile_Arrow");
};

class C_TFProjectile_GrapplingHook : public C_TFProjectile_Arrow
{
public:
};

class C_TFProjectile_HealingBolt : public C_TFProjectile_Arrow
{
public:
};

class C_TFProjectile_EnergyBall : public C_TFBaseRocket
{
public:
	NETVAR(bool, m_bChargedShot, "DT_TFProjectile_EnergyBall");
	NETVAR(Vector, m_vColor1, "DT_TFProjectile_EnergyBall");
	NETVAR(Vector, m_vColor2, "DT_TFProjectile_EnergyBall");
};

class C_TFProjectile_Flare : public C_TFBaseRocket
{
public:
	NETVAR(bool, m_bCritical, "DT_TFProjectile_Flare");
};

class C_TFProjectile_Rocket : public C_TFBaseRocket
{
public:
	NETVAR(bool, m_bCritical, "DT_TFProjectile_Rocket");

	bool isRocketJumper()
	{
		C_TFWeaponBase *const weapon{ this->m_hLauncher()->cast<C_TFWeaponBase>() };
		if (weapon && weapon->m_iItemDefinitionIndex() == Soldier_m_RocketJumper) {
			return true;
		}

		return false;
	}
};

class C_TFProjectile_MechanicalArmOrb : public C_TFProjectile_Rocket
{
public:
};

class C_TFProjectile_SentryRocket : public C_TFProjectile_Rocket
{
public:
};

class C_TFProjectile_BallOfFire : public C_TFProjectile_Rocket
{
public:
	NETVAR(Vector, m_vecInitialVelocity, "DT_TFProjectile_BallOfFire");
	NETVAR(Vector, m_vecSpawnOrigin, "DT_TFProjectile_BallOfFire");
};

class C_TFProjectile_Jar : public C_TFGrenadePipebombProjectile
{
public:
	NETVAR(byte, m_bTouched, "DT_TFProjectile_Pipebomb"); // + 3176
	NETVAR(int, m_iType, "DT_TFProjectile_Pipebomb"); // + 3180
	NETVAR(EHANDLE, m_hLauncher, "DT_TFProjectile_Pipebomb"); // + 3156
	NETVAR(byte, m_bDefensiveBomb, "DT_TFProjectile_Pipebomb"); // + 3200
};

class C_TFProjectile_JarGas : public C_TFProjectile_Jar
{
public:
};

class C_TFProjectile_Cleaver : public C_TFProjectile_Jar
{
public:
};

class C_TFProjectile_JarMilk : public C_TFProjectile_Jar
{
public:
};

class C_TFProjectile_Throwable : public C_TFProjectile_Jar
{
public:
};

class C_TFProjectile_ThrowableBreadMonster : public C_TFProjectile_Throwable
{
public:
};

class C_TFProjectile_ThrowableBrick : public C_TFProjectile_Throwable
{
public:
};

class C_TFProjectile_ThrowableRepel : public C_TFProjectile_Throwable
{
public:
};

class C_BaseObject : public C_BaseCombatCharacter
{
public:
	NETVAR(int, m_iHealth, "DT_BaseObject");
	NETVAR(int, m_iMaxHealth, "DT_BaseObject");
	NETVAR(bool, m_bHasSapper, "DT_BaseObject");
	NETVAR(int, m_iObjectType, "DT_BaseObject");
	NETVAR(bool, m_bBuilding, "DT_BaseObject");
	NETVAR(bool, m_bPlacing, "DT_BaseObject");
	NETVAR(bool, m_bCarried, "DT_BaseObject");
	NETVAR(bool, m_bCarryDeploy, "DT_BaseObject");
	NETVAR(bool, m_bMiniBuilding, "DT_BaseObject");
	NETVAR(float, m_flPercentageConstructed, "DT_BaseObject");
	NETVAR(int, m_fObjectFlags, "DT_BaseObject");
	NETVAR(EHANDLE, m_hBuiltOnEntity, "DT_BaseObject");
	NETVAR(bool, m_bDisabled, "DT_BaseObject");
	NETVAR(EHANDLE, m_hBuilder, "DT_BaseObject");
	NETVAR(Vector, m_vecBuildMaxs, "DT_BaseObject");
	NETVAR(Vector, m_vecBuildMins, "DT_BaseObject");
	NETVAR(int, m_iDesiredBuildRotations, "DT_BaseObject");
	NETVAR(bool, m_bServerOverridePlacement, "DT_BaseObject");
	NETVAR(int, m_iUpgradeLevel, "DT_BaseObject");
	NETVAR(int, m_iUpgradeMetal, "DT_BaseObject");
	NETVAR(int, m_iUpgradeMetalRequired, "DT_BaseObject");
	NETVAR(int, m_iHighestUpgradeLevel, "DT_BaseObject");
	NETVAR(int, m_iObjectMode, "DT_BaseObject");
	NETVAR(bool, m_bDisposableBuilding, "DT_BaseObject");
	NETVAR(bool, m_bWasMapPlaced, "DT_BaseObject");
	NETVAR(bool, m_bPlasmaDisable, "DT_BaseObject");

public:
	bool IsDisabled()
	{
		return m_bDisabled() || m_bHasSapper();
	}
};

class C_ObjectDispenser : public C_BaseObject
{
public:
	NETVAR(int, m_iState, "DT_ObjectDispenser");
	NETVAR(int, m_iAmmoMetal, "DT_ObjectDispenser");
	NETVAR(int, m_iMiniBombCounter, "DT_ObjectDispenser");
};

class C_ObjectSentrygun : public C_BaseObject
{
public:
	NETVAR(int, m_iAmmoShells, "DT_ObjectSentrygun");
	NETVAR(int, m_iAmmoRockets, "DT_ObjectSentrygun");
	NETVAR(int, m_iState, "DT_ObjectSentrygun");
	NETVAR(bool, m_bPlayerControlled, "DT_ObjectSentrygun");
	NETVAR(int, m_nShieldLevel, "DT_ObjectSentrygun");
	NETVAR(int, m_bShielded, "DT_ObjectSentrygun");
	NETVAR(EHANDLE, m_hEnemy, "DT_ObjectSentrygun");
	NETVAR(EHANDLE, m_hAutoAimTarget, "DT_ObjectSentrygun");
	NETVAR(int, m_iKills, "DT_SentrygunLocalData");
	NETVAR(int, m_iAssists, "DT_SentrygunLocalData");
};

class C_ObjectTeleporter : public C_BaseObject
{
public:
	NETVAR(int, m_iState, "DT_ObjectTeleporter");
	NETVAR(float, m_flRechargeTime, "DT_ObjectTeleporter");
	NETVAR(float, m_flCurrentRechargeDuration, "DT_ObjectTeleporter");
	NETVAR(int, m_iTimesUsed, "DT_ObjectTeleporter");
	NETVAR(float, m_flYawToExit, "DT_ObjectTeleporter");
	NETVAR(bool, m_bMatchBuilding, "DT_ObjectTeleporter");
};

class C_CurrencyPack : public C_BaseAnimating
{
public:
	NETVAR(bool, m_bDistributed, "DT_CurrencyPack");
};

class C_PlayerResource : public C_BaseEntity
{
public:
	int GetPing(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_iPing")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetScore(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_iScore")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetDeaths(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_iDeaths")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	bool GetConnected(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_bConnected")) };
		return *reinterpret_cast<bool *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(bool) * index);
	}

	bool GetAlive(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_bAlive")) };
		return *reinterpret_cast<bool *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(bool) * index);
	}

	void SetAlive(int index, bool value)
	{
		if (index < 0 || index > 101)
			return;

		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_bAlive")) };
		*reinterpret_cast<bool *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(bool) * index) = value;
	}

	int GetHealth(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_iHealth")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetAccountID(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_iAccountID")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	bool GetValid(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_bValid")) };
		return *reinterpret_cast<bool *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(bool) * index);
	}

	int GetUserID(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_PlayerResource"), HASH_CT("m_iUserID")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}
};

class C_TFPlayerResource : public C_PlayerResource
{
public:
	int GetTotalScore(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iTotalScore")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetMaxHealth(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iMaxHealth")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetMaxBuffedHealth(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iMaxBuffedHealth")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetPlayerClass(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iPlayerClass")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	bool GetArenaSpectator(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_bArenaSpectator")) };
		return *reinterpret_cast<bool *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(bool) * index);
	}

	int GetActiveDominations(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iActiveDominations")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	float GetNextRespawnTime(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_flNextRespawnTime")) };
		return *reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(float) * index);
	}

	int GetChargeLevel(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iChargeLevel")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetDamage(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iDamage")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetDamageAssist(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iDamageAssist")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetDamageBoss(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iDamageBoss")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetHealingAssist(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iHealingAssist")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetDamageBlocked(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iDamageBlocked")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetCurrencyCollected(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iCurrencyCollected")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetBonusPoints(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iBonusPoints")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetPlayerLevel(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iPlayerLevel")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetStreaks(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iStreaks")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetUpgradeRefundCredits(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iUpgradeRefundCredits")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetBuybackCredits(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iBuybackCredits")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetPlayerClassWhenKilled(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iPlayerClassWhenKilled")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	int GetConnectionState(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_iConnectionState")) };
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(int) * index);
	}

	float GetConnectTime(int index)
	{
		static uintptr_t offset{ netvars->getOffset(HASH_CT("DT_TFPlayerResource"), HASH_CT("m_flConnectTime")) };
		return *reinterpret_cast<float *>(reinterpret_cast<uintptr_t>(this) + offset + sizeof(float) * index);
	}

	NETVAR(int, m_iPartyLeaderRedTeamIndex, "DT_TFPlayerResource");
	NETVAR(int, m_iPartyLeaderBlueTeamIndex, "DT_TFPlayerResource");
	NETVAR(int, m_iEventTeamStatus, "DT_TFPlayerResource");
};

class CMultiPlayerAnimState
{
private:
	void *thisptr{};

public:
	bool m_bForceAimYaw{};
	CUtlVector<GestureSlot_t> m_aGestureSlots{};
	C_BasePlayer *m_pPlayer{};
	QAngle m_angRender{};
	bool m_bPoseParameterInit{};
	MultiPlayerPoseData_t m_PoseParameterData{};
	DebugPlayerAnimData_t m_DebugAnimData{};
	bool m_bCurrentFeetYawInitialized{};
	float m_flLastAnimationStateClearTime{};
	float m_flEyeYaw{};
	float m_flEyePitch{};
	float m_flGoalFeetYaw{};
	float m_flCurrentFeetYaw{};
	float m_flLastAimTurnTime{};
	MultiPlayerMovementData_t m_MovementData{};
	bool m_bJumping{};
	float m_flJumpStartTime{};
	bool m_bFirstJumpFrame{};
	bool m_bInSwim{};
	bool m_bFirstSwimFrame{};
	bool m_bDying{};
	bool m_bFirstDyingFrame{};
	Activity m_eCurrentMainSequenceActivity{};
	int m_nSpecificMainSequence{};
	CHandle<C_BaseCombatWeapon> m_hActiveWeapon{};
	float m_flLastGroundSpeedUpdateTime{};
	CInterpolatedVar<float> m_iv_flMaxGroundSpeed{};
	float m_flMaxGroundSpeed{};
	int m_nMovementSequence{};
	LegAnimType_t m_LegAnimType{};
};

class CTFPlayerAnimState : public CMultiPlayerAnimState
{
public:
	C_TFPlayer *m_pTFPlayer{};
	bool m_bInAirWalk{};
	float m_flHoldDeployedPoseUntilTime{};
	float m_flTauntMoveX{};
	float m_flTauntMoveY{};
	float m_flVehicleLeanVel{};
	float m_flVehicleLeanPos{};
	Vector m_vecSmoothedUp{};

public:
	void ClearAnimationState()
	{
		mem::callVirtual<1, void>(this);
	}

	void Update(float eyeYaw, float eyePitch)
	{
		mem::callVirtual<4, void, float, float>(this, eyeYaw, eyePitch);
	}
};

class C_TFGameRulesProxy : public C_BaseEntity
{
public:
	NETVAR(int, m_iRoundState, "DT_TeamplayRoundBasedRules");
	NETVAR(byte, m_bInWaitingForPlayers, "DT_TeamplayRoundBasedRules");
	NETVAR(int, m_iWinningTeam, "DT_TeamplayRoundBasedRules");
	NETVAR(byte, m_bInOvertime, "DT_TeamplayRoundBasedRules");
	NETVAR(byte, m_bInSetup, "DT_TeamplayRoundBasedRules");
	NETVAR(byte, m_bSwitchedTeamsThisRound, "DT_TeamplayRoundBasedRules");
	NETVAR(byte, m_bAwaitingReadyRestart, "DT_TeamplayRoundBasedRules");
	NETVAR(float, m_flRestartRoundTime, "DT_TeamplayRoundBasedRules");
	NETVAR(float, m_flMapResetTime, "DT_TeamplayRoundBasedRules");
	NETVAR(int, m_nRoundsPlayed, "DT_TeamplayRoundBasedRules");
	NETVAR(float *, m_flNextRespawnWave, "DT_TeamplayRoundBasedRules");
	NETVAR(float *, m_TeamRespawnWaveTimes, "DT_TeamplayRoundBasedRules");
	NETVAR(bool *, m_bTeamReady, "DT_TeamplayRoundBasedRules");
	NETVAR(int, m_bStopWatch, "DT_TeamplayRoundBasedRules");
	NETVAR(byte, m_bMultipleTrains, "DT_TeamplayRoundBasedRules");
	//NETVAR(bool *, m_bPlayerReady, "DT_TeamplayRoundBasedRules");
	NETVAR(byte, m_bCheatsEnabledDuringLevel, "DT_TeamplayRoundBasedRules");
	NETVAR(float, m_flCountdownTime, "DT_TeamplayRoundBasedRules");
	NETVAR(float, m_flStateTransitionTime, "DT_TeamplayRoundBasedRules");
	NETVAR(int, m_nGameType, "DT_TFGameRules");
	NETVAR(int, m_nStopWatchState, "DT_TFGameRules");
	NETVAR(const char *, m_pszTeamGoalStringRed, "DT_TFGameRules");
	NETVAR(const char *, m_pszTeamGoalStringBlue, "DT_TFGameRules");
	NETVAR(float, m_flCapturePointEnableTime, "DT_TFGameRules");
	NETVAR(int, m_nHudType, "DT_TFGameRules");
	NETVAR(byte, m_bIsInTraining, "DT_TFGameRules");
	NETVAR(byte, m_bAllowTrainingAchievements, "DT_TFGameRules");
	NETVAR(byte, m_bIsWaitingForTrainingContinue, "DT_TFGameRules");
	NETVAR(byte, m_bIsTrainingHUDVisible, "DT_TFGameRules");
	NETVAR(byte, m_bIsInItemTestingMode, "DT_TFGameRules");
	NETVAR(EHANDLE, m_hBonusLogic, "DT_TFGameRules");
	NETVAR(byte, m_bPlayingKoth, "DT_TFGameRules");
	NETVAR(byte, m_bPlayingMedieval, "DT_TFGameRules");
	NETVAR(byte, m_bPlayingHybrid_CTF_CP, "DT_TFGameRules");
	NETVAR(byte, m_bPlayingSpecialDeliveryMode, "DT_TFGameRules");
	NETVAR(byte, m_bPlayingRobotDestructionMode, "DT_TFGameRules");
	NETVAR(EHANDLE, m_hRedKothTimer, "DT_TFGameRules");
	NETVAR(EHANDLE, m_hBlueKothTimer, "DT_TFGameRules");
	NETVAR(int, m_nMapHolidayType, "DT_TFGameRules");
	NETVAR(int, m_itHandle, "DT_TFGameRules");
	NETVAR(byte, m_bPlayingMannVsMachine, "DT_TFGameRules");
	NETVAR(EHANDLE, m_hBirthdayPlayer, "DT_TFGameRules");
	NETVAR(int, m_nBossHealth, "DT_TFGameRules");
	NETVAR(int, m_nMaxBossHealth, "DT_TFGameRules");
	NETVAR(int, m_fBossNormalizedTravelDistance, "DT_TFGameRules");
	NETVAR(byte, m_bMannVsMachineAlarmStatus, "DT_TFGameRules");
	NETVAR(byte, m_bHaveMinPlayersToEnableReady, "DT_TFGameRules");
	NETVAR(byte, m_bBountyModeEnabled, "DT_TFGameRules");
	NETVAR(int, m_nHalloweenEffect, "DT_TFGameRules");
	NETVAR(float, m_fHalloweenEffectStartTime, "DT_TFGameRules");
	NETVAR(float, m_fHalloweenEffectDuration, "DT_TFGameRules");
	NETVAR(EHANDLE, m_halloweenScenario, "DT_TFGameRules");
	NETVAR(byte, m_bHelltowerPlayersInHell, "DT_TFGameRules");
	NETVAR(byte, m_bIsUsingSpells, "DT_TFGameRules");
	NETVAR(int, m_bCompetitiveMode, "DT_TFGameRules");
	NETVAR(int, m_nMatchGroupType, "DT_TFGameRules");
	NETVAR(byte, m_bMatchEnded, "DT_TFGameRules");
	NETVAR(byte, m_bPowerupMode, "DT_TFGameRules");
	NETVAR(const char *, m_pszCustomUpgradesFile, "DT_TFGameRules");
	NETVAR(byte, m_bTruceActive, "DT_TFGameRules");
	NETVAR(int, m_bShowMatchSummary, "DT_TFGameRules");
	NETVAR(int, m_bShowCompetitiveMatchSummary, "DT_TFGameRules");
	NETVAR(byte, m_bTeamsSwitched, "DT_TFGameRules");
	NETVAR(byte, m_bMapHasMatchSummaryStage, "DT_TFGameRules");
	NETVAR(byte, m_bPlayersAreOnMatchSummaryStage, "DT_TFGameRules");
	NETVAR(byte, m_bStopWatchWinner, "DT_TFGameRules");
	NETVAR(int, m_eRematchState, "DT_TFGameRules");
	NETVAR(int, m_nForceUpgrades, "DT_TFGameRules");
	NETVAR(int, m_nForceEscortPushLogic, "DT_TFGameRules");
	NETVAR(byte, m_bRopesHolidayLightsAllowed, "DT_TFGameRules");

	inline bool IsPlayerReady(int iIndex)
	{
		static std::size_t offset{ netvars->getOffset(HASH_CT("DT_TeamplayRoundBasedRules"), HASH_CT("m_bPlayerReady")) };

		std::uintptr_t player_ready{ reinterpret_cast<std::uintptr_t>(this) + offset };

		if (iIndex < 0 || iIndex >= 100) {
			return false;
		}

		return *reinterpret_cast<bool *>(player_ready + iIndex);
	}
};

class C_BreakableProp : public C_BaseAnimating
{
public:
};

class C_DynamicProp : public C_BreakableProp
{
public:
	NETVAR(byte, m_bUseHitboxesForRenderBox, "DT_DynamicProp"); // + 3096
};

class C_TFItem : public C_DynamicProp
{
public:
};

class C_CaptureFlag : public C_TFItem
{
public:
	NETVAR(int, m_bDisabled, "DT_CaptureFlag"); // + 3152
	NETVAR(int, m_bVisibleWhenDisabled, "DT_CaptureFlag"); // + 3153
	NETVAR(int, m_nType, "DT_CaptureFlag"); // + 3156
	NETVAR(int, m_nFlagStatus, "DT_CaptureFlag"); // + 3160
	NETVAR(float, m_flResetTime, "DT_CaptureFlag"); // + 3164
	NETVAR(float, m_flNeutralTime, "DT_CaptureFlag"); // + 3172
	NETVAR(float, m_flMaxResetTime, "DT_CaptureFlag"); // + 3168
	NETVAR(EHANDLE, m_hPrevOwner, "DT_CaptureFlag"); // + 3176
	NETVAR(const char *, m_szModel, "DT_CaptureFlag"); // + 3189
	NETVAR(const char *, m_szHudIcon, "DT_CaptureFlag"); // + 3449
	NETVAR(const char *, m_szPaperEffect, "DT_CaptureFlag"); // + 3709
	NETVAR(const char *, m_szTrailEffect, "DT_CaptureFlag"); // + 3969
	NETVAR(int, m_nUseTrailEffect, "DT_CaptureFlag"); // + 4232
	NETVAR(int, m_nPointValue, "DT_CaptureFlag"); // + 3180
	NETVAR(float, m_flAutoCapTime, "DT_CaptureFlag"); // + 3184
	NETVAR(byte, m_bGlowEnabled, "DT_CaptureFlag"); // + 3188
	NETVAR(float, m_flTimeToSetPoisonous, "DT_CaptureFlag"); // + 4268
};

class C_BaseViewModel : public C_BaseAnimating
{
public:
	NETVAR(short, m_nModelIndex, "DT_BaseViewModel"); // + 204
	NETVAR(int, m_nSkin, "DT_BaseViewModel"); // + 2000
	NETVAR(int, m_nBody, "DT_BaseViewModel"); // + 2004
	NETVAR(int, m_nSequence, "DT_BaseViewModel"); // + 2824
	NETVAR(int, m_nViewModelIndex, "DT_BaseViewModel"); // + 3116
	NETVAR(float, m_flPlaybackRate, "DT_BaseViewModel"); // + 2060
	NETVAR(int, m_fEffects, "DT_BaseViewModel"); // + 184
	NETVAR(int, m_nAnimationParity, "DT_BaseViewModel"); // + 3132
	NETVAR(EHANDLE, m_hWeapon, "DT_BaseViewModel"); // + 3156
	NETVAR(EHANDLE, m_hOwner, "DT_BaseViewModel"); // + 3120
	NETVAR(int, m_nNewSequenceParity, "DT_BaseViewModel"); // + 2780
	NETVAR(int, m_nResetEventsParity, "DT_BaseViewModel"); // + 2784
	NETVAR(byte, m_nMuzzleFlashParity, "DT_BaseViewModel"); // + 3040
	NETVAR(float, m_flPoseParameter, "DT_BaseViewModel"); // + 2336
};

class C_TFViewModel : public C_BaseViewModel
{};