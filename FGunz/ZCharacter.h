#pragma once

class ZItem
{
public:

};

struct ZCharacterProperty_CharClanName
{
	char		szName[MATCHOBJECT_NAME_LENGTH];
	char		szClanName[CLAN_NAME_LENGTH];
};

struct ZUserAndClanName
{
	char m_szUserName[MATCHOBJECT_NAME_LENGTH];
	char m_szUserAndClanName[MATCHOBJECT_NAME_LENGTH];
};

struct ZCharacterProperty
{
	MProtectValue<ZCharacterProperty_CharClanName> nameCharClan;
	MMatchSex	nSex;
	int			nHair;
	int			nFace;
	int			nLevel;
	MProtectValue<float>		fMaxHP;
	MProtectValue<float>		fMaxAP;

	DWORD GetMask()
	{
		DWORD dwThis = (DWORD)this;
		return *(DWORD*)(*(DWORD*)(dwThis + 0x10) - (dwThis + 0x10) - 0xD6);
	}

	const char* GetName()
	{
		DWORD dwThis = (DWORD)this;
		return *(const char**)(*(DWORD*)(dwThis + 0) - (dwThis + 0) - 0xD6) - GetMask();
	}
};

class ZCharacterItem
{
public:
	char _pad[0x1C];
	int a;
};

class ZModule_Movable
{
public:
	char _pad_0x00[0x38];   	// 0x00
	D3DXVECTOR3 m_Velocity;		// 0x38
};

class ZObject
{
public:
	char _pad[0x14];							// 0x0000
	ZModule_Movable* m_pModule_Movable;			// 0x0014 (Maybe)
	char _pad_0x0018[0x7E4];					// 0x0018

	RVisualMesh* m_pVMesh;		// 0x07F8
	rvector m_Direction;		// 0x07FC

	float m_fSpawnTime;			// 0x0203

	bool m_bVisible;			// 0x0808

	MUID m_UID;					// 0x0814
	bool m_bIsNPC;				// 0x081C
	bool m_bInitialized;		// 0x081D
};

enum ZDELAYEDWORK
{
	ZDW_SHOT,
	ZDW_UPPERCUT,
	ZDW_DASH,
	ZDW_SLASH,
};

enum ZSTUNTYPE 
{
	ZST_NONE = -1,
	ZST_DAMAGE1 = 0,
	ZST_DAMAGE2,
	ZST_SLASH,			// 강베기 스턴
	ZST_BLOCKED,		// 칼 막혔을때 스턴
	ZST_LIGHTNING,		// 인챈트중 Lightning
	ZST_LOOP,			// 스킬중 root 속성
};

struct ZCharaterStatusBitPacking 
{
	union
	{
		struct
		{
			bool	m_bLand : 1;				// 지금 발을 땅에 대고있는지..
			bool	m_bWallJump : 1;			// 벽점프 중인지
			bool	m_bJumpUp : 1;			// 점프올라가는중
			bool	m_bJumpDown : 1;			// 내려가는중
			bool	m_bWallJump2 : 1;			// 이건 walljump 후에 착지시 두번째 튕겨져 나오는 점프..
			bool	m_bTumble : 1;			// 덤블링 중
			bool	m_bBlast : 1;				// 띄워짐당할때 ( 올라갈때 )
			bool	m_bBlastFall : 1;			// 띄워져서 떨어질때
			bool	m_bBlastDrop : 1;			// 떨어지다 땅에 튕길때
			bool	m_bBlastStand : 1;		// 일어날때
			bool	m_bBlastAirmove : 1;		// 공중회전후 착지
			bool	m_bSpMotion : 1;
			bool	m_bCommander : 1;			///< 대장
												//	bool	m_bCharging:1;			// 힘모으고 있는중
												//	bool	m_bCharged:1;			// 힘모인상태
			bool	m_bLostConEffect : 1;		// 네트웍 응답이 없을때 머리에 뜨는 이펙트가 나와야 하는지.
			bool	m_bChatEffect : 1;		// 채팅시 머리에 뜨는 이펙트가 나와야 하는지.
			bool	m_bBackMoving : 1;		// 뒤로 이동할때

			bool	m_bAdminHide : 1;					///< admin hide 되어있는지..
			bool	m_bDie : 1;						///< 죽었는지 체크
			bool	m_bStylishShoted : 1;				///< 마지막으로 쏜게 스타일리쉬 했는지 체크
			bool	m_bFallingToNarak : 1;			///< 나락으로 떨어지고 있는지 여부
			bool	m_bStun : 1;						///< stun ..움직일수없게된상태.
			bool	m_bDamaged : 1;					///< 데미지 여부

			bool	m_bPlayDone : 1;				// 애니메이션 플레이가 다 되었는지. 다음동작으로 넘어가는 기준
			bool	m_bPlayDone_upper : 1;		// 상체 애니메이션 플레이가 다 되었는지. 다음동작으로 넘어가는 기준
			bool	m_bIsLowModel : 1;
			bool	m_bTagger : 1;					///< 술래
			bool	m_bSniping : 1;				// Custom: Snipers
		};

		DWORD dwFlagsPublic;
	};
};

class ZCharacter : public ZObject
{
public:
	virtual void Function0();
	virtual void Function1();
	virtual void Function2();
	virtual void Function3();
	virtual void Function4();
	virtual void Function5();
	virtual void Function6();
	virtual void Function7();
	virtual void Function8();
	virtual void Function9();
	virtual void Function10();
	virtual void Function11();
	virtual void Function12();
	virtual void Function13();
	virtual void Function14();
	virtual bool IsDie();
	virtual void SetDirection(D3DXVECTOR3& dir); // 16
	virtual bool IsGuard(); // 17
	virtual MMatchTeam GetTeamID(); // 18 (Reversing: "Your team (%s) has captured the enemy flag!")
	virtual void Function19();
	virtual void Function20();
	virtual void Function21();
	virtual void OnBlast(rvector& dir); // 22
	virtual void Function23();
	virtual void Function24();
	virtual void Function25();
	virtual void OnKnockback(rvector& dir, float fForce); // 26
	virtual void Function27();
	virtual void OnDamagedSkill(ZObject* pAttacker, D3DXVECTOR3 srcPos, /*ZDAMAGETYPE*/int damageType, /*MMatchWeaponType*/int weaponType, float fDamage, float fPiercingRatio, int nMeleeType); // 28
	virtual void Function29();
	virtual void Function30();
	virtual void Function31();
	virtual void Function32();
	virtual void UpdateSound(); // 33
	virtual void Function34();
	virtual void Function35();
	virtual void Function36();
	virtual void SetAnimation(char *AnimationName, bool bEnableCancel, int time); // 37
	virtual void UpdateSpeed(); // 38
	virtual void Function39();
	virtual void Function40();
	virtual void UpdateHeight(float fDelta); // 41
	virtual void UpdateMotion(float fDelta); // 42
	virtual void Function43();
	virtual void Function44();

	// It doesn't set the SetInvincibleTime, so when pressed u because inivisible (unlink the Revival function)
	virtual void InitStatus(); // 45 

public:
	//char _pad[0x223];
	//ZCharacterProperty m_Property;								// 0x0223

	//MProtectValue<ZUserAndClanName>* m_pMUserAndClanName;		// 0x022E
	char _pad[0x022E];
	char m_szUserName[MATCHOBJECT_NAME_LENGTH]; // 0x022E

public:
	// "^%d%s^9 has committed suicide."
	const char* GetName()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x88C);
		DWORD dwB = DWORD(dwThis + 0x88C);
		dwA -= dwB;
		return (const char*)(dwA - 0xD6);
	}

	ZCharaterStatusBitPacking GetZCharacterStatusBitPacking()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x9CC);
		DWORD dwB = DWORD(dwThis + 0x9CC);
		dwA -= dwB;
		//return *(ZCharaterStatusBitPacking*)(dwA - 0xD6);
		return *(ZCharaterStatusBitPacking*)(dwA - 0xD3);
	}

	//  55 8B EC 83 E4 F8 83 EC 14 53 56 57 8B F9 E8
	void InitModuleStatus(void)
	{
		typedef void*(__thiscall* InitModuleStatusT)(ZCharacter* pChar);
		InitModuleStatusT InitModuleStatusF = (InitModuleStatusT)0x4F1480;
		InitModuleStatusF(this);
	}

	// 83 EC 08 56 57 6A 08
	void AddDelayedWork(float fTime, ZDELAYEDWORK nWork)
	{
		typedef void*(__thiscall* AddDelayedWorkT)(ZCharacter* pChar, float fTime, ZDELAYEDWORK nWork);
		AddDelayedWorkT AddDelayedWorkF = (AddDelayedWorkT)0x505E40;
		AddDelayedWorkF(this, fTime, nWork);
	}

public:
	float ZCharacter::GetHP()
	{
		typedef float (__thiscall* GetHPT)(ZCharacter* pMyChar);
		GetHPT GetHPF = (GetHPT)OFFSET_GetHP;
		return GetHPF(this);
	}

	float ZCharacter::GetAP()
	{
		typedef float(__thiscall* GetAPT)(ZCharacter* pMyChar);
		GetAPT GetAPF = (GetAPT)OFFSET_GetAP;
		return GetAPF(this);
	}

	ZCharacterItem* GetItems()
	{
		return (ZCharacterItem*)(*(DWORD*)this + 0x7F0);
	}

	DWORD GetCommandInt()
	{
		return *(DWORD*)this + 0x7F0;
	}

	ZSTUNTYPE GetStunType()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x8C0);
		DWORD dwB = DWORD(dwThis + 0x8C0);
		dwA -= dwB;
		return *(ZSTUNTYPE*)(dwA - 0xD2);
	}

	void SetStunType(ZSTUNTYPE type)
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x8C0);
		DWORD dwB = DWORD(dwThis + 0x8C0);
		dwA -= dwB;
		memset((ZSTUNTYPE*)(dwA - 0xD2), type, sizeof(DWORD));
	}

	// Not really working
	MTD_CharInfo* GetCharInfo()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x22B);
		DWORD dwB = DWORD(dwThis + 0x22B);
		dwA -= dwB;
		return (MTD_CharInfo*)(dwA - 0xD6);
	}

	// Noting working
	ZItem* GetSelectedWeapon()
	{
		typedef ZItem* (__thiscall* GetSelectedWeaponT)(ZCharacter* pChar, DWORD* _this_0x1c);
		GetSelectedWeaponT GetSelectedWeaponF = (GetSelectedWeaponT)0x4F7D20;
		return GetSelectedWeaponF(this, (DWORD*)this + 0x1C);
	}

	// Noting working
	void* RenameTest(DWORD* ptr)
	{
		typedef void* (__thiscall* _T)(DWORD* ptr);
		_T _F = (_T)0x4F7070;
		return _F(ptr);
	}

	// GetTeamID also in: v2 = *(*(v4 + 0x5C) + 0x9CC) - (*(v4 + 0x5C) + 0x9CC);

	/*void SetInvincibleTime(int nDuration)
	{
		// 51 56 8B F1 FF 15
		typedef void(__thiscall* SetInvincibleTimeType)(ZCharacter* pCharacter, int nDuration);
		SetInvincibleTimeType SetInvincibleTimeF = (SetInvincibleTimeType)0x4F6180; // v7 0x4DABA0
		SetInvincibleTimeF(this, nDuration);
	}*/


	// Works but crash ;/
	void Revival()
	{
		// 53 8B D9 55 56 57 80 BB ? ? ? ? ? 0F 84 ? ? ? ? A1
		typedef void* (__thiscall* onRevivalT)(ZCharacter* ZChar);
		onRevivalT onRevival = (onRevivalT)0x4F0871; // v7 0x4D58F0
		onRevival(this);

		//typedef void* (__thiscall* RevivalT)(ZCharacter* pMyCharacter);
		//RevivalT RevivalF = (RevivalT)ZCharacter_Revival;
		//RevivalF(this);
	}

	void SetPosition(D3DXVECTOR3& pos) // Even with this function, server checks the positon and gives you IP ban
	{
		// 83 EC 0C 56 57 8D 44 24 08 8B F9 50 8D 44 24 10 89 7C 24 0C 50 B9 ? ? ? ? E8 ? ? ? ? 8B 44 24 0C 3B 05 ? ? ? ? 74 05 8B 70 14 EB 02 33 F6 8B CF E8 ? ? ? ? 3B C6 74 06 FF 15 ? ? ? ? 8B 17 8B CF 8B 44 24 18 2B D7 F3 0F 7E 00
		typedef void* (__thiscall* T)(ZCharacter* pChar, D3DXVECTOR3& pos);
		T F = (T)0x41D650;
		F(this + 0x1FD, pos);
	}

	void SetInvincibleTime(int nDuration)
	{
		// 51 56 8B F1 FF 15
		typedef void* (__thiscall* T)(ZCharacter* pChar, int nDuration);
		T F = (T)0x4F6180;
		F(this, nDuration);
	}
};

struct ZMyCharaterStatusBitPacking
{
	union {
		struct {
			bool	m_bWallHang : 1;		// 벽에 매달린 상태일때 true

			bool	m_bLimitJump : 1;
			bool	m_bLimitTumble : 1;
			bool	m_bLimitWall : 1;

			bool	m_bMoveLimit : 1;
			bool	m_bMoving : 1;

			bool	m_bReleasedJump : 1;				// 점프를 한번 하면 떼었다가 점프를 해야한다.
			bool	m_bJumpQueued : 1;
			bool	m_bWallJumpQueued : 1;
			bool	m_bHangSuccess : 1;
			bool	m_bSniferMode : 1;

			bool	m_bEnterCharge : 1;				// 첫번째 칼질이후 계속 누르고있었는지.

			bool	m_bJumpShot : 1;
			bool	m_bShot : 1;						// 칼질중일때.
			bool	m_bShotReturn : 1;				// 칼질하다 idle 되돌아오는 동작중

			bool	m_bSkill : 1;						// 여러가지 오른쪽 스킬
			bool	m_b1ShotSended : 1;				// 첫번째 칼질은 딜레이가 있다

													//		bool	m_bSplashShot:1;				// 마지막 스플래시 칼질 해야 하는지..
			bool	m_bGuard : 1;
			bool	m_bGuardBlock_ret : 1;
			bool	m_bGuardStart : 1;
			bool	m_bGuardCancel : 1;
			bool	m_bGuardKey : 1;
			bool	m_bGuardByKey : 1;
			bool	m_bDrop : 1;
			bool	m_bSlash : 1;				// 힘모아 베기
			bool	m_bJumpSlash : 1;			// 공중에서 힘모아 베기
			bool	m_bJumpSlashLanding : 1;	// 이후 착지동작
			bool	m_bReserveDashAttacked : 1;

			bool	m_bLButtonPressed : 1;			// LButton 눌려져있는지.
			bool	m_bLButtonFirstPressed : 1;		// LButton 이 처음으로 눌려진건지
			bool	m_bLButtonQueued : 1;				// LButton 예약되어있는지

			bool	m_bRButtonPressed : 1;
			bool	m_bRButtonFirstPressed : 1;		// RButton 이 처음으로 눌려진건지
			bool	m_bRButtonFirstReleased : 1;		// RButton 이 눌려져있다가 떼졌을때
		};

		DWORD dwFlags[2];	// 2005.12.9 현재 35개
	}; // 패킹 끝.
};

class ZMyCharacter : public ZCharacter
{
public:
	void ShotBlocked()
	{
		// 83 EC 10 8B D1 C7 44 24
		typedef void* (__thiscall* T)(ZCharacter* pChar);
		T F = (T)0x5068B0;
		F(this);
	}

	void Discharged()
	{
		// A1 ? ? ? ? 83 EC 08 56 8B F1 85 C0 74 0D 83 38 00 74 08 8B 0D ? ? ? ? EB 02 33 C9 6A 0B 81 C1 ? ? ? ? E8 ? ? ? ? 84 C0 75 59
		typedef void* (__thiscall* T)(ZCharacter* pChar);
		T F = (T)0x506BE0;
		F(this);
	}

	void ChargedShot()
	{
		// 83 EC 0C 56 57 8B F9 C7 44 24
		typedef void* (__thiscall* T)(ZCharacter* pChar);
		T F = (T)0x506630;
		F(this);
	}

	void JumpChargedShot()
	{
		// 83 EC 10 56 57 8B F9 C6 44 24
		typedef void* (__thiscall* T)(ZCharacter* pChar);
		T F = (T)0x506720;
		F(this);
	}

	ZMyCharaterStatusBitPacking GetZMyCharacterStatusBitPacking()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x8D8);
		DWORD dwB = DWORD(dwThis + 0x8D8);
		dwA -= dwB;
		return *(ZMyCharaterStatusBitPacking*)(dwA - 0xD6);
	}
};