#pragma once

#include "sigs.hpp"

namespace tf_utils
{
	inline float attribHookValue(float value, const char *name, void *econent)
	{
		return s::CAttributeManager_AttribHookValue.call<float>(value, name, econent, nullptr, true);
	}
}

#undef min
#undef max
#undef GetProp
#undef CreateFont

enum
{
	SPEED_CROPPED_RESET = 0,
	SPEED_CROPPED_DUCK = 1,
	SPEED_CROPPED_WEAPON = 2
};

#define TF_MAX_SPEED (400.0f * 1.3f)
#define MAX_PLAYERS 33
#define	STOP_EPSILON 0.1f
#define	MAX_CLIP_PLANES 5

#define TIME_TO_UNDUCK 0.2f
#define TIME_TO_UNDUCK_MS 200.0f

#define GAMEMOVEMENT_DUCK_TIME 1000.0f
#define GAMEMOVEMENT_JUMP_TIME 510.0f
#define GAMEMOVEMENT_JUMP_HEIGHT 21.0f
#define GAMEMOVEMENT_TIME_TO_UNDUCK ( TIME_TO_UNDUCK * 1000.0f )
#define GAMEMOVEMENT_TIME_TO_UNDUCK_INV ( GAMEMOVEMENT_DUCK_TIME - GAMEMOVEMENT_TIME_TO_UNDUCK )

enum
{
	PITCH,
	YAW,
	ROLL
};

typedef enum
{
	GROUND,
	STUCK,
	LADDER
} IntervalType_t;

struct surfacephysicsparams_t
{
	float friction{};
	float elasticity{};
	float density{};
	float thickness{};
	float dampening{};
};

struct surfaceaudioparams_t
{
	float reflectivity{};
	float hardnessFactor{};
	float roughnessFactor{};
	float roughThreshold{};
	float hardThreshold{};
	float hardVelocityThreshold{};
};

struct surfacegameprops_t
{
	float maxSpeedFactor{};
	float jumpFactor{};
	unsigned short material{};
	unsigned char climbable{};
	unsigned char pad{};
};

struct surfacesoundnames_t
{
	unsigned short stepleft{};
	unsigned short stepright{};
	unsigned short impactSoft{};
	unsigned short impactHard{};
	unsigned short scrapeSmooth{};
	unsigned short scrapeRough{};
	unsigned short bulletImpact{};
	unsigned short rolling{};
	unsigned short breakSound{};
	unsigned short strainSound{};
};

struct surfacesoundhandles_t
{
	short stepleft{};
	short stepright{};
	short impactSoft{};
	short impactHard{};
	short scrapeSmooth{};
	short scrapeRough{};
	short bulletImpact{};
	short rolling{};
	short breakSound{};
	short strainSound{};
};

struct surfacedata_t
{
	surfacephysicsparams_t physics{};
	surfaceaudioparams_t audio{};
	surfacesoundnames_t sounds{};
	surfacegameprops_t game{};
	surfacesoundhandles_t soundhandles{};
};

enum
{
	MAX_PC_CACHE_SLOTS = 3
};

class Vector4D
{
public:
	float x{};
	float y{};
	float z{};
	float w{};
};

enum MapLoadType_t
{
	MapLoad_NewGame = 0,
	MapLoad_LoadGame,
	MapLoad_Transition,
	MapLoad_Background,
};

typedef const char *string_t;

using Vector = vec3;
using Vector2D = vec2;
using QAngle = vec3;
using matrix3x4_t = matrix3x4;
using RadianEuler = Vector;
using Quaternion = Vector4D;

using byte = unsigned char;

class IClientNetworkable;

typedef IClientNetworkable *(*CreateClientClassFn)(int entnum, int serialNum);
typedef IClientNetworkable *(*CreateEventFn)();

typedef enum
{
	DPT_Int,
	DPT_Float,
	DPT_Vector,
	DPT_VectorXY,
	DPT_String,
	DPT_Array,
	DPT_DataTable,
	DPT_NUMSendPropTypes
} SendPropType;

class DVariant
{
public:
	DVariant() { m_Type = DPT_Float; }
	DVariant(float val) { m_Type = DPT_Float; m_Float = val; }

	union
	{
		float m_Float;
		int m_Int{};
		const char *m_pString;
		void *m_pData;
	};

	SendPropType m_Type{};
};

class RecvTable;
class RecvProp;

class CRecvProxyData
{
public:
	const RecvProp *m_pRecvProp{};
	DVariant m_Value{};
	int m_iElement{};
	int m_ObjectID{};
};

typedef void (*RecvVarProxyFn)(const CRecvProxyData *pData, void *pStruct, void *pOut);
typedef void (*ArrayLengthRecvProxyFn)(void *pStruct, int objectID, int currentArrayLength);
typedef void (*DataTableRecvVarProxyFn)(const RecvProp *pProp, void **pOut, void *pData, int objectID);

class CStandardRecvProxies
{
public:
	RecvVarProxyFn m_Int32ToInt8{};
	RecvVarProxyFn m_Int32ToInt16{};
	RecvVarProxyFn m_Int32ToInt32{};
	RecvVarProxyFn m_FloatToFloat{};
	RecvVarProxyFn m_VectorToVector{};
};

class CRecvDecoder;

class RecvProp
{
public:
	void InitArray(int nElements, int elementStride);
	int GetNumElements() const;
	void SetNumElements(int nElements);
	int GetElementStride() const;
	void SetElementStride(int stride);
	int GetFlags() const;
	const char *GetName() const;
	SendPropType GetType() const;
	RecvTable *GetDataTable() const;
	void SetDataTable(RecvTable *pTable);
	RecvVarProxyFn GetProxyFn() const;
	void SetProxyFn(RecvVarProxyFn fn);
	DataTableRecvVarProxyFn	GetDataTableProxyFn() const;
	void SetDataTableProxyFn(DataTableRecvVarProxyFn fn);
	int GetOffset() const;
	void SetOffset(int o);
	RecvProp *GetArrayProp() const;
	void SetArrayProp(RecvProp *pProp);
	void SetArrayLengthProxy(ArrayLengthRecvProxyFn proxy);
	ArrayLengthRecvProxyFn	GetArrayLengthProxy() const;
	bool IsInsideArray() const;
	void SetInsideArray();
	const void *GetExtraData() const;
	void SetExtraData(const void *pData);
	const char *GetParentArrayPropName();
	void SetParentArrayPropName(const char *pArrayPropName);

public:
	const char *m_pVarName{};
	SendPropType m_RecvType{};
	int m_Flags{};
	int m_StringBufferSize{};

private:
	bool m_bInsideArray{};
	const void *m_pExtraData{};
	RecvProp *m_pArrayProp{};
	ArrayLengthRecvProxyFn	m_ArrayLengthProxy{};
	RecvVarProxyFn m_ProxyFn{};
	DataTableRecvVarProxyFn	m_DataTableProxyFn{};
	RecvTable *m_pDataTable{};
	int m_Offset{};
	int m_ElementStride{};
	int m_nElements{};
	const char *m_pParentArrayPropName{};
};

inline void RecvProp::InitArray(int nElements, int elementStride)
{
	m_RecvType = DPT_Array;
	m_nElements = nElements;
	m_ElementStride = elementStride;
}

inline int RecvProp::GetNumElements() const
{
	return m_nElements;
}

inline void RecvProp::SetNumElements(int nElements)
{
	m_nElements = nElements;
}

inline int RecvProp::GetElementStride() const
{
	return m_ElementStride;
}

inline void RecvProp::SetElementStride(int stride)
{
	m_ElementStride = stride;
}

inline int RecvProp::GetFlags() const
{
	return m_Flags;
}

inline const char *RecvProp::GetName() const
{
	return m_pVarName;
}

inline SendPropType RecvProp::GetType() const
{
	return m_RecvType;
}

inline RecvTable *RecvProp::GetDataTable() const
{
	return m_pDataTable;
}

inline void RecvProp::SetDataTable(RecvTable *pTable)
{
	m_pDataTable = pTable;
}

inline RecvVarProxyFn RecvProp::GetProxyFn() const
{
	return m_ProxyFn;
}

inline void RecvProp::SetProxyFn(RecvVarProxyFn fn)
{
	m_ProxyFn = fn;
}

inline DataTableRecvVarProxyFn RecvProp::GetDataTableProxyFn() const
{
	return m_DataTableProxyFn;
}

inline void RecvProp::SetDataTableProxyFn(DataTableRecvVarProxyFn fn)
{
	m_DataTableProxyFn = fn;
}

inline int RecvProp::GetOffset() const
{
	return m_Offset;
}

inline void RecvProp::SetOffset(int o)
{
	m_Offset = o;
}

inline RecvProp *RecvProp::GetArrayProp() const
{
	return m_pArrayProp;
}

inline void RecvProp::SetArrayProp(RecvProp *pProp)
{
	m_pArrayProp = pProp;
}

inline void RecvProp::SetArrayLengthProxy(ArrayLengthRecvProxyFn proxy)
{
	m_ArrayLengthProxy = proxy;
}

inline ArrayLengthRecvProxyFn RecvProp::GetArrayLengthProxy() const
{
	return m_ArrayLengthProxy;
}

inline bool RecvProp::IsInsideArray() const
{
	return m_bInsideArray;
}

inline void RecvProp::SetInsideArray()
{
	m_bInsideArray = true;
}

inline const void *RecvProp::GetExtraData() const
{
	return m_pExtraData;
}

inline void RecvProp::SetExtraData(const void *pData)
{
	m_pExtraData = pData;
}

inline const char *RecvProp::GetParentArrayPropName()
{
	return m_pParentArrayPropName;
}

inline void	RecvProp::SetParentArrayPropName(const char *pArrayPropName)
{
	m_pParentArrayPropName = pArrayPropName;
}

class RecvTable
{
public:
	void Construct(RecvProp *pProps, int nProps, const char *pNetTableName);
	int GetNumProps() const;
	RecvProp *GetProp(int i);
	const char *GetName() const;
	void SetInitialized(bool bInitialized);
	bool IsInitialized() const;
	void SetInMainList(bool bInList);
	bool IsInMainList() const;

public:
	RecvProp *m_pProps{};
	int m_nProps{};
	CRecvDecoder *m_pDecoder{};
	const char *m_pNetTableName{};


private:
	bool m_bInitialized{};
	bool m_bInMainList{};
};

inline void RecvTable::Construct(RecvProp *pProps, int nProps, const char *pNetTableName)
{
	m_pProps = pProps;
	m_nProps = nProps;
	m_pDecoder = 0;
	m_pNetTableName = pNetTableName;
	m_bInitialized = false;
	m_bInMainList = false;
}

inline int RecvTable::GetNumProps() const
{
	return m_nProps;
}

inline RecvProp *RecvTable::GetProp(int i)
{
	return &m_pProps[i];
}

inline const char *RecvTable::GetName() const
{
	return m_pNetTableName;
}

inline void RecvTable::SetInitialized(bool bInitialized)
{
	m_bInitialized = bInitialized;
}

inline bool RecvTable::IsInitialized() const
{
	return m_bInitialized;
}

inline void RecvTable::SetInMainList(bool bInList)
{
	m_bInMainList = bInList;
}

inline bool RecvTable::IsInMainList() const
{
	return m_bInMainList;
}

#define MAX_EDICT_BITS 11
#define NUM_ENT_ENTRY_BITS (MAX_EDICT_BITS + 1)
#define NUM_ENT_ENTRIES (1 << NUM_ENT_ENTRY_BITS)
#define ENT_ENTRY_MASK (NUM_ENT_ENTRIES - 1)
#define INVALID_EHANDLE_INDEX 0xFFFFFFFF

class CBaseHandle;

class IHandleEntity
{
public:
	virtual ~IHandleEntity() {}
	virtual void SetRefEHandle(const CBaseHandle &handle) = 0;
	virtual const CBaseHandle &GetRefEHandle() const = 0;
};

class CBaseHandle
{
public:
	CBaseHandle();
	CBaseHandle(const CBaseHandle &other);
	CBaseHandle(unsigned long value);
	CBaseHandle(int iEntry, int iSerialNumber);

public:
	void Init(int iEntry, int iSerialNumber);
	void Term();

public:
	bool IsValid() const;

public:
	int GetEntryIndex() const;
	int GetSerialNumber() const;
	int ToInt() const;
	unsigned long GetIndex() const;

public:
	bool operator !=(const CBaseHandle &other) const;
	bool operator ==(const CBaseHandle &other) const;
	bool operator ==(const IHandleEntity *pEnt) const;
	bool operator !=(const IHandleEntity *pEnt) const;
	bool operator <(const CBaseHandle &other) const;
	bool operator <(const IHandleEntity *pEnt) const;

public:
	const CBaseHandle &operator=(const IHandleEntity *pEntity);
	const CBaseHandle &Set(const IHandleEntity *pEntity);

public:
	IHandleEntity *Get() const;

protected:
	unsigned long m_Index{};
};

inline unsigned long CBaseHandle::GetIndex() const
{
	return m_Index;
}

inline CBaseHandle::CBaseHandle()
{
	m_Index = INVALID_EHANDLE_INDEX;
}

inline CBaseHandle::CBaseHandle(const CBaseHandle &other)
{
	m_Index = other.m_Index;
}

inline CBaseHandle::CBaseHandle(unsigned long value)
{
	m_Index = value;
}

inline CBaseHandle::CBaseHandle(int iEntry, int iSerialNumber)
{
	Init(iEntry, iSerialNumber);
}

inline void CBaseHandle::Init(int iEntry, int iSerialNumber)
{
	m_Index = iEntry | (iSerialNumber << NUM_ENT_ENTRY_BITS);
}

inline void CBaseHandle::Term()
{
	m_Index = INVALID_EHANDLE_INDEX;
}

inline bool CBaseHandle::IsValid() const
{
	return m_Index != INVALID_EHANDLE_INDEX;
}

inline int CBaseHandle::GetEntryIndex() const
{
	return m_Index & ENT_ENTRY_MASK;
}

inline int CBaseHandle::GetSerialNumber() const
{
	return m_Index >> NUM_ENT_ENTRY_BITS;
}

inline int CBaseHandle::ToInt() const
{
	return static_cast<int>(m_Index);
}

inline bool CBaseHandle::operator !=(const CBaseHandle &other) const
{
	return m_Index != other.m_Index;
}

inline bool CBaseHandle::operator ==(const CBaseHandle &other) const
{
	return m_Index == other.m_Index;
}

inline bool CBaseHandle::operator ==(const IHandleEntity *pEnt) const
{
	return Get() == pEnt;
}

inline bool CBaseHandle::operator !=(const IHandleEntity *pEnt) const
{
	return Get() != pEnt;
}

inline bool CBaseHandle::operator <(const CBaseHandle &other) const
{
	return m_Index < other.m_Index;
}

inline bool CBaseHandle::operator <(const IHandleEntity *pEntity) const
{
	unsigned long otherIndex{ pEntity ? pEntity->GetRefEHandle().m_Index : INVALID_EHANDLE_INDEX };

	return m_Index < otherIndex;
}

inline const CBaseHandle &CBaseHandle::operator=(const IHandleEntity *pEntity)
{
	return Set(pEntity);
}

inline const CBaseHandle &CBaseHandle::Set(const IHandleEntity *pEntity)
{
	if (pEntity) {
		*this = pEntity->GetRefEHandle();
	}

	else {
		m_Index = INVALID_EHANDLE_INDEX;
	}

	return *this;
}

template <class T>
class CHandle : public CBaseHandle
{
public:
	CHandle();
	CHandle(int iEntry, int iSerialNumber);
	CHandle(const CBaseHandle &handle);
	CHandle(T *pVal);

public:
	static CHandle<T> FromIndex(int index);

public:
	T *Get() const;
	void Set(const T *pVal);

public:
	operator T *();
	operator T *() const;

public:
	bool operator !() const;
	bool operator==(T *val) const;
	bool operator!=(T *val) const;
	const CBaseHandle &operator=(const T *val);

public:
	T *operator->() const;
};

template <class T>
inline CHandle<T>::CHandle()
{
}

template <class T>
inline CHandle<T>::CHandle(int iEntry, int iSerialNumber)
{
	Init(iEntry, iSerialNumber);
}

template <class T>
inline CHandle<T>::CHandle(const CBaseHandle &handle) : CBaseHandle(handle)
{
}

template <class T>
inline CHandle<T>::CHandle(T *pObj)
{
	Term();
	Set(pObj);
}

template <class T>
inline CHandle<T> CHandle<T>::FromIndex(int index)
{
	CHandle<T> ret;
	ret.m_Index = index;
	return ret;
}

template <class T>
inline T *CHandle<T>::Get() const
{
	return reinterpret_cast<T *>(CBaseHandle::Get());
}

template <class T>
inline CHandle<T>::operator T *()
{
	return Get();
}

template <class T>
inline CHandle<T>::operator T *() const
{
	return Get();
}

template <class T>
inline bool CHandle<T>::operator !() const
{
	return !Get();
}

template <class T>
inline bool CHandle<T>::operator==(T *val) const
{
	return Get() == val;
}

template <class T>
inline bool CHandle<T>::operator!=(T *val) const
{
	return Get() != val;
}

template <class T>
inline void CHandle<T>::Set(const T *pVal)
{
	CBaseHandle::Set(reinterpret_cast<const IHandleEntity *>(pVal));
}

template <class T>
inline const CBaseHandle &CHandle<T>::operator=(const T *val)
{
	Set(val);
	return *this;
}

template <class T>
inline T *CHandle<T>::operator ->() const
{
	return Get();
}

class C_BaseEntity;

using EHANDLE = CHandle<C_BaseEntity>;

class ClientClass
{
public:
	const char *GetName() const
	{
		return m_pNetworkName;
	}

public:
	CreateClientClassFn m_pCreateFn{};
	CreateEventFn m_pCreateEventFn{};
	const char *m_pNetworkName{};
	RecvTable *m_pRecvTable{};
	ClientClass *m_pNext{};
	int m_ClassID{};
};

class VMatrix
{
private:
	Vector m[4][4]{};

public:
	inline const matrix3x4_t &As3x4() const
	{
		return *((const matrix3x4_t *)this);
	}
};

enum StereoEye_t
{
	STEREO_EYE_MONO = 0,
	STEREO_EYE_LEFT = 1,
	STEREO_EYE_RIGHT = 2,
	STEREO_EYE_MAX = 3
};

class CViewSetup
{
public:
	int x{};
	int m_nUnscaledX{};
	int y{};
	int m_nUnscaledY{};
	int width{};
	int m_nUnscaledWidth{};
	int height{};
	StereoEye_t m_eStereoEye{};
	int m_nUnscaledHeight{};
	bool m_bOrtho{};
	float m_OrthoLeft{};
	float m_OrthoTop{};
	float m_OrthoRight{};
	float m_OrthoBottom{};
	float fov{};
	float fovViewmodel{};
	Vector origin{};
	QAngle angles{};
	float zNear{};
	float zFar{};
	float zNearViewmodel{};
	float zFarViewmodel{};
	bool m_bRenderToSubrectOfLargerScreen{};
	float m_flAspectRatio{};
	bool m_bOffCenter{};
	float m_flOffCenterTop{};
	float m_flOffCenterBottom{};
	float m_flOffCenterLeft{};
	float m_flOffCenterRight{};
	bool m_bDoBloomAndToneMapping{};
	bool m_bCacheFullSceneState{};
	bool m_bViewToProjectionOverride{};
	VMatrix m_ViewToProjection{};
};

class CUserCmd
{
public:
	virtual ~CUserCmd() {};

public:
	int command_number{};
	int tick_count{};
	QAngle viewangles{};
	float forwardmove{};
	float sidemove{};
	float upmove{};
	int buttons{};
	byte impulse{};
	int weaponselect{};
	int weaponsubtype{};
	int random_seed{};
	short mousedx{};
	short mousedy{};
	bool hasbeenpredicted{};

	uint64_t GetCheckSum()
	{
		return s::CUserCmd_GetCheckSum.call<uint64_t>(this);
	}
};

class CVerifiedUserCmd
{
public:
	CUserCmd m_cmd;
	uint64_t m_crc;
};

#define FL_ONGROUND (1 << 0)
#define FL_DUCKING (1 << 1)
#define FL_WATERJUMP (1 << 2)
#define FL_ONTRAIN (1 << 3)
#define FL_INRAIN (1 << 4)
#define FL_FROZEN (1 << 5)
#define FL_ATCONTROLS (1 << 6)
#define FL_CLIENT (1 << 7)
#define FL_FAKECLIENT (1 << 8)
#define FL_INWATER (1 << 9)

#define IN_ATTACK (1 << 0)
#define IN_JUMP (1 << 1)
#define IN_DUCK (1 << 2)
#define IN_FORWARD (1 << 3)
#define IN_BACK (1 << 4)
#define IN_USE (1 << 5)
#define IN_CANCEL (1 << 6)
#define IN_LEFT (1 << 7)
#define IN_RIGHT (1 << 8)
#define IN_MOVELEFT (1 << 9)
#define IN_MOVERIGHT (1 << 10)
#define IN_ATTACK2 (1 << 11)
#define IN_RUN (1 << 12)
#define IN_RELOAD (1 << 13)
#define IN_ALT1 (1 << 14)
#define IN_ALT2 (1 << 15)
#define IN_SCORE (1 << 16)
#define IN_SPEED (1 << 17)
#define IN_WALK (1 << 18)
#define IN_ZOOM (1 << 19)
#define IN_WEAPON1 (1 << 20)
#define IN_WEAPON2 (1 << 21)
#define IN_BULLRUSH (1 << 22)
#define IN_GRENADE1 (1 << 23)
#define IN_GRENADE2 (1 << 24)
#define IN_ATTACK3 (1 << 25)

enum
{
	WL_NotInWater,
	WL_Feet,
	WL_Waist,
	WL_Eyes
};

enum MoveType_t
{
	MOVETYPE_NONE,
	MOVETYPE_ISOMETRIC,
	MOVETYPE_WALK,
	MOVETYPE_STEP,
	MOVETYPE_FLY,
	MOVETYPE_FLYGRAVITY,
	MOVETYPE_VPHYSICS,
	MOVETYPE_PUSH,
	MOVETYPE_NOCLIP,
	MOVETYPE_LADDER,
	MOVETYPE_OBSERVER,
	MOVETYPE_CUSTOM,
	MOVETYPE_LAST = MOVETYPE_CUSTOM,
	MOVETYPE_MAX_BITS = 4
};

typedef unsigned short ClientShadowHandle_t;

enum
{
	CLIENTSHADOW_INVALID_HANDLE = (ClientShadowHandle_t)~0
};

enum ShadowType_t
{
	SHADOWS_NONE = 0,
	SHADOWS_SIMPLE,
	SHADOWS_RENDER_TO_TEXTURE,
	SHADOWS_RENDER_TO_TEXTURE_DYNAMIC,
	SHADOWS_RENDER_TO_DEPTH_TEXTURE
};

class IPVSNotify
{
public:
	virtual void OnPVSStatusChanged(bool bInPVS) = 0;
};

using ClientRenderHandle_t = void *;
using ModelInstanceHandle_t = unsigned short;

enum ShouldTransmitState_t
{
	SHOULDTRANSMIT_START = 0,
	SHOULDTRANSMIT_END
};

enum DataUpdateType_t
{
	DATA_UPDATE_CREATED = 0,
	DATA_UPDATE_DATATABLE_CHANGED
};

typedef enum
{
	BITBUFERROR_VALUE_OUT_OF_RANGE = 0,		// Tried to write a value with too few bits.
	BITBUFERROR_BUFFER_OVERRUN,				// Was about to overrun a buffer.

	BITBUFERROR_NUM_ERRORS
} BitBufErrorType;


typedef void(*BitBufErrorHandler)(BitBufErrorType errorType, const char *pDebugName);

#define BITS_PER_INT 32

//-----------------------------------------------------------------------------
// Helpers.
//-----------------------------------------------------------------------------
template <typename T>
inline T WordSwapC(T w)
{
	uint16_t temp;

	temp = ((*((uint16_t *)&w) & 0xff00) >> 8);
	temp |= ((*((uint16_t *)&w) & 0x00ff) << 8);

	return *((T *)&temp);
}

template <typename T>
inline T DWordSwapC(T dw)
{
	uint32_t temp;

	temp = *((uint32_t *)&dw) >> 24;
	temp |= ((*((uint32_t *)&dw) & 0x00FF0000) >> 8);
	temp |= ((*((uint32_t *)&dw) & 0x0000FF00) << 8);
	temp |= ((*((uint32_t *)&dw) & 0x000000FF) << 24);

	return *((T *)&temp);
}

template <typename T>
inline T QWordSwapC(T dw)
{
	// Assert sizes passed to this are already correct, otherwise
	// the cast to uint64_t * below is unsafe and may have wrong results 
	// or even crash.
	//PLAT_COMPILE_TIME_nullAssert(sizeof(dw) == sizeof(uint64_t));

	uint64_t temp;

	temp = *((uint64_t *)&dw) >> 56;
	temp |= ((*((uint64_t *)&dw) & 0x00FF000000000000ull) >> 40);
	temp |= ((*((uint64_t *)&dw) & 0x0000FF0000000000ull) >> 24);
	temp |= ((*((uint64_t *)&dw) & 0x000000FF00000000ull) >> 8);
	temp |= ((*((uint64_t *)&dw) & 0x00000000FF000000ull) << 8);
	temp |= ((*((uint64_t *)&dw) & 0x0000000000FF0000ull) << 24);
	temp |= ((*((uint64_t *)&dw) & 0x000000000000FF00ull) << 40);
	temp |= ((*((uint64_t *)&dw) & 0x00000000000000FFull) << 56);

	return *((T *)&temp);
}

inline int BitByte(int bits)
{
	// return PAD_NUMBER( bits, 8 ) >> 3;
	return (bits + 7) >> 3;
}

inline unsigned GetEndMask(int numBits)
{
	static unsigned bitStringEndMasks[] =
	{
		0xffffffff,
		0x00000001,
		0x00000003,
		0x00000007,
		0x0000000f,
		0x0000001f,
		0x0000003f,
		0x0000007f,
		0x000000ff,
		0x000001ff,
		0x000003ff,
		0x000007ff,
		0x00000fff,
		0x00001fff,
		0x00003fff,
		0x00007fff,
		0x0000ffff,
		0x0001ffff,
		0x0003ffff,
		0x0007ffff,
		0x000fffff,
		0x001fffff,
		0x003fffff,
		0x007fffff,
		0x00ffffff,
		0x01ffffff,
		0x03ffffff,
		0x07ffffff,
		0x0fffffff,
		0x1fffffff,
		0x3fffffff,
		0x7fffffff,
	};

	return bitStringEndMasks[numBits % BITS_PER_INT];
}


inline int GetBitForBitnum(int bitNum)
{
	static int bitsForBitnum[] =
	{
		(1 << 0),
		(1 << 1),
		(1 << 2),
		(1 << 3),
		(1 << 4),
		(1 << 5),
		(1 << 6),
		(1 << 7),
		(1 << 8),
		(1 << 9),
		(1 << 10),
		(1 << 11),
		(1 << 12),
		(1 << 13),
		(1 << 14),
		(1 << 15),
		(1 << 16),
		(1 << 17),
		(1 << 18),
		(1 << 19),
		(1 << 20),
		(1 << 21),
		(1 << 22),
		(1 << 23),
		(1 << 24),
		(1 << 25),
		(1 << 26),
		(1 << 27),
		(1 << 28),
		(1 << 29),
		(1 << 30),
		(1 << 31),
	};

	return bitsForBitnum[(bitNum) & (BITS_PER_INT - 1)];
}

inline int GetBitForBitnumByte(int bitNum)
{
	static int bitsForBitnum[] =
	{
		(1 << 0),
		(1 << 1),
		(1 << 2),
		(1 << 3),
		(1 << 4),
		(1 << 5),
		(1 << 6),
		(1 << 7),
	};

	return bitsForBitnum[bitNum & 7];
}

inline int CalcNumIntsForBits(int numBits) { return (numBits + (BITS_PER_INT - 1)) / BITS_PER_INT; }



#define WordSwap  WordSwapC
#define DWordSwap DWordSwapC
#define QWordSwap QWordSwapC


#define SafeSwapFloat( pOut, pIn )	(*((unsigned int*)pOut) = DWordSwap( *((unsigned int*)pIn) ))
inline short BigShort(short val) { int test = 1; return (*(char *)&test == 1) ? WordSwap(val) : val; }
inline uint16_t BigWord(uint16_t val) { int test = 1; return (*(char *)&test == 1) ? WordSwap(val) : val; }
inline long BigLong(long val) { int test = 1; return (*(char *)&test == 1) ? DWordSwap(val) : val; }
inline uint32_t BigDWord(uint32_t val) { int test = 1; return (*(char *)&test == 1) ? DWordSwap(val) : val; }
inline short LittleShort(short val) { int test = 1; return (*(char *)&test == 1) ? val : WordSwap(val); }
inline uint16_t LittleWord(uint16_t val) { int test = 1; return (*(char *)&test == 1) ? val : WordSwap(val); }
inline long LittleLong(long val) { int test = 1; return (*(char *)&test == 1) ? val : DWordSwap(val); }
inline uint32_t LittleDWord(uint32_t val) { int test = 1; return (*(char *)&test == 1) ? val : DWordSwap(val); }
inline uint64_t LittleQWord(uint64_t val) { int test = 1; return (*(char *)&test == 1) ? val : QWordSwap(val); }
inline short SwapShort(short val) { return WordSwap(val); }
inline uint16_t SwapWord(uint16_t val) { return WordSwap(val); }
inline long SwapLong(long val) { return DWordSwap(val); }
inline uint32_t SwapDWord(uint32_t val) { return DWordSwap(val); }

// Pass floats by pointer for swapping to avoid truncation in the fpu
inline void BigFloat(float *pOut, const float *pIn) { int test = 1; (*(char *)&test == 1) ? SafeSwapFloat(pOut, pIn) : (*pOut = *pIn); }
inline void LittleFloat(float *pOut, const float *pIn) { int test = 1; (*(char *)&test == 1) ? (*pOut = *pIn) : SafeSwapFloat(pOut, pIn); }
inline void SwapFloat(float *pOut, const float *pIn) { SafeSwapFloat(pOut, pIn); }

__forceinline unsigned long LoadLittleDWord(const unsigned long *base, unsigned int dwordIndex)
{
	return LittleDWord(base[dwordIndex]);
}

__forceinline void StoreLittleDWord(unsigned long *base, unsigned int dwordIndex, unsigned long dword)
{
	base[dwordIndex] = LittleDWord(dword);
}

namespace bitbuf {
	inline uint32_t ZigZagEncode32(int32_t n)
	{
		// Note:  the right-shift must be arithmetic
		return(n << 1) ^ (n >> 31);
	}

	inline int32_t ZigZagDecode32(uint32_t n)
	{
		return(n >> 1) ^ -static_cast<int32_t>(n & 1);
	}

	inline uint64_t ZigZagEncode64(int64_t n)
	{
		// Note:  the right-shift must be arithmetic
		return(n << 1) ^ (n >> 63);
	}

	inline int64_t ZigZagDecode64(uint64_t n)
	{
		return(n >> 1) ^ -static_cast<int64_t>(n & 1);
	}

	const int kMaxVarintBytes = 10;
	const int kMaxVarint32Bytes = 5;
}

class bf_write
{
public:
	bf_write();
	bf_write(void *pData, int nBytes, int nMaxBits = -1);
	bf_write(const char *pDebugName, void *pData, int nBytes, int nMaxBits = -1);

public:
	void StartWriting(void *pData, int nBytes, int iStartBit = 0, int nMaxBits = -1);
	void Reset();
	unsigned char *GetBasePointer() { return (unsigned char *)m_pData; }
	void SetAssertOnOverflow(bool bAssert);
	const char *GetDebugName();
	void SetDebugName(const char *pDebugName);
	void SeekToBit(int bitPos);
	void WriteOneBit(int nValue);
	void WriteOneBitNoCheck(int nValue);
	void WriteOneBitAt(int iBit, int nValue);
	void WriteUBitLong(unsigned int data, int numbits, bool bCheckRange = true);
	void WriteSBitLong(int data, int numbits);
	void WriteBitLong(unsigned int data, int numbits, bool bSigned);
	bool WriteBits(const void *pIn, int nBits);
	void WriteUBitVar(unsigned int data);
	void WriteVarInt32(uint32_t data);
	void WriteVarInt64(uint64_t data);
	void WriteSignedVarInt32(int32_t data);
	void WriteSignedVarInt64(int64_t data);
	int ByteSizeVarInt32(uint32_t data);
	int ByteSizeVarInt64(uint64_t data);
	int ByteSizeSignedVarInt32(int32_t data);
	int ByteSizeSignedVarInt64(int64_t data);
	bool WriteBitsFromBuffer(class bf_read *pIn, int nBits);
	void WriteBitAngle(float fAngle, int numbits);
	void WriteBitCoord(const float f);
	void WriteBitCoordMP(const float f, bool bIntegral, bool bLowPrecision);
	void WriteBitFloat(float val);
	void WriteBitVec3Coord(const Vector &fa);
	void WriteBitNormal(float f);
	void WriteBitVec3Normal(const Vector &fa);
	void WriteBitAngles(const Vector &fa);
	void WriteChar(int val);
	void WriteByte(int val);
	void WriteShort(int val);
	void WriteWord(int val);
	void WriteLong(long val);
	void WriteLongLong(int64_t val);
	void WriteFloat(float val);
	bool WriteBytes(const void *pBuf, int nBytes);
	bool WriteString(const char *pStr);
	int GetNumBytesWritten() const;
	int GetNumBitsWritten() const;
	int GetMaxNumBits();
	int GetNumBitsLeft();
	int GetNumBytesLeft();
	unsigned char *GetData();
	const unsigned char *GetData() const;
	bool CheckForOverflow(int nBits);
	inline bool	IsOverflowed() const { return m_bOverflow; }
	void SetOverflowFlag();

public:
	unsigned long *m_pData{};
	int m_nDataBytes{};
	int m_nDataBits{};
	int m_iCurBit{};
	bool m_bOverflow{};
	bool m_bAssertOnOverflow{};
	const char *m_pDebugName{};
};


//-----------------------------------------------------------------------------
// Inlined methods
//-----------------------------------------------------------------------------

// How many bytes are filled in?
inline int bf_write::GetNumBytesWritten() const
{
	return BitByte(m_iCurBit);
}

inline int bf_write::GetNumBitsWritten() const
{
	return m_iCurBit;
}

inline int bf_write::GetMaxNumBits()
{
	return m_nDataBits;
}

inline int bf_write::GetNumBitsLeft()
{
	return m_nDataBits - m_iCurBit;
}

inline int bf_write::GetNumBytesLeft()
{
	return GetNumBitsLeft() >> 3;
}

inline unsigned char *bf_write::GetData()
{
	return (unsigned char *)m_pData;
}

inline const unsigned char *bf_write::GetData()	const
{
	return (unsigned char *)m_pData;
}

__forceinline bool bf_write::CheckForOverflow(int nBits)
{
	if (m_iCurBit + nBits > m_nDataBits)
	{
		SetOverflowFlag();
	}

	return m_bOverflow;
}

__forceinline void bf_write::SetOverflowFlag()
{
#ifdef DBGFLAG_ASSERT
	if (m_bAssertOnOverflow)
	{
		nullAssert(false);
	}
#endif
	m_bOverflow = true;
}

__forceinline void bf_write::WriteOneBitNoCheck(int nValue)
{
#if __i386__
	if (nValue)
		m_pData[m_iCurBit >> 5] |= 1u << (m_iCurBit & 31);
	else
		m_pData[m_iCurBit >> 5] &= ~(1u << (m_iCurBit & 31));
#else
	extern unsigned long g_LittleBits[32];
	if (nValue)
		m_pData[m_iCurBit >> 5] |= g_LittleBits[m_iCurBit & 31];
	else
		m_pData[m_iCurBit >> 5] &= ~g_LittleBits[m_iCurBit & 31];
#endif

	++m_iCurBit;
}

inline void bf_write::WriteOneBit(int nValue)
{
	if (m_iCurBit >= m_nDataBits)
	{
		SetOverflowFlag();
		return;
	}
	WriteOneBitNoCheck(nValue);
}


inline void	bf_write::WriteOneBitAt(int iBit, int nValue)
{
	if (iBit >= m_nDataBits)
	{
		SetOverflowFlag();
		return;
	}

#if __i386__
	if (nValue)
		m_pData[iBit >> 5] |= 1u << (iBit & 31);
	else
		m_pData[iBit >> 5] &= ~(1u << (iBit & 31));
#else
	extern unsigned long g_LittleBits[32];
	if (nValue)
		m_pData[iBit >> 5] |= g_LittleBits[iBit & 31];
	else
		m_pData[iBit >> 5] &= ~g_LittleBits[iBit & 31];
#endif
}

__forceinline void bf_write::WriteUBitLong(unsigned int curData, int numbits, bool bCheckRange)
{
	if (GetNumBitsLeft() < numbits)
	{
		m_iCurBit = m_nDataBits;
		SetOverflowFlag();
		return;
	}

	int iCurBitMasked = m_iCurBit & 31;
	int iDWord = m_iCurBit >> 5;
	m_iCurBit += numbits;

	// Mask in a dword.
	unsigned long *pOut = &m_pData[iDWord];

	// Rotate data into dword alignment
	curData = (curData << iCurBitMasked) | (curData >> (32 - iCurBitMasked));

	// Calculate bitmasks for first and second word
	unsigned int temp = 1 << (numbits - 1);
	unsigned int mask1 = (temp * 2 - 1) << iCurBitMasked;
	unsigned int mask2 = (temp - 1) >> (31 - iCurBitMasked);

	// Only look beyond current word if necessary (avoid access violation)
	int i = mask2 & 1;
	unsigned long dword1 = LoadLittleDWord(pOut, 0);
	unsigned long dword2 = LoadLittleDWord(pOut, i);

	// Drop bits into place
	dword1 ^= (mask1 & (curData ^ dword1));
	dword2 ^= (mask2 & (curData ^ dword2));

	// Note reversed order of writes so that dword1 wins if mask2 == 0 && i == 0
	StoreLittleDWord(pOut, i, dword2);
	StoreLittleDWord(pOut, 0, dword1);
}

// writes an unsigned integer with variable bit length
__forceinline void bf_write::WriteUBitVar(unsigned int data)
{
	int n = (data < 0x10u ? -1 : 0) + (data < 0x100u ? -1 : 0) + (data < 0x1000u ? -1 : 0);
	WriteUBitLong(data * 4 + n + 3, 6 + n * 4 + 12);
	if (data >= 0x1000u)
	{
		WriteUBitLong(data >> 16, 16);
	}
}

// write raw IEEE float bits in little endian form
__forceinline void bf_write::WriteBitFloat(float val)
{
	long intVal;

	intVal = *((long *)&val);
	WriteUBitLong(intVal, 32);
}

template<int SIZE>
class old_bf_write_static : public bf_write
{
public:
	inline old_bf_write_static() : bf_write(m_StaticData, SIZE) {}

	char	m_StaticData[SIZE];
};

class bf_read
{
public:
	bf_read();
	bf_read(const void *pData, int nBytes, int nBits = -1);
	bf_read(const char *pDebugName, const void *pData, int nBytes, int nBits = -1);

public:
	void StartReading(const void *pData, int nBytes, int iStartBit = 0, int nBits = -1);
	void Reset();
	void SetAssertOnOverflow(bool bAssert);
	const char *GetDebugName() const { return m_pDebugName; }
	void SetDebugName(const char *pName);
	void ExciseBits(int startbit, int bitstoremove);
	int ReadOneBit();

protected:
	unsigned int CheckReadUBitLong(int numbits);
	int ReadOneBitNoCheck();
	bool CheckForOverflow(int nBits);

public:
	const unsigned long *GetBasePointer() { return m_pData; }
	__forceinline int TotalBytesAvailable(void) const
	{
		return m_nDataBytes;
	}

	void ReadBits(void *pOut, int nBits);
	int ReadBitsClamped_ptr(void *pOut, size_t outSizeBytes, size_t nBits);

	template <typename T, size_t N>
	int ReadBitsClamped(T(&pOut)[N], size_t nBits)
	{
		return ReadBitsClamped_ptr(pOut, N * sizeof(T), nBits);
	}

	float ReadBitAngle(int numbits);
	unsigned int ReadUBitLong(int numbits);
	unsigned int ReadUBitLongNoInline(int numbits);
	unsigned int PeekUBitLong(int numbits);
	int ReadSBitLong(int numbits);
	unsigned int ReadUBitVar();
	unsigned int ReadUBitVarInternal(int encodingType);
	uint32_t ReadVarInt32();
	uint64_t ReadVarInt64();
	int32_t ReadSignedVarInt32();
	int64_t ReadSignedVarInt64();
	unsigned int ReadBitLong(int numbits, bool bSigned);
	float ReadBitCoord();
	float ReadBitCoordMP(bool bIntegral, bool bLowPrecision);
	float ReadBitFloat();
	float ReadBitNormal();
	void ReadBitVec3Coord(Vector &fa);
	void ReadBitVec3Normal(Vector &fa);
	void ReadBitAngles(Vector &fa);
	unsigned int ReadBitCoordBits();
	unsigned int ReadBitCoordMPBits(bool bIntegral, bool bLowPrecision);
	__forceinline int ReadChar() { return (char)ReadUBitLong(8); }
	__forceinline int ReadByte() { return ReadUBitLong(8); }
	__forceinline int ReadShort() { return (short)ReadUBitLong(16); }
	__forceinline int ReadWord() { return ReadUBitLong(16); }
	__forceinline long ReadLong() { return ReadUBitLong(32); }
	int64_t ReadLongLong();
	float ReadFloat();
	bool ReadBytes(void *pOut, int nBytes);
	bool ReadString(char *pStr, int bufLen, bool bLine = false, int *pOutNumChars = NULL);
	char *ReadAndAllocateString(bool *pOverflow = 0);
	int CompareBits(bf_read *other, int bits);
	int CompareBitsAt(int offset, bf_read *other, int otherOffset, int bits);
	int GetNumBytesLeft();
	int GetNumBytesRead();
	int GetNumBitsLeft();
	int GetNumBitsRead() const;
	inline bool IsOverflowed() const { return m_bOverflow; }
	inline bool Seek(int iBit);
	inline bool SeekRelative(int iBitDelta);
	void SetOverflowFlag();

public:
	unsigned long *m_pData{};
	int m_nDataBytes{};
	int m_nDataBits{};
	int m_iCurBit{};
	bool m_bOverflow{};
	bool m_bAssertOnOverflow{};
	const char *m_pDebugName{};
};

inline int bf_read::GetNumBytesRead()
{
	return BitByte(m_iCurBit);
}

inline int bf_read::GetNumBitsLeft()
{
	return m_nDataBits - m_iCurBit;
}

inline int bf_read::GetNumBytesLeft()
{
	return GetNumBitsLeft() >> 3;
}

inline int bf_read::GetNumBitsRead() const
{
	return m_iCurBit;
}

inline bool bf_read::Seek(int iBit)
{
	if (iBit < 0 || iBit > m_nDataBits)
	{
		SetOverflowFlag();
		m_iCurBit = m_nDataBits;
		return false;
	}
	else
	{
		m_iCurBit = iBit;
		return true;
	}
}

// Seek to an offset from the current position.
inline bool	bf_read::SeekRelative(int iBitDelta)
{
	return Seek(m_iCurBit + iBitDelta);
}

inline bool bf_read::CheckForOverflow(int nBits)
{
	if (m_iCurBit + nBits > m_nDataBits)
	{
		SetOverflowFlag();
	}

	return m_bOverflow;
}

#pragma warning (disable: 4244)

inline int bf_read::ReadOneBitNoCheck()
{
#if VALVE_LITTLE_ENDIAN
	unsigned int value = ((unsigned long *RESTRICT)m_pData)[m_iCurBit >> 5] >> (m_iCurBit & 31);
#else
	unsigned char value = m_pData[m_iCurBit >> 3] >> (m_iCurBit & 7);
#endif
	++m_iCurBit;
	return value & 1;
}

inline int bf_read::ReadOneBit()
{
	if (GetNumBitsLeft() <= 0)
	{
		SetOverflowFlag();
		return 0;
	}
	return ReadOneBitNoCheck();
}

inline float bf_read::ReadBitFloat()
{
	union { uint32_t u; float f; } c = { ReadUBitLong(32) };
	return c.f;
}

__forceinline unsigned int bf_read::ReadUBitVar()
{
	// six bits: low 2 bits for encoding + first 4 bits of value
	unsigned int sixbits = ReadUBitLong(6);
	unsigned int encoding = sixbits & 3;
	if (encoding)
	{
		// this function will seek back four bits and read the full value
		return ReadUBitVarInternal(encoding);
	}
	return sixbits >> 2;
}

__forceinline unsigned int bf_read::ReadUBitLong(int numbits)
{
	if (GetNumBitsLeft() < numbits)
	{
		m_iCurBit = m_nDataBits;
		SetOverflowFlag();
		return 0;
	}

	unsigned int iStartBit = m_iCurBit & 31u;
	int iLastBit = m_iCurBit + numbits - 1;
	unsigned int iWordOffset1 = m_iCurBit >> 5;
	unsigned int iWordOffset2 = iLastBit >> 5;
	m_iCurBit += numbits;

#if __i386__
	unsigned int bitmask = (2 << (numbits - 1)) - 1;
#else
	extern unsigned long g_ExtraMasks[33];
	unsigned int bitmask = g_ExtraMasks[numbits];
#endif

	unsigned int dw1 = LoadLittleDWord((unsigned long *)m_pData, iWordOffset1) >> iStartBit;
	unsigned int dw2 = LoadLittleDWord((unsigned long *)m_pData, iWordOffset2) << (32 - iStartBit);

	return (dw1 | dw2) & bitmask;
}

__forceinline int bf_read::CompareBits(bf_read *other, int numbits)
{
	return (ReadUBitLong(numbits) != other->ReadUBitLong(numbits));
}


class CMouthInfo;
class SpatializationInfo_t;
class CClientThinkHandlePtr;
class CBaseHandle;
class C_BaseEntity;
class IClientRenderable;
class ICollideable;
class IClientEntity;
class IClientThinkable;
class model_t;
class CSaveRestoreData;

typedef CClientThinkHandlePtr *ClientThinkHandle_t;

enum
{
	TF_TEAM_SPECTATOR = 1,
	TF_TEAM_RED,
	TF_TEAM_BLU
};

#define MAX_WEAPONS 48

enum ETFCond
{
	TF_COND_INVALID = -1,
	TF_COND_AIMING,
	TF_COND_ZOOMED,
	TF_COND_DISGUISING,
	TF_COND_DISGUISED,
	TF_COND_STEALTHED,
	TF_COND_INVULNERABLE,
	TF_COND_TELEPORTED,
	TF_COND_TAUNTING,
	TF_COND_INVULNERABLE_WEARINGOFF,
	TF_COND_STEALTHED_BLINK,
	TF_COND_SELECTED_TO_TELEPORT,
	TF_COND_CRITBOOSTED,
	TF_COND_TMPDAMAGEBONUS,
	TF_COND_FEIGN_DEATH,
	TF_COND_PHASE,
	TF_COND_STUNNED,
	TF_COND_OFFENSEBUFF,
	TF_COND_SHIELD_CHARGE,
	TF_COND_DEMO_BUFF,
	TF_COND_ENERGY_BUFF,
	TF_COND_RADIUSHEAL,
	TF_COND_HEALTH_BUFF,
	TF_COND_BURNING,
	TF_COND_HEALTH_OVERHEALED,
	TF_COND_URINE,
	TF_COND_BLEEDING,
	TF_COND_DEFENSEBUFF,
	TF_COND_MAD_MILK,
	TF_COND_MEGAHEAL,
	TF_COND_REGENONDAMAGEBUFF,
	TF_COND_MARKEDFORDEATH,
	TF_COND_NOHEALINGDAMAGEBUFF,
	TF_COND_SPEED_BOOST,
	TF_COND_CRITBOOSTED_PUMPKIN,
	TF_COND_CRITBOOSTED_USER_BUFF,
	TF_COND_CRITBOOSTED_DEMO_CHARGE,
	TF_COND_SODAPOPPER_HYPE,
	TF_COND_CRITBOOSTED_FIRST_BLOOD,
	TF_COND_CRITBOOSTED_BONUS_TIME,
	TF_COND_CRITBOOSTED_CTF_CAPTURE,
	TF_COND_CRITBOOSTED_ON_KILL,
	TF_COND_CANNOT_SWITCH_FROM_MELEE,
	TF_COND_DEFENSEBUFF_NO_CRIT_BLOCK,
	TF_COND_REPROGRAMMED,
	TF_COND_CRITBOOSTED_RAGE_BUFF,
	TF_COND_DEFENSEBUFF_HIGH,
	TF_COND_SNIPERCHARGE_RAGE_BUFF,
	TF_COND_DISGUISE_WEARINGOFF,
	TF_COND_MARKEDFORDEATH_SILENT,
	TF_COND_DISGUISED_AS_DISPENSER,
	TF_COND_SAPPED,
	TF_COND_INVULNERABLE_HIDE_UNLESS_DAMAGE,
	TF_COND_INVULNERABLE_USER_BUFF,
	TF_COND_HALLOWEEN_BOMB_HEAD,
	TF_COND_HALLOWEEN_THRILLER,
	TF_COND_RADIUSHEAL_ON_DAMAGE,
	TF_COND_CRITBOOSTED_CARD_EFFECT,
	TF_COND_INVULNERABLE_CARD_EFFECT,
	TF_COND_MEDIGUN_UBER_BULLET_RESIST,
	TF_COND_MEDIGUN_UBER_BLAST_RESIST,
	TF_COND_MEDIGUN_UBER_FIRE_RESIST,
	TF_COND_MEDIGUN_SMALL_BULLET_RESIST,
	TF_COND_MEDIGUN_SMALL_BLAST_RESIST,
	TF_COND_MEDIGUN_SMALL_FIRE_RESIST,
	TF_COND_STEALTHED_USER_BUFF,
	TF_COND_MEDIGUN_DEBUFF,
	TF_COND_STEALTHED_USER_BUFF_FADING,
	TF_COND_BULLET_IMMUNE,
	TF_COND_BLAST_IMMUNE,
	TF_COND_FIRE_IMMUNE,
	TF_COND_PREVENT_DEATH,
	TF_COND_MVM_BOT_STUN_RADIOWAVE,
	TF_COND_HALLOWEEN_SPEED_BOOST,
	TF_COND_HALLOWEEN_QUICK_HEAL,
	TF_COND_HALLOWEEN_GIANT,
	TF_COND_HALLOWEEN_TINY,
	TF_COND_HALLOWEEN_IN_HELL,
	TF_COND_HALLOWEEN_GHOST_MODE,
	TF_COND_MINICRITBOOSTED_ON_KILL,
	TF_COND_OBSCURED_SMOKE,
	TF_COND_PARACHUTE_ACTIVE,
	TF_COND_BLASTJUMPING,
	TF_COND_HALLOWEEN_KART,
	TF_COND_HALLOWEEN_KART_DASH,
	TF_COND_BALLOON_HEAD,
	TF_COND_MELEE_ONLY,
	TF_COND_SWIMMING_CURSE,
	TF_COND_FREEZE_INPUT,
	TF_COND_HALLOWEEN_KART_CAGE,
	TF_COND_DONOTUSE_0,
	TF_COND_RUNE_STRENGTH,
	TF_COND_RUNE_HASTE,
	TF_COND_RUNE_REGEN,
	TF_COND_RUNE_RESIST,
	TF_COND_RUNE_VAMPIRE,
	TF_COND_RUNE_REFLECT,
	TF_COND_RUNE_PRECISION,
	TF_COND_RUNE_AGILITY,
	TF_COND_GRAPPLINGHOOK,
	TF_COND_GRAPPLINGHOOK_SAFEFALL,
	TF_COND_GRAPPLINGHOOK_LATCHED,
	TF_COND_GRAPPLINGHOOK_BLEEDING,
	TF_COND_AFTERBURN_IMMUNE,
	TF_COND_RUNE_KNOCKOUT,
	TF_COND_RUNE_IMBALANCE,
	TF_COND_CRITBOOSTED_RUNE_TEMP,
	TF_COND_PASSTIME_INTERCEPTION,
	TF_COND_SWIMMING_NO_EFFECTS,
	TF_COND_PURGATORY,
	TF_COND_RUNE_KING,
	TF_COND_RUNE_PLAGUE,
	TF_COND_RUNE_SUPERNOVA,
	TF_COND_PLAGUE,
	TF_COND_KING_BUFFED,
	TF_COND_TEAM_GLOWS,
	TF_COND_KNOCKED_INTO_AIR,
	TF_COND_COMPETITIVE_WINNER,
	TF_COND_COMPETITIVE_LOSER,
	TF_COND_HEALING_DEBUFF,
	TF_COND_PASSTIME_PENALTY_DEBUFF,
	TF_COND_GRAPPLED_TO_PLAYER,
	TF_COND_GRAPPLED_BY_PLAYER,
	TF_COND_PARACHUTE_DEPLOYED,
	TF_COND_GAS,
	TF_COND_BURNING_PYRO,
	TF_COND_ROCKETPACK,
	TF_COND_LOST_FOOTING,
	TF_COND_AIR_CURRENT,
	TF_COND_HALLOWEEN_HELL_HEAL,
	TF_COND_POWERUPMODE_DOMINANT
};

enum
{
	MODE_TELEPORTER_ENTRANCE = 0,
	MODE_TELEPORTER_EXIT
};

enum
{
	TF_GL_MODE_REGULAR = 0,
	TF_GL_MODE_REMOTE_DETONATE,
	TF_GL_MODE_REMOTE_DETONATE_PRACTICE,
	TF_GL_MODE_CANNONBALL
};

enum ETFWeaponType
{
	TF_WEAPON_NONE,
	TF_WEAPON_BAT,
	TF_WEAPON_BAT_WOOD,
	TF_WEAPON_BOTTLE,
	TF_WEAPON_FIREAXE,
	TF_WEAPON_CLUB,
	TF_WEAPON_CROWBAR,
	TF_WEAPON_KNIFE,
	TF_WEAPON_FISTS,
	TF_WEAPON_SHOVEL,
	TF_WEAPON_WRENCH,
	TF_WEAPON_BONESAW,
	TF_WEAPON_SHOTGUN_PRIMARY,
	TF_WEAPON_SHOTGUN_SOLDIER,
	TF_WEAPON_SHOTGUN_HWG,
	TF_WEAPON_SHOTGUN_PYRO,
	TF_WEAPON_SCATTERGUN,
	TF_WEAPON_SNIPERRIFLE,
	TF_WEAPON_MINIGUN,
	TF_WEAPON_SMG,
	TF_WEAPON_SYRINGEGUN_MEDIC,
	TF_WEAPON_TRANQ,
	TF_WEAPON_ROCKETLAUNCHER,
	TF_WEAPON_GRENADELAUNCHER,
	TF_WEAPON_PIPEBOMBLAUNCHER,
	TF_WEAPON_FLAMETHROWER,
	TF_WEAPON_GRENADE_NORMAL,
	TF_WEAPON_GRENADE_CONCUSSION,
	TF_WEAPON_GRENADE_NAIL,
	TF_WEAPON_GRENADE_MIRV,
	TF_WEAPON_GRENADE_MIRV_DEMOMAN,
	TF_WEAPON_GRENADE_NAPALM,
	TF_WEAPON_GRENADE_GAS,
	TF_WEAPON_GRENADE_EMP,
	TF_WEAPON_GRENADE_CALTROP,
	TF_WEAPON_GRENADE_PIPEBOMB,
	TF_WEAPON_GRENADE_SMOKE_BOMB,
	TF_WEAPON_GRENADE_HEAL,
	TF_WEAPON_GRENADE_STUNBALL,
	TF_WEAPON_GRENADE_JAR,
	TF_WEAPON_GRENADE_JAR_MILK,
	TF_WEAPON_PISTOL,
	TF_WEAPON_PISTOL_SCOUT,
	TF_WEAPON_REVOLVER,
	TF_WEAPON_NAILGUN,
	TF_WEAPON_PDA,
	TF_WEAPON_PDA_ENGINEER_BUILD,
	TF_WEAPON_PDA_ENGINEER_DESTROY,
	TF_WEAPON_PDA_SPY,
	TF_WEAPON_BUILDER,
	TF_WEAPON_MEDIGUN,
	TF_WEAPON_GRENADE_MIRVBOMB,
	TF_WEAPON_FLAMETHROWER_ROCKET,
	TF_WEAPON_GRENADE_DEMOMAN,
	TF_WEAPON_SENTRY_BULLET,
	TF_WEAPON_SENTRY_ROCKET,
	TF_WEAPON_DISPENSER,
	TF_WEAPON_INVIS,
	TF_WEAPON_FLAREGUN,
	TF_WEAPON_LUNCHBOX,
	TF_WEAPON_JAR,
	TF_WEAPON_COMPOUND_BOW,
	TF_WEAPON_BUFF_ITEM,
	TF_WEAPON_PUMPKIN_BOMB,
	TF_WEAPON_SWORD,
	TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT,
	TF_WEAPON_LIFELINE,
	TF_WEAPON_LASER_POINTER,
	TF_WEAPON_DISPENSER_GUN,
	TF_WEAPON_SENTRY_REVENGE,
	TF_WEAPON_JAR_MILK,
	TF_WEAPON_HANDGUN_SCOUT_PRIMARY,
	TF_WEAPON_BAT_FISH,
	TF_WEAPON_CROSSBOW,
	TF_WEAPON_STICKBOMB,
	TF_WEAPON_HANDGUN_SCOUT_SECONDARY,
	TF_WEAPON_SODA_POPPER,
	TF_WEAPON_SNIPERRIFLE_DECAP,
	TF_WEAPON_RAYGUN,
	TF_WEAPON_PARTICLE_CANNON,
	TF_WEAPON_MECHANICAL_ARM,
	TF_WEAPON_DRG_POMSON,
	TF_WEAPON_BAT_GIFTWRAP,
	TF_WEAPON_GRENADE_ORNAMENT_BALL,
	TF_WEAPON_FLAREGUN_REVENGE,
	TF_WEAPON_PEP_BRAWLER_BLASTER,
	TF_WEAPON_CLEAVER,
	TF_WEAPON_GRENADE_CLEAVER,
	TF_WEAPON_STICKY_BALL_LAUNCHER,
	TF_WEAPON_GRENADE_STICKY_BALL,
	TF_WEAPON_SHOTGUN_BUILDING_RESCUE,
	TF_WEAPON_CANNON,
	TF_WEAPON_THROWABLE,
	TF_WEAPON_GRENADE_THROWABLE,
	TF_WEAPON_PDA_SPY_BUILD,
	TF_WEAPON_GRENADE_WATERBALLOON,
	TF_WEAPON_HARVESTER_SAW,
	TF_WEAPON_SPELLBOOK,
	TF_WEAPON_SPELLBOOK_PROJECTILE,
	TF_WEAPON_SNIPERRIFLE_CLASSIC,
	TF_WEAPON_PARACHUTE,
	TF_WEAPON_GRAPPLINGHOOK,
	TF_WEAPON_PASSTIME_GUN,
	TF_WEAPON_CHARGED_SMG,
	TF_WEAPON_BREAKABLE_SIGN,
	TF_WEAPON_ROCKETPACK,
	TF_WEAPON_SLAP,
	TF_WEAPON_JAR_GAS,
	TF_WEAPON_GRENADE_JAR_GAS,
	TF_WEAPON_FLAME_BALL
};

enum medigun_weapontypes_t
{
	MEDIGUN_STANDARD,
	MEDIGUN_UBER,
	MEDIGUN_QUICKFIX,
	MEDIGUN_RESIST
};

enum medigun_resist_types_t
{
	MEDIGUN_BULLET_RESIST,
	MEDIGUN_BLAST_RESIST,
	MEDIGUN_FIRE_RESIST,
	MEDIGUN_NUM_RESISTS
};

enum FlareGunTypes_t
{
	FLAREGUN_NORMAL = 0,
	FLAREGUN_DETONATE,
	FLAREGUN_GRORDBORT,
	FLAREGUN_SCORCHSHOT
};

enum medigun_charge_types
{
	MEDIGUN_CHARGE_INVALID = -1,
	MEDIGUN_CHARGE_INVULN,
	MEDIGUN_CHARGE_CRITICALBOOST,
	MEDIGUN_CHARGE_MEGAHEAL,
	MEDIGUN_CHARGE_BULLET_RESIST,
	MEDIGUN_CHARGE_BLAST_RESIST,
	MEDIGUN_CHARGE_FIRE_RESIST,
	MEDIGUN_NUM_CHARGE_TYPES
};

enum MinigunState_t
{
	AC_STATE_IDLE,
	AC_STATE_STARTFIRING,
	AC_STATE_FIRING,
	AC_STATE_SPINNING,
	AC_STATE_DRYFIRE
};

enum minigun_weapontypes_t
{
	MINIGUN_STANDARD,
	MINIGUN_STUN
};

class IConVar
{
public:
	virtual void SetValue(const char *pValue) = 0;
	virtual void SetValue(float flValue) = 0;
	virtual void SetValue(int nValue) = 0;
	virtual const char *GetName(void) const = 0;
	virtual bool IsFlagSet(int nFlag) const = 0;
};

typedef void (*FnChangeCallback_t)(IConVar *var, const char *pOldValue, float flOldValue);

class ConCommandBase
{
public:
	virtual ~ConCommandBase(void);

public:
	ConCommandBase *m_pNext{};
	bool m_bRegistered{};
	const char *m_pszName{};
	const char *m_pszHelpString{};
	int m_nFlags{};
};

class ConVar : public ConCommandBase, public IConVar
{
public:
	ConVar *m_pParent{};
	const char *m_pszDefaultValue{};
	char *m_pszString{};
	int m_StringLength{};
	float m_fValue{};
	int m_nValue{};
	bool m_bHasMin{};
	float m_fMinVal{};
	bool m_bHasMax{};
	float m_fMaxVal{};
	bool m_bHasCompMin{};
	float m_fCompMinVal{};
	bool m_bHasCompMax{};
	float m_fCompMaxVal{};
	bool m_bCompetitiveRestrictions{};
	FnChangeCallback_t m_fnChangeCallback{};

public:
	int GetInt() const
	{
		return m_nValue;
	}

	bool GetBool() const
	{
		return !!GetInt();
	}

	float GetFloat() const
	{
		return m_fValue;
	}

	const char *GetString() const
	{
		return m_pszString;
	}
};

#define MAX_PLAYER_NAME_LENGTH 32
#define SIGNED_GUID_LEN 32
#define MAX_CUSTOM_FILES 4

using CRC32_t = unsigned int;

struct player_info_t
{
	char name[MAX_PLAYER_NAME_LENGTH]{};
	int userID{};
	char guid[SIGNED_GUID_LEN + 1]{};
	unsigned int friendsID{};
	char friendsName[MAX_PLAYER_NAME_LENGTH]{};
	bool fakeplayer{};
	bool ishltv{};
	bool isreplay{};
	CRC32_t customFiles[MAX_CUSTOM_FILES]{};
	unsigned char filesDownloaded{};
};

enum types_t
{
	TYPE_NONE = 0,
	TYPE_STRING,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_PTR,
	TYPE_WSTRING,
	TYPE_COLOR,
	TYPE_UINT64,
	TYPE_NUMTYPES,
};

class KeyValues
{
public:
	int m_iKeyName{};
	char *m_sValue{};
	wchar_t *m_wsValue{};

	union
	{
		int m_iValue{};
		float m_flValue;
		void *m_pValue;
		unsigned char m_Color[4];
	};

	char m_iDataType{};
	char m_bHasEscapeSequences{};
	char m_bEvaluateConditionals{};
	char unused[1]{};

	KeyValues *m_pPeer{};
	KeyValues *m_pSub{};
	KeyValues *m_pChain{};

public:
	KeyValues(const char *name)
	{
		s::KeyValues_KeyValues.call<void>(this, name);
	}

	KeyValues *FindKey(const char *keyName, bool bCreate)
	{
		return s::KeyValues_FindKey.call<KeyValues *>(this, keyName, bCreate);
	}

	void SetString(const char *keyName, const char *value)
	{
		s::KeyValues_SetString.call<void>(this, keyName, value);
	}

	const char *GetName()
	{
		return s::KeyValues_GetName.call<const char *>(this);
	}
};

class IMaterialVar
{
public:
	void SetIntValue(int val)
	{
		mem::callVirtual<4, void, int>(this, val);
	}

	void SetStringValue(char const *val)
	{
		mem::callVirtual<5, void, char const *>(this, val);
	}

	void SetVecValue(float x, float y, float z)
	{
		mem::callVirtual<10, void, float, float, float>(this, x, y, z);
	}
};

class IMaterial
{
public:
	const char *GetName()
	{
		return mem::callVirtual<0, const char *>(this);
	}

	const char *GetTextureGroupName()
	{
		return mem::callVirtual<1, const char *>(this);
	}

	IMaterialVar *FindVar(const char *varName, bool *found, bool complain = true)
	{
		return mem::callVirtual<11, IMaterialVar *, const char *, bool *, bool>(this, varName, found, complain);
	}

	void IncrementReferenceCount()
	{
		mem::callVirtual<12, void>(this);
	}

	void DecrementReferenceCount()
	{
		mem::callVirtual<13, void>(this);
	}

	void ColorModulate(float r, float g, float b)
	{
		mem::callVirtual<28, void, float, float, float>(this, r, g, b);
	}

	bool IsErrorMaterial()
	{
		return mem::callVirtual<42, bool>(this);
	}

	void DeleteIfUnreferenced()
	{
		mem::callVirtual<50, void>(this);
	}

	bool IsPrecached()
	{
		return mem::callVirtual<56, bool>(this);
	}
};

class ITexture
{
public:
	bool IsTranslucent()
	{
		return mem::callVirtual<6, bool>(this);
	}

	void GetLowResColorSample(float s, float t, float *color)
	{
		mem::callVirtual<8, void, float, float, float *>(this, s, t, color);
	}
		
	void IncrementReferenceCount()
	{
		mem::callVirtual<10, void>(this);
	}

	void DecrementReferenceCount()
	{
		mem::callVirtual<11, void>(this);
	}

	void DeleteIfUnreferenced()
	{
		mem::callVirtual<25, void>(this);
	}
};

enum MaterialFogMode_t
{
	MATERIAL_FOG_NONE,
	MATERIAL_FOG_LINEAR,
	MATERIAL_FOG_LINEAR_BELOW_FOG_Z
};

enum MaterialCullMode_t
{
	MATERIAL_CULLMODE_CCW,	// this culls polygons with counterclockwise winding
	MATERIAL_CULLMODE_CW	// this culls polygons with clockwise winding
};

enum StencilOperation_t
{
	STENCILOPERATION_KEEP = 1,
	STENCILOPERATION_ZERO = 2,
	STENCILOPERATION_REPLACE = 3,
	STENCILOPERATION_INCRSAT = 4,
	STENCILOPERATION_DECRSAT = 5,
	STENCILOPERATION_INVERT = 6,
	STENCILOPERATION_INCR = 7,
	STENCILOPERATION_DECR = 8,
	STENCILOPERATION_FORCE_DWORD = 0x7fffffff
};

enum StencilComparisonFunction_t
{
	STENCILCOMPARISONFUNCTION_NEVER = 1,
	STENCILCOMPARISONFUNCTION_LESS = 2,
	STENCILCOMPARISONFUNCTION_EQUAL = 3,
	STENCILCOMPARISONFUNCTION_LESSEQUAL = 4,
	STENCILCOMPARISONFUNCTION_GREATER = 5,
	STENCILCOMPARISONFUNCTION_NOTEQUAL = 6,
	STENCILCOMPARISONFUNCTION_GREATEREQUAL = 7,
	STENCILCOMPARISONFUNCTION_ALWAYS = 8,
	STENCILCOMPARISONFUNCTION_FORCE_DWORD = 0x7fffffff
};

enum MaterialMatrixMode_t
{
	MATERIAL_VIEW = 0,
	MATERIAL_PROJECTION,
	MATERIAL_TEXTURE0,
	MATERIAL_TEXTURE1,
	MATERIAL_TEXTURE2,
	MATERIAL_TEXTURE3,
	MATERIAL_TEXTURE4,
	MATERIAL_TEXTURE5,
	MATERIAL_TEXTURE6,
	MATERIAL_TEXTURE7,
	MATERIAL_MODEL,
	NUM_MATRIX_MODES = MATERIAL_MODEL + 1,
	NUM_TEXTURE_TRANSFORMS = MATERIAL_TEXTURE7 - MATERIAL_TEXTURE0 + 1
};

class IMatRenderContext
{
public:
	void SetRenderTarget(ITexture *pTexture)
	{
		mem::callVirtual<6, void, ITexture *>(this, pTexture);
	}

	void DepthRange(float zNear, float zFar)
	{
		mem::callVirtual<11, void, float, float>(this, zNear, zFar);
	}

	void ClearBuffers(bool bClearColor, bool bClearDepth, bool bClearStencil)
	{
		mem::callVirtual<12, void, bool, bool, bool>(this, bClearColor, bClearDepth, bClearStencil);
	}

	void SetAmbientLight(float r, float g, float b)
	{
		mem::callVirtual<14, void, float, float, float>(this, r, g, b);
	}

	void MatrixMode(MaterialMatrixMode_t matrixMode)
	{
		mem::callVirtual<20, void, MaterialMatrixMode_t>(this, matrixMode);
	}

	void PushMatrix()
	{
		mem::callVirtual<21, void>(this);
	}

	void PopMatrix()
	{
		mem::callVirtual<22, void>(this);
	}

	void LoadIdentity()
	{
		mem::callVirtual<31, void>(this);
	}

	void Scale(float x, float y, float z)
	{
		mem::callVirtual<37, void, float, float, float>(this, x, y, z);
	}

	void Viewport(int x, int y, int width, int height)
	{
		mem::callVirtual<38, void, int, int, int, int>(this, x, y, width, height);
	}

	void CullMode(MaterialCullMode_t cullMode)
	{
		mem::callVirtual<40, void, MaterialCullMode_t>(this, cullMode);
	}

	void FogMode(MaterialFogMode_t fogMode)
	{
		mem::callVirtual<43, void, MaterialFogMode_t>(this, fogMode);
	}

	void FogStart(float fStart)
	{
		mem::callVirtual<44, void, float>(this, fStart);
	}

	void FogEnd(float fEnd)
	{
		mem::callVirtual<45, void, float>(this, fEnd);
	}

	void FogColor3f(float r, float g, float b)
	{
		mem::callVirtual<48, void, float, float, float>(this, r, g, b);
	}

	void SetNumBoneWeights(int numBones)
	{
		mem::callVirtual<53, void, int>(this, numBones);
	}

	void ClearColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		mem::callVirtual<73, void, unsigned char, unsigned char, unsigned char, unsigned char>(this, r, g, b, a);
	}

	void OverrideDepthEnable(bool bEnable, bool bDepthEnable)
	{
		mem::callVirtual<74, void, bool, bool>(this, bEnable, bDepthEnable);
	}

	void DrawScreenSpaceRectangle(IMaterial *pMaterial, int destx, int desty, int width, int height, float src_texture_x0, float src_texture_y0, float src_texture_x1, float src_texture_y1, int src_texture_width, int src_texture_height, void *pClientRenderable = nullptr, int nXDice = 1, int nYDice = 1)
	{
		mem::callVirtual<103, void, IMaterial *, int, int, int, int, float, float, float, float, int, int, void *, int, int>(this, pMaterial, destx, desty, width, height, src_texture_x0, src_texture_y0, src_texture_x1, src_texture_y1, src_texture_width, src_texture_height, pClientRenderable, nXDice, nYDice);
	}

	void PushRenderTargetAndViewport()
	{
		mem::callVirtual<108, void>(this);
	}

	void PopRenderTargetAndViewport()
	{
		mem::callVirtual<109, void>(this);
	}

	void SetStencilEnable(bool onoff)
	{
		mem::callVirtual<117, void, bool>(this, onoff);
	}

	void SetStencilFailOperation(StencilOperation_t op)
	{
		mem::callVirtual<118, void, StencilOperation_t>(this, op);
	}

	void SetStencilZFailOperation(StencilOperation_t op)
	{
		mem::callVirtual<119, void, StencilOperation_t>(this, op);
	}

	void SetStencilPassOperation(StencilOperation_t op)
	{
		mem::callVirtual<120, void, StencilOperation_t>(this, op);
	}

	void SetStencilCompareFunction(StencilComparisonFunction_t cmpfn)
	{
		mem::callVirtual<121, void, StencilComparisonFunction_t>(this, cmpfn);
	}

	void SetStencilReferenceValue(int ref)
	{
		mem::callVirtual<122, void, int>(this, ref);
	}

	void SetStencilTestMask(unsigned int msk)
	{
		mem::callVirtual<123, void, unsigned int>(this, msk);
	}

	void SetStencilWriteMask(unsigned int msk)
	{
		mem::callVirtual<124, void, unsigned int>(this, msk);
	}

	void SetLightingOrigin(Vector vLightingOrigin)
	{
		mem::callVirtual<157, void, Vector>(this, vLightingOrigin);
	}

	void FogMaxDensity(float flMaxDensity)
	{
		mem::callVirtual<167, void, float>(this, flMaxDensity);
	}

	void OverrideAlphaWriteEnable(bool bEnable, bool bAlphaWriteEnable)
	{
		mem::callVirtual<193, void, bool, bool>(this, bEnable, bAlphaWriteEnable);
	}

	void OverrideColorWriteEnable(bool bOverrideEnable, bool bColorWriteEnable)
	{
		mem::callVirtual<194, void, bool, bool>(this, bOverrideEnable, bColorWriteEnable);
	}
};

struct ShaderStencilState_t
{
	bool m_bEnable{};
	StencilOperation_t m_FailOp{};
	StencilOperation_t m_ZFailOp{};
	StencilOperation_t m_PassOp{};
	StencilComparisonFunction_t m_CompareFunc{};
	int m_nReferenceValue{};
	unsigned int m_nTestMask{};
	unsigned int m_nWriteMask{};

	ShaderStencilState_t()
	{
		m_bEnable = false;
		m_PassOp = m_FailOp = m_ZFailOp = STENCILOPERATION_KEEP;
		m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		m_nReferenceValue = 0;
		m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
	}

	void SetStencilState(IMatRenderContext *const pRenderContext) const
	{
		pRenderContext->SetStencilEnable(m_bEnable);
		pRenderContext->SetStencilFailOperation(m_FailOp);
		pRenderContext->SetStencilZFailOperation(m_ZFailOp);
		pRenderContext->SetStencilPassOperation(m_PassOp);
		pRenderContext->SetStencilCompareFunction(m_CompareFunc);
		pRenderContext->SetStencilReferenceValue(m_nReferenceValue);
		pRenderContext->SetStencilTestMask(m_nTestMask);
		pRenderContext->SetStencilWriteMask(m_nWriteMask);
	}
};

#define FLOW_OUTGOING 0
#define FLOW_INCOMING 1

class INetChannelInfo
{
public:
	enum
	{
		GENERIC = 0,
		LOCALPLAYER,
		OTHERPLAYERS,
		ENTITIES,
		SOUNDS,
		EVENTS,
		USERMESSAGES,
		ENTMESSAGES,
		VOICE,
		STRINGTABLE,
		MOVE,
		STRINGCMD,
		SIGNON,
		TOTAL
	};

	bool IsLoopback()
	{
		return mem::callVirtual<6, bool>(this);
	}

	bool IsTimingOut()
	{
		return mem::callVirtual<7, bool>(this);
	}

	float GetLatency(int flow)
	{
		return mem::callVirtual<9, float, int>(this, flow);
	}

	float GetAvgLatency(int flow)
	{
		return mem::callVirtual<10, float, int>(this, flow);
	}

	bool CanPacket()
	{
		return mem::callVirtual<53, bool>(this);
	}

	void SetChoked()
	{
		return mem::callVirtual<42, void>(this);
	}

public:
	int &m_nInSequenceNr()
	{
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + 16);
	}

	int &m_nOutSequenceNr()
	{
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + 12);
	}

	int &m_nInReliableState()
	{
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + 28);
	}

	int &m_nChokedPackets()
	{
		return *reinterpret_cast<int *>(reinterpret_cast<uintptr_t>(this) + 32);
	}
};

class INetChannel : public INetChannelInfo
{
};

typedef enum
{
	NA_NULL = 0,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
} netadrtype_t;

typedef struct netadr_s
{
public: // members are public to avoid to much changes
	//netadrtype_t type;
	netadrtype_t type;
	unsigned char ip[4];
	unsigned short port;
} netadr_t;

typedef void *FileHandle_t;

enum ConnectionStatus_t
{
	CONNECTION_STATE_DISCONNECTED = 0,
	CONNECTION_STATE_CONNECTING,
	CONNECTION_STATE_CONNECTION_FAILED,
	CONNECTION_STATE_CONNECTED
};

#define NET_MAX_DATAGRAM_PAYLOAD 1400

#define	MAX_OSPATH		260			// max length of a filesystem pathname
#define MAX_STREAMS			2    

// 32 bytes, align 8 - same as CUtlVector
__declspec(align(8)) struct FuckYou
{
	char pad[32];
};

class CNetChan : public INetChannel
{
	typedef struct dataFragments_s
	{
		FileHandle_t file;                 // open file handle
		char         filename[260]; // filename
		char *buffer;               // if NULL it's a file
		unsigned int BYTEs;                // size in BYTEs
		unsigned int bits;                 // size in bits
		unsigned int transferID;           // only for files
		bool         isCompressed;         // true if data is bzip compressed
		unsigned int nUncompressedSize;    // full size in BYTEs
		bool         asTCP;                // send as TCP stream
		int          numFragments;         // number of total fragments
		int          ackedFragments;       // number of fragments send & acknowledged
		int          pendingFragments;     // number of fragments send, but not acknowledged yet
	} dataFragments_t;
	struct subChannel_s
	{
		int startFraggment[MAX_STREAMS];
		int numFragments[MAX_STREAMS];
		int sendSeqNr;
		int state; // 0 = free, 1 = scheduled to send, 2 = send & waiting, 3 = dirty
		int index; // index in m_SubChannels[]
	};
	typedef struct netframe_s
	{
		float          time;        // net_time received/send
		int            size;        // total size in BYTEs
		float          latency;     // raw ping for this packet, not cleaned. set when acknowledged otherwise -1.
		float          avg_latency; // averaged ping for this packet
		bool           valid;       // false if dropped, lost, flushed
		int            choked;      // number of previously chocked packets
		int            dropped;
		float          m_flInterpolationAmount;
		unsigned short msggroups[INetChannelInfo::TOTAL]; // received BYTEs for each message group
	} netframe_t;
	typedef struct
	{
		float       nextcompute;      // Time when we should recompute k/sec data
		float       avgBYTEspersec;   // average BYTEs/sec
		float       avgpacketspersec; // average packets/sec
		float       avgloss;          // average packet loss [0..1]
		float       avgchoke;         // average packet choke [0..1]
		float       avglatency;       // average ping, not cleaned
		float       latency;          // current ping, more accurate also more jittering
		int         totalpackets;     // total processed packets
		int         totalBYTEs;       // total processed BYTEs
		int         currentindex;     // current frame index
		netframe_t  frames[64];       // frame history
		netframe_t *currentframe;     // current frame
	} netflow_t;
	typedef struct netpacket_s
	{
		netadr_t		from;		// sender IP
		int				source;		// received source 
		double			received;	// received time
		unsigned char *data;		// pointer to raw packet data
		bf_read			message;	// easy bitbuf data access
		int				size;		// size in bytes
		int				wiresize;   // size in bytes before decompression
		bool			stream;		// was send as stream
		struct netpacket_s *pNext;	// for internal use, should be NULL in public
	} netpacket_t;

public:
	ConnectionStatus_t m_ConnectionState;
	int m_nOutSequenceNr;
	int m_nInSequenceNr;
	int m_nOutSequenceNrAck;
	int m_nOutReliableState;
	int m_nInReliableState;
	int m_nChokedPackets;
	int m_PacketDrop;
	bf_write m_StreamReliable;
	byte m_ReliableDataBuffer[8 * 1024];
	FuckYou m_RawData;
	bf_write m_StreamUnreliable;
	byte m_UnreliableDataBuffer[NET_MAX_DATAGRAM_PAYLOAD];
	void *m_pSocket;
	int m_StreamSocket;
	unsigned int m_MaxReliablePayloadSize;
	netadr_t remote_address;
	float last_received;
	float connect_time;
	int m_Rate;
	float m_fClearTime;
	float m_Timeout;
	char m_Name[32];
	void *m_MessageHandler;
};

template<class T, class I = int>
class CUtlMemory
{
private:
	T *m_pMemory{};
	int m_nAllocationCount{};
	int m_nGrowSize{};

public:
	T &operator[](I i)
	{
		return m_pMemory[(uint32_t)i];
	}

	const T &operator[](I i) const
	{
		return m_pMemory[(uint32_t)i];
	}

	T &Element(I i)
	{
		return m_pMemory[(uint32_t)i];
	}

	const T &Element(I i) const
	{
		return m_pMemory[(uint32_t)i];
	}

	T *Base()
	{
		return m_pMemory;
	}

	const T *Base() const
	{
		return m_pMemory;
	}

	int Count() const
	{
		return m_nAllocationCount;
	}
};

template<class T, class A = CUtlMemory<T>>
class CUtlVector
{
private:
	A m_Memory{};
	int m_Size{};
	T *m_pElements{};

public:
	T &operator[](int i)
	{
		return m_Memory[i];
	}

	const T &operator[](int i) const
	{
		return m_Memory[i];
	}

	T &Element(int i)
	{
		return m_Memory[i];
	}

	const T &Element(int i) const
	{
		return m_Memory[i];
	}

	T *Base()
	{
		return m_Memory.Base();
	}

	const T *Base() const
	{
		return m_Memory.Base();
	}

	int Count() const
	{
		return m_Size;
	}

	bool IsEmpty(void) const
	{
		return (Count() == 0);
	}
};

typedef float vec_t;

class __declspec(align(16)) VectorAligned : public Vector
{
public:
	inline VectorAligned(void) {};

	inline VectorAligned(vec_t X, vec_t Y, vec_t Z)
	{
		set(X, Y, Z);
	}

public:
	explicit VectorAligned(const Vector &vOther)
	{
		set(vOther.x, vOther.y, vOther.z);
	}

	VectorAligned &operator=(const Vector &vOther)
	{
		set(vOther.x, vOther.y, vOther.z);

		return *this;
	}

	float w{};
};

struct Ray_t
{
	VectorAligned m_Start{};
	VectorAligned m_Delta{};
	VectorAligned m_StartOffset{};
	VectorAligned m_Extents{};
	bool m_IsRay{};
	bool m_IsSwept{};

	void Init(Vector const &start, Vector const &end)
	{
		m_Delta = end - start;
		m_IsSwept = m_Delta.lengthSqr() != 0.0f;
		m_Extents.set();
		m_IsRay = true;
		m_StartOffset.set();
		m_Start = start;
	}

	void Init(Vector const &start, Vector const &end, Vector const &mins, Vector const &maxs)
	{
		m_Delta = end - start;
		m_IsSwept = m_Delta.lengthSqr() != 0.0f;
		m_Extents = maxs - mins;
		m_Extents *= 0.5f;
		m_IsRay = (m_Extents.lengthSqr() < 1e-6f);
		m_StartOffset = mins + maxs;
		m_StartOffset *= 0.5f;
		m_Start = start + m_StartOffset;
		m_StartOffset *= -1.0f;
	}

	Vector InvDelta() const
	{
		Vector vecInvDelta{};

		for (int iAxis = 0; iAxis < 3; ++iAxis)
		{
			if (m_Delta[iAxis] != 0.0f) {
				vecInvDelta[iAxis] = 1.0f / m_Delta[iAxis];
			}

			else {
				vecInvDelta[iAxis] = FLT_MAX;
			}
		}

		return vecInvDelta;
	}
};

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,
	TRACE_ENTITIES_ONLY,
	TRACE_EVERYTHING_FILTER_PROPS
};

class IHandleEntity;

class ITraceFilter
{
public:
	virtual bool ShouldHitEntity(IHandleEntity *pEntity, int contentsMask) = 0;
	virtual TraceType_t	GetTraceType() const = 0;
};

struct cplane_t
{
	Vector normal{};
	float dist{};
	byte type{};
	byte signbits{};
	byte pad[2]{};
};

class CBaseTrace
{
public:
	Vector startpos{};
	Vector endpos{};
	cplane_t plane{};
	float fraction{};
	int contents{};
	unsigned short dispFlags{};
	bool allsolid{};
	bool startsolid{};
};

struct csurface_t
{
	const char *name{};
	short surfaceProps{};
	unsigned short flags{};
};

class C_BaseEntity;

class CGameTrace : public CBaseTrace
{
public:
	float fractionleftsolid{};
	csurface_t surface{};
	int hitgroup{};
	short physicsbone{};
	C_BaseEntity *m_pEnt{};
	int hitbox{};
};

typedef CGameTrace trace_t;

#define CONTENTS_EMPTY (0x0)
#define CONTENTS_SOLID (0x1)
#define CONTENTS_WINDOW (0x2)
#define CONTENTS_AUX (0x4)
#define CONTENTS_GRATE (0x8)
#define CONTENTS_SLIME (0x10)
#define CONTENTS_WATER (0x20)
#define CONTENTS_BLOCKLOS (0x40)
#define CONTENTS_OPAQUE (0x80)
#define LAST_VISIBLE_CONTENTS (0x80)
#define ALL_VISIBLE_CONTENTS (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS - 1))
#define CONTENTS_TESTFOGVOLUME (0x100)
#define CONTENTS_UNUSED (0x200)
#define CONTENTS_UNUSED6 (0x400)
#define CONTENTS_TEAM1 (0x800)
#define CONTENTS_TEAM2 (0x1000)
#define CONTENTS_IGNORE_NODRAW_OPAQUE (0x2000)
#define CONTENTS_MOVEABLE (0x4000)
#define CONTENTS_AREAPORTAL (0x8000)
#define CONTENTS_PLAYERCLIP (0x10000)
#define CONTENTS_MONSTERCLIP (0x20000)
#define CONTENTS_CURRENT_0 (0x40000)
#define CONTENTS_CURRENT_90 (0x80000)
#define CONTENTS_CURRENT_180 (0x100000)
#define CONTENTS_CURRENT_270 (0x200000)
#define CONTENTS_CURRENT_UP (0x400000)
#define CONTENTS_CURRENT_DOWN (0x800000)
#define CONTENTS_ORIGIN (0x1000000)
#define CONTENTS_MONSTER (0x2000000)
#define CONTENTS_DEBRIS (0x4000000)
#define CONTENTS_DETAIL (0x8000000)
#define CONTENTS_TRANSLUCENT (0x10000000)
#define CONTENTS_LADDER (0x20000000)
#define CONTENTS_HITBOX (0x40000000)
#define MASK_ALL (0xFFFFFFFF)
#define MASK_SOLID (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE)
#define MASK_PLAYERSOLID (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE)
#define MASK_NPCSOLID (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTERCLIP | CONTENTS_WINDOW | CONTENTS_MONSTER | CONTENTS_GRATE)
#define MASK_WATER (CONTENTS_WATER | CONTENTS_MOVEABLE | CONTENTS_SLIME)
#define MASK_OPAQUE (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_OPAQUE)
#define MASK_OPAQUE_AND_NPCS (MASK_OPAQUE | CONTENTS_MONSTER)
#define MASK_BLOCKLOS (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_BLOCKLOS)
#define MASK_BLOCKLOS_AND_NPCS (MASK_BLOCKLOS | CONTENTS_MONSTER)
#define MASK_VISIBLE (MASK_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE)
#define MASK_VISIBLE_AND_NPCS (MASK_OPAQUE_AND_NPCS | CONTENTS_IGNORE_NODRAW_OPAQUE)
#define MASK_SHOT (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_HITBOX)
#define MASK_SHOT_HULL (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_GRATE)
#define MASK_SHOT_PORTAL (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTER)
#define MASK_SOLID_BRUSHONLY (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_GRATE)
#define MASK_PLAYERSOLID_BRUSHONLY (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_PLAYERCLIP | CONTENTS_GRATE)
#define MASK_NPCSOLID_BRUSHONLY (CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE)
#define MASK_NPCWORLDSTATIC (CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_MONSTERCLIP | CONTENTS_GRATE)
#define MASK_SPLITAREAPORTAL (CONTENTS_WATER | CONTENTS_SLIME)
#define MASK_CURRENT (CONTENTS_CURRENT_0 | CONTENTS_CURRENT_90 | CONTENTS_CURRENT_180 | CONTENTS_CURRENT_270 | CONTENTS_CURRENT_UP | CONTENTS_CURRENT_DOWN)
#define MASK_DEADSOLID (CONTENTS_SOLID | CONTENTS_PLAYERCLIP | CONTENTS_WINDOW | CONTENTS_GRATE)

#define CONTENTS_REDTEAM CONTENTS_TEAM1
#define CONTENTS_BLUETEAM CONTENTS_TEAM2

enum Collision_Group_t
{
	COLLISION_GROUP_NONE,
	COLLISION_GROUP_DEBRIS,
	COLLISION_GROUP_DEBRIS_TRIGGER,
	COLLISION_GROUP_INTERACTIVE_DEBRIS,
	COLLISION_GROUP_INTERACTIVE,
	COLLISION_GROUP_PLAYER,
	COLLISION_GROUP_BREAKABLE_GLASS,
	COLLISION_GROUP_VEHICLE,
	COLLISION_GROUP_PLAYER_MOVEMENT,
	COLLISION_GROUP_NPC,
	COLLISION_GROUP_IN_VEHICLE,
	COLLISION_GROUP_WEAPON,
	COLLISION_GROUP_VEHICLE_CLIP,
	COLLISION_GROUP_PROJECTILE,
	COLLISION_GROUP_DOOR_BLOCKER,
	COLLISION_GROUP_PASSABLE_DOOR,
	COLLISION_GROUP_DISSOLVING,
	COLLISION_GROUP_PUSHAWAY,
	COLLISION_GROUP_NPC_ACTOR,
	COLLISION_GROUP_NPC_SCRIPTED,
	LAST_SHARED_COLLISION_GROUP
};

#define TF_WATERJUMP_FORWARD 30.0f
#define TF_WATERJUMP_UP 300.0f
#define TF_TIME_TO_DUCK 0.3f
#define TF_AIRDUCKED_COUNT 2

#define WATERJUMP_HEIGHT 8.0f

using EntityHandle_t = CBaseHandle;

#pragma warning (push)
#pragma warning (disable : 26495)

class CMoveData
{
public:
	bool m_bFirstRunOfFunctions : 1;
	bool m_bGameCodeMovedPlayer : 1;
	EntityHandle_t m_nPlayerHandle{};
	int m_nImpulseCommand{};
	QAngle m_vecViewAngles{};
	QAngle m_vecAbsViewAngles{};
	int m_nButtons{};
	int m_nOldButtons{};
	float m_flForwardMove{};
	float m_flOldForwardMove{};
	float m_flSideMove{};
	float m_flUpMove{};
	float m_flMaxSpeed{};
	float m_flClientMaxSpeed{};
	Vector m_vecVelocity{};
	QAngle m_vecAngles{};
	QAngle m_vecOldAngles{};
	float m_outStepHeight{};
	Vector m_outWishVel{};
	Vector m_outJumpVel{};
	Vector m_vecConstraintCenter{};
	float m_flConstraintRadius{};
	float m_flConstraintWidth{};
	float m_flConstraintSpeedFactor{};
	Vector m_vecAbsOrigin{};

public:
	const Vector &GetAbsOrigin() const {
		return m_vecAbsOrigin;
	}

	void SetAbsOrigin(const Vector &vec) {
		m_vecAbsOrigin = vec;
	}
};

#pragma warning (pop)

typedef unsigned short MaterialHandle_t;

enum RenderTargetSizeMode_t
{
	RT_SIZE_NO_CHANGE = 0,
	RT_SIZE_DEFAULT = 1,
	RT_SIZE_PICMIP = 2,
	RT_SIZE_HDR = 3,
	RT_SIZE_FULL_FRAME_BUFFER = 4,
	RT_SIZE_OFFSCREEN = 5,
	RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP = 6,
	RT_SIZE_REPLAY_SCREENSHOT = 7,
	RT_SIZE_LITERAL = 8,
	RT_SIZE_LITERAL_PICMIP = 9
};

enum ImageFormat
{
	IMAGE_FORMAT_UNKNOWN = -1,
	IMAGE_FORMAT_RGBA8888 = 0,
	IMAGE_FORMAT_ABGR8888,
	IMAGE_FORMAT_RGB888,
	IMAGE_FORMAT_BGR888,
	IMAGE_FORMAT_RGB565,
	IMAGE_FORMAT_I8,
	IMAGE_FORMAT_IA88,
	IMAGE_FORMAT_P8,
	IMAGE_FORMAT_A8,
	IMAGE_FORMAT_RGB888_BLUESCREEN,
	IMAGE_FORMAT_BGR888_BLUESCREEN,
	IMAGE_FORMAT_ARGB8888,
	IMAGE_FORMAT_BGRA8888,
	IMAGE_FORMAT_DXT1,
	IMAGE_FORMAT_DXT3,
	IMAGE_FORMAT_DXT5,
	IMAGE_FORMAT_BGRX8888,
	IMAGE_FORMAT_BGR565,
	IMAGE_FORMAT_BGRX5551,
	IMAGE_FORMAT_BGRA4444,
	IMAGE_FORMAT_DXT1_ONEBITALPHA,
	IMAGE_FORMAT_BGRA5551,
	IMAGE_FORMAT_UV88,
	IMAGE_FORMAT_UVWQ8888,
	IMAGE_FORMAT_RGBA16161616F,
	IMAGE_FORMAT_RGBA16161616,
	IMAGE_FORMAT_UVLX8888,
	IMAGE_FORMAT_R32F,
	IMAGE_FORMAT_RGB323232F,
	IMAGE_FORMAT_RGBA32323232F,
	IMAGE_FORMAT_NV_DST16,
	IMAGE_FORMAT_NV_DST24,
	IMAGE_FORMAT_NV_INTZ,
	IMAGE_FORMAT_NV_RAWZ,
	IMAGE_FORMAT_ATI_DST16,
	IMAGE_FORMAT_ATI_DST24,
	IMAGE_FORMAT_NV_NULL,
	IMAGE_FORMAT_ATI2N,
	IMAGE_FORMAT_ATI1N,
	IMAGE_FORMAT_DXT1_RUNTIME,
	IMAGE_FORMAT_DXT5_RUNTIME,
	NUM_IMAGE_FORMATS
};

enum MaterialRenderTargetDepth_t
{
	MATERIAL_RT_DEPTH_SHARED = 0x0,
	MATERIAL_RT_DEPTH_SEPARATE = 0x1,
	MATERIAL_RT_DEPTH_NONE = 0x2,
	MATERIAL_RT_DEPTH_ONLY = 0x3,
};

class mstudiobbox_t
{
public:
	int bone{};
	int group{};
	Vector bbmin{};
	Vector bbmax{};
	int szhitboxnameindex{};
	int unused[8]{};
};

class mstudiohitboxset_t
{
public:
	int sznameindex{};
	int numhitboxes{};
	int hitboxindex{};

public:
	mstudiobbox_t *pHitbox(int i) const
	{
		return (mstudiobbox_t *)(((byte *)this) + hitboxindex) + i;
	};
};

class mstudiobone_t
{
public:
	int sznameindex{};
	int parent{};
	int bonecontroller[6]{};
	Vector pos{};
	Quaternion quat{};
	RadianEuler rot{};
	Vector posscale{};
	Vector rotscale{};
	matrix3x4_t poseToBone{};
	Quaternion qAlignment{};
	int flags{};
	int proctype{};
	int procindex{};
	int physicsbone{};
	int surfacepropidx{};
	int contents{};
	int unused[8]{};
};

class studiohdr_t
{
public:
	int id{};
	int version{};
	int checksum{};
	char name[64]{};
	int length{};
	Vector eyeposition{};
	Vector illumposition{};
	Vector hull_min{};
	Vector hull_max{};
	Vector view_bbmin{};
	Vector view_bbmax{};
	int flags{};
	int numbones{};
	int boneindex{};
	int numbonecontrollers{};
	int bonecontrollerindex{};
	int numhitboxsets{};
	int hitboxsetindex{};
	int numlocalanim{};
	int localanimindex{};
	int numlocalseq{};
	int localseqindex{};
	int activitylistversion{};
	int eventsindexed{};
	int numtextures{};
	int textureindex{};
	int numcdtextures{};
	int cdtextureindex{};
	int numskinref{};
	int numskinfamilies{};
	int skinindex{};
	int numbodyparts{};
	int bodypartindex{};
	int numlocalattachments{};
	int localattachmentindex{};
	int numlocalnodes{};
	int localnodeindex{};
	int localnodenameindex{};
	int numflexdesc{};
	int flexdescindex{};
	int numflexcontrollers{};
	int flexcontrollerindex{};
	int numflexrules{};
	int flexruleindex{};
	int numikchains{};
	int ikchainindex{};
	int nummouths{};
	int mouthindex{};
	int numlocalposeparameters{};
	int localposeparamindex{};
	int surfacepropindex{};
	int keyvalueindex{};
	int keyvaluesize{};
	int numlocalikautoplaylocks{};
	int localikautoplaylockindex{};
	float mass{};
	int contents{};
	int numincludemodels{};
	int includemodelindex{};
	void *virtualModel{};
	int szanimblocknameindex{};
	int numanimblocks{};
	int animblockindex{};
	void *animblockModel{};
	int bonetablebynameindex{};
	void *pVertexBase{};
	void *pIndexBase{};
	byte constdirectionallightdot{};
	byte rootLOD{};
	byte numAllowedRootLODs{};
	byte unused[1]{};
	int unused4{};
	int numflexcontrollerui{};
	int flexcontrolleruiindex{};
	float flVertAnimFixedPointScale{};
	int unused3[1]{};
	int studiohdr2index{};
	int unused2[1]{};

public:
	inline mstudiobone_t *pBone(int i) const
	{
		return (mstudiobone_t *)(((byte *)this) + boneindex) + i;
	};

	mstudiohitboxset_t *pHitboxSet(int i) const
	{
		return (mstudiohitboxset_t *)(((byte *)this) + hitboxsetindex) + i;
	};

	mstudiobbox_t *pHitbox(int i, int set) const
	{
		mstudiohitboxset_t const *s{ pHitboxSet(set) };

		if (!s) {
			return nullptr;
		}

		return s->pHitbox(i);
	};

	int iHitboxCount(int set) const
	{
		mstudiohitboxset_t const *s{ pHitboxSet(set) };

		if (!s) {
			return 0;
		}

		return s->numhitboxes;
	};
};

#define BONE_CALCULATE_MASK 0x1F
#define BONE_PHYSICALLY_SIMULATED 0x01
#define BONE_PHYSICS_PROCEDURAL 0x02
#define BONE_ALWAYS_PROCEDURAL 0x04
#define BONE_SCREEN_ALIGN_SPHERE 0x08
#define BONE_SCREEN_ALIGN_CYLINDER 0x10
#define BONE_USED_MASK 0x0007FF00
#define BONE_USED_BY_ANYTHING 0x0007FF00
#define BONE_USED_BY_HITBOX 0x00000100
#define BONE_USED_BY_ATTACHMENT 0x00000200
#define BONE_USED_BY_VERTEX_MASK 0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0 0x00000400
#define BONE_USED_BY_VERTEX_LOD1 0x00000800
#define BONE_USED_BY_VERTEX_LOD2 0x00001000
#define BONE_USED_BY_VERTEX_LOD3 0x00002000
#define BONE_USED_BY_VERTEX_LOD4 0x00004000
#define BONE_USED_BY_VERTEX_LOD5 0x00008000
#define BONE_USED_BY_VERTEX_LOD6 0x00010000
#define BONE_USED_BY_VERTEX_LOD7 0x00020000
#define BONE_USED_BY_BONE_MERGE 0x00040000

enum TFHitboxes
{
	HITBOX_HEAD,
	HITBOX_NECK,
	HITBOX_LOWER_NECK,
	HITBOX_PELVIS,
	HITBOX_BODY,
	HITBOX_THORAX,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_FOREARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_FOREARM
};

enum OverrideType_t
{
	OVERRIDE_NORMAL,
	OVERRIDE_BUILD_SHADOWS,
	OVERRIDE_DEPTH_WRITE,
	OVERRIDE_SSAO_DEPTH_WRITE,
};

enum FontDrawType_t
{
	FONT_DRAW_DEFAULT = 0,
	FONT_DRAW_NONADDITIVE,
	FONT_DRAW_ADDITIVE,
	FONT_DRAW_TYPE_COUNT = 2
};

struct Vertex_t
{
	Vector2D m_Position{};
	Vector2D m_TexCoord{};

	Vertex_t() {}

	Vertex_t(const Vector2D &pos, const Vector2D &coord = Vector2D(0.0f, 0.0f))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}

	void Init(const Vector2D &pos, const Vector2D &coord = Vector2D(0.0f, 0.0f))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}
};

using VPANEL = unsigned int;
using HScheme = unsigned long;
using HTexture = unsigned long;
using HCursor = unsigned long;
using HPanel = unsigned long;
using HFont = unsigned long;

enum PaintMode_t
{
	PAINT_UIPANELS = (1 << 0),
	PAINT_INGAMEPANELS = (1 << 1),
	PAINT_CURSOR = (1 << 2)
};

#define FONTFLAG_NONE 0x000
#define FONTFLAG_ITALIC 0x001
#define FONTFLAG_UNDERLINE 0x002
#define FONTFLAG_STRIKEOUT 0x004
#define FONTFLAG_SYMBOL 0x008
#define FONTFLAG_ANTIALIAS 0x010
#define FONTFLAG_GAUSSIANBLUR 0x020
#define FONTFLAG_ROTARY 0x040
#define FONTFLAG_DROPSHADOW 0x080
#define FONTFLAG_ADDITIVE 0x100
#define FONTFLAG_OUTLINE 0x200
#define FONTFLAG_CUSTOM 0x400
#define FONTFLAG_BITMAP 0x800

enum ClientFrameStage_t
{
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

enum CursorCode
{
	dc_user,
	dc_none,
	dc_arrow,
	dc_ibeam,
	dc_hourglass,
	dc_waitarrow,
	dc_crosshair,
	dc_up,
	dc_sizenwse,
	dc_sizenesw,
	dc_sizewe,
	dc_sizens,
	dc_sizeall,
	dc_no,
	dc_hand,
	dc_blank,
	dc_last,
	dc_alwaysvisible_push,
	dc_alwaysvisible_pop
};

enum view_id_t
{
	VIEW_ILLEGAL = -2,
	VIEW_NONE = -1,
	VIEW_MAIN = 0,
	VIEW_3DSKY = 1,
	VIEW_MONITOR = 2,
	VIEW_REFLECTION = 3,
	VIEW_REFRACTION = 4,
	VIEW_INTRO_PLAYER = 5,
	VIEW_INTRO_CAMERA = 6,
	VIEW_SHADOW_DEPTH_TEXTURE = 7,
	VIEW_SSAO = 8,
	VIEW_ID_COUNT
};

enum
{
	TF_CLASS_UNDEFINED,
	TF_CLASS_SCOUT,
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HEAVYWEAPONS,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,
	TF_CLASS_CIVILIAN,
	TF_CLASS_COUNT_ALL,
	TF_CLASS_RANDOM
};

#define STUDIO_NONE 0x00000000
#define STUDIO_RENDER 0x00000001
#define STUDIO_VIEWXFORMATTACHMENTS 0x00000002
#define STUDIO_DRAWTRANSLUCENTSUBMODELS 0x00000004
#define STUDIO_TWOPASS 0x00000008
#define STUDIO_STATIC_LIGHTING 0x00000010
#define STUDIO_WIREFRAME 0x00000020
#define STUDIO_ITEM_BLINK 0x00000040
#define STUDIO_NOSHADOWS 0x00000080
#define STUDIO_WIREFRAME_VCOLLIDE 0x00000100
#define STUDIO_NO_OVERRIDE_FOR_ATTACH 0x00000200
#define STUDIO_GENERATE_STATS 0x01000000
#define STUDIO_SSAODEPTHTEXTURE 0x08000000
#define STUDIO_SHADOWDEPTHTEXTURE 0x40000000
#define STUDIO_TRANSPARENCY 0x80000000

#define TEXTURE_GROUP_LIGHTMAP "Lightmaps"
#define TEXTURE_GROUP_WORLD "World textures"
#define TEXTURE_GROUP_MODEL "Model textures"
#define TEXTURE_GROUP_VGUI "VGUI textures"
#define TEXTURE_GROUP_PARTICLE "Particle textures"
#define TEXTURE_GROUP_DECAL "Decal textures"
#define TEXTURE_GROUP_SKYBOX "SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS "ClientEffect textures"
#define TEXTURE_GROUP_OTHER "Other textures"
#define TEXTURE_GROUP_PRECACHED "Precached"
#define TEXTURE_GROUP_CUBE_MAP "CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET "RenderTargets"
#define TEXTURE_GROUP_RUNTIME_COMPOSITE "Runtime Composite"
#define TEXTURE_GROUP_UNACCOUNTED "Unaccounted textures"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER "Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP "Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR "Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD "World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS "Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER "Other Verts"
#define TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER "Dynamic Indices"
#define TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER "Dynamic Verts"
#define TEXTURE_GROUP_DEPTH_BUFFER "DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL "ViewModel"
#define TEXTURE_GROUP_PIXEL_SHADERS "Pixel Shaders"
#define TEXTURE_GROUP_VERTEX_SHADERS "Vertex Shaders"
#define TEXTURE_GROUP_RENDER_TARGET_SURFACE "RenderTarget Surfaces"
#define TEXTURE_GROUP_MORPH_TARGETS "Morph Targets"

#define CREATERENDERTARGETFLAGS_HDR 0x00000001
#define CREATERENDERTARGETFLAGS_AUTOMIPMAP 0x00000002
#define CREATERENDERTARGETFLAGS_UNFILTERABLE_OK 0x00000004
#define CREATERENDERTARGETFLAGS_NOEDRAM 0x00000008
#define CREATERENDERTARGETFLAGS_TEMP 0x00000010

enum CompiledVtfFlags
{
	TEXTUREFLAGS_POINTSAMPLE = 0x00000001,
	TEXTUREFLAGS_TRILINEAR = 0x00000002,
	TEXTUREFLAGS_CLAMPS = 0x00000004,
	TEXTUREFLAGS_CLAMPT = 0x00000008,
	TEXTUREFLAGS_ANISOTROPIC = 0x00000010,
	TEXTUREFLAGS_HINT_DXT5 = 0x00000020,
	TEXTUREFLAGS_SRGB = 0x00000040,
	TEXTUREFLAGS_NORMAL = 0x00000080,
	TEXTUREFLAGS_NOMIP = 0x00000100,
	TEXTUREFLAGS_NOLOD = 0x00000200,
	TEXTUREFLAGS_ALL_MIPS = 0x00000400,
	TEXTUREFLAGS_PROCEDURAL = 0x00000800,
	TEXTUREFLAGS_ONEBITALPHA = 0x00001000,
	TEXTUREFLAGS_EIGHTBITALPHA = 0x00002000,
	TEXTUREFLAGS_ENVMAP = 0x00004000,
	TEXTUREFLAGS_RENDERTARGET = 0x00008000,
	TEXTUREFLAGS_DEPTHRENDERTARGET = 0x00010000,
	TEXTUREFLAGS_NODEBUGOVERRIDE = 0x00020000,
	TEXTUREFLAGS_SINGLECOPY = 0x00040000,
	TEXTUREFLAGS_STAGING_MEMORY = 0x00080000,
	TEXTUREFLAGS_IMMEDIATE_CLEANUP = 0x00100000,
	TEXTUREFLAGS_IGNORE_PICMIP = 0x00200000,
	TEXTUREFLAGS_UNUSED_00400000 = 0x00400000,
	TEXTUREFLAGS_NODEPTHBUFFER = 0x00800000,
	TEXTUREFLAGS_UNUSED_01000000 = 0x01000000,
	TEXTUREFLAGS_CLAMPU = 0x02000000,
	TEXTUREFLAGS_VERTEXTEXTURE = 0x04000000,
	TEXTUREFLAGS_SSBUMP = 0x08000000,
	TEXTUREFLAGS_UNUSED_10000000 = 0x10000000,
	TEXTUREFLAGS_BORDER = 0x20000000,
	TEXTUREFLAGS_STREAMABLE_COARSE = 0x40000000,
	TEXTUREFLAGS_STREAMABLE_FINE = 0x80000000,
	TEXTUREFLAGS_STREAMABLE = (TEXTUREFLAGS_STREAMABLE_COARSE | TEXTUREFLAGS_STREAMABLE_FINE)
};

#define TF_WEAPON_ZOOM_FOV 20

enum EObserverModes : int
{
	OBS_MODE_NONE = 0,
	OBS_MODE_DEATHCAM,
	OBS_MODE_FREEZECAM,
	OBS_MODE_FIXED,
	OBS_MODE_IN_EYE,
	OBS_MODE_CHASE,
	OBS_MODE_POI,
	OBS_MODE_ROAMING,
	NUM_OBSERVER_MODES
};

struct ModelRenderInfo_t
{
	Vector origin{};
	QAngle angles{};
	IClientRenderable *pRenderable{};
	const model_t *pModel{};
	const matrix3x4_t *pModelToWorld{};
	const matrix3x4_t *pLightingOffset{};
	const Vector *pLightingOrigin{};
	int flags{};
	int entity_index{};
	int skin{};
	int body{};
	int hitboxset{};
	ModelInstanceHandle_t instance{};
};

class studiohdr_t;
class studiohwdata_t;
using StudioDecalHandle_t = void *;

struct DrawModelState_t
{
	studiohdr_t *m_pStudioHdr{};
	studiohwdata_t *m_pStudioHWData{};
	IClientRenderable *m_pRenderable{};
	const matrix3x4_t *m_pModelToWorld{};
	StudioDecalHandle_t m_decals{};
	int m_drawFlags{};
	int m_lod{};
};

using LightCacheHandle_t = void *;

class ColorMeshInfo_t;

enum LightType_t
{
	MATERIAL_LIGHT_DISABLE = 0,
	MATERIAL_LIGHT_POINT,
	MATERIAL_LIGHT_DIRECTIONAL,
	MATERIAL_LIGHT_SPOT,
};

struct LightDesc_t
{
	LightType_t m_Type{};
	Vector m_Color{};
	Vector m_Position{};
	Vector m_Direction{};
	float  m_Range{};
	float m_Falloff{};
	float m_Attenuation0{};
	float m_Attenuation1{};
	float m_Attenuation2{};
	float m_Theta{};
	float m_Phi{};
	float m_ThetaDot{};
	float m_PhiDot{};
	unsigned int m_Flags{};
	float OneOver_ThetaDot_Minus_PhiDot{};
	float m_RangeSquared{};
};

struct DrawModelInfo_t
{
	studiohdr_t *m_pStudioHdr{};
	studiohwdata_t *m_pHardwareData{};
	StudioDecalHandle_t m_Decals{};
	int m_Skin{};
	int m_Body{};
	int m_HitboxSet{};
	void *m_pClientEntity{};
	int m_Lod{};
	ColorMeshInfo_t *m_pColorMeshes{};
	bool m_bStaticLighting{};
	Vector m_vecAmbientCube[6]{};
	int m_nLocalLightCount{};
	LightDesc_t m_LocalLightDescs[4]{};
};

enum TauntIndex_t : uint16_t
{
	Taunt_High_Five = 167,
	Taunt_Replay = 438,
	Taunt_Laugh = 463,
	Taunt_Meet_The_Medic_Heroic = 477,
	Taunt_The_Shred_Alert = 1015,
	Taunt_Square_Dance = 1106,
	Taunt_Flippin_Awesome = 1107,
	Taunt_Buy_A_Life = 1108,
	Taunt_Results_Are_In = 1109,
	Taunt_Rps = 1110,
	Taunt_Skullcracker = 1111,
	Taunt_Party_Trick = 1112,
	Taunt_Fresh_Brewed_Victory = 1113,
	Taunt_Spent_Well_Spirits = 1114,
	Taunt_Rancho_Relaxo = 1115,
	Taunt_I_See_You = 1116,
	Taunt_Battin_A_Thousand = 1117,
	Taunt_Conga = 1118,
	Taunt_Deep_Fried_Desire = 1119,
	Taunt_Oblooterated = 1120,
	Taunt_Kazotsky_Kick = 1157,
	Taunt_Mannrobics = 1162,
	Taunt_The_Carlton = 1168,
	Taunt_The_Victory_Lap = 1172,
	Taunt_The_Table_Tantrum = 1174,
	Taunt_The_Boiling_Point = 1175,
	Taunt_Yeti_Punch = 1182,
	Taunt_Yeti_Smash = 1183,
	Taunt_Panzer_Pants = 1196,
	Taunt_The_Scooty_Scoot = 1197,
	Taunt_Pool_Party = 30570,
	Taunt_The_Boston_Breakdance = 30572,
	Taunt_The_Killer_Solo = 30609,
	Taunt_Most_Wanted = 30614,
	Taunt_The_Boxtrot = 30615,
	Taunt_The_Proletariat_Showoff = 30616,
	Taunt_Bucking_Bronco = 30618,
	Taunt_Burstchester = 30621,
	Taunt_True_Scotsmans_Call = 30671,
	Taunt_Zoomin_Broom = 30672,
	Taunt_Soldiers_Requiem = 30673,
	Taunt_The_Fubar_Fanfare = 30761,
	Taunt_Disco_Fever = 30762,
	Taunt_The_Balloonibouncer = 30763,
	Taunt_Second_Rate_Sorcery = 30816,
	Taunt_Didgeridrongo = 30839,
	Taunt_Scotsmanns_Stagger = 30840,
	Taunt_The_Dueling_Banjo = 30842,
	Taunt_The_Russian_Arms_Race = 30843,
	Taunt_The_Soviet_Strongarm = 30844,
	Taunt_The_Jumping_Jack = 30845,
	Taunt_The_Headcase = 30876,
	Taunt_The_Trackmans_Touchdown = 30917,
	Taunt_Surgeons_Squeezebox = 30918,
	Taunt_The_Skating_Scorcher = 30919,
	Taunt_The_Bunnyhopper = 30920,
	Taunt_Runners_Rhythm = 30921,
	Taunt_Luxury_Lounge = 30922,
	Taunt_The_Pooped_Deck = 31153,
	Taunt_Time_Out_Therapy = 31154,
	Taunt_Rocket_Jockey = 31155,
	Taunt_The_Boston_Boarder = 31156,
	Taunt_Scorchers_Solo = 31157,
	Taunt_Texas_Truckin = 31160,
	Taunt_Spin_To_Win = 31161,
	Taunt_The_Fist_Bump = 31162,
	Taunt_The_Drunken_Sailor = 31201,
	Taunt_The_Profane_Puppeteer = 31202,
	Taunt_The_Mannbulance = 31203,
	Taunt_Bare_Knuckle_Beatdown = 31207,
	Taunt_The_Homerunners_Hobby = 31233,
	Taunt_Doctors_Defibrillators = 31236,
	Taunt_Shooters_Stakeout = 31237,
	Taunt_The_Hot_Wheeler = 31239,
	Taunt_Texas_Twirl_Em = 31286,
	Taunt_The_Scaredy_Cat = 31288,
	Taunt_The_Travel_Agent = 31290,
	Taunt_Drunk_Manns_Cannon = 31291,
	Taunt_Shanty_Shipmate = 31292,
	Taunt_Russian_Rubdown = 31320,
	Taunt_Tailored_Terminal = 31321,
	Taunt_Roasty_Toasty = 31322,
	Taunt_Star_Spangled_Strategy = 31347,
	Taunt_Killer_Joke = 31348,
	Taunt_The_Head_Doctor = 31349,
	Taunt_Tuefort_Tango = 31351,
	Taunt_The_Road_Rager = 31352,
	Taunt_The_Killer_Signature = 31354,
};

typedef uint16_t	item_definition_index_t;
const item_definition_index_t INVALID_ITEM_DEF_INDEX = ((item_definition_index_t)-1);

enum ProjectileType_t
{
	TF_PROJECTILE_NONE,
	TF_PROJECTILE_BULLET,
	TF_PROJECTILE_ROCKET,
	TF_PROJECTILE_PIPEBOMB,
	TF_PROJECTILE_PIPEBOMB_REMOTE,
	TF_PROJECTILE_SYRINGE,
	TF_PROJECTILE_FLARE,
	TF_PROJECTILE_JAR,
	TF_PROJECTILE_ARROW,
	TF_PROJECTILE_FLAME_ROCKET,
	TF_PROJECTILE_JAR_MILK,
	TF_PROJECTILE_HEALING_BOLT,
	TF_PROJECTILE_ENERGY_BALL,
	TF_PROJECTILE_ENERGY_RING,
	TF_PROJECTILE_PIPEBOMB_PRACTICE,
	TF_PROJECTILE_CLEAVER,
	TF_PROJECTILE_STICKY_BALL,
	TF_PROJECTILE_CANNONBALL,
	TF_PROJECTILE_BUILDING_REPAIR_BOLT,
	TF_PROJECTILE_FESTIVE_ARROW,
	TF_PROJECTILE_THROWABLE,
	TF_PROJECTILE_SPELL,
	TF_PROJECTILE_FESTIVE_JAR,
	TF_PROJECTILE_FESTIVE_HEALING_BOLT,
	TF_PROJECTILE_BREADMONSTER_JARATE,
	TF_PROJECTILE_BREADMONSTER_MADMILK,
	TF_PROJECTILE_GRAPPLINGHOOK,
	TF_PROJECTILE_SENTRY_ROCKET,
	TF_PROJECTILE_BREAD_MONSTER,
	TF_NUM_PROJECTILES
};

enum ETFWeapons
{
	Scout_m_Scattergun = 13,
	Scout_m_ScattergunR = 200,
	Scout_m_ForceANature = 45,
	Scout_m_TheShortstop = 220,
	Scout_m_TheSodaPopper = 448,
	Scout_m_FestiveScattergun = 669,
	Scout_m_BabyFacesBlaster = 772,
	Scout_m_SilverBotkillerScattergunMkI = 799,
	Scout_m_GoldBotkillerScattergunMkI = 808,
	Scout_m_RustBotkillerScattergunMkI = 888,
	Scout_m_BloodBotkillerScattergunMkI = 897,
	Scout_m_CarbonadoBotkillerScattergunMkI = 906,
	Scout_m_DiamondBotkillerScattergunMkI = 915,
	Scout_m_SilverBotkillerScattergunMkII = 964,
	Scout_m_GoldBotkillerScattergunMkII = 973,
	Scout_m_FestiveForceANature = 1078,
	Scout_m_TheBackScatter = 1103,
	Scout_m_NightTerror = 15002,
	Scout_m_TartanTorpedo = 15015,
	Scout_m_CountryCrusher = 15021,
	Scout_m_BackcountryBlaster = 15029,
	Scout_m_SpruceDeuce = 15036,
	Scout_m_CurrentEvent = 15053,
	Scout_m_MacabreWeb = 15065,
	Scout_m_Nutcracker = 15069,
	Scout_m_BlueMew = 15106,
	Scout_m_FlowerPower = 15107,
	Scout_m_ShottoHell = 15108,
	Scout_m_CoffinNail = 15131,
	Scout_m_KillerBee = 15151,
	Scout_m_Corsair = 15157,
	Scout_s_ScoutsPistol = 23,
	Scout_s_PistolR = 209,
	Scout_s_BonkAtomicPunch = 46,
	Scout_s_VintageLugermorph = 160,
	Scout_s_CritaCola = 163,
	Scout_s_MadMilk = 222,
	Scout_s_Lugermorph = 294,
	Scout_s_TheWinger = 449,
	Scout_s_PrettyBoysPocketPistol = 773,
	Scout_s_TheFlyingGuillotine = 812,
	Scout_s_TheFlyingGuillotineG = 833,
	Scout_s_MutatedMilk = 1121,
	Scout_s_FestiveBonk = 1145,
	Scout_s_RedRockRoscoe = 15013,
	Scout_s_HomemadeHeater = 15018,
	Scout_s_HickoryHolepuncher = 15035,
	Scout_s_LocalHero = 15041,
	Scout_s_BlackDahlia = 15046,
	Scout_s_SandstoneSpecial = 15056,
	Scout_s_MacabreWeb = 15060,
	Scout_s_Nutcracker = 15061,
	Scout_s_BlueMew = 15100,
	Scout_s_BrainCandy = 15101,
	Scout_s_ShottoHell = 15102,
	Scout_s_DressedToKill = 15126,
	Scout_s_Blitzkrieg = 15148,
	Scout_s_TheCAPPER = 30666,
	Scout_t_Bat = 0,
	Scout_t_BatR = 190,
	Scout_t_TheSandman = 44,
	Scout_t_TheHolyMackerel = 221,
	Scout_t_TheCandyCane = 317,
	Scout_t_TheBostonBasher = 325,
	Scout_t_SunonaStick = 349,
	Scout_t_TheFanOWar = 355,
	Scout_t_TheAtomizer = 450,
	Scout_t_ThreeRuneBlade = 452,
	Scout_t_TheConscientiousObjector = 474,
	Scout_t_UnarmedCombat = 572,
	Scout_t_TheWrapAssassin = 648,
	Scout_t_FestiveBat = 660,
	Scout_t_TheFreedomStaff = 880,
	Scout_t_TheBatOuttaHell = 939,
	Scout_t_TheMemoryMaker = 954,
	Scout_t_FestiveHolyMackerel = 999,
	Scout_t_TheHamShank = 1013,
	Scout_t_TheNecroSmasher = 1123,
	Scout_t_TheCrossingGuard = 1127,
	Scout_t_Batsaber = 30667,
	Scout_t_PrinnyMachete = 30758,
	Soldier_m_RocketLauncher = 18,
	Soldier_m_RocketLauncherR = 205,
	Soldier_m_TheDirectHit = 127,
	Soldier_m_TheBlackBox = 228,
	Soldier_m_RocketJumper = 237,
	Soldier_m_TheLibertyLauncher = 414,
	Soldier_m_TheCowMangler5000 = 441,
	Soldier_m_TheOriginal = 513,
	Soldier_m_FestiveRocketLauncher = 658,
	Soldier_m_TheBeggarsBazooka = 730,
	Soldier_m_SilverBotkillerRocketLauncherMkI = 800,
	Soldier_m_GoldBotkillerRocketLauncherMkI = 809,
	Soldier_m_RustBotkillerRocketLauncherMkI = 889,
	Soldier_m_BloodBotkillerRocketLauncherMkI = 898,
	Soldier_m_CarbonadoBotkillerRocketLauncherMkI = 907,
	Soldier_m_DiamondBotkillerRocketLauncherMkI = 916,
	Soldier_m_SilverBotkillerRocketLauncherMkII = 965,
	Soldier_m_GoldBotkillerRocketLauncherMkII = 974,
	Soldier_m_FestiveBlackBox = 1085,
	Soldier_m_TheAirStrike = 1104,
	Soldier_m_WoodlandWarrior = 15006,
	Soldier_m_SandCannon = 15014,
	Soldier_m_AmericanPastoral = 15028,
	Soldier_m_SmalltownBringdown = 15043,
	Soldier_m_ShellShocker = 15052,
	Soldier_m_AquaMarine = 15057,
	Soldier_m_Autumn = 15081,
	Soldier_m_BlueMew = 15104,
	Soldier_m_BrainCandy = 15105,
	Soldier_m_CoffinNail = 15129,
	Soldier_m_HighRollers = 15130,
	Soldier_m_Warhawk = 15150,
	Soldier_s_SoldiersShotgun = 10,
	Soldier_s_ShotgunR = 199,
	Soldier_s_TheBuffBanner = 129,
	Soldier_s_Gunboats = 133,
	Soldier_s_TheBattalionsBackup = 226,
	Soldier_s_TheConcheror = 354,
	Soldier_s_TheReserveShooter = 415,
	Soldier_s_TheRighteousBison = 442,
	Soldier_s_TheMantreads = 444,
	Soldier_s_FestiveBuffBanner = 1001,
	Soldier_s_TheBASEJumper = 1101,
	Soldier_s_FestiveShotgun = 1141,
	Soldier_s_PanicAttack = 1153,
	Soldier_s_BackwoodsBoomstick = 15003,
	Soldier_s_RusticRuiner = 15016,
	Soldier_s_CivicDuty = 15044,
	Soldier_s_LightningRod = 15047,
	Soldier_s_Autumn = 15085,
	Soldier_s_FlowerPower = 15109,
	Soldier_s_CoffinNail = 15132,
	Soldier_s_DressedtoKill = 15133,
	Soldier_s_RedBear = 15152,
	Soldier_t_Shovel = 6,
	Soldier_t_ShovelR = 196,
	Soldier_t_TheEqualizer = 128,
	Soldier_t_ThePainTrain = 154,
	Soldier_t_TheHalfZatoichi = 357,
	Soldier_t_TheMarketGardener = 416,
	Soldier_t_TheDisciplinaryAction = 447,
	Soldier_t_TheConscientiousObjector = 474,
	Soldier_t_TheEscapePlan = 775,
	Soldier_t_TheFreedomStaff = 880,
	Soldier_t_TheBatOuttaHell = 939,
	Soldier_t_TheMemoryMaker = 954,
	Soldier_t_TheHamShank = 1013,
	Soldier_t_TheNecroSmasher = 1123,
	Soldier_t_TheCrossingGuard = 1127,
	Soldier_t_PrinnyMachete = 30758,
	Pyro_m_FlameThrower = 21,
	Pyro_m_FlameThrowerR = 208,
	Pyro_m_TheBackburner = 40,
	Pyro_m_TheDegreaser = 215,
	Pyro_m_ThePhlogistinator = 594,
	Pyro_m_FestiveFlameThrower = 659,
	Pyro_m_TheRainblower = 741,
	Pyro_m_SilverBotkillerFlameThrowerMkI = 798,
	Pyro_m_GoldBotkillerFlameThrowerMkI = 807,
	Pyro_m_RustBotkillerFlameThrowerMkI = 887,
	Pyro_m_BloodBotkillerFlameThrowerMkI = 896,
	Pyro_m_CarbonadoBotkillerFlameThrowerMkI = 905,
	Pyro_m_DiamondBotkillerFlameThrowerMkI = 914,
	Pyro_m_SilverBotkillerFlameThrowerMkII = 963,
	Pyro_m_GoldBotkillerFlameThrowerMkII = 972,
	Pyro_m_FestiveBackburner = 1146,
	Pyro_m_DragonsFury = 1178,
	Pyro_m_ForestFire = 15005,
	Pyro_m_BarnBurner = 15017,
	Pyro_m_BovineBlazemaker = 15030,
	Pyro_m_EarthSkyandFire = 15034,
	Pyro_m_FlashFryer = 15049,
	Pyro_m_TurbineTorcher = 15054,
	Pyro_m_Autumn = 15066,
	Pyro_m_PumpkinPatch = 15067,
	Pyro_m_Nutcracker = 15068,
	Pyro_m_Balloonicorn = 15089,
	Pyro_m_Rainbow = 15090,
	Pyro_m_CoffinNail = 15115,
	Pyro_m_Warhawk = 15141,
	Pyro_m_NostromoNapalmer = 30474,
	Pyro_s_PyrosShotgun = 12,
	Pyro_s_ShotgunR = 199,
	Pyro_s_TheFlareGun = 39,
	Pyro_s_TheDetonator = 351,
	Pyro_s_TheReserveShooter = 415,
	Pyro_s_TheManmelter = 595,
	Pyro_s_TheScorchShot = 740,
	Pyro_s_FestiveFlareGun = 1081,
	Pyro_s_FestiveShotgun = 1141,
	Pyro_s_PanicAttack = 1153,
	Pyro_s_ThermalThruster = 1179,
	Pyro_s_GasPasser = 1180,
	Pyro_s_BackwoodsBoomstick = 15003,
	Pyro_s_RusticRuiner = 15016,
	Pyro_s_CivicDuty = 15044,
	Pyro_s_LightningRod = 15047,
	Pyro_s_Autumn = 15085,
	Pyro_s_FlowerPower = 15109,
	Pyro_s_CoffinNail = 15132,
	Pyro_s_DressedtoKill = 15133,
	Pyro_s_RedBear = 15152,
	Pyro_t_FireAxe = 2,
	Pyro_t_FireAxeR = 192,
	Pyro_t_TheAxtinguisher = 38,
	Pyro_t_Homewrecker = 153,
	Pyro_t_ThePowerjack = 214,
	Pyro_t_TheBackScratcher = 326,
	Pyro_t_SharpenedVolcanoFragment = 348,
	Pyro_t_ThePostalPummeler = 457,
	Pyro_t_TheMaul = 466,
	Pyro_t_TheConscientiousObjector = 474,
	Pyro_t_TheThirdDegree = 593,
	Pyro_t_TheLollichop = 739,
	Pyro_t_NeonAnnihilator = 813,
	Pyro_t_NeonAnnihilatorG = 834,
	Pyro_t_TheFreedomStaff = 880,
	Pyro_t_TheBatOuttaHell = 939,
	Pyro_t_TheMemoryMaker = 954,
	Pyro_t_TheFestiveAxtinguisher = 1000,
	Pyro_t_TheHamShank = 1013,
	Pyro_t_TheNecroSmasher = 1123,
	Pyro_t_TheCrossingGuard = 1127,
	Pyro_t_HotHand = 1181,
	Pyro_t_PrinnyMachete = 30758,
	Demoman_m_GrenadeLauncher = 19,
	Demoman_m_GrenadeLauncherR = 206,
	Demoman_m_TheLochnLoad = 308,
	Demoman_m_AliBabasWeeBooties = 405,
	Demoman_m_TheBootlegger = 608,
	Demoman_m_TheLooseCannon = 996,
	Demoman_m_FestiveGrenadeLauncher = 1007,
	Demoman_m_TheBASEJumper = 1101,
	Demoman_m_TheIronBomber = 1151,
	Demoman_m_Autumn = 15077,
	Demoman_m_MacabreWeb = 15079,
	Demoman_m_Rainbow = 15091,
	Demoman_m_SweetDreams = 15092,
	Demoman_m_CoffinNail = 15116,
	Demoman_m_TopShelf = 15117,
	Demoman_m_Warhawk = 15142,
	Demoman_m_ButcherBird = 15158,
	Demoman_s_StickybombLauncher = 20,
	Demoman_s_StickybombLauncherR = 207,
	Demoman_s_TheScottishResistance = 130,
	Demoman_s_TheCharginTarge = 131,
	Demoman_s_StickyJumper = 265,
	Demoman_s_TheSplendidScreen = 406,
	Demoman_s_FestiveStickybombLauncher = 661,
	Demoman_s_SilverBotkillerStickybombLauncherMkI = 797,
	Demoman_s_GoldBotkillerStickybombLauncherMkI = 806,
	Demoman_s_RustBotkillerStickybombLauncherMkI = 886,
	Demoman_s_BloodBotkillerStickybombLauncherMkI = 895,
	Demoman_s_CarbonadoBotkillerStickybombLauncherMkI = 904,
	Demoman_s_DiamondBotkillerStickybombLauncherMkI = 913,
	Demoman_s_SilverBotkillerStickybombLauncherMkII = 962,
	Demoman_s_GoldBotkillerStickybombLauncherMkII = 971,
	Demoman_s_TheTideTurner = 1099,
	Demoman_s_FestiveTarge = 1144,
	Demoman_s_TheQuickiebombLauncher = 1150,
	Demoman_s_SuddenFlurry = 15009,
	Demoman_s_CarpetBomber = 15012,
	Demoman_s_BlastedBombardier = 15024,
	Demoman_s_RooftopWrangler = 15038,
	Demoman_s_LiquidAsset = 15045,
	Demoman_s_PinkElephant = 15048,
	Demoman_s_Autumn = 15082,
	Demoman_s_PumpkinPatch = 15083,
	Demoman_s_MacabreWeb = 15084,
	Demoman_s_SweetDreams = 15113,
	Demoman_s_CoffinNail = 15137,
	Demoman_s_DressedtoKill = 15138,
	Demoman_s_Blitzkrieg = 15155,
	Demoman_t_Bottle = 1,
	Demoman_t_BottleR = 191,
	Demoman_t_TheEyelander = 132,
	Demoman_t_ThePainTrain = 154,
	Demoman_t_TheScotsmansSkullcutter = 172,
	Demoman_t_HorselessHeadlessHorsemannsHeadtaker = 266,
	Demoman_t_UllapoolCaber = 307,
	Demoman_t_TheClaidheamhMor = 327,
	Demoman_t_TheHalfZatoichi = 357,
	Demoman_t_ThePersianPersuader = 404,
	Demoman_t_TheConscientiousObjector = 474,
	Demoman_t_NessiesNineIron = 482,
	Demoman_t_TheScottishHandshake = 609,
	Demoman_t_TheFreedomStaff = 880,
	Demoman_t_TheBatOuttaHell = 939,
	Demoman_t_TheMemoryMaker = 954,
	Demoman_t_TheHamShank = 1013,
	Demoman_t_FestiveEyelander = 1082,
	Demoman_t_TheNecroSmasher = 1123,
	Demoman_t_TheCrossingGuard = 1127,
	Demoman_t_PrinnyMachete = 30758,
	Heavy_m_Minigun = 15,
	Heavy_m_MinigunR = 202,
	Heavy_m_Natascha = 41,
	Heavy_m_IronCurtain = 298,
	Heavy_m_TheBrassBeast = 312,
	Heavy_m_Tomislav = 424,
	Heavy_m_FestiveMinigun = 654,
	Heavy_m_SilverBotkillerMinigunMkI = 793,
	Heavy_m_GoldBotkillerMinigunMkI = 802,
	Heavy_m_TheHuoLongHeater = 811,
	Heavy_m_TheHuoLongHeaterG = 832,
	Heavy_m_Deflector_mvm = 850,
	Heavy_m_RustBotkillerMinigunMkI = 882,
	Heavy_m_BloodBotkillerMinigunMkI = 891,
	Heavy_m_CarbonadoBotkillerMinigunMkI = 900,
	Heavy_m_DiamondBotkillerMinigunMkI = 909,
	Heavy_m_SilverBotkillerMinigunMkII = 958,
	Heavy_m_GoldBotkillerMinigunMkII = 967,
	Heavy_m_KingoftheJungle = 15004,
	Heavy_m_IronWood = 15020,
	Heavy_m_AntiqueAnnihilator = 15026,
	Heavy_m_WarRoom = 15031,
	Heavy_m_CitizenPain = 15040,
	Heavy_m_BrickHouse = 15055,
	Heavy_m_MacabreWeb = 15086,
	Heavy_m_PumpkinPatch = 15087,
	Heavy_m_Nutcracker = 15088,
	Heavy_m_BrainCandy = 15098,
	Heavy_m_MisterCuddles = 15099,
	Heavy_m_CoffinNail = 15123,
	Heavy_m_DressedtoKill = 15124,
	Heavy_m_TopShelf = 15125,
	Heavy_m_ButcherBird = 15147,
	Heavy_s_HeavysShotgun = 11,
	Heavy_s_ShotgunR = 199,
	Heavy_s_Sandvich = 42,
	Heavy_s_TheDalokohsBar = 159,
	Heavy_s_TheBuffaloSteakSandvich = 311,
	Heavy_s_TheFamilyBusiness = 425,
	Heavy_s_Fishcake = 433,
	Heavy_s_RoboSandvich = 863,
	Heavy_s_FestiveSandvich = 1002,
	Heavy_s_FestiveShotgun = 1141,
	Heavy_s_PanicAttack = 1153,
	Heavy_s_SecondBanana = 1190,
	Heavy_s_BackwoodsBoomstick = 15003,
	Heavy_s_RusticRuiner = 15016,
	Heavy_s_CivicDuty = 15044,
	Heavy_s_LightningRod = 15047,
	Heavy_s_Autumn = 15085,
	Heavy_s_FlowerPower = 15109,
	Heavy_s_CoffinNail = 15132,
	Heavy_s_DressedtoKill = 15133,
	Heavy_s_RedBear = 15152,
	Heavy_t_Fists = 5,
	Heavy_t_FistsR = 195,
	Heavy_t_TheKillingGlovesofBoxing = 43,
	Heavy_t_GlovesofRunningUrgently = 239,
	Heavy_t_WarriorsSpirit = 310,
	Heavy_t_FistsofSteel = 331,
	Heavy_t_TheEvictionNotice = 426,
	Heavy_t_TheConscientiousObjector = 474,
	Heavy_t_ApocoFists = 587,
	Heavy_t_TheHolidayPunch = 656,
	Heavy_t_TheFreedomStaff = 880,
	Heavy_t_TheBatOuttaHell = 939,
	Heavy_t_TheMemoryMaker = 954,
	Heavy_t_TheHamShank = 1013,
	Heavy_t_FestiveGlovesofRunningUrgently = 1084,
	Heavy_t_TheBreadBite = 1100,
	Heavy_t_TheNecroSmasher = 1123,
	Heavy_t_TheCrossingGuard = 1127,
	Heavy_t_GlovesofRunningUrgentlyMvM = 1184,
	Heavy_t_PrinnyMachete = 30758,
	Engi_m_EngineersShotgun = 9,
	Engi_m_ShotgunR = 199,
	Engi_m_TheFrontierJustice = 141,
	Engi_m_TheWidowmaker = 527,
	Engi_m_ThePomson6000 = 588,
	Engi_m_TheRescueRanger = 997,
	Engi_m_FestiveFrontierJustice = 1004,
	Engi_m_FestiveShotgun = 1141,
	Engi_m_PanicAttack = 1153,
	Engi_m_BackwoodsBoomstick = 15003,
	Engi_m_RusticRuiner = 15016,
	Engi_m_CivicDuty = 15044,
	Engi_m_LightningRod = 15047,
	Engi_m_Autumn = 15085,
	Engi_m_FlowerPower = 15109,
	Engi_m_CoffinNail = 15132,
	Engi_m_DressedtoKill = 15133,
	Engi_m_RedBear = 15152,
	Engi_s_EngineersPistol = 22,
	Engi_s_PistolR = 209,
	Engi_s_TheWrangler = 140,
	Engi_s_VintageLugermorph = 160,
	Engi_s_Lugermorph = 294,
	Engi_s_TheShortCircuit = 528,
	Engi_s_FestiveWrangler = 1086,
	Engi_s_RedRockRoscoe = 15013,
	Engi_s_HomemadeHeater = 15018,
	Engi_s_HickoryHolepuncher = 15035,
	Engi_s_LocalHero = 15041,
	Engi_s_BlackDahlia = 15046,
	Engi_s_SandstoneSpecial = 15056,
	Engi_s_MacabreWeb = 15060,
	Engi_s_Nutcracker = 15061,
	Engi_s_BlueMew = 15100,
	Engi_s_BrainCandy = 15101,
	Engi_s_ShottoHell = 15102,
	Engi_s_DressedToKill = 15126,
	Engi_s_Blitzkrieg = 15148,
	Engi_s_TheCAPPER = 30666,
	Engi_s_TheGigarCounter = 30668,
	Engi_t_Wrench = 7,
	Engi_t_WrenchR = 197,
	Engi_t_TheGunslinger = 142,
	Engi_t_TheSouthernHospitality = 155,
	Engi_t_GoldenWrench = 169,
	Engi_t_TheJag = 329,
	Engi_t_TheEurekaEffect = 589,
	Engi_t_FestiveWrench = 662,
	Engi_t_SilverBotkillerWrenchMkI = 795,
	Engi_t_GoldBotkillerWrenchMkI = 804,
	Engi_t_RustBotkillerWrenchMkI = 884,
	Engi_t_BloodBotkillerWrenchMkI = 893,
	Engi_t_CarbonadoBotkillerWrenchMkI = 902,
	Engi_t_DiamondBotkillerWrenchMkI = 911,
	Engi_t_SilverBotkillerWrenchMkII = 960,
	Engi_t_GoldBotkillerWrenchMkII = 969,
	Engi_t_TheNecroSmasher = 1123,
	Engi_t_Nutcracker = 15073,
	Engi_t_Autumn = 15074,
	Engi_t_Boneyard = 15075,
	Engi_t_DressedtoKill = 15139,
	Engi_t_TopShelf = 15140,
	Engi_t_TorquedtoHell = 15114,
	Engi_t_Airwolf = 15156,
	Engi_t_PrinnyMachete = 30758,
	Engi_p_ConstructionPDA = 25,
	Engi_p_ConstructionPDAR = 737,
	Engi_p_DestructionPDA = 26,
	Engi_p_PDA = 28,
	Medic_m_SyringeGun = 17,
	Medic_m_SyringeGunR = 204,
	Medic_m_TheBlutsauger = 36,
	Medic_m_CrusadersCrossbow = 305,
	Medic_m_TheOverdose = 412,
	Medic_m_FestiveCrusadersCrossbow = 1079,
	Medic_s_MediGun = 29,
	Medic_s_MediGunR = 211,
	Medic_s_TheKritzkrieg = 35,
	Medic_s_TheQuickFix = 411,
	Medic_s_FestiveMediGun = 663,
	Medic_s_SilverBotkillerMediGunMkI = 796,
	Medic_s_GoldBotkillerMediGunMkI = 805,
	Medic_s_RustBotkillerMediGunMkI = 885,
	Medic_s_BloodBotkillerMediGunMkI = 894,
	Medic_s_CarbonadoBotkillerMediGunMkI = 903,
	Medic_s_DiamondBotkillerMediGunMkI = 912,
	Medic_s_SilverBotkillerMediGunMkII = 961,
	Medic_s_GoldBotkillerMediGunMkII = 970,
	Medic_s_TheVaccinator = 998,
	Medic_s_MaskedMender = 15008,
	Medic_s_WrappedReviver = 15010,
	Medic_s_ReclaimedReanimator = 15025,
	Medic_s_CivilServant = 15039,
	Medic_s_SparkofLife = 15050,
	Medic_s_Wildwood = 15078,
	Medic_s_FlowerPower = 15097,
	Medic_s_DressedToKill = 15121,
	Medic_s_HighRollers = 15122,
	Medic_s_CoffinNail = 15123,
	Medic_s_Blitzkrieg = 15145,
	Medic_s_Corsair = 15146,
	Medic_t_Bonesaw = 8,
	Medic_t_BonesawR = 198,
	Medic_t_TheUbersaw = 37,
	Medic_t_TheVitaSaw = 173,
	Medic_t_Amputator = 304,
	Medic_t_TheSolemnVow = 413,
	Medic_t_TheConscientiousObjector = 474,
	Medic_t_TheFreedomStaff = 880,
	Medic_t_TheBatOuttaHell = 939,
	Medic_t_TheMemoryMaker = 954,
	Medic_t_FestiveUbersaw = 1003,
	Medic_t_TheHamShank = 1013,
	Medic_t_TheNecroSmasher = 1123,
	Medic_t_TheCrossingGuard = 1127,
	Medic_t_FestiveBonesaw = 1143,
	Medic_t_PrinnyMachete = 30758,
	Sniper_m_SniperRifle = 14,
	Sniper_m_SniperRifleR = 201,
	Sniper_m_TheHuntsman = 56,
	Sniper_m_TheSydneySleeper = 230,
	Sniper_m_TheBazaarBargain = 402,
	Sniper_m_TheMachina = 526,
	Sniper_m_FestiveSniperRifle = 664,
	Sniper_m_TheHitmansHeatmaker = 752,
	Sniper_m_SilverBotkillerSniperRifleMkI = 792,
	Sniper_m_GoldBotkillerSniperRifleMkI = 801,
	Sniper_m_TheAWPerHand = 851,
	Sniper_m_RustBotkillerSniperRifleMkI = 881,
	Sniper_m_BloodBotkillerSniperRifleMkI = 890,
	Sniper_m_CarbonadoBotkillerSniperRifleMkI = 899,
	Sniper_m_DiamondBotkillerSniperRifleMkI = 908,
	Sniper_m_SilverBotkillerSniperRifleMkII = 957,
	Sniper_m_GoldBotkillerSniperRifleMkII = 966,
	Sniper_m_FestiveHuntsman = 1005,
	Sniper_m_TheFortifiedCompound = 1092,
	Sniper_m_TheClassic = 1098,
	Sniper_m_NightOwl = 15000,
	Sniper_m_PurpleRange = 15007,
	Sniper_m_LumberFromDownUnder = 15019,
	Sniper_m_ShotintheDark = 15023,
	Sniper_m_Bogtrotter = 15033,
	Sniper_m_Thunderbolt = 15059,
	Sniper_m_PumpkinPatch = 15070,
	Sniper_m_Boneyard = 15071,
	Sniper_m_Wildwood = 15072,
	Sniper_m_Balloonicorn = 15111,
	Sniper_m_Rainbow = 15112,
	Sniper_m_CoffinNail = 15135,
	Sniper_m_DressedtoKill = 15136,
	Sniper_m_Airwolf = 15154,
	Sniper_m_ShootingStar = 30665,
	Sniper_s_SMG = 16,
	Sniper_s_SMGR = 203,
	Sniper_s_TheRazorback = 57,
	Sniper_s_Jarate = 58,
	Sniper_s_DarwinsDangerShield = 231,
	Sniper_s_CozyCamper = 642,
	Sniper_s_TheCleanersCarbine = 751,
	Sniper_s_FestiveJarate = 1083,
	Sniper_s_TheSelfAwareBeautyMark = 1105,
	Sniper_s_FestiveSMG = 1149,
	Sniper_s_WoodsyWidowmaker = 15001,
	Sniper_s_PlaidPotshotter = 15022,
	Sniper_s_TreadplateTormenter = 15032,
	Sniper_s_TeamSprayer = 15037,
	Sniper_s_LowProfile = 15058,
	Sniper_s_Wildwood = 15076,
	Sniper_s_BlueMew = 15110,
	Sniper_s_HighRollers = 15134,
	Sniper_s_Blitzkrieg = 15153,
	Sniper_t_Kukri = 3,
	Sniper_t_KukriR = 193,
	Sniper_t_TheTribalmansShiv = 171,
	Sniper_t_TheBushwacka = 232,
	Sniper_t_TheShahanshah = 401,
	Sniper_t_TheConscientiousObjector = 474,
	Sniper_t_TheFreedomStaff = 880,
	Sniper_t_TheBatOuttaHell = 939,
	Sniper_t_TheMemoryMaker = 954,
	Sniper_t_TheHamShank = 1013,
	Sniper_t_TheNecroSmasher = 1123,
	Sniper_t_TheCrossingGuard = 1127,
	Sniper_t_PrinnyMachete = 30758,
	Spy_m_Revolver = 24,
	Spy_m_RevolverR = 210,
	Spy_m_TheAmbassador = 61,
	Spy_m_BigKill = 161,
	Spy_m_LEtranger = 224,
	Spy_m_TheEnforcer = 460,
	Spy_m_TheDiamondback = 525,
	Spy_m_FestiveAmbassador = 1006,
	Spy_m_FestiveRevolver = 1142,
	Spy_m_PsychedelicSlugger = 15011,
	Spy_m_OldCountry = 15027,
	Spy_m_Mayor = 15042,
	Spy_m_DeadReckoner = 15051,
	Spy_m_Boneyard = 15062,
	Spy_m_Wildwood = 15063,
	Spy_m_MacabreWeb = 15064,
	Spy_m_FlowerPower = 15103,
	Spy_m_TopShelf = 15128,
	Spy_m_CoffinNail = 15129,
	Spy_m_Blitzkrieg = 15149,
	Spy_s_Sapper = 735,
	Spy_s_SapperR = 736,
	Spy_s_TheRedTapeRecorder = 810,
	Spy_s_TheRedTapeRecorderG = 831,
	Spy_s_TheApSapG = 933,
	Spy_s_FestiveSapper = 1080,
	Spy_s_TheSnackAttack = 1102,
	Spy_t_Knife = 4,
	Spy_t_KnifeR = 194,
	Spy_t_YourEternalReward = 225,
	Spy_t_ConniversKunai = 356,
	Spy_t_TheBigEarner = 461,
	Spy_t_TheWangaPrick = 574,
	Spy_t_TheSharpDresser = 638,
	Spy_t_TheSpycicle = 649,
	Spy_t_FestiveKnife = 665,
	Spy_t_TheBlackRose = 727,
	Spy_t_SilverBotkillerKnifeMkI = 794,
	Spy_t_GoldBotkillerKnifeMkI = 803,
	Spy_t_RustBotkillerKnifeMkI = 883,
	Spy_t_BloodBotkillerKnifeMkI = 892,
	Spy_t_CarbonadoBotkillerKnifeMkI = 901,
	Spy_t_DiamondBotkillerKnifeMkI = 910,
	Spy_t_SilverBotkillerKnifeMkII = 959,
	Spy_t_GoldBotkillerKnifeMkII = 968,
	Spy_t_Boneyard = 15062,
	Spy_t_BlueMew = 15094,
	Spy_t_BrainCandy = 15095,
	Spy_t_StabbedtoHell = 15096,
	Spy_t_DressedtoKill = 15118,
	Spy_t_TopShelf = 15119,
	Spy_t_Blitzkrieg = 15143,
	Spy_t_Airwolf = 15144,
	Spy_t_PrinnyMachete = 30758,
	Spy_d_DisguiseKitPDA = 27,
	Spy_w_InvisWatch = 30,
	Spy_w_InvisWatchR = 212,
	Spy_w_TheDeadRinger = 59,
	Spy_w_TheCloakandDagger = 60,
	Spy_w_EnthusiastsTimepiece = 297,
	Spy_w_TheQuackenbirdt = 947,
	Misc_t_FryingPan = 264,
	Misc_t_GoldFryingPan = 1071,
	Misc_t_Saxxy = 423
};

class CPhysCollide;

struct objectparams_t
{
	Vector *massCenterOverride{};
	float mass{};
	float inertia{};
	float damping{};
	float rotdamping{};
	float rotInertiaLimit{};
	const char *pName{};
	void *pGameData{};
	float volume{};
	float dragCoefficient{};
	bool enableCollisions{};
};

const objectparams_t g_PhysDefaultObjectParams{ 0, 1.0f, 1.0f, 0.1f, 0.1f, 0.05f, "DEFAULT", 0, 0.0f, 1.0f, true };

constexpr float k_flMaxVelocity = 2000.0f;
constexpr float k_flMaxAngularVelocity = 360.0f * 10.0f;

constexpr float DEFAULT_MIN_FRICTION_MASS = 10.0f;
constexpr float DEFAULT_MAX_FRICTION_MASS = 2500.0f;

struct physics_performanceparams_t
{
	int maxCollisionsPerObjectPerTimestep{};
	int maxCollisionChecksPerTimestep{};
	float maxVelocity{};
	float maxAngularVelocity{};
	float lookAheadTimeObjectsVsWorld{};
	float lookAheadTimeObjectsVsObject{};
	float minFrictionMass{};
	float maxFrictionMass{};

	void Defaults()
	{
		maxCollisionsPerObjectPerTimestep = 6;
		maxCollisionChecksPerTimestep = 250;
		maxVelocity = k_flMaxVelocity;
		maxAngularVelocity = k_flMaxAngularVelocity;
		lookAheadTimeObjectsVsWorld = 1.0f;
		lookAheadTimeObjectsVsObject = 0.5f;
		minFrictionMass = DEFAULT_MIN_FRICTION_MASS;
		maxFrictionMass = DEFAULT_MAX_FRICTION_MASS;
	}
};

typedef Vector AngularImpulse;

class IVP_Real_Object;
class IPhysicsShadowController;

class IPhysicsObject
{
private:
	void *thisptr{};

public:
	void *m_pGameData{};
	IVP_Real_Object *m_pObject{};
	const CPhysCollide *m_pCollide{};
	IPhysicsShadowController *m_pShadow{};
	Vector m_dragBasis{};
	Vector m_angDragBasis{};
	bool m_shadowTempGravityDisable : 5;
	bool m_hasTouchedDynamic : 1;
	bool m_asleepSinceCreation : 1;
	bool m_forceSilentDelete : 1;
	unsigned char m_sleepState : 2;
	unsigned char m_hingedAxis : 3;
	unsigned char m_collideType : 3;
	unsigned short m_gameIndex{};
	unsigned short m_materialIndex{};
	unsigned short m_activeIndex{};
	unsigned short m_callbacks{};
	unsigned short m_gameFlags{};
	unsigned int m_contentsMask{};
	float m_volume{};
	float m_buoyancyRatio{};
	float m_dragCoefficient{};
	float m_angDragCoefficient{};

public:
	void Wake()
	{
		mem::callVirtual<25, void>(this);
	}

	void SetDragCoefficient(float *pDrag, float *pAngularDrag)
	{
		mem::callVirtual<37, void, float *, float *>(this, pDrag, pAngularDrag);
	}

	void SetPosition(const Vector &worldPosition, const QAngle &angles, bool isTeleport)
	{
		mem::callVirtual<45, void, const Vector &, const QAngle &, bool>(this, worldPosition, angles, isTeleport);
	}

	void GetPosition(Vector *worldPosition, QAngle *angles)
	{
		mem::callVirtual<47, void, Vector *, QAngle *>(this, worldPosition, angles);
	}

	void SetVelocity(const Vector *velocity, const AngularImpulse *angularVelocity)
	{
		mem::callVirtual<49, void, const Vector *, const AngularImpulse *>(this, velocity, angularVelocity);
	}
};

class IPhysicsEnvironment
{
public:
	void SetGravity(const Vector &gravityVector)
	{
		mem::callVirtual<3, void, const Vector &>(this, gravityVector);
	}

	void SetAirDensity(float density)
	{
		mem::callVirtual<5, void, float>(this, density);
	}

	IPhysicsObject *CreatePolyObject(const CPhysCollide *pCollisionModel, int materialIndex, const Vector &position, const QAngle &angles, objectparams_t *pParams)
	{
		return mem::callVirtual<7, IPhysicsObject *, const CPhysCollide *, int, const Vector &, const QAngle &, objectparams_t *>
			(this, pCollisionModel, materialIndex, position, angles, pParams);
	}

	void Simulate(float deltaTime)
	{
		mem::callVirtual<34, void, float>(this, deltaTime);
	}

	void ResetSimulationClock()
	{
		mem::callVirtual<39, void>(this);
	}

	void SetPerformanceSettings(const physics_performanceparams_t *pSettings)
	{
		mem::callVirtual<59, void, const physics_performanceparams_t *>(this, pSettings);
	}
};

enum
{
	PC_EVERYTHING = 0,
	PC_NON_NETWORKED_ONLY,
	PC_NETWORKED_ONLY,
};

typedef union
{
	float m128_f32[4];
	uint32_t m128_u32[4];
} fltx4;

struct SpriteRenderInfo_t
{
	size_t m_nXYZStride{};
	fltx4 *m_pXYZ{};
	size_t m_nRotStride{};
	fltx4 *m_pRot{};
	size_t m_nYawStride{};
	fltx4 *m_pYaw{};
	size_t m_nRGBStride{};
	fltx4 *m_pRGB{};
	size_t m_nCreationTimeStride{};
	fltx4 *m_pCreationTimeStamp{};
	size_t m_nSequenceStride{};
	fltx4 *m_pSequenceNumber{};
	size_t m_nSequence1Stride{};
	fltx4 *m_pSequence1Number{};
	float m_flAgeScale{};
	float m_flAgeScale2{};
	void *m_pSheet{};
	int m_nVertexOffset{};
	void *m_pParticles{};
};

class CEconNotification
{
public:
	enum EType
	{
		eType_Basic,
		eType_AcceptDecline,
		eType_Trigger,
		eType_MustTrigger,
	};

	virtual ~CEconNotification() = 0;
	virtual void SetLifetime(float flSeconds) = 0;
	virtual float GetExpireTime() const = 0;
	virtual float GetInGameLifeTime() const = 0;
	virtual bool BShowInGameElements() const = 0;
	virtual void MarkForDeletion() = 0;
	virtual EType NotificationType() = 0;
	virtual bool BHighPriority() = 0;
	virtual void Trigger() = 0;
	virtual void Accept() = 0;
	virtual void Decline() = 0;
	virtual void Deleted() = 0;
	virtual void Expired() = 0;
	virtual void UpdateTick() = 0;
	virtual const char *GetUnlocalizedHelpText() = 0;
	virtual void *CreateUIElement(bool bMainMenu) const = 0;

public:
	const char *m_pText;
	const char *m_pSoundFilename;
	float m_flExpireTime;
	KeyValues *m_pKeyValues;
	wchar_t m_wszBuffer[1024];
	__int64 m_steamID;

private:
	friend class CEconNotificationQueue;
	int m_iID;
	bool m_bInUse;
};


#define MD5_DIGEST_LENGTH 16  
#define MD5_BIT_LENGTH ( MD5_DIGEST_LENGTH * sizeof(unsigned char) )

struct MD5Value_t
{
	unsigned char bits[MD5_DIGEST_LENGTH];

	void Zero();
	bool IsZero() const;

	bool operator==(const MD5Value_t &src) const;
	bool operator!=(const MD5Value_t &src) const;

};

// MD5 Hash
typedef struct
{
	unsigned int	buf[4];
	unsigned int	bits[2];
	unsigned char	in[64];
} MD5Context_t;

void MD5Init(MD5Context_t *context);
void MD5Update(MD5Context_t *context, unsigned char const *buf, unsigned int len);
void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5Context_t *context);

unsigned int MD5_PseudoRandom(unsigned int nSeed);

bool MD5_Compare(const MD5Value_t &data, const MD5Value_t &compare);

inline bool MD5Value_t::operator==(const MD5Value_t &src) const
{
	return MD5_Compare(*this, src);
}

inline bool MD5Value_t::operator!=(const MD5Value_t &src) const
{
	return !MD5_Compare(*this, src);
}

#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

#define MD5STEP(f, w, x, y, z, data, s) \
        ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

#pragma warning(disable: 5033) // register is no longer a supported storage class - use __declspec(register) instead

inline void MD5Transform(unsigned int buf[4], unsigned int const in[16])
{
	unsigned int a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

inline void MD5Init(MD5Context_t *ctx)
{
	ctx->buf[0] = 0x67452301;
	ctx->buf[1] = 0xefcdab89;
	ctx->buf[2] = 0x98badcfe;
	ctx->buf[3] = 0x10325476;

	ctx->bits[0] = 0;
	ctx->bits[1] = 0;
}

inline void MD5Update(MD5Context_t *ctx, unsigned char const *buf, unsigned int len)
{
	unsigned int t;

	t = ctx->bits[0];
	if ((ctx->bits[0] = t + ((unsigned int)len << 3)) < t)
		ctx->bits[1]++;
	ctx->bits[1] += len >> 29;

	t = (t >> 3) & 0x3f;

	if (t)
	{
		unsigned char *p = (unsigned char *)ctx->in + t;

		t = 64 - t;
		if (len < t)
		{
			memcpy(p, buf, len);
			return;
		}
		memcpy(p, buf, t);
		MD5Transform(ctx->buf, (unsigned int *)ctx->in);
		buf += t;
		len -= t;
	}

	while (len >= 64)
	{
		memcpy(ctx->in, buf, 64);
		MD5Transform(ctx->buf, (unsigned int *)ctx->in);
		buf += 64;
		len -= 64;
	}

	memcpy(ctx->in, buf, len);
}

inline void MD5Final(unsigned char digest[MD5_DIGEST_LENGTH], MD5Context_t *ctx)
{
	unsigned count;
	unsigned char *p;

	count = (ctx->bits[0] >> 3) & 0x3F;

	p = ctx->in + count;
	*p++ = 0x80;

	count = 64 - 1 - count;

	if (count < 8)
	{
		memset(p, 0, count);
		MD5Transform(ctx->buf, (unsigned int *)ctx->in);

		memset(ctx->in, 0, 56);
	}
	else
	{
		memset(p, 0, count - 8);
	}

	((unsigned int *)ctx->in)[14] = ctx->bits[0];
	((unsigned int *)ctx->in)[15] = ctx->bits[1];

	MD5Transform(ctx->buf, (unsigned int *)ctx->in);
	memcpy(digest, ctx->buf, MD5_DIGEST_LENGTH);
	memset(ctx, 0, sizeof(*ctx));
}

inline unsigned int MD5_PseudoRandom(unsigned int nSeed)
{
	MD5Context_t ctx;
	unsigned char digest[MD5_DIGEST_LENGTH];

	memset(&ctx, 0, sizeof(ctx));

	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char *)&nSeed, sizeof(nSeed));
	MD5Final(digest, &ctx);

	return *(unsigned int *)(digest + 6);
}

inline bool MD5_Compare(const MD5Value_t &data, const MD5Value_t &compare)
{
	return memcmp(data.bits, compare.bits, MD5_DIGEST_LENGTH) == 0;
}

inline void MD5Value_t::Zero()
{
	memset(bits, 0, sizeof(bits));
}

inline bool MD5Value_t::IsZero() const
{
	for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
		if (bits[i] != 0)
			return false;
	}

	return true;
}

#define DMG_GENERIC 0		
#define DMG_CRUSH (1 << 0)
#define DMG_BULLET (1 << 1)
#define DMG_SLASH (1 << 2)
#define DMG_BURN (1 << 3)
#define DMG_VEHICLE (1 << 4)
#define DMG_FALL (1 << 5)
#define DMG_BLAST (1 << 6)
#define DMG_CLUB (1 << 7)
#define DMG_SHOCK (1 << 8)
#define DMG_SONIC (1 << 9)
#define DMG_ENERGYBEAM (1 << 10)
#define DMG_PREVENT_PHYSICS_FORCE (1 << 11) 
#define DMG_NEVERGIB (1 << 12)
#define DMG_ALWAYSGIB (1 << 13)
#define DMG_DROWN (1 << 14)
#define DMG_PARALYZE (1 << 15)
#define DMG_NERVEGAS (1 << 16)
#define DMG_POISON (1 << 17)
#define DMG_RADIATION (1 << 18)
#define DMG_DROWNRECOVER (1 << 19)
#define DMG_ACID (1 << 20)
#define DMG_SLOWBURN (1 << 21)
#define DMG_REMOVENORAGDOLL	(1<<22)
#define DMG_PHYSGUN (1<<23)
#define DMG_PLASMA (1<<24)
#define DMG_AIRBOAT (1<<25)
#define DMG_DISSOLVE (1<<26)
#define DMG_BLAST_SURFACE (1<<27)
#define DMG_DIRECT (1<<28)
#define DMG_BUCKSHOT (1<<29)
#define DMG_LASTGENERICFLAG	DMG_BUCKSHOT
#define	DAMAGE_NO 0
#define DAMAGE_EVENTS_ONLY 1
#define	DAMAGE_YES 2
#define	DAMAGE_AIM 3

#define DMG_USE_HITLOCATIONS (DMG_AIRBOAT)
#define DMG_HALF_FALLOFF (DMG_RADIATION)
#define DMG_CRITICAL (DMG_ACID)
#define DMG_RADIUS_MAX (DMG_ENERGYBEAM)
#define DMG_IGNITE (DMG_PLASMA)
#define DMG_USEDISTANCEMOD (DMG_SLOWBURN)
#define DMG_NOCLOSEDISTANCEMOD (DMG_POISON)
#define DMG_FROM_OTHER_SAPPER (DMG_IGNITE)
#define DMG_MELEE (DMG_BLAST_SURFACE)
#define DMG_DONT_COUNT_DAMAGE_TOWARDS_CRIT_RATE	(DMG_DISSOLVE)
#define DMG_IGNORE_MAXHEALTH (DMG_BULLET)
#define DMG_IGNORE_DEBUFFS (DMG_SLASH)

enum
{
	TF_RELOAD_START,
	TF_RELOADING,
	TF_RELOADING_CONTINUE,
	TF_RELOAD_FINISH
};

#define MULTIPLAYER_BACKUP 90

class INetMessage
{
public:
	virtual ~INetMessage() {};
	virtual void SetNetChannel(INetChannelInfo *netchan) = 0;
	virtual void SetReliable(bool state) = 0;
	virtual bool Process(void) = 0;
	virtual bool ReadFromBuffer(bf_read &buffer) = 0;
	virtual bool WriteToBuffer(bf_write &buffer) = 0;
	virtual bool IsReliable(void) const = 0;
	virtual int GetType(void) const = 0;
	virtual int GetGroup(void) const = 0;
	virtual const char *GetName(void) const = 0;
	virtual INetChannelInfo *GetNetChannel(void) const = 0;
	virtual const char *ToString(void) const = 0;
};

class CNetMessage : public INetMessage
{
public:
	CNetMessage()
	{
		m_bReliable = true;
		m_NetChannel = nullptr;
	}

public:
	virtual ~CNetMessage() {};
	virtual int GetGroup() const { return INetChannelInfo::GENERIC; }
	virtual void SetReliable(bool state) { m_bReliable = state; };
	virtual bool IsReliable() const { return m_bReliable; };
	virtual void SetNetChannel(INetChannelInfo *netchan) { m_NetChannel = netchan; }
	virtual bool Process() { return false; }; // no handler set

public:
	INetChannelInfo *GetNetChannel() const { return m_NetChannel; }

protected:
	bool m_bReliable{};
	INetChannelInfo *m_NetChannel{};
	byte pad0[8]{};
};

#define net_NOP 0
#define net_Disconnect 1
#define net_File 2
#define net_Tick 3
#define net_StringCmd 4
#define net_SetConVar 5
#define net_SignonState 6
#define clc_ClientInfo 8       
#define clc_Move 9             
#define clc_VoiceData 10       
#define clc_BaselineAck 11     
#define clc_ListenEvents 12    
#define clc_RespondCvarValue 13
#define clc_FileCRCCheck 14    
#define clc_CmdKeyValues 16    

#define DELTA_OFFSET_BITS 5
#define DELTA_OFFSET_MAX ( ( 1 << DELTA_OFFSET_BITS ) - 1 )
#define DELTASIZE_BITS 20	
#define NUM_NEW_COMMAND_BITS 4
#define MAX_NEW_COMMANDS ((1 << NUM_NEW_COMMAND_BITS)-1)
#define NUM_BACKUP_COMMAND_BITS 3
#define MAX_BACKUP_COMMANDS ((1 << NUM_BACKUP_COMMAND_BITS)-1)
#define MAX_PLAYER_NAME_LENGTH 32
#define MAX_CUSTOM_FILES 4
#define NETMSG_TYPE_BITS 6

class CLC_Move : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read &buffer);
	bool WriteToBuffer(bf_write &buffer);
	const char *ToString() const { return "hi"; }
	int GetType() const { return clc_Move; }
	const char *GetName() const { return "clc_Move"; }
	void *m_pMessageHandler{ nullptr };
	int GetGroup() const { return INetChannelInfo::MOVE; }
	CLC_Move() { m_bReliable = false; }

public:
	int m_nBackupCommands{};
	int m_nNewCommands{};
	int m_nLength{};
	bf_read m_DataIn{};
	bf_write m_DataOut{};
};

#define NET_TICK_SCALEUP 100000.0f

class NET_Tick : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read &buffer);
	bool WriteToBuffer(bf_write &buffer);
	const char *ToString() const;
	int GetType() const { return net_Tick; }
	const char *GetName() const { return "net_Tick"; }
	void *m_pMessageHandler{ nullptr };

public:
	NET_Tick()
	{
		m_bReliable = false;
		m_flHostFrameTime = 0;
		m_flHostFrameTimeStdDeviation = 0;
	};

	NET_Tick(int tick, float hostFrametime, float hostFrametime_stddeviation)
	{
		m_bReliable = false;
		m_nTick = tick;
		m_flHostFrameTime = hostFrametime;
		m_flHostFrameTimeStdDeviation = hostFrametime_stddeviation;
	};

public:
	int m_nTick{};
	float m_flHostFrameTime{};
	float m_flHostFrameTimeStdDeviation{};
};

class NET_SetConVar : public CNetMessage
{
public:
	bool ReadFromBuffer(bf_read &buffer);
	bool WriteToBuffer(bf_write &buffer);
	const char *ToString() const;
	int GetType() const { return net_SetConVar; }
	const char *GetName() const { return "net_SetConVar"; }
	void *m_pMessageHandler{ nullptr };
	int GetGroup() const { return INetChannelInfo::STRINGCMD; }

	NET_SetConVar() {}
	NET_SetConVar(const char *name, const char *value)
	{
		strncpy_s(m_ConVars.name, name, sizeof(m_ConVars.name));
		strncpy_s(m_ConVars.value, value, sizeof(m_ConVars.value));
	}

public:
	typedef struct cvar_s
	{
		char name[MAX_OSPATH];
		char value[MAX_OSPATH];
	} cvar_t;
	cvar_t m_ConVars{};
};

inline bool NET_SetConVar::ReadFromBuffer(bf_read &buffer)
{
	const int num_vars = buffer.ReadByte();

	for (int i = 0; i < num_vars; i++)
	{
		cvar_t var{};
		buffer.ReadString(var.name, sizeof(var.name));
		buffer.ReadString(var.value, sizeof(var.value));
	}

	return !buffer.IsOverflowed();
}

inline bool NET_SetConVar::WriteToBuffer(bf_write &buffer)
{
	buffer.WriteUBitLong(GetType(), 6);
	buffer.WriteByte(1);
	buffer.WriteString(m_ConVars.name);
	buffer.WriteString(m_ConVars.value);

	return !buffer.IsOverflowed();
}

inline const char *NET_SetConVar::ToString() const
{
	const std::string result{ std::format("{}: \"{}\" = \"{}\"", GetName(), m_ConVars.name, m_ConVars.value) };

#pragma warning (push)
#pragma warning (disable : 26816)
	return result.c_str();
#pragma warning (pop)
}

inline bool CLC_Move::WriteToBuffer(bf_write &buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong(m_nNewCommands, NUM_NEW_COMMAND_BITS);
	buffer.WriteUBitLong(m_nBackupCommands, NUM_BACKUP_COMMAND_BITS);

	buffer.WriteWord(m_nLength);

	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

inline bool CLC_Move::ReadFromBuffer(bf_read &buffer)
{
	m_nNewCommands = buffer.ReadUBitLong(NUM_NEW_COMMAND_BITS);
	m_nBackupCommands = buffer.ReadUBitLong(NUM_BACKUP_COMMAND_BITS);
	m_nLength = buffer.ReadWord();
	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

inline bool NET_Tick::WriteToBuffer(bf_write &buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteLong(m_nTick);
	buffer.WriteUBitLong(std::clamp((int)(NET_TICK_SCALEUP * m_flHostFrameTime), 0, 65535), 16);
	buffer.WriteUBitLong(std::clamp((int)(NET_TICK_SCALEUP * m_flHostFrameTimeStdDeviation), 0, 65535), 16);
	return !buffer.IsOverflowed();
}

inline bool NET_Tick::ReadFromBuffer(bf_read &buffer)
{
	m_nTick = buffer.ReadLong();
	m_flHostFrameTime = (float)buffer.ReadUBitLong(16) / NET_TICK_SCALEUP;
	m_flHostFrameTimeStdDeviation = (float)buffer.ReadUBitLong(16) / NET_TICK_SCALEUP;
	return !buffer.IsOverflowed();
}

inline const char *NET_Tick::ToString(void) const
{
	const std::string result = std::format("{}: tick {}", GetName(), m_nTick);

#pragma warning (push)
#pragma warning (disable : 26816)
	return result.c_str();
#pragma warning (pop)
}

struct CUtlString
{
	char *m_pString{};
};

struct MatchMakerHealthData_t
{
	float m_flRatio;
	Color m_colorBar;
	CUtlString m_strLocToken;
};

struct CClockDriftMgr
{
	float m_ClockOffsets[16]{};
	int m_iCurClockOffset{};
	int m_nServerTick{};
	int m_nClientTick{};
};

#define SIGNONSTATE_NONE 0
#define SIGNONSTATE_CHALLENGE 1
#define SIGNONSTATE_CONNECTED 2
#define SIGNONSTATE_NEW 3
#define SIGNONSTATE_PRESPAWN 4
#define SIGNONSTATE_SPAWN 5
#define SIGNONSTATE_FULL 6
#define SIGNONSTATE_CHANGELEVEL	7

enum RuneTypes_t
{
	RUNE_NONE = -1,
	RUNE_STRENGTH,
	RUNE_HASTE,
	RUNE_REGEN,
	RUNE_RESIST,
	RUNE_VAMPIRE,
	RUNE_REFLECT,
	RUNE_PRECISION,
	RUNE_AGILITY,
	RUNE_KNOCKOUT,
	RUNE_KING,
	RUNE_PLAGUE,
	RUNE_SUPERNOVA,
	RUNE_TYPES_MAX
};

class CStudioHdr
{
public:
	mutable const studiohdr_t *m_pStudioHdr;
};

enum Activity
{
	ACT_INVALID = -1,
	ACT_RESET = 0,
	ACT_IDLE,
	ACT_TRANSITION,
	ACT_COVER,
	ACT_COVER_MED,
	ACT_COVER_LOW,
	ACT_WALK,
	ACT_WALK_AIM,
	ACT_WALK_CROUCH,
	ACT_WALK_CROUCH_AIM,
	ACT_RUN,
	ACT_RUN_AIM,
	ACT_RUN_CROUCH,
	ACT_RUN_CROUCH_AIM,
	ACT_RUN_PROTECTED,
	ACT_SCRIPT_CUSTOM_MOVE,
	ACT_RANGE_ATTACK1,
	ACT_RANGE_ATTACK2,
	ACT_RANGE_ATTACK1_LOW,
	ACT_RANGE_ATTACK2_LOW,
	ACT_DIESIMPLE,
	ACT_DIEBACKWARD,
	ACT_DIEFORWARD,
	ACT_DIEVIOLENT,
	ACT_DIERAGDOLL,
	ACT_FLY,
	ACT_HOVER,
	ACT_GLIDE,
	ACT_SWIM,
	ACT_JUMP,
	ACT_HOP,
	ACT_LEAP,
	ACT_LAND,
	ACT_CLIMB_UP,
	ACT_CLIMB_DOWN,
	ACT_CLIMB_DISMOUNT,
	ACT_SHIPLADDER_UP,
	ACT_SHIPLADDER_DOWN,
	ACT_STRAFE_LEFT,
	ACT_STRAFE_RIGHT,
	ACT_ROLL_LEFT,
	ACT_ROLL_RIGHT,
	ACT_TURN_LEFT,
	ACT_TURN_RIGHT,
	ACT_CROUCH,
	ACT_CROUCHIDLE,
	ACT_STAND,
	ACT_USE,
	ACT_SIGNAL1,
	ACT_SIGNAL2,
	ACT_SIGNAL3,
	ACT_SIGNAL_ADVANCE,
	ACT_SIGNAL_FORWARD,
	ACT_SIGNAL_GROUP,
	ACT_SIGNAL_HALT,
	ACT_SIGNAL_LEFT,
	ACT_SIGNAL_RIGHT,
	ACT_SIGNAL_TAKECOVER,
	ACT_LOOKBACK_RIGHT,
	ACT_LOOKBACK_LEFT,
	ACT_COWER,
	ACT_SMALL_FLINCH,
	ACT_BIG_FLINCH,
	ACT_MELEE_ATTACK1,
	ACT_MELEE_ATTACK2,
	ACT_RELOAD,
	ACT_RELOAD_START,
	ACT_RELOAD_FINISH,
	ACT_RELOAD_LOW,
	ACT_ARM,
	ACT_DISARM,
	ACT_DROP_WEAPON,
	ACT_DROP_WEAPON_SHOTGUN,
	ACT_PICKUP_GROUND,
	ACT_PICKUP_RACK,
	ACT_IDLE_ANGRY,
	ACT_IDLE_RELAXED,
	ACT_IDLE_STIMULATED,
	ACT_IDLE_AGITATED,
	ACT_IDLE_STEALTH,
	ACT_IDLE_HURT,
	ACT_WALK_RELAXED,
	ACT_WALK_STIMULATED,
	ACT_WALK_AGITATED,
	ACT_WALK_STEALTH,
	ACT_RUN_RELAXED,
	ACT_RUN_STIMULATED,
	ACT_RUN_AGITATED,
	ACT_RUN_STEALTH,
	ACT_IDLE_AIM_RELAXED,
	ACT_IDLE_AIM_STIMULATED,
	ACT_IDLE_AIM_AGITATED,
	ACT_IDLE_AIM_STEALTH,
	ACT_WALK_AIM_RELAXED,
	ACT_WALK_AIM_STIMULATED,
	ACT_WALK_AIM_AGITATED,
	ACT_WALK_AIM_STEALTH,
	ACT_RUN_AIM_RELAXED,
	ACT_RUN_AIM_STIMULATED,
	ACT_RUN_AIM_AGITATED,
	ACT_RUN_AIM_STEALTH,
	ACT_CROUCHIDLE_STIMULATED,
	ACT_CROUCHIDLE_AIM_STIMULATED,
	ACT_CROUCHIDLE_AGITATED,
	ACT_WALK_HURT,
	ACT_RUN_HURT,
	ACT_SPECIAL_ATTACK1,
	ACT_SPECIAL_ATTACK2,
	ACT_COMBAT_IDLE,
	ACT_WALK_SCARED,
	ACT_RUN_SCARED,
	ACT_VICTORY_DANCE,
	ACT_DIE_HEADSHOT,
	ACT_DIE_CHESTSHOT,
	ACT_DIE_GUTSHOT,
	ACT_DIE_BACKSHOT,
	ACT_FLINCH_HEAD,
	ACT_FLINCH_CHEST,
	ACT_FLINCH_STOMACH,
	ACT_FLINCH_LEFTARM,
	ACT_FLINCH_RIGHTARM,
	ACT_FLINCH_LEFTLEG,
	ACT_FLINCH_RIGHTLEG,
	ACT_FLINCH_PHYSICS,
	ACT_IDLE_ON_FIRE,
	ACT_WALK_ON_FIRE,
	ACT_RUN_ON_FIRE,
	ACT_RAPPEL_LOOP,
	ACT_180_LEFT,
	ACT_180_RIGHT,
	ACT_90_LEFT,
	ACT_90_RIGHT,
	ACT_STEP_LEFT,
	ACT_STEP_RIGHT,
	ACT_STEP_BACK,
	ACT_STEP_FORE,
	ACT_GESTURE_RANGE_ATTACK1,
	ACT_GESTURE_RANGE_ATTACK2,
	ACT_GESTURE_MELEE_ATTACK1,
	ACT_GESTURE_MELEE_ATTACK2,
	ACT_GESTURE_RANGE_ATTACK1_LOW,
	ACT_GESTURE_RANGE_ATTACK2_LOW,
	ACT_MELEE_ATTACK_SWING_GESTURE,
	ACT_GESTURE_SMALL_FLINCH,
	ACT_GESTURE_BIG_FLINCH,
	ACT_GESTURE_FLINCH_BLAST,
	ACT_GESTURE_FLINCH_BLAST_SHOTGUN,
	ACT_GESTURE_FLINCH_BLAST_DAMAGED,
	ACT_GESTURE_FLINCH_BLAST_DAMAGED_SHOTGUN,
	ACT_GESTURE_FLINCH_HEAD,
	ACT_GESTURE_FLINCH_CHEST,
	ACT_GESTURE_FLINCH_STOMACH,
	ACT_GESTURE_FLINCH_LEFTARM,
	ACT_GESTURE_FLINCH_RIGHTARM,
	ACT_GESTURE_FLINCH_LEFTLEG,
	ACT_GESTURE_FLINCH_RIGHTLEG,
	ACT_GESTURE_TURN_LEFT,
	ACT_GESTURE_TURN_RIGHT,
	ACT_GESTURE_TURN_LEFT45,
	ACT_GESTURE_TURN_RIGHT45,
	ACT_GESTURE_TURN_LEFT90,
	ACT_GESTURE_TURN_RIGHT90,
	ACT_GESTURE_TURN_LEFT45_FLAT,
	ACT_GESTURE_TURN_RIGHT45_FLAT,
	ACT_GESTURE_TURN_LEFT90_FLAT,
	ACT_GESTURE_TURN_RIGHT90_FLAT,
	ACT_BARNACLE_HIT,
	ACT_BARNACLE_PULL,
	ACT_BARNACLE_CHOMP,
	ACT_BARNACLE_CHEW,
	ACT_DO_NOT_DISTURB,
	ACT_VM_DRAW,
	ACT_VM_HOLSTER,
	ACT_VM_IDLE,
	ACT_VM_FIDGET,
	ACT_VM_PULLBACK,
	ACT_VM_PULLBACK_HIGH,
	ACT_VM_PULLBACK_LOW,
	ACT_VM_THROW,
	ACT_VM_PULLPIN,
	ACT_VM_PRIMARYATTACK,
	ACT_VM_SECONDARYATTACK,
	ACT_VM_RELOAD,
	ACT_VM_RELOAD_START,
	ACT_VM_RELOAD_FINISH,
	ACT_VM_DRYFIRE,
	ACT_VM_HITLEFT,
	ACT_VM_HITLEFT2,
	ACT_VM_HITRIGHT,
	ACT_VM_HITRIGHT2,
	ACT_VM_HITCENTER,
	ACT_VM_HITCENTER2,
	ACT_VM_MISSLEFT,
	ACT_VM_MISSLEFT2,
	ACT_VM_MISSRIGHT,
	ACT_VM_MISSRIGHT2,
	ACT_VM_MISSCENTER,
	ACT_VM_MISSCENTER2,
	ACT_VM_HAULBACK,
	ACT_VM_SWINGHARD,
	ACT_VM_SWINGMISS,
	ACT_VM_SWINGHIT,
	ACT_VM_IDLE_TO_LOWERED,
	ACT_VM_IDLE_LOWERED,
	ACT_VM_LOWERED_TO_IDLE,
	ACT_VM_RECOIL1,
	ACT_VM_RECOIL2,
	ACT_VM_RECOIL3,
	ACT_VM_PICKUP,
	ACT_VM_RELEASE,
	ACT_VM_ATTACH_SILENCER,
	ACT_VM_DETACH_SILENCER,
	ACT_VM_DRAW_SPECIAL,
	ACT_VM_HOLSTER_SPECIAL,
	ACT_VM_IDLE_SPECIAL,
	ACT_VM_PULLBACK_SPECIAL,
	ACT_VM_PRIMARYATTACK_SPECIAL,
	ACT_VM_SECONDARYATTACK_SPECIAL,
	ACT_VM_HITCENTER_SPECIAL,
	ACT_VM_SWINGHARD_SPECIAL,
	ACT_VM_IDLE_TO_LOWERED_SPECIAL,
	ACT_VM_IDLE_LOWERED_SPECIAL,
	ACT_VM_LOWERED_TO_IDLE_SPECIAL,
	ACT_FISTS_VM_HITLEFT,
	ACT_FISTS_VM_HITRIGHT,
	ACT_FISTS_VM_SWINGHARD,
	ACT_FISTS_VM_IDLE,
	ACT_FISTS_VM_DRAW,
	ACT_SLAM_STICKWALL_IDLE,
	ACT_SLAM_STICKWALL_ND_IDLE,
	ACT_SLAM_STICKWALL_ATTACH,
	ACT_SLAM_STICKWALL_ATTACH2,
	ACT_SLAM_STICKWALL_ND_ATTACH,
	ACT_SLAM_STICKWALL_ND_ATTACH2,
	ACT_SLAM_STICKWALL_DETONATE,
	ACT_SLAM_STICKWALL_DETONATOR_HOLSTER,
	ACT_SLAM_STICKWALL_DRAW,
	ACT_SLAM_STICKWALL_ND_DRAW,
	ACT_SLAM_STICKWALL_TO_THROW,
	ACT_SLAM_STICKWALL_TO_THROW_ND,
	ACT_SLAM_STICKWALL_TO_TRIPMINE_ND,
	ACT_SLAM_THROW_IDLE,
	ACT_SLAM_THROW_ND_IDLE,
	ACT_SLAM_THROW_THROW,
	ACT_SLAM_THROW_THROW2,
	ACT_SLAM_THROW_THROW_ND,
	ACT_SLAM_THROW_THROW_ND2,
	ACT_SLAM_THROW_DRAW,
	ACT_SLAM_THROW_ND_DRAW,
	ACT_SLAM_THROW_TO_STICKWALL,
	ACT_SLAM_THROW_TO_STICKWALL_ND,
	ACT_SLAM_THROW_DETONATE,
	ACT_SLAM_THROW_DETONATOR_HOLSTER,
	ACT_SLAM_THROW_TO_TRIPMINE_ND,
	ACT_SLAM_TRIPMINE_IDLE,
	ACT_SLAM_TRIPMINE_DRAW,
	ACT_SLAM_TRIPMINE_ATTACH,
	ACT_SLAM_TRIPMINE_ATTACH2,
	ACT_SLAM_TRIPMINE_TO_STICKWALL_ND,
	ACT_SLAM_TRIPMINE_TO_THROW_ND,
	ACT_SLAM_DETONATOR_IDLE,
	ACT_SLAM_DETONATOR_DRAW,
	ACT_SLAM_DETONATOR_DETONATE,
	ACT_SLAM_DETONATOR_HOLSTER,
	ACT_SLAM_DETONATOR_STICKWALL_DRAW,
	ACT_SLAM_DETONATOR_THROW_DRAW,
	ACT_SHOTGUN_RELOAD_START,
	ACT_SHOTGUN_RELOAD_FINISH,
	ACT_SHOTGUN_PUMP,
	ACT_SMG2_IDLE2,
	ACT_SMG2_FIRE2,
	ACT_SMG2_DRAW2,
	ACT_SMG2_RELOAD2,
	ACT_SMG2_DRYFIRE2,
	ACT_SMG2_TOAUTO,
	ACT_SMG2_TOBURST,
	ACT_PHYSCANNON_UPGRADE,
	ACT_RANGE_ATTACK_AR1,
	ACT_RANGE_ATTACK_AR2,
	ACT_RANGE_ATTACK_AR2_LOW,
	ACT_RANGE_ATTACK_AR2_GRENADE,
	ACT_RANGE_ATTACK_HMG1,
	ACT_RANGE_ATTACK_ML,
	ACT_RANGE_ATTACK_SMG1,
	ACT_RANGE_ATTACK_SMG1_LOW,
	ACT_RANGE_ATTACK_SMG2,
	ACT_RANGE_ATTACK_SHOTGUN,
	ACT_RANGE_ATTACK_SHOTGUN_LOW,
	ACT_RANGE_ATTACK_PISTOL,
	ACT_RANGE_ATTACK_PISTOL_LOW,
	ACT_RANGE_ATTACK_SLAM,
	ACT_RANGE_ATTACK_TRIPWIRE,
	ACT_RANGE_ATTACK_THROW,
	ACT_RANGE_ATTACK_SNIPER_RIFLE,
	ACT_RANGE_ATTACK_RPG,
	ACT_MELEE_ATTACK_SWING,
	ACT_RANGE_AIM_LOW,
	ACT_RANGE_AIM_SMG1_LOW,
	ACT_RANGE_AIM_PISTOL_LOW,
	ACT_RANGE_AIM_AR2_LOW,
	ACT_COVER_PISTOL_LOW,
	ACT_COVER_SMG1_LOW,
	ACT_GESTURE_RANGE_ATTACK_AR1,
	ACT_GESTURE_RANGE_ATTACK_AR2,
	ACT_GESTURE_RANGE_ATTACK_AR2_GRENADE,
	ACT_GESTURE_RANGE_ATTACK_HMG1,
	ACT_GESTURE_RANGE_ATTACK_ML,
	ACT_GESTURE_RANGE_ATTACK_SMG1,
	ACT_GESTURE_RANGE_ATTACK_SMG1_LOW,
	ACT_GESTURE_RANGE_ATTACK_SMG2,
	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,
	ACT_GESTURE_RANGE_ATTACK_PISTOL,
	ACT_GESTURE_RANGE_ATTACK_PISTOL_LOW,
	ACT_GESTURE_RANGE_ATTACK_SLAM,
	ACT_GESTURE_RANGE_ATTACK_TRIPWIRE,
	ACT_GESTURE_RANGE_ATTACK_THROW,
	ACT_GESTURE_RANGE_ATTACK_SNIPER_RIFLE,
	ACT_GESTURE_MELEE_ATTACK_SWING,
	ACT_IDLE_RIFLE,
	ACT_IDLE_SMG1,
	ACT_IDLE_ANGRY_SMG1,
	ACT_IDLE_PISTOL,
	ACT_IDLE_ANGRY_PISTOL,
	ACT_IDLE_ANGRY_SHOTGUN,
	ACT_IDLE_STEALTH_PISTOL,
	ACT_IDLE_PACKAGE,
	ACT_WALK_PACKAGE,
	ACT_IDLE_SUITCASE,
	ACT_WALK_SUITCASE,
	ACT_IDLE_SMG1_RELAXED,
	ACT_IDLE_SMG1_STIMULATED,
	ACT_WALK_RIFLE_RELAXED,
	ACT_RUN_RIFLE_RELAXED,
	ACT_WALK_RIFLE_STIMULATED,
	ACT_RUN_RIFLE_STIMULATED,
	ACT_IDLE_AIM_RIFLE_STIMULATED,
	ACT_WALK_AIM_RIFLE_STIMULATED,
	ACT_RUN_AIM_RIFLE_STIMULATED,
	ACT_IDLE_SHOTGUN_RELAXED,
	ACT_IDLE_SHOTGUN_STIMULATED,
	ACT_IDLE_SHOTGUN_AGITATED,
	ACT_WALK_ANGRY,
	ACT_POLICE_HARASS1,
	ACT_POLICE_HARASS2,
	ACT_IDLE_MANNEDGUN,
	ACT_IDLE_MELEE,
	ACT_IDLE_ANGRY_MELEE,
	ACT_IDLE_RPG_RELAXED,
	ACT_IDLE_RPG,
	ACT_IDLE_ANGRY_RPG,
	ACT_COVER_LOW_RPG,
	ACT_WALK_RPG,
	ACT_RUN_RPG,
	ACT_WALK_CROUCH_RPG,
	ACT_RUN_CROUCH_RPG,
	ACT_WALK_RPG_RELAXED,
	ACT_RUN_RPG_RELAXED,
	ACT_WALK_RIFLE,
	ACT_WALK_AIM_RIFLE,
	ACT_WALK_CROUCH_RIFLE,
	ACT_WALK_CROUCH_AIM_RIFLE,
	ACT_RUN_RIFLE,
	ACT_RUN_AIM_RIFLE,
	ACT_RUN_CROUCH_RIFLE,
	ACT_RUN_CROUCH_AIM_RIFLE,
	ACT_RUN_STEALTH_PISTOL,
	ACT_WALK_AIM_SHOTGUN,
	ACT_RUN_AIM_SHOTGUN,
	ACT_WALK_PISTOL,
	ACT_RUN_PISTOL,
	ACT_WALK_AIM_PISTOL,
	ACT_RUN_AIM_PISTOL,
	ACT_WALK_STEALTH_PISTOL,
	ACT_WALK_AIM_STEALTH_PISTOL,
	ACT_RUN_AIM_STEALTH_PISTOL,
	ACT_RELOAD_PISTOL,
	ACT_RELOAD_PISTOL_LOW,
	ACT_RELOAD_SMG1,
	ACT_RELOAD_SMG1_LOW,
	ACT_RELOAD_SHOTGUN,
	ACT_RELOAD_SHOTGUN_LOW,
	ACT_GESTURE_RELOAD,
	ACT_GESTURE_RELOAD_PISTOL,
	ACT_GESTURE_RELOAD_SMG1,
	ACT_GESTURE_RELOAD_SHOTGUN,
	ACT_BUSY_LEAN_LEFT,
	ACT_BUSY_LEAN_LEFT_ENTRY,
	ACT_BUSY_LEAN_LEFT_EXIT,
	ACT_BUSY_LEAN_BACK,
	ACT_BUSY_LEAN_BACK_ENTRY,
	ACT_BUSY_LEAN_BACK_EXIT,
	ACT_BUSY_SIT_GROUND,
	ACT_BUSY_SIT_GROUND_ENTRY,
	ACT_BUSY_SIT_GROUND_EXIT,
	ACT_BUSY_SIT_CHAIR,
	ACT_BUSY_SIT_CHAIR_ENTRY,
	ACT_BUSY_SIT_CHAIR_EXIT,
	ACT_BUSY_STAND,
	ACT_BUSY_QUEUE,
	ACT_DUCK_DODGE,
	ACT_DIE_BARNACLE_SWALLOW,
	ACT_GESTURE_BARNACLE_STRANGLE,
	ACT_PHYSCANNON_DETACH,
	ACT_PHYSCANNON_ANIMATE,
	ACT_PHYSCANNON_ANIMATE_PRE,
	ACT_PHYSCANNON_ANIMATE_POST,
	ACT_DIE_FRONTSIDE,
	ACT_DIE_RIGHTSIDE,
	ACT_DIE_BACKSIDE,
	ACT_DIE_LEFTSIDE,
	ACT_OPEN_DOOR,
	ACT_DI_ALYX_ZOMBIE_MELEE,
	ACT_DI_ALYX_ZOMBIE_TORSO_MELEE,
	ACT_DI_ALYX_HEADCRAB_MELEE,
	ACT_DI_ALYX_ANTLION,
	ACT_DI_ALYX_ZOMBIE_SHOTGUN64,
	ACT_DI_ALYX_ZOMBIE_SHOTGUN26,
	ACT_READINESS_RELAXED_TO_STIMULATED,
	ACT_READINESS_RELAXED_TO_STIMULATED_WALK,
	ACT_READINESS_AGITATED_TO_STIMULATED,
	ACT_READINESS_STIMULATED_TO_RELAXED,
	ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED,
	ACT_READINESS_PISTOL_RELAXED_TO_STIMULATED_WALK,
	ACT_READINESS_PISTOL_AGITATED_TO_STIMULATED,
	ACT_READINESS_PISTOL_STIMULATED_TO_RELAXED,
	ACT_IDLE_CARRY,
	ACT_WALK_CARRY,
	ACT_STARTDYING,
	ACT_DYINGLOOP,
	ACT_DYINGTODEAD,
	ACT_RIDE_MANNED_GUN,
	ACT_VM_SPRINT_ENTER,
	ACT_VM_SPRINT_IDLE,
	ACT_VM_SPRINT_LEAVE,
	ACT_FIRE_START,
	ACT_FIRE_LOOP,
	ACT_FIRE_END,
	ACT_CROUCHING_GRENADEIDLE,
	ACT_CROUCHING_GRENADEREADY,
	ACT_CROUCHING_PRIMARYATTACK,
	ACT_OVERLAY_GRENADEIDLE,
	ACT_OVERLAY_GRENADEREADY,
	ACT_OVERLAY_PRIMARYATTACK,
	ACT_OVERLAY_SHIELD_UP,
	ACT_OVERLAY_SHIELD_DOWN,
	ACT_OVERLAY_SHIELD_UP_IDLE,
	ACT_OVERLAY_SHIELD_ATTACK,
	ACT_OVERLAY_SHIELD_KNOCKBACK,
	ACT_SHIELD_UP,
	ACT_SHIELD_DOWN,
	ACT_SHIELD_UP_IDLE,
	ACT_SHIELD_ATTACK,
	ACT_SHIELD_KNOCKBACK,
	ACT_CROUCHING_SHIELD_UP,
	ACT_CROUCHING_SHIELD_DOWN,
	ACT_CROUCHING_SHIELD_UP_IDLE,
	ACT_CROUCHING_SHIELD_ATTACK,
	ACT_CROUCHING_SHIELD_KNOCKBACK,
	ACT_TURNRIGHT45,
	ACT_TURNLEFT45,
	ACT_TURN,
	ACT_OBJ_ASSEMBLING,
	ACT_OBJ_DISMANTLING,
	ACT_OBJ_STARTUP,
	ACT_OBJ_RUNNING,
	ACT_OBJ_IDLE,
	ACT_OBJ_PLACING,
	ACT_OBJ_DETERIORATING,
	ACT_OBJ_UPGRADING,
	ACT_DEPLOY,
	ACT_DEPLOY_IDLE,
	ACT_UNDEPLOY,
	ACT_GRENADE_ROLL,
	ACT_GRENADE_TOSS,
	ACT_HANDGRENADE_THROW1,
	ACT_HANDGRENADE_THROW2,
	ACT_HANDGRENADE_THROW3,
	ACT_SHOTGUN_IDLE_DEEP,
	ACT_SHOTGUN_IDLE4,
	ACT_GLOCK_SHOOTEMPTY,
	ACT_GLOCK_SHOOT_RELOAD,
	ACT_RPG_DRAW_UNLOADED,
	ACT_RPG_HOLSTER_UNLOADED,
	ACT_RPG_IDLE_UNLOADED,
	ACT_RPG_FIDGET_UNLOADED,
	ACT_CROSSBOW_DRAW_UNLOADED,
	ACT_CROSSBOW_IDLE_UNLOADED,
	ACT_CROSSBOW_FIDGET_UNLOADED,
	ACT_GAUSS_SPINUP,
	ACT_GAUSS_SPINCYCLE,
	ACT_TRIPMINE_GROUND,
	ACT_TRIPMINE_WORLD,
	ACT_VM_PRIMARYATTACK_SILENCED,
	ACT_VM_RELOAD_SILENCED,
	ACT_VM_DRYFIRE_SILENCED,
	ACT_VM_IDLE_SILENCED,
	ACT_VM_DRAW_SILENCED,
	ACT_VM_IDLE_EMPTY_LEFT,
	ACT_VM_DRYFIRE_LEFT,
	ACT_PLAYER_IDLE_FIRE,
	ACT_PLAYER_CROUCH_FIRE,
	ACT_PLAYER_CROUCH_WALK_FIRE,
	ACT_PLAYER_WALK_FIRE,
	ACT_PLAYER_RUN_FIRE,
	ACT_IDLETORUN,
	ACT_RUNTOIDLE,
	ACT_SPRINT,
	ACT_GET_DOWN_STAND,
	ACT_GET_UP_STAND,
	ACT_GET_DOWN_CROUCH,
	ACT_GET_UP_CROUCH,
	ACT_PRONE_FORWARD,
	ACT_PRONE_IDLE,
	ACT_DEEPIDLE1,
	ACT_DEEPIDLE2,
	ACT_DEEPIDLE3,
	ACT_DEEPIDLE4,
	ACT_VM_RELOAD_DEPLOYED,
	ACT_VM_RELOAD_IDLE,
	ACT_VM_DRAW_DEPLOYED,
	ACT_VM_DRAW_EMPTY,
	ACT_VM_PRIMARYATTACK_EMPTY,
	ACT_VM_RELOAD_EMPTY,
	ACT_VM_IDLE_EMPTY,
	ACT_VM_IDLE_DEPLOYED_EMPTY,
	ACT_VM_IDLE_8,
	ACT_VM_IDLE_7,
	ACT_VM_IDLE_6,
	ACT_VM_IDLE_5,
	ACT_VM_IDLE_4,
	ACT_VM_IDLE_3,
	ACT_VM_IDLE_2,
	ACT_VM_IDLE_1,
	ACT_VM_IDLE_DEPLOYED,
	ACT_VM_IDLE_DEPLOYED_8,
	ACT_VM_IDLE_DEPLOYED_7,
	ACT_VM_IDLE_DEPLOYED_6,
	ACT_VM_IDLE_DEPLOYED_5,
	ACT_VM_IDLE_DEPLOYED_4,
	ACT_VM_IDLE_DEPLOYED_3,
	ACT_VM_IDLE_DEPLOYED_2,
	ACT_VM_IDLE_DEPLOYED_1,
	ACT_VM_UNDEPLOY,
	ACT_VM_UNDEPLOY_8,
	ACT_VM_UNDEPLOY_7,
	ACT_VM_UNDEPLOY_6,
	ACT_VM_UNDEPLOY_5,
	ACT_VM_UNDEPLOY_4,
	ACT_VM_UNDEPLOY_3,
	ACT_VM_UNDEPLOY_2,
	ACT_VM_UNDEPLOY_1,
	ACT_VM_UNDEPLOY_EMPTY,
	ACT_VM_DEPLOY,
	ACT_VM_DEPLOY_8,
	ACT_VM_DEPLOY_7,
	ACT_VM_DEPLOY_6,
	ACT_VM_DEPLOY_5,
	ACT_VM_DEPLOY_4,
	ACT_VM_DEPLOY_3,
	ACT_VM_DEPLOY_2,
	ACT_VM_DEPLOY_1,
	ACT_VM_DEPLOY_EMPTY,
	ACT_VM_PRIMARYATTACK_8,
	ACT_VM_PRIMARYATTACK_7,
	ACT_VM_PRIMARYATTACK_6,
	ACT_VM_PRIMARYATTACK_5,
	ACT_VM_PRIMARYATTACK_4,
	ACT_VM_PRIMARYATTACK_3,
	ACT_VM_PRIMARYATTACK_2,
	ACT_VM_PRIMARYATTACK_1,
	ACT_VM_PRIMARYATTACK_DEPLOYED,
	ACT_VM_PRIMARYATTACK_DEPLOYED_8,
	ACT_VM_PRIMARYATTACK_DEPLOYED_7,
	ACT_VM_PRIMARYATTACK_DEPLOYED_6,
	ACT_VM_PRIMARYATTACK_DEPLOYED_5,
	ACT_VM_PRIMARYATTACK_DEPLOYED_4,
	ACT_VM_PRIMARYATTACK_DEPLOYED_3,
	ACT_VM_PRIMARYATTACK_DEPLOYED_2,
	ACT_VM_PRIMARYATTACK_DEPLOYED_1,
	ACT_VM_PRIMARYATTACK_DEPLOYED_EMPTY,
	ACT_DOD_DEPLOYED,
	ACT_DOD_PRONE_DEPLOYED,
	ACT_DOD_IDLE_ZOOMED,
	ACT_DOD_WALK_ZOOMED,
	ACT_DOD_CROUCH_ZOOMED,
	ACT_DOD_CROUCHWALK_ZOOMED,
	ACT_DOD_PRONE_ZOOMED,
	ACT_DOD_PRONE_FORWARD_ZOOMED,
	ACT_DOD_PRIMARYATTACK_DEPLOYED,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED,
	ACT_DOD_RELOAD_DEPLOYED,
	ACT_DOD_RELOAD_PRONE_DEPLOYED,
	ACT_DOD_PRIMARYATTACK_PRONE,
	ACT_DOD_SECONDARYATTACK_PRONE,
	ACT_DOD_RELOAD_CROUCH,
	ACT_DOD_RELOAD_PRONE,
	ACT_DOD_STAND_IDLE,
	ACT_DOD_STAND_AIM,
	ACT_DOD_CROUCH_IDLE,
	ACT_DOD_CROUCH_AIM,
	ACT_DOD_CROUCHWALK_IDLE,
	ACT_DOD_CROUCHWALK_AIM,
	ACT_DOD_WALK_IDLE,
	ACT_DOD_WALK_AIM,
	ACT_DOD_RUN_IDLE,
	ACT_DOD_RUN_AIM,
	ACT_DOD_STAND_AIM_PISTOL,
	ACT_DOD_CROUCH_AIM_PISTOL,
	ACT_DOD_CROUCHWALK_AIM_PISTOL,
	ACT_DOD_WALK_AIM_PISTOL,
	ACT_DOD_RUN_AIM_PISTOL,
	ACT_DOD_PRONE_AIM_PISTOL,
	ACT_DOD_STAND_IDLE_PISTOL,
	ACT_DOD_CROUCH_IDLE_PISTOL,
	ACT_DOD_CROUCHWALK_IDLE_PISTOL,
	ACT_DOD_WALK_IDLE_PISTOL,
	ACT_DOD_RUN_IDLE_PISTOL,
	ACT_DOD_SPRINT_IDLE_PISTOL,
	ACT_DOD_PRONEWALK_IDLE_PISTOL,
	ACT_DOD_STAND_AIM_C96,
	ACT_DOD_CROUCH_AIM_C96,
	ACT_DOD_CROUCHWALK_AIM_C96,
	ACT_DOD_WALK_AIM_C96,
	ACT_DOD_RUN_AIM_C96,
	ACT_DOD_PRONE_AIM_C96,
	ACT_DOD_STAND_IDLE_C96,
	ACT_DOD_CROUCH_IDLE_C96,
	ACT_DOD_CROUCHWALK_IDLE_C96,
	ACT_DOD_WALK_IDLE_C96,
	ACT_DOD_RUN_IDLE_C96,
	ACT_DOD_SPRINT_IDLE_C96,
	ACT_DOD_PRONEWALK_IDLE_C96,
	ACT_DOD_STAND_AIM_RIFLE,
	ACT_DOD_CROUCH_AIM_RIFLE,
	ACT_DOD_CROUCHWALK_AIM_RIFLE,
	ACT_DOD_WALK_AIM_RIFLE,
	ACT_DOD_RUN_AIM_RIFLE,
	ACT_DOD_PRONE_AIM_RIFLE,
	ACT_DOD_STAND_IDLE_RIFLE,
	ACT_DOD_CROUCH_IDLE_RIFLE,
	ACT_DOD_CROUCHWALK_IDLE_RIFLE,
	ACT_DOD_WALK_IDLE_RIFLE,
	ACT_DOD_RUN_IDLE_RIFLE,
	ACT_DOD_SPRINT_IDLE_RIFLE,
	ACT_DOD_PRONEWALK_IDLE_RIFLE,
	ACT_DOD_STAND_AIM_BOLT,
	ACT_DOD_CROUCH_AIM_BOLT,
	ACT_DOD_CROUCHWALK_AIM_BOLT,
	ACT_DOD_WALK_AIM_BOLT,
	ACT_DOD_RUN_AIM_BOLT,
	ACT_DOD_PRONE_AIM_BOLT,
	ACT_DOD_STAND_IDLE_BOLT,
	ACT_DOD_CROUCH_IDLE_BOLT,
	ACT_DOD_CROUCHWALK_IDLE_BOLT,
	ACT_DOD_WALK_IDLE_BOLT,
	ACT_DOD_RUN_IDLE_BOLT,
	ACT_DOD_SPRINT_IDLE_BOLT,
	ACT_DOD_PRONEWALK_IDLE_BOLT,
	ACT_DOD_STAND_AIM_TOMMY,
	ACT_DOD_CROUCH_AIM_TOMMY,
	ACT_DOD_CROUCHWALK_AIM_TOMMY,
	ACT_DOD_WALK_AIM_TOMMY,
	ACT_DOD_RUN_AIM_TOMMY,
	ACT_DOD_PRONE_AIM_TOMMY,
	ACT_DOD_STAND_IDLE_TOMMY,
	ACT_DOD_CROUCH_IDLE_TOMMY,
	ACT_DOD_CROUCHWALK_IDLE_TOMMY,
	ACT_DOD_WALK_IDLE_TOMMY,
	ACT_DOD_RUN_IDLE_TOMMY,
	ACT_DOD_SPRINT_IDLE_TOMMY,
	ACT_DOD_PRONEWALK_IDLE_TOMMY,
	ACT_DOD_STAND_AIM_MP40,
	ACT_DOD_CROUCH_AIM_MP40,
	ACT_DOD_CROUCHWALK_AIM_MP40,
	ACT_DOD_WALK_AIM_MP40,
	ACT_DOD_RUN_AIM_MP40,
	ACT_DOD_PRONE_AIM_MP40,
	ACT_DOD_STAND_IDLE_MP40,
	ACT_DOD_CROUCH_IDLE_MP40,
	ACT_DOD_CROUCHWALK_IDLE_MP40,
	ACT_DOD_WALK_IDLE_MP40,
	ACT_DOD_RUN_IDLE_MP40,
	ACT_DOD_SPRINT_IDLE_MP40,
	ACT_DOD_PRONEWALK_IDLE_MP40,
	ACT_DOD_STAND_AIM_MP44,
	ACT_DOD_CROUCH_AIM_MP44,
	ACT_DOD_CROUCHWALK_AIM_MP44,
	ACT_DOD_WALK_AIM_MP44,
	ACT_DOD_RUN_AIM_MP44,
	ACT_DOD_PRONE_AIM_MP44,
	ACT_DOD_STAND_IDLE_MP44,
	ACT_DOD_CROUCH_IDLE_MP44,
	ACT_DOD_CROUCHWALK_IDLE_MP44,
	ACT_DOD_WALK_IDLE_MP44,
	ACT_DOD_RUN_IDLE_MP44,
	ACT_DOD_SPRINT_IDLE_MP44,
	ACT_DOD_PRONEWALK_IDLE_MP44,
	ACT_DOD_STAND_AIM_GREASE,
	ACT_DOD_CROUCH_AIM_GREASE,
	ACT_DOD_CROUCHWALK_AIM_GREASE,
	ACT_DOD_WALK_AIM_GREASE,
	ACT_DOD_RUN_AIM_GREASE,
	ACT_DOD_PRONE_AIM_GREASE,
	ACT_DOD_STAND_IDLE_GREASE,
	ACT_DOD_CROUCH_IDLE_GREASE,
	ACT_DOD_CROUCHWALK_IDLE_GREASE,
	ACT_DOD_WALK_IDLE_GREASE,
	ACT_DOD_RUN_IDLE_GREASE,
	ACT_DOD_SPRINT_IDLE_GREASE,
	ACT_DOD_PRONEWALK_IDLE_GREASE,
	ACT_DOD_STAND_AIM_MG,
	ACT_DOD_CROUCH_AIM_MG,
	ACT_DOD_CROUCHWALK_AIM_MG,
	ACT_DOD_WALK_AIM_MG,
	ACT_DOD_RUN_AIM_MG,
	ACT_DOD_PRONE_AIM_MG,
	ACT_DOD_STAND_IDLE_MG,
	ACT_DOD_CROUCH_IDLE_MG,
	ACT_DOD_CROUCHWALK_IDLE_MG,
	ACT_DOD_WALK_IDLE_MG,
	ACT_DOD_RUN_IDLE_MG,
	ACT_DOD_SPRINT_IDLE_MG,
	ACT_DOD_PRONEWALK_IDLE_MG,
	ACT_DOD_STAND_AIM_30CAL,
	ACT_DOD_CROUCH_AIM_30CAL,
	ACT_DOD_CROUCHWALK_AIM_30CAL,
	ACT_DOD_WALK_AIM_30CAL,
	ACT_DOD_RUN_AIM_30CAL,
	ACT_DOD_PRONE_AIM_30CAL,
	ACT_DOD_STAND_IDLE_30CAL,
	ACT_DOD_CROUCH_IDLE_30CAL,
	ACT_DOD_CROUCHWALK_IDLE_30CAL,
	ACT_DOD_WALK_IDLE_30CAL,
	ACT_DOD_RUN_IDLE_30CAL,
	ACT_DOD_SPRINT_IDLE_30CAL,
	ACT_DOD_PRONEWALK_IDLE_30CAL,
	ACT_DOD_STAND_AIM_GREN_FRAG,
	ACT_DOD_CROUCH_AIM_GREN_FRAG,
	ACT_DOD_CROUCHWALK_AIM_GREN_FRAG,
	ACT_DOD_WALK_AIM_GREN_FRAG,
	ACT_DOD_RUN_AIM_GREN_FRAG,
	ACT_DOD_PRONE_AIM_GREN_FRAG,
	ACT_DOD_SPRINT_AIM_GREN_FRAG,
	ACT_DOD_PRONEWALK_AIM_GREN_FRAG,
	ACT_DOD_STAND_AIM_GREN_STICK,
	ACT_DOD_CROUCH_AIM_GREN_STICK,
	ACT_DOD_CROUCHWALK_AIM_GREN_STICK,
	ACT_DOD_WALK_AIM_GREN_STICK,
	ACT_DOD_RUN_AIM_GREN_STICK,
	ACT_DOD_PRONE_AIM_GREN_STICK,
	ACT_DOD_SPRINT_AIM_GREN_STICK,
	ACT_DOD_PRONEWALK_AIM_GREN_STICK,
	ACT_DOD_STAND_AIM_KNIFE,
	ACT_DOD_CROUCH_AIM_KNIFE,
	ACT_DOD_CROUCHWALK_AIM_KNIFE,
	ACT_DOD_WALK_AIM_KNIFE,
	ACT_DOD_RUN_AIM_KNIFE,
	ACT_DOD_PRONE_AIM_KNIFE,
	ACT_DOD_SPRINT_AIM_KNIFE,
	ACT_DOD_PRONEWALK_AIM_KNIFE,
	ACT_DOD_STAND_AIM_SPADE,
	ACT_DOD_CROUCH_AIM_SPADE,
	ACT_DOD_CROUCHWALK_AIM_SPADE,
	ACT_DOD_WALK_AIM_SPADE,
	ACT_DOD_RUN_AIM_SPADE,
	ACT_DOD_PRONE_AIM_SPADE,
	ACT_DOD_SPRINT_AIM_SPADE,
	ACT_DOD_PRONEWALK_AIM_SPADE,
	ACT_DOD_STAND_AIM_BAZOOKA,
	ACT_DOD_CROUCH_AIM_BAZOOKA,
	ACT_DOD_CROUCHWALK_AIM_BAZOOKA,
	ACT_DOD_WALK_AIM_BAZOOKA,
	ACT_DOD_RUN_AIM_BAZOOKA,
	ACT_DOD_PRONE_AIM_BAZOOKA,
	ACT_DOD_STAND_IDLE_BAZOOKA,
	ACT_DOD_CROUCH_IDLE_BAZOOKA,
	ACT_DOD_CROUCHWALK_IDLE_BAZOOKA,
	ACT_DOD_WALK_IDLE_BAZOOKA,
	ACT_DOD_RUN_IDLE_BAZOOKA,
	ACT_DOD_SPRINT_IDLE_BAZOOKA,
	ACT_DOD_PRONEWALK_IDLE_BAZOOKA,
	ACT_DOD_STAND_AIM_PSCHRECK,
	ACT_DOD_CROUCH_AIM_PSCHRECK,
	ACT_DOD_CROUCHWALK_AIM_PSCHRECK,
	ACT_DOD_WALK_AIM_PSCHRECK,
	ACT_DOD_RUN_AIM_PSCHRECK,
	ACT_DOD_PRONE_AIM_PSCHRECK,
	ACT_DOD_STAND_IDLE_PSCHRECK,
	ACT_DOD_CROUCH_IDLE_PSCHRECK,
	ACT_DOD_CROUCHWALK_IDLE_PSCHRECK,
	ACT_DOD_WALK_IDLE_PSCHRECK,
	ACT_DOD_RUN_IDLE_PSCHRECK,
	ACT_DOD_SPRINT_IDLE_PSCHRECK,
	ACT_DOD_PRONEWALK_IDLE_PSCHRECK,
	ACT_DOD_STAND_AIM_BAR,
	ACT_DOD_CROUCH_AIM_BAR,
	ACT_DOD_CROUCHWALK_AIM_BAR,
	ACT_DOD_WALK_AIM_BAR,
	ACT_DOD_RUN_AIM_BAR,
	ACT_DOD_PRONE_AIM_BAR,
	ACT_DOD_STAND_IDLE_BAR,
	ACT_DOD_CROUCH_IDLE_BAR,
	ACT_DOD_CROUCHWALK_IDLE_BAR,
	ACT_DOD_WALK_IDLE_BAR,
	ACT_DOD_RUN_IDLE_BAR,
	ACT_DOD_SPRINT_IDLE_BAR,
	ACT_DOD_PRONEWALK_IDLE_BAR,
	ACT_DOD_STAND_ZOOM_RIFLE,
	ACT_DOD_CROUCH_ZOOM_RIFLE,
	ACT_DOD_CROUCHWALK_ZOOM_RIFLE,
	ACT_DOD_WALK_ZOOM_RIFLE,
	ACT_DOD_RUN_ZOOM_RIFLE,
	ACT_DOD_PRONE_ZOOM_RIFLE,
	ACT_DOD_STAND_ZOOM_BOLT,
	ACT_DOD_CROUCH_ZOOM_BOLT,
	ACT_DOD_CROUCHWALK_ZOOM_BOLT,
	ACT_DOD_WALK_ZOOM_BOLT,
	ACT_DOD_RUN_ZOOM_BOLT,
	ACT_DOD_PRONE_ZOOM_BOLT,
	ACT_DOD_STAND_ZOOM_BAZOOKA,
	ACT_DOD_CROUCH_ZOOM_BAZOOKA,
	ACT_DOD_CROUCHWALK_ZOOM_BAZOOKA,
	ACT_DOD_WALK_ZOOM_BAZOOKA,
	ACT_DOD_RUN_ZOOM_BAZOOKA,
	ACT_DOD_PRONE_ZOOM_BAZOOKA,
	ACT_DOD_STAND_ZOOM_PSCHRECK,
	ACT_DOD_CROUCH_ZOOM_PSCHRECK,
	ACT_DOD_CROUCHWALK_ZOOM_PSCHRECK,
	ACT_DOD_WALK_ZOOM_PSCHRECK,
	ACT_DOD_RUN_ZOOM_PSCHRECK,
	ACT_DOD_PRONE_ZOOM_PSCHRECK,
	ACT_DOD_DEPLOY_RIFLE,
	ACT_DOD_DEPLOY_TOMMY,
	ACT_DOD_DEPLOY_MG,
	ACT_DOD_DEPLOY_30CAL,
	ACT_DOD_PRONE_DEPLOY_RIFLE,
	ACT_DOD_PRONE_DEPLOY_TOMMY,
	ACT_DOD_PRONE_DEPLOY_MG,
	ACT_DOD_PRONE_DEPLOY_30CAL,
	ACT_DOD_PRIMARYATTACK_RIFLE,
	ACT_DOD_SECONDARYATTACK_RIFLE,
	ACT_DOD_PRIMARYATTACK_PRONE_RIFLE,
	ACT_DOD_SECONDARYATTACK_PRONE_RIFLE,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED_RIFLE,
	ACT_DOD_PRIMARYATTACK_DEPLOYED_RIFLE,
	ACT_DOD_PRIMARYATTACK_BOLT,
	ACT_DOD_SECONDARYATTACK_BOLT,
	ACT_DOD_PRIMARYATTACK_PRONE_BOLT,
	ACT_DOD_SECONDARYATTACK_PRONE_BOLT,
	ACT_DOD_PRIMARYATTACK_TOMMY,
	ACT_DOD_PRIMARYATTACK_PRONE_TOMMY,
	ACT_DOD_SECONDARYATTACK_TOMMY,
	ACT_DOD_SECONDARYATTACK_PRONE_TOMMY,
	ACT_DOD_PRIMARYATTACK_MP40,
	ACT_DOD_PRIMARYATTACK_PRONE_MP40,
	ACT_DOD_SECONDARYATTACK_MP40,
	ACT_DOD_SECONDARYATTACK_PRONE_MP40,
	ACT_DOD_PRIMARYATTACK_MP44,
	ACT_DOD_PRIMARYATTACK_PRONE_MP44,
	ACT_DOD_PRIMARYATTACK_GREASE,
	ACT_DOD_PRIMARYATTACK_PRONE_GREASE,
	ACT_DOD_PRIMARYATTACK_PISTOL,
	ACT_DOD_PRIMARYATTACK_PRONE_PISTOL,
	ACT_DOD_PRIMARYATTACK_C96,
	ACT_DOD_PRIMARYATTACK_PRONE_C96,
	ACT_DOD_PRIMARYATTACK_MG,
	ACT_DOD_PRIMARYATTACK_PRONE_MG,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED_MG,
	ACT_DOD_PRIMARYATTACK_DEPLOYED_MG,
	ACT_DOD_PRIMARYATTACK_30CAL,
	ACT_DOD_PRIMARYATTACK_PRONE_30CAL,
	ACT_DOD_PRIMARYATTACK_DEPLOYED_30CAL,
	ACT_DOD_PRIMARYATTACK_PRONE_DEPLOYED_30CAL,
	ACT_DOD_PRIMARYATTACK_GREN_FRAG,
	ACT_DOD_PRIMARYATTACK_PRONE_GREN_FRAG,
	ACT_DOD_PRIMARYATTACK_GREN_STICK,
	ACT_DOD_PRIMARYATTACK_PRONE_GREN_STICK,
	ACT_DOD_PRIMARYATTACK_KNIFE,
	ACT_DOD_PRIMARYATTACK_PRONE_KNIFE,
	ACT_DOD_PRIMARYATTACK_SPADE,
	ACT_DOD_PRIMARYATTACK_PRONE_SPADE,
	ACT_DOD_PRIMARYATTACK_BAZOOKA,
	ACT_DOD_PRIMARYATTACK_PRONE_BAZOOKA,
	ACT_DOD_PRIMARYATTACK_PSCHRECK,
	ACT_DOD_PRIMARYATTACK_PRONE_PSCHRECK,
	ACT_DOD_PRIMARYATTACK_BAR,
	ACT_DOD_PRIMARYATTACK_PRONE_BAR,
	ACT_DOD_RELOAD_GARAND,
	ACT_DOD_RELOAD_K43,
	ACT_DOD_RELOAD_BAR,
	ACT_DOD_RELOAD_MP40,
	ACT_DOD_RELOAD_MP44,
	ACT_DOD_RELOAD_BOLT,
	ACT_DOD_RELOAD_M1CARBINE,
	ACT_DOD_RELOAD_TOMMY,
	ACT_DOD_RELOAD_GREASEGUN,
	ACT_DOD_RELOAD_PISTOL,
	ACT_DOD_RELOAD_FG42,
	ACT_DOD_RELOAD_RIFLE,
	ACT_DOD_RELOAD_RIFLEGRENADE,
	ACT_DOD_RELOAD_C96,
	ACT_DOD_RELOAD_CROUCH_BAR,
	ACT_DOD_RELOAD_CROUCH_RIFLE,
	ACT_DOD_RELOAD_CROUCH_RIFLEGRENADE,
	ACT_DOD_RELOAD_CROUCH_BOLT,
	ACT_DOD_RELOAD_CROUCH_MP44,
	ACT_DOD_RELOAD_CROUCH_MP40,
	ACT_DOD_RELOAD_CROUCH_TOMMY,
	ACT_DOD_RELOAD_CROUCH_BAZOOKA,
	ACT_DOD_RELOAD_CROUCH_PSCHRECK,
	ACT_DOD_RELOAD_CROUCH_PISTOL,
	ACT_DOD_RELOAD_CROUCH_M1CARBINE,
	ACT_DOD_RELOAD_CROUCH_C96,
	ACT_DOD_RELOAD_BAZOOKA,
	ACT_DOD_ZOOMLOAD_BAZOOKA,
	ACT_DOD_RELOAD_PSCHRECK,
	ACT_DOD_ZOOMLOAD_PSCHRECK,
	ACT_DOD_RELOAD_DEPLOYED_FG42,
	ACT_DOD_RELOAD_DEPLOYED_30CAL,
	ACT_DOD_RELOAD_DEPLOYED_MG,
	ACT_DOD_RELOAD_DEPLOYED_MG34,
	ACT_DOD_RELOAD_DEPLOYED_BAR,
	ACT_DOD_RELOAD_PRONE_PISTOL,
	ACT_DOD_RELOAD_PRONE_GARAND,
	ACT_DOD_RELOAD_PRONE_M1CARBINE,
	ACT_DOD_RELOAD_PRONE_BOLT,
	ACT_DOD_RELOAD_PRONE_K43,
	ACT_DOD_RELOAD_PRONE_MP40,
	ACT_DOD_RELOAD_PRONE_MP44,
	ACT_DOD_RELOAD_PRONE_BAR,
	ACT_DOD_RELOAD_PRONE_GREASEGUN,
	ACT_DOD_RELOAD_PRONE_TOMMY,
	ACT_DOD_RELOAD_PRONE_FG42,
	ACT_DOD_RELOAD_PRONE_RIFLE,
	ACT_DOD_RELOAD_PRONE_RIFLEGRENADE,
	ACT_DOD_RELOAD_PRONE_C96,
	ACT_DOD_RELOAD_PRONE_BAZOOKA,
	ACT_DOD_ZOOMLOAD_PRONE_BAZOOKA,
	ACT_DOD_RELOAD_PRONE_PSCHRECK,
	ACT_DOD_ZOOMLOAD_PRONE_PSCHRECK,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_BAR,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_FG42,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_30CAL,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_MG,
	ACT_DOD_RELOAD_PRONE_DEPLOYED_MG34,
	ACT_DOD_PRONE_ZOOM_FORWARD_RIFLE,
	ACT_DOD_PRONE_ZOOM_FORWARD_BOLT,
	ACT_DOD_PRONE_ZOOM_FORWARD_BAZOOKA,
	ACT_DOD_PRONE_ZOOM_FORWARD_PSCHRECK,
	ACT_DOD_PRIMARYATTACK_CROUCH,
	ACT_DOD_PRIMARYATTACK_CROUCH_SPADE,
	ACT_DOD_PRIMARYATTACK_CROUCH_KNIFE,
	ACT_DOD_PRIMARYATTACK_CROUCH_GREN_FRAG,
	ACT_DOD_PRIMARYATTACK_CROUCH_GREN_STICK,
	ACT_DOD_SECONDARYATTACK_CROUCH,
	ACT_DOD_SECONDARYATTACK_CROUCH_TOMMY,
	ACT_DOD_SECONDARYATTACK_CROUCH_MP40,
	ACT_DOD_HS_IDLE,
	ACT_DOD_HS_CROUCH,
	ACT_DOD_HS_IDLE_30CAL,
	ACT_DOD_HS_IDLE_BAZOOKA,
	ACT_DOD_HS_IDLE_PSCHRECK,
	ACT_DOD_HS_IDLE_KNIFE,
	ACT_DOD_HS_IDLE_MG42,
	ACT_DOD_HS_IDLE_PISTOL,
	ACT_DOD_HS_IDLE_STICKGRENADE,
	ACT_DOD_HS_IDLE_TOMMY,
	ACT_DOD_HS_IDLE_MP44,
	ACT_DOD_HS_IDLE_K98,
	ACT_DOD_HS_CROUCH_30CAL,
	ACT_DOD_HS_CROUCH_BAZOOKA,
	ACT_DOD_HS_CROUCH_PSCHRECK,
	ACT_DOD_HS_CROUCH_KNIFE,
	ACT_DOD_HS_CROUCH_MG42,
	ACT_DOD_HS_CROUCH_PISTOL,
	ACT_DOD_HS_CROUCH_STICKGRENADE,
	ACT_DOD_HS_CROUCH_TOMMY,
	ACT_DOD_HS_CROUCH_MP44,
	ACT_DOD_HS_CROUCH_K98,
	ACT_DOD_STAND_IDLE_TNT,
	ACT_DOD_CROUCH_IDLE_TNT,
	ACT_DOD_CROUCHWALK_IDLE_TNT,
	ACT_DOD_WALK_IDLE_TNT,
	ACT_DOD_RUN_IDLE_TNT,
	ACT_DOD_SPRINT_IDLE_TNT,
	ACT_DOD_PRONEWALK_IDLE_TNT,
	ACT_DOD_PLANT_TNT,
	ACT_DOD_DEFUSE_TNT,
	ACT_HL2MP_IDLE,
	ACT_HL2MP_RUN,
	ACT_HL2MP_IDLE_CROUCH,
	ACT_HL2MP_WALK_CROUCH,
	ACT_HL2MP_GESTURE_RANGE_ATTACK,
	ACT_HL2MP_GESTURE_RELOAD,
	ACT_HL2MP_JUMP,
	ACT_HL2MP_IDLE_PISTOL,
	ACT_HL2MP_RUN_PISTOL,
	ACT_HL2MP_IDLE_CROUCH_PISTOL,
	ACT_HL2MP_WALK_CROUCH_PISTOL,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,
	ACT_HL2MP_GESTURE_RELOAD_PISTOL,
	ACT_HL2MP_JUMP_PISTOL,
	ACT_HL2MP_IDLE_SMG1,
	ACT_HL2MP_RUN_SMG1,
	ACT_HL2MP_IDLE_CROUCH_SMG1,
	ACT_HL2MP_WALK_CROUCH_SMG1,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,
	ACT_HL2MP_GESTURE_RELOAD_SMG1,
	ACT_HL2MP_JUMP_SMG1,
	ACT_HL2MP_IDLE_AR2,
	ACT_HL2MP_RUN_AR2,
	ACT_HL2MP_IDLE_CROUCH_AR2,
	ACT_HL2MP_WALK_CROUCH_AR2,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2,
	ACT_HL2MP_GESTURE_RELOAD_AR2,
	ACT_HL2MP_JUMP_AR2,
	ACT_HL2MP_IDLE_SHOTGUN,
	ACT_HL2MP_RUN_SHOTGUN,
	ACT_HL2MP_IDLE_CROUCH_SHOTGUN,
	ACT_HL2MP_WALK_CROUCH_SHOTGUN,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_SHOTGUN,
	ACT_HL2MP_GESTURE_RELOAD_SHOTGUN,
	ACT_HL2MP_JUMP_SHOTGUN,
	ACT_HL2MP_IDLE_RPG,
	ACT_HL2MP_RUN_RPG,
	ACT_HL2MP_IDLE_CROUCH_RPG,
	ACT_HL2MP_WALK_CROUCH_RPG,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_RPG,
	ACT_HL2MP_GESTURE_RELOAD_RPG,
	ACT_HL2MP_JUMP_RPG,
	ACT_HL2MP_IDLE_GRENADE,
	ACT_HL2MP_RUN_GRENADE,
	ACT_HL2MP_IDLE_CROUCH_GRENADE,
	ACT_HL2MP_WALK_CROUCH_GRENADE,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,
	ACT_HL2MP_GESTURE_RELOAD_GRENADE,
	ACT_HL2MP_JUMP_GRENADE,
	ACT_HL2MP_IDLE_PHYSGUN,
	ACT_HL2MP_RUN_PHYSGUN,
	ACT_HL2MP_IDLE_CROUCH_PHYSGUN,
	ACT_HL2MP_WALK_CROUCH_PHYSGUN,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_PHYSGUN,
	ACT_HL2MP_GESTURE_RELOAD_PHYSGUN,
	ACT_HL2MP_JUMP_PHYSGUN,
	ACT_HL2MP_IDLE_CROSSBOW,
	ACT_HL2MP_RUN_CROSSBOW,
	ACT_HL2MP_IDLE_CROUCH_CROSSBOW,
	ACT_HL2MP_WALK_CROUCH_CROSSBOW,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_CROSSBOW,
	ACT_HL2MP_GESTURE_RELOAD_CROSSBOW,
	ACT_HL2MP_JUMP_CROSSBOW,
	ACT_HL2MP_IDLE_MELEE,
	ACT_HL2MP_RUN_MELEE,
	ACT_HL2MP_IDLE_CROUCH_MELEE,
	ACT_HL2MP_WALK_CROUCH_MELEE,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE,
	ACT_HL2MP_GESTURE_RELOAD_MELEE,
	ACT_HL2MP_JUMP_MELEE,
	ACT_HL2MP_IDLE_SLAM,
	ACT_HL2MP_RUN_SLAM,
	ACT_HL2MP_IDLE_CROUCH_SLAM,
	ACT_HL2MP_WALK_CROUCH_SLAM,
	ACT_HL2MP_GESTURE_RANGE_ATTACK_SLAM,
	ACT_HL2MP_GESTURE_RELOAD_SLAM,
	ACT_HL2MP_JUMP_SLAM,
	ACT_VM_FIZZLE,
	ACT_MP_STAND_IDLE,
	ACT_MP_CROUCH_IDLE,
	ACT_MP_CROUCH_DEPLOYED_IDLE,
	ACT_MP_CROUCH_DEPLOYED,
	ACT_MP_CROUCHWALK_DEPLOYED,
	ACT_MP_DEPLOYED_IDLE,
	ACT_MP_RUN,
	ACT_MP_WALK,
	ACT_MP_AIRWALK,
	ACT_MP_CROUCHWALK,
	ACT_MP_SPRINT,
	ACT_MP_JUMP,
	ACT_MP_JUMP_START,
	ACT_MP_JUMP_FLOAT,
	ACT_MP_JUMP_LAND,
	ACT_MP_DOUBLEJUMP,
	ACT_MP_SWIM,
	ACT_MP_DEPLOYED,
	ACT_MP_SWIM_DEPLOYED,
	ACT_MP_VCD,
	ACT_MP_ATTACK_STAND_PRIMARYFIRE,
	ACT_MP_ATTACK_STAND_PRIMARYFIRE_DEPLOYED,
	ACT_MP_ATTACK_STAND_SECONDARYFIRE,
	ACT_MP_ATTACK_STAND_GRENADE,
	ACT_MP_ATTACK_CROUCH_PRIMARYFIRE,
	ACT_MP_ATTACK_CROUCH_PRIMARYFIRE_DEPLOYED,
	ACT_MP_ATTACK_CROUCH_SECONDARYFIRE,
	ACT_MP_ATTACK_CROUCH_GRENADE,
	ACT_MP_ATTACK_SWIM_PRIMARYFIRE,
	ACT_MP_ATTACK_SWIM_SECONDARYFIRE,
	ACT_MP_ATTACK_SWIM_GRENADE,
	ACT_MP_ATTACK_AIRWALK_PRIMARYFIRE,
	ACT_MP_ATTACK_AIRWALK_SECONDARYFIRE,
	ACT_MP_ATTACK_AIRWALK_GRENADE,
	ACT_MP_RELOAD_STAND,
	ACT_MP_RELOAD_STAND_LOOP,
	ACT_MP_RELOAD_STAND_END,
	ACT_MP_RELOAD_CROUCH,
	ACT_MP_RELOAD_CROUCH_LOOP,
	ACT_MP_RELOAD_CROUCH_END,
	ACT_MP_RELOAD_SWIM,
	ACT_MP_RELOAD_SWIM_LOOP,
	ACT_MP_RELOAD_SWIM_END,
	ACT_MP_RELOAD_AIRWALK,
	ACT_MP_RELOAD_AIRWALK_LOOP,
	ACT_MP_RELOAD_AIRWALK_END,
	ACT_MP_ATTACK_STAND_PREFIRE,
	ACT_MP_ATTACK_STAND_POSTFIRE,
	ACT_MP_ATTACK_STAND_STARTFIRE,
	ACT_MP_ATTACK_CROUCH_PREFIRE,
	ACT_MP_ATTACK_CROUCH_POSTFIRE,
	ACT_MP_ATTACK_SWIM_PREFIRE,
	ACT_MP_ATTACK_SWIM_POSTFIRE,
	ACT_MP_STAND_PRIMARY,
	ACT_MP_CROUCH_PRIMARY,
	ACT_MP_RUN_PRIMARY,
	ACT_MP_WALK_PRIMARY,
	ACT_MP_AIRWALK_PRIMARY,
	ACT_MP_CROUCHWALK_PRIMARY,
	ACT_MP_JUMP_PRIMARY,
	ACT_MP_JUMP_START_PRIMARY,
	ACT_MP_JUMP_FLOAT_PRIMARY,
	ACT_MP_JUMP_LAND_PRIMARY,
	ACT_MP_SWIM_PRIMARY,
	ACT_MP_DEPLOYED_PRIMARY,
	ACT_MP_SWIM_DEPLOYED_PRIMARY,
	ACT_MP_CROUCHWALK_DEPLOYED_PRIMARY,
	ACT_MP_CROUCH_DEPLOYED_IDLE_PRIMARY,
	ACT_MP_ATTACK_STAND_PRIMARY,
	ACT_MP_ATTACK_STAND_PRIMARY_DEPLOYED,
	ACT_MP_ATTACK_CROUCH_PRIMARY,
	ACT_MP_ATTACK_CROUCH_PRIMARY_DEPLOYED,
	ACT_MP_ATTACK_SWIM_PRIMARY,
	ACT_MP_ATTACK_AIRWALK_PRIMARY,
	ACT_MP_RELOAD_STAND_PRIMARY,
	ACT_MP_RELOAD_STAND_PRIMARY_LOOP,
	ACT_MP_RELOAD_STAND_PRIMARY_END,
	ACT_MP_RELOAD_CROUCH_PRIMARY,
	ACT_MP_RELOAD_CROUCH_PRIMARY_LOOP,
	ACT_MP_RELOAD_CROUCH_PRIMARY_END,
	ACT_MP_RELOAD_SWIM_PRIMARY,
	ACT_MP_RELOAD_SWIM_PRIMARY_LOOP,
	ACT_MP_RELOAD_SWIM_PRIMARY_END,
	ACT_MP_RELOAD_AIRWALK_PRIMARY,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_LOOP,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_END,
	ACT_MP_RELOAD_STAND_PRIMARY_2,
	ACT_MP_RELOAD_STAND_PRIMARY_LOOP_2,
	ACT_MP_RELOAD_STAND_PRIMARY_END_2,
	ACT_MP_RELOAD_CROUCH_PRIMARY_2,
	ACT_MP_RELOAD_CROUCH_PRIMARY_LOOP_2,
	ACT_MP_RELOAD_CROUCH_PRIMARY_END_2,
	ACT_MP_RELOAD_SWIM_PRIMARY_2,
	ACT_MP_RELOAD_SWIM_PRIMARY_LOOP_2,
	ACT_MP_RELOAD_SWIM_PRIMARY_END_2,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_2,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_LOOP_2,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_END_2,
	ACT_MP_ATTACK_STAND_PRIMARY_ALT,
	ACT_MP_ATTACK_CROUCH_PRIMARY_ALT,
	ACT_MP_ATTACK_SWIM_PRIMARY_ALT,
	ACT_MP_RELOAD_STAND_PRIMARY_ALT,
	ACT_MP_RELOAD_CROUCH_PRIMARY_ALT,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_ALT,
	ACT_MP_RELOAD_STAND_PRIMARY_LOOP_ALT,
	ACT_MP_RELOAD_CROUCH_PRIMARY_LOOP_ALT,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_LOOP_ALT,
	ACT_MP_RELOAD_STAND_PRIMARY_END_ALT,
	ACT_MP_RELOAD_CROUCH_PRIMARY_END_ALT,
	ACT_MP_RELOAD_AIRWALK_PRIMARY_END_ALT,
	ACT_MP_RELOAD_SWIM_PRIMARY_ALT,
	ACT_MP_ATTACK_STAND_PRIMARY_SUPER,
	ACT_MP_ATTACK_CROUCH_PRIMARY_SUPER,
	ACT_MP_ATTACK_SWIM_PRIMARY_SUPER,
	ACT_MP_ATTACK_STAND_GRENADE_PRIMARY,
	ACT_MP_ATTACK_CROUCH_GRENADE_PRIMARY,
	ACT_MP_ATTACK_SWIM_GRENADE_PRIMARY,
	ACT_MP_ATTACK_AIRWALK_GRENADE_PRIMARY,
	ACT_MP_STAND_SECONDARY,
	ACT_MP_CROUCH_SECONDARY,
	ACT_MP_RUN_SECONDARY,
	ACT_MP_WALK_SECONDARY,
	ACT_MP_AIRWALK_SECONDARY,
	ACT_MP_CROUCHWALK_SECONDARY,
	ACT_MP_JUMP_SECONDARY,
	ACT_MP_JUMP_START_SECONDARY,
	ACT_MP_JUMP_FLOAT_SECONDARY,
	ACT_MP_JUMP_LAND_SECONDARY,
	ACT_MP_SWIM_SECONDARY,
	ACT_MP_ATTACK_STAND_SECONDARY,
	ACT_MP_ATTACK_CROUCH_SECONDARY,
	ACT_MP_ATTACK_SWIM_SECONDARY,
	ACT_MP_ATTACK_AIRWALK_SECONDARY,
	ACT_MP_RELOAD_STAND_SECONDARY,
	ACT_MP_RELOAD_STAND_SECONDARY_LOOP,
	ACT_MP_RELOAD_STAND_SECONDARY_END,
	ACT_MP_RELOAD_CROUCH_SECONDARY,
	ACT_MP_RELOAD_CROUCH_SECONDARY_LOOP,
	ACT_MP_RELOAD_CROUCH_SECONDARY_END,
	ACT_MP_RELOAD_SWIM_SECONDARY,
	ACT_MP_RELOAD_SWIM_SECONDARY_LOOP,
	ACT_MP_RELOAD_SWIM_SECONDARY_END,
	ACT_MP_RELOAD_AIRWALK_SECONDARY,
	ACT_MP_RELOAD_AIRWALK_SECONDARY_LOOP,
	ACT_MP_RELOAD_AIRWALK_SECONDARY_END,
	ACT_MP_RELOAD_STAND_SECONDARY_2,
	ACT_MP_RELOAD_CROUCH_SECONDARY_2,
	ACT_MP_RELOAD_SWIM_SECONDARY_2,
	ACT_MP_RELOAD_AIRWALK_SECONDARY_2,
	ACT_MP_ATTACK_STAND_GRENADE_SECONDARY,
	ACT_MP_ATTACK_CROUCH_GRENADE_SECONDARY,
	ACT_MP_ATTACK_SWIM_GRENADE_SECONDARY,
	ACT_MP_ATTACK_AIRWALK_GRENADE_SECONDARY,
	ACT_MP_STAND_SECONDARY2,
	ACT_MP_CROUCH_SECONDARY2,
	ACT_MP_RUN_SECONDARY2,
	ACT_MP_WALK_SECONDARY2,
	ACT_MP_AIRWALK_SECONDARY2,
	ACT_MP_CROUCHWALK_SECONDARY2,
	ACT_MP_JUMP_SECONDARY2,
	ACT_MP_JUMP_START_SECONDARY2,
	ACT_MP_JUMP_FLOAT_SECONDARY2,
	ACT_MP_JUMP_LAND_SECONDARY2,
	ACT_MP_SWIM_SECONDARY2,
	ACT_MP_ATTACK_STAND_SECONDARY2,
	ACT_MP_ATTACK_CROUCH_SECONDARY2,
	ACT_MP_ATTACK_SWIM_SECONDARY2,
	ACT_MP_ATTACK_AIRWALK_SECONDARY2,
	ACT_MP_RELOAD_STAND_SECONDARY2,
	ACT_MP_RELOAD_STAND_SECONDARY2_LOOP,
	ACT_MP_RELOAD_STAND_SECONDARY2_END,
	ACT_MP_RELOAD_CROUCH_SECONDARY2,
	ACT_MP_RELOAD_CROUCH_SECONDARY2_LOOP,
	ACT_MP_RELOAD_CROUCH_SECONDARY2_END,
	ACT_MP_RELOAD_SWIM_SECONDARY2,
	ACT_MP_RELOAD_SWIM_SECONDARY2_LOOP,
	ACT_MP_RELOAD_SWIM_SECONDARY2_END,
	ACT_MP_RELOAD_AIRWALK_SECONDARY2,
	ACT_MP_RELOAD_AIRWALK_SECONDARY2_LOOP,
	ACT_MP_RELOAD_AIRWALK_SECONDARY2_END,
	ACT_MP_STAND_MELEE,
	ACT_MP_CROUCH_MELEE,
	ACT_MP_RUN_MELEE,
	ACT_MP_WALK_MELEE,
	ACT_MP_AIRWALK_MELEE,
	ACT_MP_CROUCHWALK_MELEE,
	ACT_MP_JUMP_MELEE,
	ACT_MP_JUMP_START_MELEE,
	ACT_MP_JUMP_FLOAT_MELEE,
	ACT_MP_JUMP_LAND_MELEE,
	ACT_MP_SWIM_MELEE,
	ACT_MP_ATTACK_STAND_MELEE,
	ACT_MP_ATTACK_STAND_MELEE_SECONDARY,
	ACT_MP_ATTACK_CROUCH_MELEE,
	ACT_MP_ATTACK_CROUCH_MELEE_SECONDARY,
	ACT_MP_ATTACK_SWIM_MELEE,
	ACT_MP_ATTACK_AIRWALK_MELEE,
	ACT_MP_ATTACK_STAND_GRENADE_MELEE,
	ACT_MP_ATTACK_CROUCH_GRENADE_MELEE,
	ACT_MP_ATTACK_SWIM_GRENADE_MELEE,
	ACT_MP_ATTACK_AIRWALK_GRENADE_MELEE,
	ACT_MP_STAND_ITEM1,
	ACT_MP_CROUCH_ITEM1,
	ACT_MP_RUN_ITEM1,
	ACT_MP_WALK_ITEM1,
	ACT_MP_AIRWALK_ITEM1,
	ACT_MP_CROUCHWALK_ITEM1,
	ACT_MP_JUMP_ITEM1,
	ACT_MP_JUMP_START_ITEM1,
	ACT_MP_JUMP_FLOAT_ITEM1,
	ACT_MP_JUMP_LAND_ITEM1,
	ACT_MP_SWIM_ITEM1,
	ACT_MP_ATTACK_STAND_ITEM1,
	ACT_MP_ATTACK_STAND_ITEM1_SECONDARY,
	ACT_MP_ATTACK_CROUCH_ITEM1,
	ACT_MP_ATTACK_CROUCH_ITEM1_SECONDARY,
	ACT_MP_ATTACK_SWIM_ITEM1,
	ACT_MP_ATTACK_AIRWALK_ITEM1,
	ACT_MP_DEPLOYED_ITEM1,
	ACT_MP_DEPLOYED_IDLE_ITEM1,
	ACT_MP_CROUCHWALK_DEPLOYED_ITEM1,
	ACT_MP_CROUCH_DEPLOYED_IDLE_ITEM1,
	ACT_MP_ATTACK_STAND_PRIMARY_DEPLOYED_ITEM1,
	ACT_MP_ATTACK_CROUCH_PRIMARY_DEPLOYED_ITEM1,
	ACT_MP_STAND_ITEM2,
	ACT_MP_CROUCH_ITEM2,
	ACT_MP_RUN_ITEM2,
	ACT_MP_WALK_ITEM2,
	ACT_MP_AIRWALK_ITEM2,
	ACT_MP_CROUCHWALK_ITEM2,
	ACT_MP_JUMP_ITEM2,
	ACT_MP_JUMP_START_ITEM2,
	ACT_MP_JUMP_FLOAT_ITEM2,
	ACT_MP_JUMP_LAND_ITEM2,
	ACT_MP_SWIM_ITEM2,
	ACT_MP_ATTACK_STAND_ITEM2,
	ACT_MP_ATTACK_STAND_ITEM2_SECONDARY,
	ACT_MP_ATTACK_CROUCH_ITEM2,
	ACT_MP_ATTACK_CROUCH_ITEM2_SECONDARY,
	ACT_MP_ATTACK_SWIM_ITEM2,
	ACT_MP_ATTACK_AIRWALK_ITEM2,
	ACT_MP_ATTACK_STAND_HARD_ITEM2,
	ACT_MP_ATTACK_CROUCH_HARD_ITEM2,
	ACT_MP_ATTACK_SWIM_HARD_ITEM2,
	ACT_MP_DEPLOYED_ITEM2,
	ACT_MP_DEPLOYED_IDLE_ITEM2,
	ACT_MP_CROUCHWALK_DEPLOYED_ITEM2,
	ACT_MP_CROUCH_DEPLOYED_IDLE_ITEM2,
	ACT_MP_ATTACK_STAND_PRIMARY_DEPLOYED_ITEM2,
	ACT_MP_ATTACK_CROUCH_PRIMARY_DEPLOYED_ITEM2,
	ACT_MP_RELOAD_STAND_ITEM2,
	ACT_MP_RELOAD_STAND_ITEM2_LOOP,
	ACT_MP_RELOAD_STAND_ITEM2_END,
	ACT_MP_RELOAD_CROUCH_ITEM2,
	ACT_MP_RELOAD_CROUCH_ITEM2_LOOP,
	ACT_MP_RELOAD_CROUCH_ITEM2_END,
	ACT_MP_RELOAD_SWIM_ITEM2,
	ACT_MP_RELOAD_SWIM_ITEM2_LOOP,
	ACT_MP_RELOAD_SWIM_ITEM2_END,
	ACT_MP_RELOAD_AIRWALK_ITEM2,
	ACT_MP_RELOAD_AIRWALK_ITEM2_LOOP,
	ACT_MP_RELOAD_AIRWALK_ITEM2_END,
	ACT_MP_RELOAD_NO_AMMO_ITEM2,
	ACT_MP_ATTACK_STAND_GRENADE_ITEM2,
	ACT_MP_ATTACK_CROUCH_GRENADE_ITEM2,
	ACT_MP_ATTACK_SWIM_GRENADE_ITEM2,
	ACT_MP_ATTACK_AIRWALK_GRENADE_ITEM2,
	ACT_MP_STAND_PASSTIME,
	ACT_MP_RUN_PASSTIME,
	ACT_MP_CROUCHWALK_PASSTIME,
	ACT_MP_GESTURE_FLINCH,
	ACT_MP_GESTURE_FLINCH_PRIMARY,
	ACT_MP_GESTURE_FLINCH_SECONDARY,
	ACT_MP_GESTURE_FLINCH_MELEE,
	ACT_MP_GESTURE_FLINCH_ITEM1,
	ACT_MP_GESTURE_FLINCH_ITEM2,
	ACT_MP_GESTURE_FLINCH_HEAD,
	ACT_MP_GESTURE_FLINCH_CHEST,
	ACT_MP_GESTURE_FLINCH_STOMACH,
	ACT_MP_GESTURE_FLINCH_LEFTARM,
	ACT_MP_GESTURE_FLINCH_RIGHTARM,
	ACT_MP_GESTURE_FLINCH_LEFTLEG,
	ACT_MP_GESTURE_FLINCH_RIGHTLEG,
	ACT_MP_GRENADE1_DRAW,
	ACT_MP_GRENADE1_IDLE,
	ACT_MP_GRENADE1_ATTACK,
	ACT_MP_GRENADE2_DRAW,
	ACT_MP_GRENADE2_IDLE,
	ACT_MP_GRENADE2_ATTACK,
	ACT_MP_PRIMARY_GRENADE1_DRAW,
	ACT_MP_PRIMARY_GRENADE1_IDLE,
	ACT_MP_PRIMARY_GRENADE1_ATTACK,
	ACT_MP_PRIMARY_GRENADE2_DRAW,
	ACT_MP_PRIMARY_GRENADE2_IDLE,
	ACT_MP_PRIMARY_GRENADE2_ATTACK,
	ACT_MP_SECONDARY_GRENADE1_DRAW,
	ACT_MP_SECONDARY_GRENADE1_IDLE,
	ACT_MP_SECONDARY_GRENADE1_ATTACK,
	ACT_MP_SECONDARY_GRENADE2_DRAW,
	ACT_MP_SECONDARY_GRENADE2_IDLE,
	ACT_MP_SECONDARY_GRENADE2_ATTACK,
	ACT_MP_MELEE_GRENADE1_DRAW,
	ACT_MP_MELEE_GRENADE1_IDLE,
	ACT_MP_MELEE_GRENADE1_ATTACK,
	ACT_MP_MELEE_GRENADE2_DRAW,
	ACT_MP_MELEE_GRENADE2_IDLE,
	ACT_MP_MELEE_GRENADE2_ATTACK,
	ACT_MP_ITEM1_GRENADE1_DRAW,
	ACT_MP_ITEM1_GRENADE1_IDLE,
	ACT_MP_ITEM1_GRENADE1_ATTACK,
	ACT_MP_ITEM1_GRENADE2_DRAW,
	ACT_MP_ITEM1_GRENADE2_IDLE,
	ACT_MP_ITEM1_GRENADE2_ATTACK,
	ACT_MP_ITEM2_GRENADE1_DRAW,
	ACT_MP_ITEM2_GRENADE1_IDLE,
	ACT_MP_ITEM2_GRENADE1_ATTACK,
	ACT_MP_ITEM2_GRENADE2_DRAW,
	ACT_MP_ITEM2_GRENADE2_IDLE,
	ACT_MP_ITEM2_GRENADE2_ATTACK,
	ACT_MP_STAND_BUILDING,
	ACT_MP_CROUCH_BUILDING,
	ACT_MP_RUN_BUILDING,
	ACT_MP_WALK_BUILDING,
	ACT_MP_AIRWALK_BUILDING,
	ACT_MP_CROUCHWALK_BUILDING,
	ACT_MP_JUMP_BUILDING,
	ACT_MP_JUMP_START_BUILDING,
	ACT_MP_JUMP_FLOAT_BUILDING,
	ACT_MP_JUMP_LAND_BUILDING,
	ACT_MP_SWIM_BUILDING,
	ACT_MP_ATTACK_STAND_BUILDING,
	ACT_MP_ATTACK_CROUCH_BUILDING,
	ACT_MP_ATTACK_SWIM_BUILDING,
	ACT_MP_ATTACK_AIRWALK_BUILDING,
	ACT_MP_ATTACK_STAND_GRENADE_BUILDING,
	ACT_MP_ATTACK_CROUCH_GRENADE_BUILDING,
	ACT_MP_ATTACK_SWIM_GRENADE_BUILDING,
	ACT_MP_ATTACK_AIRWALK_GRENADE_BUILDING,
	ACT_MP_STAND_BUILDING_DEPLOYED,
	ACT_MP_CROUCH_BUILDING_DEPLOYED,
	ACT_MP_RUN_BUILDING_DEPLOYED,
	ACT_MP_WALK_BUILDING_DEPLOYED,
	ACT_MP_AIRWALK_BUILDING_DEPLOYED,
	ACT_MP_CROUCHWALK_BUILDING_DEPLOYED,
	ACT_MP_JUMP_BUILDING_DEPLOYED,
	ACT_MP_JUMP_START_BUILDING_DEPLOYED,
	ACT_MP_JUMP_FLOAT_BUILDING_DEPLOYED,
	ACT_MP_JUMP_LAND_BUILDING_DEPLOYED,
	ACT_MP_SWIM_BUILDING_DEPLOYED,
	ACT_MP_ATTACK_STAND_BUILDING_DEPLOYED,
	ACT_MP_ATTACK_CROUCH_BUILDING_DEPLOYED,
	ACT_MP_ATTACK_SWIM_BUILDING_DEPLOYED,
	ACT_MP_ATTACK_AIRWALK_BUILDING_DEPLOYED,
	ACT_MP_ATTACK_STAND_GRENADE_BUILDING_DEPLOYED,
	ACT_MP_ATTACK_CROUCH_GRENADE_BUILDING_DEPLOYED,
	ACT_MP_ATTACK_SWIM_GRENADE_BUILDING_DEPLOYED,
	ACT_MP_ATTACK_AIRWALK_GRENADE_BUILDING_DEPLOYED,
	ACT_MP_STAND_PDA,
	ACT_MP_CROUCH_PDA,
	ACT_MP_RUN_PDA,
	ACT_MP_WALK_PDA,
	ACT_MP_AIRWALK_PDA,
	ACT_MP_CROUCHWALK_PDA,
	ACT_MP_JUMP_PDA,
	ACT_MP_JUMP_START_PDA,
	ACT_MP_JUMP_FLOAT_PDA,
	ACT_MP_JUMP_LAND_PDA,
	ACT_MP_SWIM_PDA,
	ACT_MP_ATTACK_STAND_PDA,
	ACT_MP_ATTACK_SWIM_PDA,
	ACT_MP_STAND_LOSERSTATE,
	ACT_MP_CROUCH_LOSERSTATE,
	ACT_MP_RUN_LOSERSTATE,
	ACT_MP_WALK_LOSERSTATE,
	ACT_MP_AIRWALK_LOSERSTATE,
	ACT_MP_CROUCHWALK_LOSERSTATE,
	ACT_MP_JUMP_LOSERSTATE,
	ACT_MP_JUMP_START_LOSERSTATE,
	ACT_MP_JUMP_FLOAT_LOSERSTATE,
	ACT_MP_JUMP_LAND_LOSERSTATE,
	ACT_MP_SWIM_LOSERSTATE,
	ACT_MP_DOUBLEJUMP_LOSERSTATE,
	ACT_MP_DOUBLEJUMP_CROUCH,
	ACT_MP_DOUBLEJUMP_CROUCH_PRIMARY,
	ACT_MP_DOUBLEJUMP_CROUCH_SECONDARY,
	ACT_MP_DOUBLEJUMP_CROUCH_MELEE,
	ACT_MP_DOUBLEJUMP_CROUCH_ITEM1,
	ACT_MP_DOUBLEJUMP_CROUCH_ITEM2,
	ACT_MP_DOUBLEJUMP_CROUCH_LOSERSTATE,
	ACT_MP_DOUBLEJUMP_CROUCH_PASSTIME,
	ACT_MP_GESTURE_VC_HANDMOUTH,
	ACT_MP_GESTURE_VC_FINGERPOINT,
	ACT_MP_GESTURE_VC_FISTPUMP,
	ACT_MP_GESTURE_VC_THUMBSUP,
	ACT_MP_GESTURE_VC_NODYES,
	ACT_MP_GESTURE_VC_NODNO,
	ACT_MP_GESTURE_VC_HANDMOUTH_PRIMARY,
	ACT_MP_GESTURE_VC_FINGERPOINT_PRIMARY,
	ACT_MP_GESTURE_VC_FISTPUMP_PRIMARY,
	ACT_MP_GESTURE_VC_THUMBSUP_PRIMARY,
	ACT_MP_GESTURE_VC_NODYES_PRIMARY,
	ACT_MP_GESTURE_VC_NODNO_PRIMARY,
	ACT_MP_GESTURE_VC_HANDMOUTH_SECONDARY,
	ACT_MP_GESTURE_VC_FINGERPOINT_SECONDARY,
	ACT_MP_GESTURE_VC_FISTPUMP_SECONDARY,
	ACT_MP_GESTURE_VC_THUMBSUP_SECONDARY,
	ACT_MP_GESTURE_VC_NODYES_SECONDARY,
	ACT_MP_GESTURE_VC_NODNO_SECONDARY,
	ACT_MP_GESTURE_VC_HANDMOUTH_MELEE,
	ACT_MP_GESTURE_VC_FINGERPOINT_MELEE,
	ACT_MP_GESTURE_VC_FISTPUMP_MELEE,
	ACT_MP_GESTURE_VC_THUMBSUP_MELEE,
	ACT_MP_GESTURE_VC_NODYES_MELEE,
	ACT_MP_GESTURE_VC_NODNO_MELEE,
	ACT_MP_GESTURE_VC_HANDMOUTH_ITEM1,
	ACT_MP_GESTURE_VC_FINGERPOINT_ITEM1,
	ACT_MP_GESTURE_VC_FISTPUMP_ITEM1,
	ACT_MP_GESTURE_VC_THUMBSUP_ITEM1,
	ACT_MP_GESTURE_VC_NODYES_ITEM1,
	ACT_MP_GESTURE_VC_NODNO_ITEM1,
	ACT_MP_GESTURE_VC_HANDMOUTH_ITEM2,
	ACT_MP_GESTURE_VC_FINGERPOINT_ITEM2,
	ACT_MP_GESTURE_VC_FISTPUMP_ITEM2,
	ACT_MP_GESTURE_VC_THUMBSUP_ITEM2,
	ACT_MP_GESTURE_VC_NODYES_ITEM2,
	ACT_MP_GESTURE_VC_NODNO_ITEM2,
	ACT_MP_GESTURE_VC_HANDMOUTH_BUILDING,
	ACT_MP_GESTURE_VC_FINGERPOINT_BUILDING,
	ACT_MP_GESTURE_VC_FISTPUMP_BUILDING,
	ACT_MP_GESTURE_VC_THUMBSUP_BUILDING,
	ACT_MP_GESTURE_VC_NODYES_BUILDING,
	ACT_MP_GESTURE_VC_NODNO_BUILDING,
	ACT_MP_GESTURE_VC_HANDMOUTH_PDA,
	ACT_MP_GESTURE_VC_FINGERPOINT_PDA,
	ACT_MP_GESTURE_VC_FISTPUMP_PDA,
	ACT_MP_GESTURE_VC_THUMBSUP_PDA,
	ACT_MP_GESTURE_VC_NODYES_PDA,
	ACT_MP_GESTURE_VC_NODNO_PDA,
	ACT_MP_STUN_BEGIN,
	ACT_MP_STUN_MIDDLE,
	ACT_MP_STUN_END,
	ACT_MP_PASSTIME_THROW_BEGIN,
	ACT_MP_PASSTIME_THROW_MIDDLE,
	ACT_MP_PASSTIME_THROW_END,
	ACT_MP_PASSTIME_THROW_CANCEL,
	ACT_VM_UNUSABLE,
	ACT_VM_UNUSABLE_TO_USABLE,
	ACT_VM_USABLE_TO_UNUSABLE,
	ACT_PRIMARY_VM_DRAW,
	ACT_PRIMARY_VM_HOLSTER,
	ACT_PRIMARY_VM_IDLE,
	ACT_PRIMARY_VM_PULLBACK,
	ACT_PRIMARY_VM_PRIMARYATTACK,
	ACT_PRIMARY_VM_SECONDARYATTACK,
	ACT_PRIMARY_VM_RELOAD,
	ACT_PRIMARY_RELOAD_START,
	ACT_PRIMARY_RELOAD_FINISH,
	ACT_PRIMARY_VM_DRYFIRE,
	ACT_PRIMARY_VM_IDLE_TO_LOWERED,
	ACT_PRIMARY_VM_IDLE_LOWERED,
	ACT_PRIMARY_VM_LOWERED_TO_IDLE,
	ACT_PRIMARY_VM_RELOAD_2,
	ACT_PRIMARY_RELOAD_START_2,
	ACT_PRIMARY_RELOAD_FINISH_2,
	ACT_PRIMARY_VM_RELOAD_3,
	ACT_PRIMARY_RELOAD_START_3,
	ACT_PRIMARY_RELOAD_FINISH_3,
	ACT_PRIMARY_VM_PRIMARYATTACK_3,
	ACT_SECONDARY_VM_DRAW,
	ACT_SECONDARY_VM_HOLSTER,
	ACT_SECONDARY_VM_IDLE,
	ACT_SECONDARY_VM_PULLBACK,
	ACT_SECONDARY_VM_PRIMARYATTACK,
	ACT_SECONDARY_VM_SECONDARYATTACK,
	ACT_SECONDARY_VM_RELOAD,
	ACT_SECONDARY_RELOAD_START,
	ACT_SECONDARY_RELOAD_FINISH,
	ACT_SECONDARY_VM_RELOAD2,
	ACT_SECONDARY_VM_DRYFIRE,
	ACT_SECONDARY_VM_IDLE_TO_LOWERED,
	ACT_SECONDARY_VM_IDLE_LOWERED,
	ACT_SECONDARY_VM_LOWERED_TO_IDLE,
	ACT_SECONDARY_VM_DRAW_2,
	ACT_SECONDARY_VM_IDLE_2,
	ACT_SECONDARY_VM_PRIMARYATTACK_2,
	ACT_SECONDARY_VM_RELOAD_2,
	ACT_MELEE_VM_DRAW,
	ACT_MELEE_VM_HOLSTER,
	ACT_MELEE_VM_IDLE,
	ACT_MELEE_VM_PULLBACK,
	ACT_MELEE_VM_PRIMARYATTACK,
	ACT_MELEE_VM_SECONDARYATTACK,
	ACT_MELEE_VM_RELOAD,
	ACT_MELEE_VM_DRYFIRE,
	ACT_MELEE_VM_IDLE_TO_LOWERED,
	ACT_MELEE_VM_IDLE_LOWERED,
	ACT_MELEE_VM_LOWERED_TO_IDLE,
	ACT_MELEE_VM_STUN,
	ACT_MELEE_VM_HITCENTER,
	ACT_MELEE_VM_SWINGHARD,
	ACT_PDA_VM_DRAW,
	ACT_PDA_VM_HOLSTER,
	ACT_PDA_VM_IDLE,
	ACT_PDA_VM_PULLBACK,
	ACT_PDA_VM_PRIMARYATTACK,
	ACT_PDA_VM_SECONDARYATTACK,
	ACT_PDA_VM_RELOAD,
	ACT_PDA_VM_DRYFIRE,
	ACT_PDA_VM_IDLE_TO_LOWERED,
	ACT_PDA_VM_IDLE_LOWERED,
	ACT_PDA_VM_LOWERED_TO_IDLE,
	ACT_ENGINEER_PDA1_VM_DRAW,
	ACT_ENGINEER_PDA2_VM_DRAW,
	ACT_ENGINEER_BLD_VM_DRAW,
	ACT_ENGINEER_PDA1_VM_IDLE,
	ACT_ENGINEER_PDA2_VM_IDLE,
	ACT_ENGINEER_BLD_VM_IDLE,
	ACT_ITEM1_VM_DRAW,
	ACT_ITEM1_VM_HOLSTER,
	ACT_ITEM1_VM_IDLE,
	ACT_ITEM1_VM_IDLE_2,
	ACT_ITEM1_VM_PULLBACK,
	ACT_ITEM1_VM_PRIMARYATTACK,
	ACT_ITEM1_VM_SECONDARYATTACK,
	ACT_ITEM1_VM_RELOAD,
	ACT_ITEM1_VM_DRYFIRE,
	ACT_ITEM1_VM_IDLE_TO_LOWERED,
	ACT_ITEM1_VM_IDLE_LOWERED,
	ACT_ITEM1_VM_LOWERED_TO_IDLE,
	ACT_ITEM1_RELOAD_START,
	ACT_ITEM1_RELOAD_FINISH,
	ACT_ITEM1_VM_HITCENTER,
	ACT_ITEM1_VM_SWINGHARD,
	ACT_ITEM1_BACKSTAB_VM_UP,
	ACT_ITEM1_BACKSTAB_VM_DOWN,
	ACT_ITEM1_BACKSTAB_VM_IDLE,
	ACT_MELEE_VM_ITEM1_STUN,
	ACT_ITEM2_VM_DRAW,
	ACT_ITEM2_VM_HOLSTER,
	ACT_ITEM2_VM_IDLE,
	ACT_ITEM2_VM_PULLBACK,
	ACT_ITEM2_VM_PRIMARYATTACK,
	ACT_ITEM2_VM_SECONDARYATTACK,
	ACT_ITEM2_VM_RELOAD,
	ACT_ITEM2_VM_DRYFIRE,
	ACT_ITEM2_VM_IDLE_TO_LOWERED,
	ACT_ITEM2_VM_IDLE_LOWERED,
	ACT_ITEM2_VM_LOWERED_TO_IDLE,
	ACT_ITEM2_VM_CHARGE,
	ACT_ITEM2_VM_IDLE_2,
	ACT_ITEM2_VM_IDLE_3,
	ACT_ITEM2_VM_CHARGE_IDLE_3,
	ACT_ITEM2_VM_HITCENTER,
	ACT_ITEM2_VM_SWINGHARD,
	ACT_ITEM2_BACKSTAB_VM_UP,
	ACT_ITEM2_BACKSTAB_VM_DOWN,
	ACT_ITEM2_BACKSTAB_VM_IDLE,
	ACT_MELEE_VM_ITEM2_STUN,
	ACT_ITEM3_VM_DRAW,
	ACT_ITEM3_VM_HOLSTER,
	ACT_ITEM3_VM_IDLE,
	ACT_ITEM3_VM_PULLBACK,
	ACT_ITEM3_VM_PRIMARYATTACK,
	ACT_ITEM3_VM_SECONDARYATTACK,
	ACT_ITEM3_VM_RELOAD,
	ACT_ITEM3_VM_DRYFIRE,
	ACT_ITEM3_VM_IDLE_TO_LOWERED,
	ACT_ITEM3_VM_IDLE_LOWERED,
	ACT_ITEM3_VM_LOWERED_TO_IDLE,
	ACT_ITEM3_VM_CHARGE,
	ACT_ITEM3_VM_IDLE_2,
	ACT_ITEM3_VM_IDLE_3,
	ACT_ITEM3_VM_CHARGE_IDLE_3,
	ACT_ITEM3_VM_HITCENTER,
	ACT_ITEM3_VM_SWINGHARD,
	ACT_SECONDARY2_VM_DRAW,
	ACT_SECONDARY2_VM_HOLSTER,
	ACT_SECONDARY2_VM_IDLE,
	ACT_SECONDARY2_VM_PULLBACK,
	ACT_SECONDARY2_VM_PRIMARYATTACK,
	ACT_SECONDARY2_VM_SECONDARY2ATTACK,
	ACT_SECONDARY2_VM_RELOAD,
	ACT_SECONDARY2_RELOAD_START,
	ACT_SECONDARY2_RELOAD_FINISH,
	ACT_SECONDARY2_VM_RELOAD2,
	ACT_SECONDARY2_VM_DRYFIRE,
	ACT_SECONDARY2_VM_IDLE_TO_LOWERED,
	ACT_SECONDARY2_VM_IDLE_LOWERED,
	ACT_SECONDARY2_VM_LOWERED_TO_IDLE,
	ACT_BACKSTAB_VM_UP,
	ACT_BACKSTAB_VM_DOWN,
	ACT_BACKSTAB_VM_IDLE,
	ACT_PRIMARY_ATTACK_STAND_PREFIRE,
	ACT_PRIMARY_ATTACK_STAND_POSTFIRE,
	ACT_PRIMARY_ATTACK_STAND_STARTFIRE,
	ACT_PRIMARY_ATTACK_CROUCH_PREFIRE,
	ACT_PRIMARY_ATTACK_CROUCH_POSTFIRE,
	ACT_PRIMARY_ATTACK_SWIM_PREFIRE,
	ACT_PRIMARY_ATTACK_SWIM_POSTFIRE,
	ACT_SECONDARY_ATTACK_STAND_PREFIRE,
	ACT_SECONDARY_ATTACK_STAND_POSTFIRE,
	ACT_SECONDARY_ATTACK_STAND_STARTFIRE,
	ACT_SECONDARY_ATTACK_CROUCH_PREFIRE,
	ACT_SECONDARY_ATTACK_CROUCH_POSTFIRE,
	ACT_SECONDARY_ATTACK_SWIM_PREFIRE,
	ACT_SECONDARY_ATTACK_SWIM_POSTFIRE,
	ACT_MELEE_ATTACK_STAND_PREFIRE,
	ACT_MELEE_ATTACK_STAND_POSTFIRE,
	ACT_MELEE_ATTACK_STAND_STARTFIRE,
	ACT_MELEE_ATTACK_CROUCH_PREFIRE,
	ACT_MELEE_ATTACK_CROUCH_POSTFIRE,
	ACT_MELEE_ATTACK_SWIM_PREFIRE,
	ACT_MELEE_ATTACK_SWIM_POSTFIRE,
	ACT_ITEM1_ATTACK_STAND_PREFIRE,
	ACT_ITEM1_ATTACK_STAND_POSTFIRE,
	ACT_ITEM1_ATTACK_STAND_STARTFIRE,
	ACT_ITEM1_ATTACK_CROUCH_PREFIRE,
	ACT_ITEM1_ATTACK_CROUCH_POSTFIRE,
	ACT_ITEM1_ATTACK_SWIM_PREFIRE,
	ACT_ITEM1_ATTACK_SWIM_POSTFIRE,
	ACT_ITEM2_ATTACK_STAND_PREFIRE,
	ACT_ITEM2_ATTACK_STAND_POSTFIRE,
	ACT_ITEM2_ATTACK_STAND_STARTFIRE,
	ACT_ITEM2_ATTACK_CROUCH_PREFIRE,
	ACT_ITEM2_ATTACK_CROUCH_POSTFIRE,
	ACT_ITEM2_ATTACK_SWIM_PREFIRE,
	ACT_ITEM2_ATTACK_SWIM_POSTFIRE,
	ACT_MP_STAND_MELEE_ALLCLASS,
	ACT_MP_CROUCH_MELEE_ALLCLASS,
	ACT_MP_RUN_MELEE_ALLCLASS,
	ACT_MP_WALK_MELEE_ALLCLASS,
	ACT_MP_AIRWALK_MELEE_ALLCLASS,
	ACT_MP_CROUCHWALK_MELEE_ALLCLASS,
	ACT_MP_JUMP_MELEE_ALLCLASS,
	ACT_MP_JUMP_START_MELEE_ALLCLASS,
	ACT_MP_JUMP_FLOAT_MELEE_ALLCLASS,
	ACT_MP_JUMP_LAND_MELEE_ALLCLASS,
	ACT_MP_SWIM_MELEE_ALLCLASS,
	ACT_MP_ATTACK_STAND_MELEE_ALLCLASS,
	ACT_MP_ATTACK_STAND_MELEE_SECONDARY_ALLCLASS,
	ACT_MP_ATTACK_CROUCH_MELEE_ALLCLASS,
	ACT_MP_ATTACK_CROUCH_MELEE_SECONDARY_ALLCLASS,
	ACT_MP_ATTACK_SWIM_MELEE_ALLCLASS,
	ACT_MP_ATTACK_AIRWALK_MELEE_ALLCLASS,
	ACT_MELEE_ALLCLASS_VM_DRAW,
	ACT_MELEE_ALLCLASS_VM_HOLSTER,
	ACT_MELEE_ALLCLASS_VM_IDLE,
	ACT_MELEE_ALLCLASS_VM_PULLBACK,
	ACT_MELEE_ALLCLASS_VM_PRIMARYATTACK,
	ACT_MELEE_ALLCLASS_VM_SECONDARYATTACK,
	ACT_MELEE_ALLCLASS_VM_RELOAD,
	ACT_MELEE_ALLCLASS_VM_DRYFIRE,
	ACT_MELEE_ALLCLASS_VM_IDLE_TO_LOWERED,
	ACT_MELEE_ALLCLASS_VM_IDLE_LOWERED,
	ACT_MELEE_ALLCLASS_VM_LOWERED_TO_IDLE,
	ACT_MELEE_ALLCLASS_VM_STUN,
	ACT_MELEE_ALLCLASS_VM_HITCENTER,
	ACT_MELEE_ALLCLASS_VM_SWINGHARD,
	ACT_MP_STAND_BOMB,
	ACT_MP_JUMP_START_BOMB,
	ACT_MP_JUMP_FLOAT_BOMB,
	ACT_MP_JUMP_LAND_BOMB,
	ACT_MP_RUN_BOMB,
	ACT_MP_SWIM_BOMB,
	ACT_VM_DRAW_QRL,
	ACT_VM_IDLE_QRL,
	ACT_VM_PULLBACK_QRL,
	ACT_VM_PRIMARYATTACK_QRL,
	ACT_VM_RELOAD_QRL,
	ACT_VM_RELOAD_START_QRL,
	ACT_VM_RELOAD_FINISH_QRL,
	ACT_MP_RELOAD_STAND_PRIMARY3,
	ACT_MP_RELOAD_CROUCH_PRIMARY3,
	ACT_MP_RELOAD_AIRWALK_PRIMARY3,
	ACT_MP_RELOAD_STAND_PRIMARY3_LOOP,
	ACT_MP_RELOAD_CROUCH_PRIMARY3_LOOP,
	ACT_MP_RELOAD_AIRWALK_PRIMARY3_LOOP,
	ACT_MP_RELOAD_STAND_PRIMARY3_END,
	ACT_MP_RELOAD_CROUCH_PRIMARY3_END,
	ACT_MP_RELOAD_AIRWALK_PRIMARY3_END,
	ACT_MP_RELOAD_SWIM_PRIMARY3,
	ACT_MP_THROW,
	ACT_THROWABLE_VM_DRAW,
	ACT_THROWABLE_VM_IDLE,
	ACT_THROWABLE_VM_FIRE,
	ACT_SPELL_VM_DRAW,
	ACT_SPELL_VM_IDLE,
	ACT_SPELL_VM_ARM,
	ACT_SPELL_VM_FIRE,
	ACT_BREADSAPPER_VM_DRAW,
	ACT_BREADSAPPER_VM_IDLE,
	ACT_BREADGLOVES_VM_HITLEFT,
	ACT_BREADGLOVES_VM_HITRIGHT,
	ACT_BREADGLOVES_VM_SWINGHARD,
	ACT_BREADGLOVES_VM_IDLE,
	ACT_BREADGLOVES_VM_DRAW,
	ACT_BREADMONSTER_GLOVES_IDLE,
	ACT_BREADMONSTER_GLOVES_HITRIGHT,
	ACT_BREADMONSTER_GLOVES_HITUP,
	ACT_BREADMONSTER_VM_DRAW,
	ACT_BREADMONSTER_VM_IDLE,
	ACT_BREADMONSTER_VM_PRIMARYATTACK,
	ACT_PARACHUTE_DEPLOY,
	ACT_PARACHUTE_DEPLOY_IDLE,
	ACT_PARACHUTE_RETRACT,
	ACT_PARACHUTE_RETRACT_IDLE,
	ACT_BOT_SPAWN,
	ACT_BOT_PANIC,
	ACT_BOT_PRIMARY_MOVEMENT,
	ACT_BOT_GESTURE_FLINCH,
	ACT_BOT_PANIC_START,
	ACT_BOT_PANIC_END,
	ACT_ENGINEER_REVOLVER_DRAW,
	ACT_ENGINEER_REVOLVER_IDLE,
	ACT_ENGINEER_REVOLVER_PRIMARYATTACK,
	ACT_ENGINEER_REVOLVER_RELOAD,
	ACT_KART_IDLE,
	ACT_KART_ACTION_SHOOT,
	ACT_KART_ACTION_DASH,
	ACT_KART_JUMP_START,
	ACT_KART_JUMP_FLOAT,
	ACT_KART_JUMP_LAND,
	ACT_KART_IMPACT,
	ACT_KART_IMPACT_BIG,
	ACT_KART_GESTURE_POSITIVE,
	ACT_KART_GESTURE_NEGATIVE,
	ACT_GRAPPLE_DRAW,
	ACT_GRAPPLE_IDLE,
	ACT_GRAPPLE_FIRE_START,
	ACT_GRAPPLE_FIRE_IDLE,
	ACT_GRAPPLE_PULL_START,
	ACT_GRAPPLE_PULL_IDLE,
	ACT_GRAPPLE_PULL_END,
	ACT_PRIMARY_VM_INSPECT_START,
	ACT_PRIMARY_VM_INSPECT_IDLE,
	ACT_PRIMARY_VM_INSPECT_END,
	ACT_SECONDARY_VM_INSPECT_START,
	ACT_SECONDARY_VM_INSPECT_IDLE,
	ACT_SECONDARY_VM_INSPECT_END,
	ACT_MELEE_VM_INSPECT_START,
	ACT_MELEE_VM_INSPECT_IDLE,
	ACT_MELEE_VM_INSPECT_END,
	ACT_BALL_VM_CATCH,
	ACT_BALL_VM_PICKUP,
	ACT_BALL_VM_IDLE,
	ACT_BALL_VM_THROW_START,
	ACT_BALL_VM_THROW_LOOP,
	ACT_BALL_VM_THROW_END,
	ACT_MP_COMPETITIVE_LOSERSTATE,
	ACT_MP_COMPETITIVE_WINNERSTATE,
	ACT_SECONDARY_VM_ALTATTACK,
	ACT_MP_PUSH_STAND_SECONDARY,
	ACT_MP_PUSH_CROUCH_SECONDARY,
	ACT_MP_PUSH_SWIM_SECONDARY,
	LAST_SHARED_ACTIVITY,
};

template<class T, int minValue, int maxValue, int startValue>
class CRangeCheckedVar
{
public:
	T m_Val{};
};

class C_AnimationLayer
{
public:
	CRangeCheckedVar<int, -1, 65535, 0>	m_nSequence{};
	CRangeCheckedVar<float, -2, 2, 0> m_flPrevCycle{};
	CRangeCheckedVar<float, -5, 5, 0> m_flWeight{};
	int m_nOrder{};
	CRangeCheckedVar<float, -50, 50, 1> m_flPlaybackRate{};
	CRangeCheckedVar<float, -2, 2, 0> m_flCycle{};
	float m_flLayerAnimtime{};
	float m_flLayerFadeOuttime{};
	float m_flBlendIn{};
	float m_flBlendOut{};
	bool m_bClientBlend{};
};

struct GestureSlot_t
{
	int m_iGestureSlot{};
	Activity m_iActivity{};
	bool m_bAutoKill{};
	bool m_bActive{};
	C_AnimationLayer *m_pAnimLayer{};
};

struct MultiPlayerPoseData_t
{
	int m_iMoveX{};
	int m_iMoveY{};
	int m_iAimYaw{};
	int m_iAimPitch{};
	int m_iBodyHeight{};
	int m_iMoveYaw{};
	int m_iMoveScale{};
	float m_flEstimateYaw{};
	float m_flLastAimTurnTime{};
};

struct DebugPlayerAnimData_t
{
	float m_flSpeed{};
	float m_flAimPitch{};
	float m_flAimYaw{};
	float m_flBodyHeight{};
	Vector2D m_vecMoveYaw{};
};

struct MultiPlayerMovementData_t
{
	float m_flWalkSpeed{};
	float m_flRunSpeed{};
	float m_flSprintSpeed{};
	float m_flBodyYawRate{};
};

class IInterpolatedVar
{
private:
	void *vtable{};
};

template<typename Type, bool IS_ARRAY>
class CInterpolatedVarArrayBase : public IInterpolatedVar
{
};

template<typename Type>
class CInterpolatedVar : public CInterpolatedVarArrayBase<Type, false>
{
public:
	float *m_pValue{};
	float *m_pElements{};
	unsigned short m_maxElement{};
	unsigned short m_firstElement{};
	unsigned short m_count{};
	unsigned short m_growSize{};
	float *m_LastNetworkedValue{};
	float m_LastNetworkedTime{};
	byte m_fType{};
	byte m_nMaxCount{};
	byte *m_bLooping{};
	float m_InterpolationAmount{};
	const char *m_pDebugName{};
	bool m_bDebug{};
};

enum LegAnimType_t
{
	LEGANIM_9WAY,
	LEGANIM_8WAY,
	LEGANIM_GOLDSRC
};

class CMultiPlayerAnimState;
class CTFPlayerAnimState;

#define TF_ROCKET_RADIUS 146
#define TICK_NEVER_THINK (-1)

enum thinkmethods_t
{
	THINK_FIRE_ALL_FUNCTIONS,
	THINK_FIRE_BASE_ONLY,
	THINK_FIRE_ALL_BUT_BASE,
};

enum RifleTypes_t
{
	RIFLE_NORMAL = 0,
	RIFLE_JARATE,
	RIFLE_MACHINA,
	RIFLE_CLASSIC,
};

#define TF_WEAPON_SNIPERRIFLE_CHARGE_PER_SEC 50.0f
#define TF_WEAPON_SNIPERRIFLE_UNCHARGE_PER_SEC 75.0f
#define	TF_WEAPON_SNIPERRIFLE_DAMAGE_MIN 50.0f
#define TF_WEAPON_SNIPERRIFLE_DAMAGE_MAX 150.0f
#define TF_WEAPON_SNIPERRIFLE_RELOAD_TIME 1.5f
#define TF_WEAPON_SNIPERRIFLE_ZOOM_TIME 0.3f
#define TF_WEAPON_SNIPERRIFLE_NO_CRIT_AFTER_ZOOM_TIME 0.2f

class IGameEvent
{
public:
	virtual ~IGameEvent() {};
	virtual const char *GetName() const = 0;
	virtual bool IsReliable() const = 0;
	virtual bool IsLocal() const = 0;
	virtual bool IsEmpty(const char *keyName = nullptr) = 0;
	virtual bool GetBool(const char *keyName = nullptr, bool defaultValue = false) = 0;
	virtual int GetInt(const char *keyName = nullptr, int defaultValue = 0) = 0;
	virtual float GetFloat(const char *keyName = nullptr, float defaultValue = 0.0f) = 0;
	virtual const char *GetString(const char *keyName = nullptr, const char *defaultValue = "") = 0;
	virtual void SetBool(const char *keyName, bool value) = 0;
	virtual void SetInt(const char *keyName, int value) = 0;
	virtual void SetFloat(const char *keyName, float value) = 0;
	virtual void SetString(const char *keyName, const char *value) = 0;
};

enum Sampler_t
{
	SHADER_SAMPLER0 = 0,
	SHADER_SAMPLER1,
	SHADER_SAMPLER2,
	SHADER_SAMPLER3,
	SHADER_SAMPLER4,
	SHADER_SAMPLER5,
	SHADER_SAMPLER6,
	SHADER_SAMPLER7,
	SHADER_SAMPLER8,
	SHADER_SAMPLER9,
	SHADER_SAMPLER10,
	SHADER_SAMPLER11,
	SHADER_SAMPLER12,
	SHADER_SAMPLER13,
	SHADER_SAMPLER14,
	SHADER_SAMPLER15,
};

enum StandardTextureId_t
{
	TEXTURE_LIGHTMAP = 0,
	TEXTURE_LIGHTMAP_FULLBRIGHT,
	TEXTURE_LIGHTMAP_BUMPED,
	TEXTURE_LIGHTMAP_BUMPED_FULLBRIGHT,
	TEXTURE_WHITE,
	TEXTURE_BLACK,
	TEXTURE_GREY,
	TEXTURE_GREY_ALPHA_ZERO,
	TEXTURE_NORMALMAP_FLAT,
	TEXTURE_NORMALIZATION_CUBEMAP,
	TEXTURE_NORMALIZATION_CUBEMAP_SIGNED,
	TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0,
	TEXTURE_FRAME_BUFFER_FULL_TEXTURE_1,
	TEXTURE_COLOR_CORRECTION_VOLUME_0,
	TEXTURE_COLOR_CORRECTION_VOLUME_1,
	TEXTURE_COLOR_CORRECTION_VOLUME_2,
	TEXTURE_COLOR_CORRECTION_VOLUME_3,
	TEXTURE_FRAME_BUFFER_ALIAS,
	TEXTURE_SHADOW_NOISE_2D,
	TEXTURE_MORPH_ACCUMULATOR,
	TEXTURE_MORPH_WEIGHTS,
	TEXTURE_FRAME_BUFFER_FULL_DEPTH,
	TEXTURE_IDENTITY_LIGHTWARP,
	TEXTURE_DEBUG_LUXELS,
	TEXTURE_MAX_STD_TEXTURES = 32
};

class CHudTexture;

#define MAX_SHOOT_SOUNDS 16
#define MAX_WEAPON_STRING 80
#define MAX_WEAPON_PREFIX 16
#define MAX_WEAPON_AMMO_NAME 32

typedef enum
{
	EMPTY,
	SINGLE,
	SINGLE_NPC,
	WPN_DOUBLE,
	DOUBLE_NPC,
	BURST,
	RELOAD,
	RELOAD_NPC,
	MELEE_MISS,
	MELEE_HIT,
	MELEE_HIT_WORLD,
	SPECIAL1,
	SPECIAL2,
	SPECIAL3,
	TAUNT,
	DEPLOY,
	NUM_SHOOT_SOUND_TYPES,
} WeaponSound_t;

class FileWeaponInfo_t
{
public:
	virtual void Parse(KeyValues *pKeyValuesData, const char *szWeaponName);

public:
	bool bParsedScript{};
	bool bLoadedHudElements{};
	char szClassName[MAX_WEAPON_STRING]{};
	char szPrintName[MAX_WEAPON_STRING]{};
	char szViewModel[MAX_WEAPON_STRING]{};
	char szWorldModel[MAX_WEAPON_STRING]{};
	char szAnimationPrefix[MAX_WEAPON_PREFIX]{};
	int iSlot{};
	int iPosition{};
	int iMaxClip1{};
	int iMaxClip2{};
	int iDefaultClip1{};
	int iDefaultClip2{};
	int iWeight{};
	int iRumbleEffect{};
	bool bAutoSwitchTo{};
	bool bAutoSwitchFrom{};
	int iFlags{};
	char szAmmo1[MAX_WEAPON_AMMO_NAME]{};
	char szAmmo2[MAX_WEAPON_AMMO_NAME]{};
	char aShootSounds[NUM_SHOOT_SOUND_TYPES][MAX_WEAPON_STRING]{};
	int iAmmoType{};
	int iAmmo2Type{};
	bool m_bMeleeWeapon{};
	bool m_bBuiltRightHanded{};
	bool m_bAllowFlipping{};
	int iSpriteCount{};
	CHudTexture *iconActive{};
	CHudTexture *iconInactive{};
	CHudTexture *iconAmmo{};
	CHudTexture *iconAmmo2{};
	CHudTexture *iconCrosshair{};
	CHudTexture *iconAutoaim{};
	CHudTexture *iconZoomedCrosshair{};
	CHudTexture *iconZoomedAutoaim{};
	CHudTexture *iconSmall{};
	bool bShowUsageHint{};
};

struct WeaponData_t
{
	int m_nDamage{};
	int m_nBulletsPerShot{};
	float m_flRange{};
	float m_flSpread{};
	float m_flPunchAngle{};
	float m_flTimeFireDelay{};
	float m_flTimeIdle{};
	float m_flTimeIdleEmpty{};
	float m_flTimeReloadStart{};
	float m_flTimeReload{};
	bool m_bDrawCrosshair{};
	int m_iProjectile{};
	int m_iAmmoPerShot{};
	float m_flProjectileSpeed{};
	float m_flSmackDelay{};
	bool m_bUseRapidFireCrits{};
};

class CTFWeaponInfo : public FileWeaponInfo_t
{
public:
	virtual void Parse(KeyValues *pKeyValuesData, const char *szWeaponName);

public:
	WeaponData_t const &GetWeaponData(int iWeapon) const
	{
		return m_WeaponData[iWeapon];
	}

public:
	WeaponData_t m_WeaponData[2]{};
	int	m_iWeaponType{};
	bool m_bGrenade{};
	float m_flDamageRadius{};
	float m_flPrimerTime{};
	bool m_bLowerWeapon{};
	bool m_bSuppressGrenTimer{};
	bool m_bHasTeamSkins_Viewmodel{};
	bool m_bHasTeamSkins_Worldmodel{};
	char m_szMuzzleFlashModel[128]{};
	float m_flMuzzleFlashModelDuration{};
	char m_szMuzzleFlashParticleEffect[128]{};
	char m_szTracerEffect[128]{};
	bool m_bDoInstantEjectBrass{};
	char m_szBrassModel[128]{};
	char m_szExplosionSound[128]{};
	char m_szExplosionEffect[128]{};
	char m_szExplosionPlayerEffect[128]{};
	char m_szExplosionWaterEffect[128]{};
	bool m_bDontDrop{};
};

enum
{
	TF_WPN_TYPE_PRIMARY = 0,
	TF_WPN_TYPE_SECONDARY,
	TF_WPN_TYPE_MELEE,
	TF_WPN_TYPE_GRENADE,
	TF_WPN_TYPE_BUILDING,
	TF_WPN_TYPE_PDA,
	TF_WPN_TYPE_ITEM1,
	TF_WPN_TYPE_ITEM2,
	TF_WPN_TYPE_HEAD,
	TF_WPN_TYPE_MISC,
	TF_WPN_TYPE_MELEE_ALLCLASS,
	TF_WPN_TYPE_SECONDARY2,
	TF_WPN_TYPE_PRIMARY2,
	TF_WPN_TYPE_COUNT,
};

#define TF_DAMAGE_CRIT_CHANCE 0.02f
#define TF_DAMAGE_CRIT_CHANCE_RAPID 0.02f
#define TF_DAMAGE_CRIT_DURATION_RAPID 2.0f
#define TF_DAMAGE_CRIT_CHANCE_MELEE 0.15f

#define TF_DAMAGE_CRITMOD_MAXTIME 20.0f
#define TF_DAMAGE_CRITMOD_MINTIME 2.0f
#define TF_DAMAGE_CRITMOD_DAMAGE 800.0f
#define TF_DAMAGE_CRITMOD_MAXMULT 6.0f

#define TF_DAMAGE_CRIT_MULTIPLIER 3.0f
#define TF_DAMAGE_MINICRIT_MULTIPLIER 1.35f

#define WEAPON_RANDOM_RANGE 10000

#define MULTIPLAYER_BACKUP 90

enum gamerules_roundstate_t
{
	GR_STATE_INIT = 0,
	GR_STATE_PREGAME,
	GR_STATE_STARTGAME,
	GR_STATE_PREROUND,
	GR_STATE_RND_RUNNING,
	GR_STATE_TEAM_WIN,
	GR_STATE_RESTART,
	GR_STATE_STALEMATE,
	GR_STATE_GAME_OVER,
	GR_STATE_BONUS,
	GR_STATE_BETWEEN_RNDS,
	GR_NUM_ROUND_STATES
};

struct BeamTrail_t
{
	BeamTrail_t *next{};
	float die{};
	Vector org{};
	Vector vel{};
};

struct Beam_t
{
	unsigned char CDefaultClientRenderable[8]{};
	unsigned __int16 m_hRenderHandle{};
	Vector m_Mins{};
	Vector m_Maxs{};
	int *m_queryHandleHalo{};
	float m_haloProxySize{};
	Beam_t *next{};
	int type{};
	int flags{};
	int numAttachments{};
	Vector attachment[10]{};
	Vector delta{};
	float t{};
	float freq{};
	float die{};
	float width{};
	float endWidth{};
	float fadeLength{};
	float amplitude{};
	float life{};
	float r{};
	float g{};
	float b{};
	float brightness{};
	float speed{};
	float frameRate{};
	float frame{};
	int segments{};
	unsigned int entity[10]{};
	int attachmentIndex[10]{};
	int modelIndex{};
	int haloIndex{};
	float haloScale{};
	int frameCount{};
	float rgbNoise[129]{};
	BeamTrail_t *trail{};
	float start_radius{};
	float end_radius{};
	bool m_bCalculatedNoise{};
	float m_flHDRColorScale{};
};

enum
{
	FBEAM_STARTENTITY = 0x00000001,
	FBEAM_ENDENTITY = 0x00000002,
	FBEAM_FADEIN = 0x00000004,
	FBEAM_FADEOUT = 0x00000008,
	FBEAM_SINENOISE = 0x00000010,
	FBEAM_SOLID = 0x00000020,
	FBEAM_SHADEIN = 0x00000040,
	FBEAM_SHADEOUT = 0x00000080,
	FBEAM_ONLYNOISEONCE = 0x00000100,		// Only calculate our noise once
	FBEAM_NOTILE = 0x00000200,
	FBEAM_USE_HITBOXES = 0x00000400,		// Attachment indices represent hitbox indices instead when this is set.
	FBEAM_STARTVISIBLE = 0x00000800,		// Has this client actually seen this beam's start entity yet?
	FBEAM_ENDVISIBLE = 0x00001000,		// Has this client actually seen this beam's end entity yet?
	FBEAM_ISACTIVE = 0x00002000,
	FBEAM_FOREVER = 0x00004000,
	FBEAM_HALOBEAM = 0x00008000,		// When drawing a beam with a halo, don't ignore the segments and endwidth
	FBEAM_REVERSED = 0x00010000,
	NUM_BEAM_FLAGS = 17	// KEEP THIS UPDATED!
};

struct BeamInfo_t
{
	int m_nType{};
	void *m_pStartEnt{};
	int m_nStartAttachment{};
	void *m_pEndEnt{};
	int m_nEndAttachment{};
	Vector m_vecStart{};
	Vector m_vecEnd{};
	int m_nModelIndex{};
	const char *m_pszModelName{};
	int m_nHaloIndex{};
	const char *m_pszHaloName{};
	float m_flHaloScale{};
	float m_flLife{};
	float m_flWidth{};
	float m_flEndWidth{};
	float m_flFadeLength{};
	float m_flAmplitude{};
	float m_flBrightness{};
	float m_flSpeed{};
	int m_nStartFrame{};
	float m_flFrameRate{};
	float m_flRed{};
	float m_flGreen{};
	float m_flBlue{};
	bool m_bRenderable{};
	int m_nSegments{};
	int m_nFlags{};
	Vector m_vecCenter{};
	float m_flStartRadius{};
	float m_flEndRadius{};

	BeamInfo_t()
	{
		m_nType = 0;
		m_nSegments = -1;
		m_pszModelName = NULL;
		m_pszHaloName = NULL;
		m_nModelIndex = -1;
		m_nHaloIndex = -1;
		m_bRenderable = true;
		m_nFlags = 0;
	}
};

struct FireBulletsInfo_t
{
	int m_iShots{};
	Vector m_vecSrc{};
	Vector m_vecDirShooting{};
	Vector m_vecSpread{};
	float m_flDistance{};
	int m_iAmmoType{};
	int m_iTracerFreq{};
	float m_flDamage{};
	int m_iPlayerDamage{};
	int m_nFlags{};
	float m_flDamageForceScale{};
	C_BaseEntity *m_pAttacker{};
	C_BaseEntity *m_pAdditionalIgnoreEnt{};
	bool m_bPrimaryAttack{};
	bool m_bUseServerRandomSeed{};
};

enum ETFDmgCustom
{
	TF_DMG_CUSTOM_NONE = 0,
	TF_DMG_CUSTOM_HEADSHOT,
	TF_DMG_CUSTOM_BACKSTAB,
	TF_DMG_CUSTOM_BURNING,
	TF_DMG_WRENCH_FIX,
	TF_DMG_CUSTOM_MINIGUN,
	TF_DMG_CUSTOM_SUICIDE,
	TF_DMG_CUSTOM_TAUNTATK_HADOUKEN,
	TF_DMG_CUSTOM_BURNING_FLARE,
	TF_DMG_CUSTOM_TAUNTATK_HIGH_NOON,
	TF_DMG_CUSTOM_TAUNTATK_GRAND_SLAM,
	TF_DMG_CUSTOM_PENETRATE_MY_TEAM,
	TF_DMG_CUSTOM_PENETRATE_ALL_PLAYERS,
	TF_DMG_CUSTOM_TAUNTATK_FENCING,
	TF_DMG_CUSTOM_PENETRATE_NONBURNING_TEAMMATE,
	TF_DMG_CUSTOM_TAUNTATK_ARROW_STAB,
	TF_DMG_CUSTOM_TELEFRAG,
	TF_DMG_CUSTOM_BURNING_ARROW,
	TF_DMG_CUSTOM_FLYINGBURN,
	TF_DMG_CUSTOM_PUMPKIN_BOMB,
	TF_DMG_CUSTOM_DECAPITATION,
	TF_DMG_CUSTOM_TAUNTATK_GRENADE,
	TF_DMG_CUSTOM_BASEBALL,
	TF_DMG_CUSTOM_CHARGE_IMPACT,
	TF_DMG_CUSTOM_TAUNTATK_BARBARIAN_SWING,
	TF_DMG_CUSTOM_AIR_STICKY_BURST,
	TF_DMG_CUSTOM_DEFENSIVE_STICKY,
	TF_DMG_CUSTOM_PICKAXE,
	TF_DMG_CUSTOM_ROCKET_DIRECTHIT,
	TF_DMG_CUSTOM_TAUNTATK_UBERSLICE,
	TF_DMG_CUSTOM_PLAYER_SENTRY,
	TF_DMG_CUSTOM_STANDARD_STICKY,
	TF_DMG_CUSTOM_SHOTGUN_REVENGE_CRIT,
	TF_DMG_CUSTOM_TAUNTATK_ENGINEER_GUITAR_SMASH,
	TF_DMG_CUSTOM_BLEEDING,
	TF_DMG_CUSTOM_GOLD_WRENCH,
	TF_DMG_CUSTOM_CARRIED_BUILDING,
	TF_DMG_CUSTOM_COMBO_PUNCH,
	TF_DMG_CUSTOM_TAUNTATK_ENGINEER_ARM_KIL,
	TF_DMG_CUSTOM_FISH_KILL,
	TF_DMG_CUSTOM_TRIGGER_HURT,
	TF_DMG_CUSTOM_DECAPITATION_BOSS,
	TF_DMG_CUSTOM_STICKBOMB_EXPLOSION,
	TF_DMG_CUSTOM_AEGIS_ROUND,
	TF_DMG_CUSTOM_FLARE_EXPLOSION,
	TF_DMG_CUSTOM_BOOTS_STOMP,
	TF_DMG_CUSTOM_PLASMA,
	TF_DMG_CUSTOM_PLASMA_CHARGED,
	TF_DMG_CUSTOM_PLASMA_GIB,
	TF_DMG_CUSTOM_PRACTICE_STICKY,
	TF_DMG_CUSTOM_EYEBALL_ROCKET,
	TF_DMG_CUSTOM_HEADSHOT_DECAPITATION,
	TF_DMG_CUSTOM_TAUNTATK_ARMAGEDDON,
	TF_DMG_CUSTOM_FLARE_PELLET,
	TF_DMG_CUSTOM_CLEAVER,
	TF_DMG_CUSTOM_CLEAVER_CRIT,
	TF_DMG_CUSTOM_SAPPER_RECORDER_DEATH,
	TF_DMG_CUSTOM_MERASMUS_PLAYER_BOMB,
	TF_DMG_CUSTOM_MERASMUS_GRENADE,
	TF_DMG_CUSTOM_MERASMUS_ZAP,
	TF_DMG_CUSTOM_MERASMUS_DECAPITATION,
	TF_DMG_CUSTOM_CANNONBALL_PUSH,
	TF_DMG_CUSTOM_TAUNTATK_ALLCLASS_GUITAR_RIFF,
	TF_DMG_CUSTOM_THROWABLE,
	TF_DMG_CUSTOM_THROWABLE_KILL,
	TF_DMG_CUSTOM_SPELL_TELEPORT,
	TF_DMG_CUSTOM_SPELL_SKELETON,
	TF_DMG_CUSTOM_SPELL_MIRV,
	TF_DMG_CUSTOM_SPELL_METEOR,
	TF_DMG_CUSTOM_SPELL_LIGHTNING,
	TF_DMG_CUSTOM_SPELL_FIREBALL,
	TF_DMG_CUSTOM_SPELL_MONOCULUS,
	TF_DMG_CUSTOM_SPELL_BLASTJUMP,
	TF_DMG_CUSTOM_SPELL_BATS,
	TF_DMG_CUSTOM_SPELL_TINY,
	TF_DMG_CUSTOM_KART,
	TF_DMG_CUSTOM_GIANT_HAMMER,
	TF_DMG_CUSTOM_RUNE_REFLECT,
	TF_DMG_CUSTOM_DRAGONS_FURY_IGNITE,
	TF_DMG_CUSTOM_DRAGONS_FURY_BONUS_BURNING,
	TF_DMG_CUSTOM_SLAP_KILL,
	TF_DMG_CUSTOM_CROC,
	TF_DMG_CUSTOM_TAUNTATK_GASBLAST,
	TF_DMG_CUSTOM_AXTINGUISHER_BOOSTED,
	TF_DMG_CUSTOM_KRAMPUS_MELEE,
	TF_DMG_CUSTOM_KRAMPUS_RANGED
};

typedef enum _fieldtypes
{
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions

	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *

	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)

	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)

	// NOTE: Use float arrays for local transformations that don't need to be fixed up.
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)

	FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX,		// a model index
	FIELD_MATERIALINDEX,	// a material index (using the material precache string table)

	FIELD_VECTOR2D,			// 2 floats

	FIELD_TYPECOUNT,		// MUST BE LAST
} fieldtype_t;

struct typedescription_t;
class ISaveRestoreOps;
struct datamap_t;

enum
{
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,

	// Must be last
	TD_OFFSET_COUNT,
};

#define FTYPEDESC_GLOBAL			0x0001		// This field is masked for global entity save/restore
#define FTYPEDESC_SAVE				0x0002		// This field is saved to disk
#define FTYPEDESC_KEY				0x0004		// This field can be requested and written to by string name at load time
#define FTYPEDESC_INPUT				0x0008		// This field can be written to by string name at run time, and a function called
#define FTYPEDESC_OUTPUT			0x0010		// This field propogates it's value to all targets whenever it changes
#define FTYPEDESC_FUNCTIONTABLE		0x0020		// This is a table entry for a member function pointer
#define FTYPEDESC_PTR				0x0040		// This field is a pointer, not an embedded object
#define FTYPEDESC_OVERRIDE			0x0080		// The field is an override for one in a base class (only used by prediction system for now)

// Flags used by other systems (e.g., prediction system)
#define FTYPEDESC_INSENDTABLE		0x0100		// This field is present in a network SendTable
#define FTYPEDESC_PRIVATE			0x0200		// The field is local to the client or server only (not referenced by prediction code and not replicated by networking)
#define FTYPEDESC_NOERRORCHECK		0x0400		// The field is part of the prediction typedescription, but doesn't get compared when checking for errors

#define FTYPEDESC_MODELINDEX		0x0800		// The field is a model index (used for debugging output)

#define FTYPEDESC_INDEX				0x1000		// The field is an index into file data, used for byteswapping. 

// These flags apply to C_BasePlayer derived objects only
#define FTYPEDESC_VIEW_OTHER_PLAYER		0x2000		// By default you can only view fields on the local player (yourself), 
													//   but if this is set, then we allow you to see fields on other players
#define FTYPEDESC_VIEW_OWN_TEAM			0x4000		// Only show this data if the player is on the same team as the local player
#define FTYPEDESC_VIEW_NEVER			0x8000		// Never show this field to anyone, even the local player (unusual)

#define TD_MSECTOLERANCE		0.001f		// This is a FIELD_FLOAT and should only be checked to be within 0.001 of the networked info

struct typedescription_t
{
	fieldtype_t			fieldType;
	const char *fieldName;
	int					fieldOffset[TD_OFFSET_COUNT]; // 0 == normal, 1 == packed offset
	unsigned short		fieldSize;
	short				flags;
	// the name of the variable in the map/fgd data, or the name of the action
	const char *externalName;
	// pointer to the function set for save/restoring of custom data types
	ISaveRestoreOps *pSaveRestoreOps;
	// for associating function with string names
	uint8_t *inputFunc;
	// For embedding additional datatables inside this one
	datamap_t *td;

	// Stores the actual member variable size in bytes
	int					fieldSizeInBytes;

	// FTYPEDESC_OVERRIDE point to first baseclass instance if chains_validated has occurred
	struct typedescription_t *override_field;

	// Used to track exclusion of baseclass fields
	int					override_count;

	// Tolerance for field errors for float fields
	float				fieldTolerance;
};


//-----------------------------------------------------------------------------
// Purpose: stores the list of objects in the hierarchy
//			used to iterate through an object's data descriptions
//-----------------------------------------------------------------------------
struct datamap_t
{
	typedescription_t *dataDesc;
	int					dataNumFields;
	char const *dataClassName;
	datamap_t *baseMap;

	bool				chains_validated;
	// Have the "packed" offsets been computed
	bool				packed_offsets_computed;
	int					packed_size;
};

enum EMatchGroup
{
	k_nMatchGroup_Invalid = -1,
	k_nMatchGroup_First = 0,
	k_nMatchGroup_MvM_Practice = 0,
	k_nMatchGroup_MvM_MannUp,
	k_nMatchGroup_Ladder_6v6,
	k_nMatchGroup_Ladder_9v9,
	k_nMatchGroup_Ladder_12v12,
	k_nMatchGroup_Casual_6v6,
	k_nMatchGroup_Casual_9v9,
	k_nMatchGroup_Casual_12v12,
	k_nMatchGroup_Count,
};

#define TEAM_ANY				-2
#define	TEAM_INVALID			-1
#define TEAM_UNASSIGNED			0
#define TEAM_SPECTATOR			1
#define LAST_SHARED_TEAM		TEAM_SPECTATOR

#define TF_TEAM_AUTOASSIGN (TF_TEAM_COUNT + 1 )
#define TF_TEAM_HALLOWEEN	TF_TEAM_AUTOASSIGN
#define TF_TEAM_PVE_INVADERS	TF_TEAM_BLU
#define TF_TEAM_PVE_DEFENDERS	TF_TEAM_RED
#define TF_TEAM_PVE_INVADERS_GIANTS 4

enum MM_PlayerConnectionState_t
{
	MM_DISCONNECTED = 0,
	MM_CONNECTED,
	MM_CONNECTING, // the server knows that this player is coming
	MM_LOADING, // loading into the server
	MM_WAITING_FOR_PLAYER
};

enum
{
	kFootstepSoundSet_Default = 0,
	kFootstepSoundSet_SoccerCleats = 1,
	kFootstepSoundSet_HeavyGiant = 2,
	kFootstepSoundSet_SoldierGiant = 3,
	kFootstepSoundSet_DemoGiant = 4,
	kFootstepSoundSet_ScoutGiant = 5,
	kFootstepSoundSet_PyroGiant = 6,
	kFootstepSoundSet_SentryBuster = 7,
	kFootstepSoundSet_TreasureChest = 8,
	kFootstepSoundSet_Octopus = 9,
};

#define PC_DATA_PACKED			true
#define PC_DATA_NORMAL			false

typedef void (*FN_FIELD_COMPARE)(const char *classname, const char *fieldname, const char *fieldtype,
								 bool networked, bool noterrorchecked, bool differs, bool withintolerance, const char *value);

class CPredictionCopy
{
public:
	int				m_nType{};
	void *m_pDest{};
	void const *m_pSrc{};
	int				m_nDestOffsetIndex{};
	int				m_nSrcOffsetIndex{};


	bool			m_bErrorCheck{};
	bool			m_bReportErrors{};
	bool			m_bDescribeFields{};
	typedescription_t *m_pCurrentField{};
	char const *m_pCurrentClassName{};
	datamap_t *m_pCurrentMap{};
	bool			m_bShouldReport{};
	bool			m_bShouldDescribe{};
	int				m_nErrorCount{};
	bool			m_bPerformCopy{};

	FN_FIELD_COMPARE	m_FieldCompareFunc{};

	typedescription_t *m_pWatchField{};
	char const *m_pOperation{};
	void *unk{}; //ida tweaking?
public:
	CPredictionCopy(int type, void *dest, bool dest_packed, void const *src, bool src_packed,
					bool counterrors = false, bool reporterrors = false, bool performcopy = true,
					bool describefields = false, FN_FIELD_COMPARE func = nullptr)
	{
		m_nType = type;
		m_pDest = dest;
		m_pSrc = src;
		m_nDestOffsetIndex = dest_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
		m_nSrcOffsetIndex = src_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
		m_bErrorCheck = counterrors;
		m_bReportErrors = reporterrors;
		m_bPerformCopy = performcopy;
		m_bDescribeFields = describefields;

		m_pCurrentField = NULL;
		m_pCurrentMap = NULL;
		m_pCurrentClassName = NULL;
		m_bShouldReport = false;
		m_bShouldDescribe = false;
		m_nErrorCount = 0;

		m_FieldCompareFunc = func;
	}

	int TransferData(const char *operation, int entindex, datamap_t *dmap)
	{
		return s::CPredictionCopy_TransferData.call<int>(this, operation, entindex, dmap);
	}
};

enum ButtonCode_t;

enum IterationRetval_t
{
	ITERATION_CONTINUE = 0,
	ITERATION_STOP,
};

typedef int SpatialPartitionListMask_t;

class IPartitionEnumerator
{
public:
	virtual IterationRetval_t EnumElement(IHandleEntity *pHandleEntity) = 0;
};

enum
{
	PARTITION_ENGINE_SOLID_EDICTS = (1 << 0),		// every edict_t that isn't SOLID_TRIGGER or SOLID_NOT (and static props)
	PARTITION_ENGINE_TRIGGER_EDICTS = (1 << 1),		// every edict_t that IS SOLID_TRIGGER
	PARTITION_CLIENT_SOLID_EDICTS = (1 << 2),
	PARTITION_CLIENT_RESPONSIVE_EDICTS = (1 << 3),		// these are client-side only objects that respond to being forces, etc.
	PARTITION_ENGINE_NON_STATIC_EDICTS = (1 << 4),		// everything in solid & trigger except the static props, includes SOLID_NOTs
	PARTITION_CLIENT_STATIC_PROPS = (1 << 5),
	PARTITION_ENGINE_STATIC_PROPS = (1 << 6),
	PARTITION_CLIENT_NON_STATIC_EDICTS = (1 << 7),		// everything except the static props
};

// make this a fixed size so it just sits on the stack
#define MAX_SPHERE_QUERY	256
class CEntitySphereQuery
{
public:
	// currently this builds the list in the constructor
	// UNDONE: make an iterative query of ISpatialPartition so we could
	// make queries like this optimal
	CEntitySphereQuery(const Vector &center, float radius, int flagMask = 0, int partitionMask = PARTITION_CLIENT_NON_STATIC_EDICTS);
	C_BaseEntity *GetCurrentEntity();
	inline void NextEntity() { m_listIndex++; }

private:
	int			m_listIndex{};
	int			m_listCount{};
	C_BaseEntity *m_pList[MAX_SPHERE_QUERY]{};
};

typedef bool (*ShouldHitFunc_t)(IClientEntity *pHandleEntity, int contentsMask);

// this is limited by the network fractional bits used for coords
// because net coords will be only be accurate to 5 bits fractional
// Standard collision test epsilon
// 1/32nd inch collision epsilon
#define DIST_EPSILON (0.03125)

#define	EQUAL_EPSILON	0.001 

enum KeyValuesPreloadType_t
{
	TYPE_VMT,
	TYPE_SOUNDEMITTER,
	TYPE_SOUNDSCAPE,
	NUM_PRELOAD_TYPES
};

class CEconItemAttribute
{
private:
	void *m_pad{};
public:

	unsigned int m_iAttributeDefinitionIndex;

	union
	{
		int m_iRawValue32;
		float m_flValue;
	};
	int m_nRefundableCurrency{};

	CEconItemAttribute(uint16_t iAttributeDefinitionIndex, float flValue)
	{
		m_iAttributeDefinitionIndex = iAttributeDefinitionIndex;
		m_flValue = flValue;
	}
};

class CAttributeList
{
private:
	void *m_pad{};

public:
	CUtlVector<CEconItemAttribute, CUtlMemory<CEconItemAttribute>> m_Attributes{};
	void *m_pManager{};

public:
	inline void SetAttribute(int index, float value)
	{
		void *item_schema{ s::GetItemSchema.call<void *>() };

		if (!item_schema) {
			return;
		}

		void *attribute_definition{ s::CEconItemSchema_GetAttributeDefinition.call<void *>(item_schema, index) };

		if (!attribute_definition) {
			return;
		}

		s::CAttributeList_SetRuntimeAttributeValue.call<void>(this, attribute_definition, value);
	};
};

namespace attributes {
	constexpr uint16_t paintkit_proto_def_index = 834;
	constexpr uint16_t custom_paintkit_seed_lo = 866;
	constexpr uint16_t custom_paintkit_seed_hi = 867;
	constexpr uint16_t has_team_color_paintkit = 745;
	constexpr uint16_t set_item_texture_wear = 725;
	constexpr uint16_t weapon_allow_inspect = 731;
	constexpr uint16_t set_attached_particle_static = 370;
	constexpr uint16_t set_attached_particle = 134;
	constexpr uint16_t is_festivized = 2053;
	constexpr uint16_t is_australium_item = 2027;
	constexpr uint16_t loot_rarity = 2022;
	constexpr uint16_t item_style_override = 542;
	constexpr uint16_t set_turn_to_gold = 150;
	constexpr uint16_t killstreak_tier = 2025;
	constexpr uint16_t killstreak_effect = 2013;
	constexpr uint16_t killstreak_idleeffect = 2014;
	constexpr uint16_t halloween_pumpkin_explosions = 1007;
	constexpr uint16_t halloween_green_flames = 1008;
	constexpr uint16_t halloween_voice_modulation = 1006;
	constexpr uint16_t add_jingle_to_footsteps = 364;
	constexpr uint16_t set_custom_buildmenu = 295;
}

namespace weapon_unusual_effects {
	constexpr int weapon_unusual_hot = 701;
	constexpr int weapon_unusual_isotope = 702;
	constexpr int weapon_unusual_cool = 703;
	constexpr int weapon_unusual_energyorb = 704;
}

enum EAccountType
{
	k_EAccountTypeInvalid = 0,
	k_EAccountTypeIndividual = 1,
	k_EAccountTypeMultiseat = 2,
	k_EAccountTypeGameServer = 3,
	k_EAccountTypeAnonGameServer = 4,
	k_EAccountTypePending = 5,
	k_EAccountTypeContentServer = 6,
	k_EAccountTypeClan = 7,
	k_EAccountTypeChat = 8,
	k_EAccountTypeConsoleUser = 9,
	k_EAccountTypeAnonUser = 10,
	k_EAccountTypeMax
};

enum EUniverse
{
	k_EUniverseInvalid = 0,
	k_EUniversePublic = 1,
	k_EUniverseBeta = 2,
	k_EUniverseInternal = 3,
	k_EUniverseDev = 4,
	k_EUniverseMax
};

class CSteamID
{
public:
	CSteamID()
	{
		m_steamid.m_comp.m_unAccountID = 0;
		m_steamid.m_comp.m_EAccountType = k_EAccountTypeInvalid;
		m_steamid.m_comp.m_EUniverse = k_EUniverseInvalid;
		m_steamid.m_comp.m_unAccountInstance = 0;
	}

	CSteamID(uint32_t unAccountID, EUniverse eUniverse, EAccountType eAccountType)
	{
		Set(unAccountID, eUniverse, eAccountType);
	}

	CSteamID(const char *id64)
	{
		m_steamid.m_unAll64Bits = std::stoull(id64);
	}

	void Set(uint32_t unAccountID, EUniverse eUniverse, EAccountType eAccountType)
	{
		m_steamid.m_comp.m_unAccountID = unAccountID;
		m_steamid.m_comp.m_EUniverse = eUniverse;
		m_steamid.m_comp.m_EAccountType = eAccountType;

		if (eAccountType == k_EAccountTypeClan)
		{
			m_steamid.m_comp.m_unAccountInstance = 0;
		}
		else
		{
			m_steamid.m_comp.m_unAccountInstance = 1; // k_unSteamUserDesktopInstance
		}
	}

	uint64_t ConvertToUint64() const
	{
		return m_steamid.m_unAll64Bits;
	}

public:
	union SteamID_t
	{
		struct SteamIDComponent_t
		{

			uint32_t m_unAccountID : 32;
			unsigned int m_unAccountInstance : 20;
			unsigned int m_EAccountType : 4;	
			EUniverse m_EUniverse : 8;
		} m_comp;

		uint64_t m_unAll64Bits;
	} m_steamid;
};

typedef struct wrect_s
{
	int	left;
	int right;
	int top;
	int bottom;
} wrect_t;

class CHudTexture
{
public:
	virtual ~CHudTexture();

	int Width() const
	{
		return rc.right - rc.left;
	}

	int Height() const
	{
		return rc.bottom - rc.top;
	}

	char		szShortName[64];
	char		szTextureFile[64];

	bool		bRenderUsingFont;
	bool		bPrecached;
	char		cCharacterInFont;
	HFont hFont;

	int			textureId;
	float		texCoords[4];

	wrect_t		rc;
};

enum ETFFlagType
{
	TF_FLAGTYPE_CTF = 0,
	TF_FLAGTYPE_ATTACK_DEFEND,
	TF_FLAGTYPE_TERRITORY_CONTROL,
	TF_FLAGTYPE_INVADE,
	TF_FLAGTYPE_RESOURCE_CONTROL,
	TF_FLAGTYPE_ROBOT_DESTRUCTION,
	TF_FLAGTYPE_PLAYER_DESTRUCTION

	//
	// ADD NEW ITEMS HERE TO AVOID BREAKING DEMOS
	//
};

#define TF_FLAGINFO_HOME		0
#define TF_FLAGINFO_STOLEN		(1<<0)
#define TF_FLAGINFO_DROPPED		(1<<1)

enum ETFFlagEventTypes
{
	TF_FLAGEVENT_PICKUP = 1,
	TF_FLAGEVENT_CAPTURE,
	TF_FLAGEVENT_DEFEND,
	TF_FLAGEVENT_DROPPED,
	TF_FLAGEVENT_RETURNED,
	TF_NUM_FLAG_EVENTS
};

enum ETFPartyChatType
{
	k_eTFPartyChatType_Invalid = 0,
	k_eTFPartyChatType_MemberChat = 1,
	k_eTFPartyChatType_Synthetic_MemberJoin = 1000,
	k_eTFPartyChatType_Synthetic_MemberLeave = 1001,
	k_eTFPartyChatType_Synthetic_SendFailed = 1002,
	k_eTFPartyChatType_Synthetic_MemberOnline = 1003,
	k_eTFPartyChatType_Synthetic_MemberOffline = 1004,
};