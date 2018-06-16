#pragma once

class ZObject;
class ZMyCharacter;

#define RM_FLAG_ADDITIVE		0x0001
#define RM_FLAG_USEOPACITY		0x0002
#define RM_FLAG_TWOSIDED		0x0004
#define RM_FLAG_NOTWALKABLE		0x0008
#define RM_FLAG_CASTSHADOW		0x0010
#define RM_FLAG_RECEIVESHADOW	0x0020
#define RM_FLAG_PASSTHROUGH		0x0040
#define RM_FLAG_HIDE			0x0080
#define RM_FLAG_PASSBULLET		0x0100
#define RM_FLAG_PASSROCKET		0x0200
#define RM_FLAG_USEALPHATEST	0x0400
#define RM_FLAG_AI_NAVIGATION	0x1000

enum ZGAME_LASTTIME
{
	ZLASTTIME_HPINFO = 0,
	ZLASTTIME_BASICINFO,
	ZLASTTIME_PEERPINGINFO,
	ZLASTTIME_SYNC_REPORT,
	ZLASTTIME_MAX
};

typedef enum _RMeshPartsType
{

	eq_parts_etc = 0,
	eq_parts_head,
	eq_parts_face,
	eq_parts_chest,
	eq_parts_hands,
	eq_parts_legs,
	eq_parts_feet,
	eq_parts_sunglass,

	// left weapon

	eq_parts_left_pistol,//왼손더미는 권총과 SMG 뿐
	eq_parts_left_smg,
	eq_parts_left_blade,
	eq_parts_left_dagger,

	// right weapon

	eq_parts_right_katana,
	eq_parts_right_pistol,
	eq_parts_right_smg,
	eq_parts_right_shotgun,
	eq_parts_right_rifle,
	eq_parts_right_grenade,
	eq_parts_right_item,
	eq_parts_right_dagger,
	eq_parts_right_rlauncher,
	eq_parts_right_sword,
	eq_parts_right_blade,

	// etc..코드로 붙여줄것들..

	eq_parts_end,

} RMeshPartsType;

struct RPickInfo
{
	D3DXVECTOR3 vOut;
	float t;
	RMeshPartsType parts;
};

struct BSPVERTEX 
{

	float x, y, z;		// world position
						//	float nx,ny,nz;		// normal				// 지금은 의미없다
	float tu1, tv1;		// texture coordinates
	float tu2, tv2;

	rvector *Coord() { return (rvector*)&x; }
	//	rvector *Normal() { return (rvector*)&nx; }
};

struct RPOLYGONINFO 
{
	rplane	plane;
	int		nMaterial;
	int		nConvexPolygon;
	int		nLightmapTexture;
	int		nPolygonID;
	DWORD	dwFlags;

	BSPVERTEX *pVertices;
	int		nVertices;
	int		nIndicesPos;
};

struct RBSPPICKINFO 
{
	void/*RSBspNode*/* pNode;
	int nIndex;
	D3DXVECTOR3 PickPos;
	RPOLYGONINFO *pInfo;
};

struct ZPICKINFO 
{
	ZObject* pObject;
	RPickInfo info;

	bool bBspPicked;
	int nBspPicked_DebugRegister;
	RBSPPICKINFO bpi;
};

class ZGame
{
public:
	char _pad[0x2C];						// 0x00
	char _pad_0x2C[0xC];					// 0x2C
	DWORD m_nLastTime[ZLASTTIME_MAX];		// 0x38 (size= 4*4)
	char _pad_0x48[0x14];					// 0x48
	ZMyCharacter* m_pMyCharacter;			// 0x5C (TODO: Pointer not worked)

public:
	ZMyCharacter* GetMyCharacter()
	{
		return (ZMyCharacter*)*(DWORD*)(*(DWORD*)this + 0x5C);
	}

	DWORD* GetCommandData() // GetTime ?
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x2C);
		DWORD dwB = DWORD(dwThis + 0x2C);
		dwA -= dwB;
		return (DWORD*)(dwA - 0xD6);
	}

	float GetTime()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x2C);
		DWORD dwB = DWORD(dwThis + 0x2C);
		dwA -= dwB;
		return *(float*)(dwA - 0xD6);
	}

	// Not working
	void _OnExplosionGrenade(MUID uidOwner, D3DXVECTOR3 pos, float fDamage, float fRange, float fMinDamage, float fKnockBack, MMatchTeam nTeamID)
	{
		typedef void* (__thiscall* T)(ZGame* pGame, MUID uidOwner, D3DXVECTOR3 pos, float fDamage, float fRange, float fMinDamage, float fKnockBack, MMatchTeam nTeamID);
		T F = (T)OFFSET_ZGame_OnExplosionGrenade;
		F(this, uidOwner, pos, fDamage, fRange, fMinDamage, fKnockBack, nTeamID);
	}

	// Not working
	bool _PickHistory(ZObject* pOwnerObject, float fTime, const D3DXVECTOR3& origin, const D3DXVECTOR3& to, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar)
	{
		typedef bool (__thiscall* T)(ZGame* pGame, ZObject* pOwnerObject, float fTime, const D3DXVECTOR3& origin, const D3DXVECTOR3& to, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar);
		T F = (T)OFFSET_ZGame_PickHistory;
		return F(this, pOwnerObject, fTime, origin, to, pickinfo, dwPassFlag, bMyChar);
	}

	bool Pick(ZObject* pOwnerObject, rvector& origin, rvector& dir, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar)
	{
		typedef bool (__thiscall* T)(ZGame* pGame, ZObject* pOwnerObject, rvector& origin, rvector& dir, ZPICKINFO* pickinfo, DWORD dwPassFlag, bool bMyChar);
		T F = (T)OFFSET_ZGame_Pick;
		return F(this, pOwnerObject, origin, dir, pickinfo, dwPassFlag, bMyChar);
	}

	// Not working :(
	bool CheckWall(ZObject* pObj1, ZObject* pObj2, bool bCoherentToPeer)
	{
		typedef bool(__thiscall* T)(ZGame* pGame, ZObject* pObj1, ZObject* pObj2, bool bCoherentToPeer);
		T F = (T)OFFSET_ZGame_CheckWall;
		return F(this, pObj1, pObj2, bCoherentToPeer);
	}
};