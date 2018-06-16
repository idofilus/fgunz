#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <stdio.h>
//#include <D3DX10math.h>

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")

#define rvector2 D3DXVECTOR2
#define rvector D3DXVECTOR3
#define rmatrix D3DXMATRIX
#define rplane D3DXPLANE

#include "Console.h"

#include "ZCharacterObject.h"
#include "RMeshUtil.h"

#include "MUID.h"
#include "MCommandParameter.h"
//#include "MDraw.h"

#include "detours.h"
#pragma comment(lib, "detours.lib")

// Hook GetAdaptersInfo
#include <iphlpapi.h>

// Gunz Headers
#include "Gunz\MMemoryProxy.h"
#include "Gunz\MRTTI.h"

#include "Core.h"
#include "Offset.h"
#include "MTD.h"
#include "RVisualMesh.h"
#include "ZGameInterface.h"
#include "ZGame.h"
#include "ZCharacter.h"
#include "ZSoundEngine.h"
#include "ZEffectManager.h"
#include "ZApplication.h"

#include "HackBase\include\hackbase.h"
#pragma comment(lib, "HackBase\\lib\\HackBase.lib")

HackBase *mHackBase = 0;

#define MAIN_THREAD_ID 0x7645F0

#define ZCharacter_Revival 0x4D58F0 // 51 53 8B D9 55 56 57 80 BB
#define ZCharacter_SetAnimationLower 0x4D2640
#define ZCharacter_SetAnimationUpper 0x4D27B0

#define mlog_offset 0x58B610

#define ZGetMyInfoOffset 0x491100
#define GetHPOffset 0x4D56A0
#define GetAPOffset 0x4D5720
#define ZChatOutputOffset 0x464660
#define ZCombatInterfaceOffset 0x97A1BC
#define ZGameOffset 0x6EADB0 // ZApplication::m_pInstance  ZGetGame()
#define OnAnnounceOffset 0x55A8C0
#define GetPositionOffset 0x4D82E0
// 41B500 setposition
//#define ZChatOutputOffset 0x40E890

class ZGameClient
{
public:
	char _pad[0x18C];
	MUID m_uidPlayer;		// 0x18C

public:
	static ZGameClient* GetInstance()
	{
		return *(ZGameClient**)0x7645A0;
	}
};

using namespace std;

typedef void*(__cdecl* mlog_t)(const char* pFormat, ...);
mlog_t mlog = (mlog_t)mlog_offset;

typedef void(__stdcall* WHAT_T)();
WHAT_T WHAT = (WHAT_T)0x2305F42;

VOID _SetCurrentThreadId(DWORD dwThreadId)
{
	_asm
	{
		mov eax, fs:[0x18];
		mov ecx, dwThreadId;
		mov[eax + 0x24], ecx; //not sure yet if this works on al operating systems, but it does on Windows 7 x64
	}
}

#define __CALLASMAIN( call ) \
DWORD dwThreadId = GetCurrentThreadId(); \
_SetCurrentThreadId( *(DWORD*)MAIN_THREAD_ID ); \
call; \
_SetCurrentThreadId( dwThreadId );

#define CALLASMAIN( call ) \
_SetCurrentThreadId( *(DWORD*)MAIN_THREAD_ID ); \
call;

LPDWORD dwBackgroundThreadId = NULL;
LPDWORD dwRenderThreadId = NULL;
HMODULE g_hModule = NULL;
EXTERN_C IMAGE_DOS_HEADER __ImageBase; EXTERN_C IMAGE_DOS_HEADER __ImageBase;
DWORD WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpszRerversed);
bool bShutdown = false;

void ShutdownDLL()
{
	bShutdown = true;
	printf_s("Shutting down ..\n");

	FreeConsole();

	if (g_hModule)
	{
		FreeLibraryAndExitThread(g_hModule, 0);

		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCTSTR)DllMain, &g_hModule);
	}

	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)FreeLibrary, &__ImageBase, 0, NULL);
	ExitThread(0);

	if (dwBackgroundThreadId)
		TerminateThread(dwBackgroundThreadId, 0);

	if (dwRenderThreadId)
		TerminateThread(dwRenderThreadId, 0);
}

//ofstream logFile;
/*
void CreateLog()
{
	logFile.open("C:\\gunz_log.txt");
}

void ReleaseLog()
{
	logFile.close();
}

void Log(const char* text)
{
	logFile << text << endl;
}*/



class MFont
{
public:
	virtual void Function0();
	virtual void Function1();
	virtual void Function2();
	virtual int GetHeight(void);	// 3
	virtual int GetWidth(void);		// 4
};

/*class MDrawContext
{
public:
	MFont* m_pFont;			// 0x18
};*/

class MFontManager
{
public:
	// 83 EC 20 A1 ? ? ? ? 33 C4 89 44 24 18 53
	static MFont* Get(const char* szFontName)
	{
		typedef MFont*(__stdcall* T)(const char* szFontName);
		T F = (T)0x5DBA10;
		return F(szFontName);
	}
};

class MDrawContextR2
{
public:
	virtual void Function0();
	virtual void Function1();
	virtual void Function2();
	virtual void Function3();
	virtual void FillRectangleW(int x, int y, int cx, int cy); // 4
	virtual void Function5();
	virtual MFont* GetFont(); // 6
	virtual void Function7();
	virtual void Function8();
	virtual void Function9();
	virtual void Function10();
	virtual void Function11();
	virtual void Function12();
	virtual void Function13();
	virtual void Function14();
	virtual void Function15();
	virtual void Function16();
	virtual int Text(int x, int y, const char* szText);		// 17

public:
	static MDrawContextR2* GetInstance()
	{
		return *(MDrawContextR2**)0x7645DC; // g_pDC
	}

	// 8B 54 24 04 56 8B F1 57 8B 7E 18
	void SetFont(MFont* pFont)
	{
		typedef void (__thiscall* T)(MFont* pFont);
		T F = (T)0x5DCC50;
		return F(pFont);
	}
};

void ESP()
{
	//MDrawContextR2* pDC = (MDrawContextR2*)0x0; // TODO
	//pDC->Text();
}

enum GunzState
{
	GUNZ_NA = 0,
	GUNZ_GAME = 1,
	GUNZ_LOGIN = 2,
	GUNZ_DIRECTLOGIN = 3,
	GUNZ_LOBBY = 4,
	GUNZ_STAGE = 5,
	GUNZ_GREETER = 6,
	GUNZ_CHARSELECTION = 7,
	GUNZ_CHARCREATION = 8,
	GUNZ_PREVIOUS = 10,
	GUNZ_SHUTDOWN = 11,
	GUNZ_BIRDTEST
};

/*template <typename T>
class MProtectValue
{}; // TODO:*/

class Mint
{
public:
	char _pad[0x8A];
	int	m_nWorkspaceWidth;			// 0x8A
	int	m_nWorkspaceHeight;

public:
	static Mint* GetInstance()
	{
		return (Mint*)0x97A1BC;
	}
};

class ZMovingWeapon
{
public:
	D3DXVECTOR3* m_Position;	// 0x2C
};

class ZObjectCollision
{
public:
	void SetCollideable(bool b)
	{
		typedef void* (__thiscall* SetCollideableT)(ZObjectCollision* m_Collision, bool b);
		SetCollideableT SetCollideable = (SetCollideableT)0x41B750;
		SetCollideable(this, b);
	}
};

class ZModuleContainer;

class ZModule {
	friend ZModuleContainer;
protected:
	ZModuleContainer*	m_pContainer;	// �̸���� ����ִ� container

public:
	ZModule() {}
	virtual ~ZModule() {}

	void Active(bool bActive = true);	// (��)Ȱ��ȭ ��Ų��

	virtual int GetID() = 0;

	virtual bool Update(float fElapsed) { return true; }
	virtual void Destroy() {}

	virtual void InitStatus() {}

protected:
	virtual void OnAdd() {}
	virtual void OnRemove() {}
	virtual void OnActivate() {}
	virtual void OnDeactivate() {}
};

typedef map <int, ZModule *> ZMODULEMAP;

class ZModuleContainer {
	friend ZModule;

	MDeclareRootRTTI;
	ZMODULEMAP m_Modules;
	ZMODULEMAP m_ActiveModules;

public:
	virtual ~ZModuleContainer();

	// bInitialActive : Ȱ��ȭ �ʱ�ġ
	void AddModule(ZModule *pModule, bool bInitialActive = false);
	void RemoveModule(ZModule *pModule);

	// ��ϵ� ����� nID ����� ã�´�
	ZModule* GetModule(int nID);

	void ActiveModule(int nID, bool bActive = true);	// Ư�� ����� (��)Ȱ��ȭ ��Ų��
	bool IsActiveModule(int nID);

	virtual void UpdateModules(float fElapsed);
	virtual void InitModuleStatus(void);
};

class ZModule_HPAP
{
private:
public: // TODO: MAKE private
	//MProtectValue<float> m_fMaxHP;		// 0x008 ?
	//bool m_bAccumulationDamage;
	char _pad[0x8];
	MProtectValue<float> m_fMaxHP;		// 0x008 ?
	void* __pad;
	//MProtectValue<float> m_fMask1;		// 0x0
	//MProtectValue<float> m_fMask2;		// 0x04
	//MProtectValue<float> m_fMask3;		// 0x08
	//MProtectValue<float> m_fMask4;		// 0x0C
	MProtectValue<float> m_fMask;		// 0x10 ?

	float GetMask()
	{
		DWORD dwThis = (DWORD)this;
		return *(float*)(*(DWORD*)(dwThis + 16) - (dwThis + 0x10) - 214);
	}

	MProtectValue<float> GetMask2()
	{
		return m_fMask;
	}

public:

	float GetMaxHP()
	{
		DWORD dwThis = (DWORD)this;
		return *(float*)(*(DWORD*)(dwThis + 8) - (dwThis + 8) - 0xD6) - GetMask();
	}

	float GetHP()
	{
		DWORD dwThis = (DWORD)this;
		return *(float*)(*(DWORD*)(dwThis + 0x18) - (dwThis + 0x18) - 0xD6) - GetMask();
	}

	// Test: V8
	void SetHP(float fHP)
	{
		DWORD dwThis = (DWORD)this;
		*((float*)(*(DWORD*)(dwThis + 0x18) - (dwThis + 0x18) - 0xD6) - *(DWORD*)(*(DWORD*)(dwThis + 16) - (dwThis + 0x10) - 214)) = fHP;
	}

	/*float GetMaxHP2()
	{
		DWORD dwThis = (DWORD)this;
		MProtectValue<float> fMaxHP = (MProtectValue<float>)((*(DWORD*)(dwThis + 8) - (dwThis + 8) - 0xD6) - GetMask());
		return 0.0f;
	}*/
};

class ZCharacterOld
{
public:


public:
	//char _pad[0x0808];
	char _pad[0x086C];
	ZModule_HPAP* m_pModule_HPAP;			// 0x086C
	/*ZModule_QuestStatus*/void		*m_pModule_QuestStatus;             // 870
	/*ZModule_Resistance*/void		*m_pModule_Resistance; // 874
	/*ZModule_FireDamage*/void		*m_pModule_FireDamage; // 878
	/*ZModule_ColdDamage*/void		*m_pModule_ColdDamage; // 87C
	/*ZModule_PoisonDamage*/void	*m_pModule_PoisonDamage; // 880
	/*ZModule_LightningDamage*/void	*m_pModule_LightningDamage; // 884
	/*ZModule_HealOverTime*/void	*m_pModule_HealOverTime; // 888
	ZCharacterProperty					m_Property;		///88C

	//bool m_bInitialized;					// 0x081D
	//MMatchSex* nSex;						// 0x08C0

	//void* m_AniState_Lower;					// 0x092C

	//bool m_bVisible;						// 0x0808
	//ZObjectCollision* m_Collision;			// 0x0820
	//float pos1;								// 0x0814
	//float pos2;								// 0x0818
	//MProtectValue<bool>* m_bCharging;		// 0x08E0
	//bool m_bHero;							// 0x0869
	//char _pad0[0x07F4];
	//D3DXVECTOR3 position;					// 0x07F4
	//D3DXVECTOR3 m_Direction;				// 0x07FC (TODO: Test v8)
	//char _pad[0x07F8];
	//RVisualMesh** m_pVMesh;					// 0x07F8
	//float damage;							// 0x4CA0

	// Correct:
	ZCharacterItem* m_Items;				// 0x620

	// 0x80C ZObject ?

public:
	MUID GetUID()
	{
		// TODO: CHECK ? MAYBE 0x818 ?
		return *(MUID*)(*(DWORD*)((DWORD)this + 0x814));
	}

	DWORD* IntCommandRelated()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x9D4);
		DWORD dwB = DWORD(dwThis + 0x9D4);
		dwA -= dwB;
		return (DWORD*)(dwA - 0xD6);
	}

	float* GetShotTime() // V8 TESTED
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x9E0);
		DWORD dwB = DWORD(dwThis + 0x9E0);
		dwA -= dwB;
		return (float*)(dwA - 0xD6);
	}

	DWORD GetStunType()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x8C0);
		DWORD dwB = DWORD(dwThis + 0x8C0);
		dwA -= dwB;
		return *(DWORD*)(dwA - 0xD2);
	}


	void SetShotTime(float time) // MAYBE
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x9E0);
		DWORD dwB = DWORD(dwThis + 0x9E0);
		dwA -= dwB;
		*(float*)(dwA - 0xD6) = time;
	}

	void ForceDie()
	{
		typedef void* (__thiscall* ForceDieT)(ZCharacterOld* ZChar);
		ForceDieT ForceDieF = (ForceDieT)0x52DE00;
		ForceDieF(this);
	}

	bool IsDie() // TODO: Check
	{
		typedef bool* (__thiscall* IsDieT)(ZCharacterOld* pCharacter);
		IsDieT IsDieF = (IsDieT)0x4DC270;
		IsDieF(this);
	}

	// IsDie: Also can be done with: (TODO: Check)
	bool IsDie_()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x8D8);
		DWORD dwB = DWORD(dwThis + 0x8D8);
		dwA -= dwB;
		return (bool)(dwA - 0xD4);
	}


	void SetInvincibleTime(int nDuration)
	{
		// 51 56 8B F1 FF 15
		typedef void(__thiscall* SetInvincibleTimeType)(ZCharacterOld* pCharacter, int nDuration);
		SetInvincibleTimeType SetInvincibleTimeF = (SetInvincibleTimeType)0x4F6180; // v7 0x4DABA0
		SetInvincibleTimeF(this, nDuration);
	}

	void Revival()
	{
		// 53 8B D9 55 56 57 80 BB ? ? ? ? ? 0F 84 ? ? ? ? A1
		typedef void* (__thiscall* onRevivalT)(ZCharacterOld* ZChar);
		onRevivalT onRevival = (onRevivalT)0x4D58F0; // v7 0x4D58F0
		onRevival(this);

		//typedef void* (__thiscall* RevivalT)(ZCharacter* pMyCharacter);
		//RevivalT RevivalF = (RevivalT)ZCharacter_Revival;
		//RevivalF(this);
	}

	void InitStatusMaybe()
	{
		typedef void* (__thiscall* InitStatusMaybeT)(ZCharacterOld* ZChar);
		InitStatusMaybeT InitStatusMaybe = (InitStatusMaybeT)0x4107B0;
		InitStatusMaybe(this);
	}

	void SetSpawnTime(float fTime) // TODO: Check [High chance]
	{
		//  ZGame::OnPeerSpawn
		DWORD dwThis = (DWORD)this;
		DWORD dwZObject = *(DWORD*)(dwThis + 0x80C);
		DWORD dwA = *(DWORD*)(dwThis + 0x2C);
		DWORD dwB = DWORD(dwThis + 0x2C);
		dwA -= dwB;
		(void*)(dwA - 0xD6);
	}

	const char* GetUsername() // TODO: Check
	{
		/*DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x8B8);
		DWORD dwB = DWORD(dwThis + 0x8B8);
		dwA -= dwB;
		return (const char*)(dwA - 0xD6);*/

		DWORD dwCharacter = (DWORD)this;
		const char* username;

		__asm
		{
			push ebp
			mov eax, [dwCharacter + 0x8B8]
			mov ecx, [eax]
			sub ecx, eax
			sub ecx, 0xD6
			mov username, ecx
			pop ebp
		}

		return username;
	}

	MMatchTeam GetTeamID()
	{
		DWORD dwOffset = 0x8D8;
		//DWORD dwOffset = 0xACC;
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + dwOffset);
		DWORD dwB = DWORD(dwThis + dwOffset);
		dwA -= dwB;
		return (MMatchTeam)(dwA - 0xD3);
	}

	MMatchUserGradeID* GetUGradeID() // TODO: Test
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x8AC);
		DWORD dwB = DWORD(dwThis + 0x8AC);
		dwA -= dwB;
		return (MMatchUserGradeID*)(dwA - 0x18);
	}

	D3DXVECTOR3* GetPosition()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwPos1 = *(DWORD*)(dwThis + 0x7F4);
		DWORD dwPos2 = DWORD(dwThis + 0x7F4);
		dwPos1 -= dwPos2;
		return (D3DXVECTOR3*)(dwPos1 - 0xD6);
	}

	D3DXVECTOR3* GetDirection()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwPos1 = *(DWORD*)(dwThis + 0x7F4);
		DWORD dwPos2 = DWORD(dwThis + 0x7F4);
		dwPos1 -= dwPos2;
		return (D3DXVECTOR3*)(dwPos1 - 0xCE);
	}

	DWORD* GetCommandInt()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x9D4);
		DWORD dwB = DWORD(dwThis + 0x9D4);
		dwA -= dwB;
		return (DWORD*)(dwA - 0xD6);
	}

	void SetPosition(D3DXVECTOR3* position)
	{
		memcpy(this->GetPosition(), position, sizeof(D3DXVECTOR3));
	}

	void SetAnimationLower(ZC_STATE_LOWER nAni)
	{
		typedef void* (__thiscall* SetAnimationLowerT)(ZCharacterOld* pMyCharacter, ZC_STATE_LOWER nAni);
		SetAnimationLowerT SetAnimationLowerF = (SetAnimationLowerT)ZCharacter_SetAnimationLower;
		SetAnimationLowerF(this, nAni);
	}

	void SetAnimationUpper(ZC_STATE_UPPER nAni)
	{
		typedef void* (__thiscall* SetAnimationUpperT)(ZCharacterOld* pMyCharacter, ZC_STATE_UPPER nAni);
		SetAnimationUpperT SetAnimationUpperF = (SetAnimationUpperT)ZCharacter_SetAnimationUpper;
		SetAnimationUpperF(this, nAni);
	}

	RVisualMesh* GetVMesh()
	{
		return (RVisualMesh*)(*(DWORD*)this + 0x07F8);
	}

	// V8 test
	ZCharacterItem* GetItems()
	{
		return (ZCharacterItem*)(*(DWORD*)this + 0x26C);
	}

	float GetHP()
	{
		//typedef float (__thiscall* GetHPT)(ZCharacter* pMyCharacter);
		//GetHPT GetHPF = (GetHPT)0x004D56A0;
		//return GetHPF(this);

		float hp = 0;
		DWORD pChar = (DWORD)this;
		DWORD dwGetHP = 0x004D56A0;

		__asm
		{
			mov ecx, eax
			call dwGetHP
			mov hp, eax
		}

		return hp;
	}

	float GetAP()
	{
		typedef float(__thiscall* GetAPT)(ZCharacterOld* pMyCharacter);
		GetAPT GetAPF = (GetAPT)0x004D5720;
		return GetAPF(this);
	}
};

class ZCrossHair
{
public:
	char _pad[0x8];				// 0x0
	bool m_bVisible;			// 0x8
};

class ZScreenEffect
{

};

class ZResultBoardList {};

class ZCombatInterface
{
public:
	ZScreenEffect* m_pResultPanel;			// 0x2C0
	ZScreenEffect* m_pResultPanel_Team;		// 0x2C4
	ZResultBoardList m_ResultItems;			// 0x2C8 (TODO: Validate)
	ZScreenEffect* m_pResultLeft;			// 0x2D0
	ZScreenEffect* m_pResultRight;			// 0x2D4

	char _pad[0x3AC];
	ZCrossHair* m_CrossHair; // 0x3A4

public:
	ZCharacter* GetTargetCharacter()
	{
		//return (ZCharacter*)(*(DWORD*)this + 0x33C); // TODO: Test
		typedef ZCharacter* (__thiscall* T)(ZCombatInterface* pCombatInterface);
		T F = (T)0x447730; // 80 B9 ? ? ? ? ? 74 07 8B 81
		return F(this);
	}

	void SetPickTarget(bool bPick, ZCharacter* pCharacter)
	{
		typedef void* (__thiscall* SetPickTargetT)(ZCombatInterface* pCombatInterface, bool bPick, ZCharacter* pCharacter);
		SetPickTargetT SetPickTarget = (SetPickTargetT)0x4242F0;
		SetPickTarget(this, bPick, pCharacter);
	}

	void ChangeWeapon(ZChangeWeaponType nType)
	{
		typedef void (__thiscall* ChangeWeaponT)(ZCombatInterface* pCombatInterface, ZChangeWeaponType nType);
		ChangeWeaponT ChangeWeapon = (ChangeWeaponT)0x479080;
		ChangeWeapon(this, nType);
	}
};

class ZChat
{
public:

public:
};

class ZHelpScreen
{
public:

public:
};

class ZCharacterManager : public map<MUID, ZCharacter*>
{
public:

};

class ZGameOld
{
public:
	//char _0x0[0x5C];
	//ZCharacter**	m_pMyCharacter;	// 0x0005C
	//ZHelpScreen* m_HelpScreen; // 0x0064
	//ZEffectManager* pEM; // 0x02E0
	char _pad[0x70];
	ZCharacterManager m_CharacterManager; // 0x70

public:
	ZCharacter* GetMyCharacter()
	{
		return (ZCharacter*)(*(DWORD*)this + 0x5C);
	}

	DWORD* GetCommandData()
	{
		DWORD dwThis = (DWORD)this;
		DWORD dwA = *(DWORD*)(dwThis + 0x2C);
		DWORD dwB = DWORD(dwThis + 0x2C);
		dwA -= dwB;
		return (DWORD*)(dwA - 0xD6);
	}
};

class ZGameInterfaceOld
{
public:
	//char _pad[0x2430];				// 0x0000
	//GunzState m_nState;				// 0x2430
	//bool m_bWaitingArrangedGame;	// 0x2436

public:
	ZGameOld* GetGame()
	{
		return (ZGameOld*)(*(DWORD*)this + 0x2FC);
	}

	ZCombatInterface* GetCombatInterface()
	{
		return (ZCombatInterface*)(*(DWORD*)this + 0x2E8); // TODO: Test
	}

	ZChat* GetChat()
	{
		return (ZChat*)(*(DWORD*)this + 0x38C); // TODO: Test
	}

	static ZGameInterfaceOld* GetInstance()
	{
		return *(ZGameInterfaceOld**)0x764818;
	}
};

class ZRuleDuelTournament
{
public:

};

class AAA
{
public:
	char _pad[0x4CA0];
	float demage;
};

class AA
{
public:
	char _pad[0x5C];
	AAA* aaa;
};


class A
{
public:
	char _pad[0x2FC];
	AA* aa;
};

class ZMyItemList
{
public:

};

class ZMyInfo
{ 
public:
	bool bForcedChangeTeam;				// 0x0052
	//char _pad[0x57];
	//char m_szCharName[32];				// 0x0057
	//char m_szClanName[16];				// 0x005F
	char _pad0[0x68];
	BYTE m_nBP;							// 0x0068
	BYTE m_nLevel;						// 0x0069
	unsigned long int m_nXP;			// 0x006A
	//char _pad[0xEB];
	//ZMyItemList* m_ItemList;			// 0x00EB
	//char _pad[0x6B];
	//int m_nLevelPercentCache;			// 0x006B
	//char _pad[85];
	//MMatchUserGradeID* m_nUGradeID; // 85 dec

public:
	ZMyInfo* GetInstance()
	{
		return (ZMyInfo*)ZGetMyInfoOffset;
	}
};

typedef void(__thiscall* ZGame_ApplyPotionType)(ZGame* pGame, int nItemID, ZCharacter* pCharObj, float fRemainedTime);
ZGame_ApplyPotionType ZGame_ApplyPotion = (ZGame_ApplyPotionType)0x547B60;

typedef void(__thiscall* UnknonwInsideOnPeerSpawnType)(ZCharacter* pMyCharacter, void* uknown, D3DXVECTOR3* pos, ZCharacter* pCharacter);
UnknonwInsideOnPeerSpawnType UnknonwInsideOnPeerSpawn = (UnknonwInsideOnPeerSpawnType)0x4E02D0;

typedef void(__thiscall* SetInvincibleTimeType)(ZCharacter* pCharacter, int nDuration);
SetInvincibleTimeType SetInvincibleTime = (SetInvincibleTimeType)0x4DABA0;

typedef ZMyInfo* (__stdcall* ZGetMyInfoType)();
ZGetMyInfoType ZGetMyInfo = (ZGetMyInfoType)ZGetMyInfoOffset;

typedef void* (__thiscall* OnScreamType)(ZCharacter* ZChar);
OnScreamType OnScream = (OnScreamType)0x4DAA80;

typedef void* (__thiscall* onRevivalT)(ZCharacter* ZChar);
onRevivalT onRevival = (onRevivalT)0x4D58F0;

typedef float(__thiscall* ZCharGetHPT)(ZCharacter* ZChar);
ZCharGetHPT ZCharGetMaxHP = (ZCharGetHPT)0x4D55A0;

typedef float (__thiscall* ZCharGetHPT)(ZCharacter* ZChar);
ZCharGetHPT ZCharGetHP = (ZCharGetHPT)OFFSET_GetHP; //0x4D56A0;

typedef int (__thiscall* ZGetHPType)(DWORD*);
ZGetHPType ZGetHP = (ZGetHPType)GetHPOffset;

typedef void(__stdcall* ZCombatInterfaceType)();
ZCombatInterfaceType GetZCombatInterface = (ZCombatInterfaceType)ZCombatInterfaceOffset;

typedef void(__cdecl* ZChatOutputType)(const char* text, int, int, DWORD color);
ZChatOutputType ZChatOutput = (ZChatOutputType)ZChatOutputOffset;

typedef bool (__thiscall* ShowWidgetT)(ZGameInterfaceOld* _this, const char *szName, bool bVisible, bool bModal);
ShowWidgetT ShowWidget = (ShowWidgetT)0x00472070;

typedef char (__thiscall* MakeCrcT)(DWORD* pMyCharacter, DWORD* a2);
MakeCrcT MakeCrc = (MakeCrcT)0x0041B660;

typedef void (__thiscall* ZPostReactionT)(float fTime, int id);
ZPostReactionT ZPostReaction = (ZPostReactionT)0x4245D0; // 83 EC 0C 56 57 8B F9 F3 0F 11 44 24

typedef DWORD* (__thiscall* GetCommandT)(DWORD command);
GetCommandT GetCommand = (GetCommandT)0x00559D50;

typedef void (__thiscall* BonusHealthT)(ZCharacter *pMyCharacter);
BonusHealthT BonusHealth = (BonusHealthT)0x57D960;

typedef void(__thiscall* PenaltyHealthT)(ZCharacter *pMyCharacter);
PenaltyHealthT PenaltyHealth = (PenaltyHealthT)0x57DFB0;

//ZRuleBerserker::BonusHealth

enum ZREACTIONID
{
	ZR_CHARGING = 0,
	ZR_CHARGED,
	ZR_BE_UPPERCUT,
	ZR_BE_DASH_UPPERCUT,
	ZR_DISCHARGED,

	ZR_END
};

typedef void(__cdecl* _UnknownT)();
_UnknownT _Unknown = (_UnknownT)0x5049C0;

// 5049C0

void onAnnounce(unsigned int nType, char* sMsg)
{
	__asm
	{
		mov ecx, ZGameOffset
		mov eax, OnAnnounceOffset
		push nType
		push sMsg
		call eax
	}
}

DWORD* dwZGame = (DWORD*)0x6EADB0;

DWORD pMatch()
{
	if (dwZGame != NULL && *dwZGame != NULL)
		return *(DWORD*)(*dwZGame + 0x2FC);

	return NULL;
}

GunzState pState()
{
	if (dwZGame != NULL && *dwZGame != NULL)
		return *(GunzState*)(*dwZGame + 0x2430);

	return GunzState::GUNZ_NA;
}

DWORD* ZGetCharacterManager()
{
	DWORD* match = (DWORD*)pMatch();

	if (match != NULL && *match != NULL)
		return (DWORD*)(*match + 0x70);

	return NULL;
}

DWORD dwGetHP = GetHPOffset;

float GetHP()
{
	float nHP;
	//void* pCharacter = ZGetGame()->m_Match->pCharacter;
	//DWORD* character = (DWORD*)((DWORD*)(*(DWORD*)0x6EADB0 + 0x2FC) + 0x5C);
	//DWORD* character = ((DWORD*)((DWORD*)((DWORD*)(*(DWORD*)0x6EADB0 + 0x2FC) + 0x70)) + 0x18);

	DWORD* dwGameInterface = (DWORD*)ZGameInterfaceOld::GetInstance();
	DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);
	DWORD pChar = *(DWORD*)(*dwGame + 0x5C);

	__asm
	{
		mov ecx, pChar
		call dwGetHP
		mov nHP, eax
	}

	return nHP;
}

void SetHP(int hp)
{
	DWORD* dwGameInterface = (DWORD*)ZGameInterfaceOld::GetInstance();
	DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);
	DWORD pChar = *(DWORD*)(*dwGame + 0x5C);
	//DWORD* ZGame = (DWORD*)0x6EADB0;
	//DWORD character = (DWORD)ZGameInterface::GetInstance()->m_pGame->m_pMyCharacter;

	__asm
	{
		mov ecx, pChar
		mov eax, 0x4D56A0
		push hp
		call eax
	}
}

DWORD *pGame = (DWORD*)0x006EADB0;

DWORD pChar()
{
	if (pGame != NULL && *pGame != NULL)
		return *(DWORD*)(*pGame + 0x2FC);

	return NULL;
}

// Commands
#define ZNewCmdOffset 0x559950 
#define ZPostCommandOffset 0x559D50
#define MCommandAddParameter 0x580050
#define MCommandParameterFloatOffset 0x62BE30
#define MCommandParameterIntOffset 0x409FC0

// TODO:
// MCommandParameterDir
// sub_5372A0
//  &off_69A998






/*class MCommandParameterBlob : public MCommandParameter
{
public:
	void *pValue;

	MCommandParameterBlob(void *pParam, int nSize)
	{
		__asm
		{
			mov ecx, this
			push nSize
			push pParam
			call g_dwBlob
		}
	}
};

class MCommandParameterUID : public MCommandParameter
{
public:
	void *pValue;

	MCommandParameterUID(void *pParam)
	{
		__asm
		{
			mov ecx, this
			push pParam
			call g_dwUID
		}
	}
};*/

/*class MCommandParameterFloat : public MCommandParameter
{
public:
	float fValue;

	MCommandParameterFloat(float fParam)
	{
		DWORD addr = MCommandParameterFloatOffset;

		__asm
		{
			mov ecx, this
			push fParam
			call addr
		}
	}
};*/

class MCommandParameterFloat : public MCommandParameter
{
public:
	float fValue;				// 0x08
	DWORD* pCommandData;		// 0x0C
};

class MCommandParameterUInt : public MCommandParameter
{
public:
	unsigned int m_Value;
};

class MCommandParameterInt : public MCommandParameter
{
public:
	int nValue;					// 0x08
	DWORD* pCommandInt;			// 0x0C
};

class MCommandParameterBool : public MCommandParameter
{
public:
	bool m_Value;
};

class MCommandParameterUID : public MCommandParameter
{
public:
	MUID m_Value;
};

class MCommandParameterBlob : public MCommandParameter
{
public:
	void* m_Value;
	unsigned int m_nSize;
};

struct ZPACKEDSHOTINFO
{
	float	fTime;
	short	posx, posy, posz;
	short	tox, toy, toz;
	BYTE	sel_type;
};

typedef MCommand* (__thiscall* ZNewCmdT)(int nID);
ZNewCmdT ZNewCmd = (ZNewCmdT)OFFSET_MCommand_ZNewCmd;

typedef void (__thiscall* ZPostFriendListT)(int nID);
ZPostFriendListT ZPostFriendList = (ZPostFriendListT)0x45DB40;

typedef bool (__thiscall* ZPostCommandT)(MCommand* pCommand);
ZPostCommandT ZPostCommand = (ZPostCommandT)OFFSET_MCommand_MCommandZPostCommand;

typedef bool(__thiscall* AddParameterT)(MCommand* pCommand, MCommandParameter* pParam);
AddParameterT AddParameter = (AddParameterT)OFFSET_MCommand_MCommandAddParameter;

typedef MCommandParameterInt* (__cdecl* MCommandParameterIntT)();
MCommandParameterIntT MCommandParameterIntF = (MCommandParameterIntT)OFFSET_MCommand_MCommandParameterInt;

typedef MCommandParameterUInt* (__cdecl* MCommandParameterUIntT)();
MCommandParameterUIntT MCommandParameterUIntF = (MCommandParameterUIntT)0x44FCC0;
// &off_69E728

// FOR STRING: MCommandParameterString

typedef MCommandParameterBool* (__cdecl* MCommandParameterBoolT)();
MCommandParameterBoolT MCommandParameterBoolF = (MCommandParameterBoolT)0x41D140;
// &off_68A684

typedef MCommandParameterFloat* (__cdecl* MCommandParameterFloatT)();
MCommandParameterFloatT MCommandParameterFloatF = (MCommandParameterFloatT)OFFSET_MCommand_MCommandParameterFloat;

typedef MCommandParameterPos* (__cdecl* MCommandParameterPosT)();
MCommandParameterPosT MCommandParameterPosF = (MCommandParameterPosT)OFFSET_MCommand_MCommandParameterPos;

typedef MCommandParameterVector* (__cdecl* MCommandParameterVectorT)();
MCommandParameterVectorT MCommandParameterVectorF = (MCommandParameterVectorT)OFFSET_MCommand_MCommandParameterVector;

typedef MCommandParameterUID* (__cdecl* MCommandParameterUIDT)();
MCommandParameterUIDT MCommandParameterUIDF = (MCommandParameterUIDT)OFFSET_MCommand_MCommandParameterUID;


typedef MCommandParameterBlob* (__cdecl* MCommandParameterBlobT)(void* m_Value, unsigned int m_nSize);
MCommandParameterBlobT MCommandParameterBlobF = (MCommandParameterBlobT)OFFSET_MCommand_MCommandParameterBlob;
// &off_69E660

typedef void (__cdecl* AddDeniedCommandT)(int nCommandID);
AddDeniedCommandT AddDeniedCommand = (AddDeniedCommandT)0x56FA50;

float fShotTime = 0.0f;

#define MC_PEER_SHOT_MELEE			10037
#define MC_PEER_SHOT				10034
#define MC_PEER_SHOT_SP				10035


inline void ZPostDamage(MUID ChrUID, int damage, DWORD* pCommandInt)
{
	MCommand *pCommand = ZNewCmd(10032);
	ZGameInterfaceOld* pGameInterface = ZGameInterfaceOld::GetInstance();
	ZGameOld* pGame = pGameInterface->GetGame();

	MCommandParameterUID* p1 = MCommandParameterUIDF();
	p1->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	p1->m_nType = MPT_UID;
	p1->m_Value = ChrUID;
	pCommand->AddParameter(p1);

	MCommandParameterInt* pIntParam1 = MCommandParameterIntF();
	pIntParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam1->m_nType = MPT_INT;
	pIntParam1->nValue = damage;
	pIntParam1->pCommandInt = pCommandInt;
	pCommand->AddParameter(pIntParam1);

	ZPostCommand(pCommand);
}

inline void ZPostDie(MUID uidAttacker)
{
	MCommand *pCommand = ZNewCmd(10041);
	ZGameInterfaceOld* pGameInterface = ZGameInterfaceOld::GetInstance();
	ZGameOld* pGame = pGameInterface->GetGame();

	MCommandParameterUID* p1 = MCommandParameterUIDF();
	p1->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	p1->m_nType = MPT_UID;
	p1->m_Value = uidAttacker;
	pCommand->AddParameter(p1);

	ZPostCommand(pCommand);
}

inline void ZPostPeerEnchantDamage(MUID ownerUID, MUID targetUID)
{
	MCommand *pCommand = ZNewCmd(10055);
	ZGameInterfaceOld* pGameInterface = ZGameInterfaceOld::GetInstance();
	ZGameOld* pGame = pGameInterface->GetGame();

	MCommandParameterUID* p1 = MCommandParameterUIDF();
	p1->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	p1->m_nType = MPT_UID;
	p1->m_Value = ownerUID;
	pCommand->AddParameter(p1);

	MCommandParameterUID* p2 = MCommandParameterUIDF();
	p2->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	p2->m_nType = MPT_UID;
	p2->m_Value = targetUID;
	pCommand->AddParameter(p2);

	ZPostCommand(pCommand);
}

inline void ZPostVampire(const MUID& uidAttacker, float fDamage)
{
	MCommand *pCommand = ZNewCmd(10066);
	ZGameInterfaceOld* pGameInterface = ZGameInterfaceOld::GetInstance();
	ZGameOld* pGame = pGameInterface->GetGame();

	MCommandParameterUID* p1 = MCommandParameterUIDF();
	p1->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	p1->m_nType = MPT_UID;
	p1->m_Value = uidAttacker;
	pCommand->AddParameter(p1);

	MCommandParameterFloat* pFloatParam1 = MCommandParameterFloatF();
	pFloatParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterFloatVTable;
	pFloatParam1->m_nType = MPT_FLOAT;
	pFloatParam1->fValue = fDamage;
	pFloatParam1->pCommandData = ZGameInterfaceOld::GetInstance()->GetGame()->GetCommandData();
	pCommand->AddParameter(pFloatParam1);

	ZPostCommand(pCommand);
}

inline void ZPostLadderRejoin()
{
	MCommand *pCommand = ZNewCmd(1580);
	ZPostCommand(pCommand);
}

inline void ZPostAdminPKick(const MUID& uidMe, char* szTargetName, bool bBan)
{

}

#define CLOAK_CMD_ID(CMD_ID, FACTOR)	UncloakCmdId((CMD_ID) + (FACTOR), FACTOR)
int UncloakCmdId(int cloakedCmdId, int cloakFactor)
{
	return cloakedCmdId - cloakFactor;
}

inline void ZPostDamageCounter(int Damage, MUID AttackerUID, DWORD* pCommandInt)
{
	MCommand *pCommand = ZNewCmd(CLOAK_CMD_ID(10079, 1621));
	ZGameInterfaceOld* pGameInterface = ZGameInterfaceOld::GetInstance();
	ZGameOld* pGame = pGameInterface->GetGame();

	MCommandParameterInt* pIntParam1 = MCommandParameterIntF();
	pIntParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam1->m_nType = MPT_INT;
	pIntParam1->nValue = Damage;
	pIntParam1->pCommandInt = pCommandInt;
	pCommand->AddParameter(pIntParam1);

	MCommandParameterUID* p2 = MCommandParameterUIDF();
	p2->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	p2->m_nType = MPT_UID;
	p2->m_Value = AttackerUID;
	pCommand->AddParameter(p2);

	ZPostCommand(pCommand);
}

void ZPostShotMelee(D3DXVECTOR3* pos, int nShot, DWORD* pCommandInt)
{
	MCommand *pCommand = ZNewCmd(MC_PEER_SHOT_MELEE);

	/*DWORD* dwGameInterface = (DWORD*)ZGameInterface::GetInstance();
	DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);*/

	ZGameInterfaceOld* pGameInterface = ZGameInterfaceOld::GetInstance();
	ZGameOld* pGame = pGameInterface->GetGame();

	MCommandParameterFloat* pFloatParam1 = MCommandParameterFloatF();
	pFloatParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterFloatVTable;
	pFloatParam1->m_nType = MPT_FLOAT;
	pFloatParam1->fValue = fShotTime++;
	pFloatParam1->pCommandData = ZGameInterfaceOld::GetInstance()->GetGame()->GetCommandData();
	pCommand->AddParameter(pFloatParam1);

	MCommandParameterPos* pPosParam1 = MCommandParameterPosF();
	pPosParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterPosVTable;
	pPosParam1->m_nType = MPT_POS;
	pPosParam1->m_fX = pos->x;
	pPosParam1->m_fY = pos->y;
	pPosParam1->m_fZ = pos->z;
	pCommand->AddParameter(pPosParam1);

	MCommandParameterInt* pIntParam1 = MCommandParameterIntF();
	pIntParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam1->m_nType = MPT_INT;
	pIntParam1->nValue = nShot;
	pIntParam1->pCommandInt = pCommandInt;
	pCommand->AddParameter(pIntParam1);

	ZPostCommand(pCommand);
}

void ZPostShot(D3DXVECTOR3* pos, D3DXVECTOR3* dir, int sel_type)
{
	MCommand *pCommand = ZNewCmd(MC_PEER_SHOT);

	float* fPos = (float*)pos;
	float* fDir = (float*)dir;

	ZPACKEDSHOTINFO info = 
	{
		fShotTime++,
		fPos[0], fPos[1], fPos[2],
		fDir[0], fDir[1], fDir[2],
		sel_type
	};

	MCommandParameterBlob* pBloblParam = MCommandParameterBlobF(&info, sizeof(ZPACKEDSHOTINFO));
	pCommand->AddParameter(pBloblParam);

	ZPostCommand(pCommand);
}

void ZPostNPCSkillStart(MUID uidOwner, int nSkill, MUID uidTarget, D3DXVECTOR3& targetPos, DWORD* pCommandInt)
{
	MCommand *pCommand = ZNewCmd(6044);


	MCommandParameterUID* p2 = MCommandParameterUIDF();
	p2->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	p2->m_nType = MPT_UID;
	p2->m_Value = uidOwner;
	pCommand->AddParameter(p2);

	MCommandParameterInt* pIntParam1 = MCommandParameterIntF();
	pIntParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam1->m_nType = MPT_INT;
	pIntParam1->nValue = nSkill;
	pIntParam1->pCommandInt = pCommandInt;
	pCommand->AddParameter(pIntParam1);

	MCommandParameterUID* p3 = MCommandParameterUIDF();
	p3->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	p3->m_nType = MPT_UID;
	p3->m_Value = uidTarget;
	pCommand->AddParameter(p3);

	MCommandParameterPos* pPosParam1 = MCommandParameterPosF();
	pPosParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterPosVTable;
	pPosParam1->m_nType = MPT_POS;
	pPosParam1->m_fX = targetPos.x;
	pPosParam1->m_fY = targetPos.y;
	pPosParam1->m_fZ = targetPos.z;
	pCommand->AddParameter(pPosParam1);

	ZPostCommand(pCommand);
}

inline void ZPostRequestTimeSync(unsigned long nTimeStamp, DWORD* pCommandInt)
{
	MCommand *pCommand = ZNewCmd(1521);

	MCommandParameterInt* pIntParam1 = MCommandParameterIntF();
	pIntParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam1->m_nType = MPT_INT;
	pIntParam1->nValue = nTimeStamp;
	pIntParam1->pCommandInt = pCommandInt;
	pCommand->AddParameter(pIntParam1);

	ZPostCommand(pCommand);
}

// v8
void ZPostShotSp(D3DXVECTOR3 pos, D3DXVECTOR3 dir, int type, int sel_type, DWORD* pCommandInt)
{
	MCommand *pCommand = ZNewCmd(CLOAK_CMD_ID(MC_PEER_SHOT_SP, 12783));

	ZGameInterfaceOld* pGameInterface = ZGameInterfaceOld::GetInstance();
	ZGameOld* pGame = pGameInterface->GetGame();

	MCommandParameterFloat* pFloatParam1 = MCommandParameterFloatF();
	pFloatParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterFloatVTable;
	pFloatParam1->m_nType = MPT_FLOAT;
	pFloatParam1->fValue = fShotTime++;
	pFloatParam1->pCommandData = ZGameInterface::GetInstance()->GetGame()->GetCommandData();
	pCommand->AddParameter(pFloatParam1);

	MCommandParameterPos* pPosParam1 = MCommandParameterPosF();
	pPosParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterPosVTable;
	pPosParam1->m_nType = MPT_POS;
	pPosParam1->m_fX = pos.x;
	pPosParam1->m_fY = pos.y;
	pPosParam1->m_fZ = pos.z;
	pCommand->AddParameter(pPosParam1);

	MCommandParameterVector* pVectorParam1 = MCommandParameterVectorF();
	pVectorParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterVectorVTable;
	pVectorParam1->m_nType = MPT_VECTOR;
	pVectorParam1->m_fX = dir.x;
	pVectorParam1->m_fY = dir.y;
	pVectorParam1->m_fZ = dir.z;
	pCommand->AddParameter(pVectorParam1);

	MCommandParameterInt* pType = MCommandParameterIntF();
	pType->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pType->m_nType = MPT_INT;
	pType->nValue = type;
	pType->pCommandInt = pCommandInt;
	pCommand->AddParameter(pType);

	MCommandParameterInt* pSelType = MCommandParameterIntF();
	pSelType->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pSelType->m_nType = MPT_INT;
	pSelType->nValue = sel_type;
	pSelType->pCommandInt = pCommandInt;
	pCommand->AddParameter(pSelType);

	ZPostCommand(pCommand);
}

void ZPostSkill(int nSkill, int sel_type)
{
	MCommand *pCommand = ZNewCmd(0x2734);
	//pCommand->AddParameter(new MCommandParameterFloat(fShotTime++));
	//pCommand->AddParameter(new MCommandParameterInt(nSkill));
	//pCommand->AddParameter(new MCommandParameterInt(sel_type));

	DWORD* dwGameInterface = (DWORD*)ZGameInterfaceOld::GetInstance();
	DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);

	MCommandParameterFloat* pFloatParam1 = MCommandParameterFloatF();
	pFloatParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterFloatVTable;
	pFloatParam1->m_nType = MPT_FLOAT;
	pFloatParam1->fValue = fShotTime++;
	pFloatParam1->pCommandData = ZGameInterfaceOld::GetInstance()->GetGame()->GetCommandData();
	//pFloatParam1->pCommandData = *(DWORD*)(*(DWORD*)(*pGame + 0x2C) - (*pGame + 0x2C) - 0xD6);
	pCommand->AddParameter(pFloatParam1);

	MCommandParameterInt* pIntParam1 = MCommandParameterIntF();
	pIntParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam1->m_nType = MPT_INT;
	pIntParam1->nValue = nSkill;
	pCommand->AddParameter(pIntParam1);

	MCommandParameterInt* pIntParam2 = MCommandParameterIntF();
	pIntParam2->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam2->m_nType = MPT_INT;
	pIntParam2->nValue = sel_type;
	pCommand->AddParameter(pIntParam2);
	ZPostCommand(pCommand);
}

void ZPostSpMotion(ZC_SPMOTION_TYPE type)
{
	MCommand *pCommand = ZNewCmd(0x273E);
	MCommandParameterInt* pIntParam = MCommandParameterIntF();
	pIntParam->VTable = (void*)0x69E750;
	pIntParam->m_nType = MPT_INT;
	pIntParam->nValue = type;
	pCommand->AddParameter(pIntParam);
	ZPostCommand(pCommand);

	/*DWORD dwZNewCmd = 0x559950;
	DWORD dwZPostCommand = 0x559D50;
	DWORD dwMCommandParameterInt = 0x409FC0;
	DWORD dwOffset = 0x69E750;
	DWORD dwAddParameter = 0x580050;

	__asm
	{
		mov ecx, 0x273E
		call dwZNewCmd
		mov edi, eax
		call dwMCommandParameterInt
		mov [esp+8+4], eax
		push eax // nParam
		mov ecx, edi // this
		mov dword ptr [eax+4], 0 // type ?
		mov ebx, dwOffset
 		mov dword ptr [eax], ebx
		mov [eax+0xC], esi
		call dwAddParameter
		mov ecx, edi
		call dwZPostCommand
		pop edi
		pop esi
	}*/
}

void ZPostReload()
{
	DWORD dwZNewCmd = 0x559950;
	DWORD dwZPostCommand = 0x559D50;

	__asm
	{
		push ecx
		mov ecx, MC_PEER_RELOAD
		call dwZNewCmd
		mov ecx, eax
		call dwZPostCommand
		pop ecx
	}
}

#define MC_PEER_HPAPINFO			10014

void ZPostHPAPInfo(float fHP, float fAP)
{
	MCommand *pCommand = ZNewCmd(MC_PEER_HPAPINFO);

	MCommandParameterFloat* pFloatParam1 = MCommandParameterFloatF();
	pFloatParam1->VTable = (void*)0x69E700;
	pFloatParam1->m_nType = MPT_FLOAT;
	pFloatParam1->fValue = fHP;
	pCommand->AddParameter(pFloatParam1);

	MCommandParameterFloat* pFloatParam2 = MCommandParameterFloatF();
	pFloatParam2->VTable = (void*)0x69E700;
	pFloatParam2->m_nType = MPT_FLOAT;
	pFloatParam2->fValue = fAP;
	pCommand->AddParameter(pFloatParam2);

	ZPostCommand(pCommand);
}

#define MC_REQUEST_GIVE_ONESELF_UP 9101
void ZPostHacker(const int nHackID)
{
	MCommand *pCommand = ZNewCmd(MC_REQUEST_GIVE_ONESELF_UP);

	MCommandParameterInt* pIntParam1 = MCommandParameterIntF();
	pIntParam1->VTable = (void*)0x69E750;
	pIntParam1->m_nType = MPT_INT;
	pIntParam1->nValue = nHackID;
	pCommand->AddParameter(pIntParam1);

	ZPostCommand(pCommand);
}

void _ZPostReaction(float fTime, int id)
{
	MCommand *pCommand = ZNewCmd(0x2746);

	MCommandParameterFloat* pFloatParam1 = MCommandParameterFloatF();
	pFloatParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterFloatVTable;
	pFloatParam1->m_nType = MPT_FLOAT;
	pFloatParam1->fValue = fTime;
	//pFloatParam1->pCommandData = ZGameInterface::GetInstance()->GetGame()->GetCommandData();
	//pFloatParam1->pCommandData = *(DWORD*)(*(DWORD*)(*pGame + 0x2C) - (*pGame + 0x2C) - 0xD6);
	pCommand->AddParameter(pFloatParam1);

	MCommandParameterInt* pIntParam1 = MCommandParameterIntF();
	pIntParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam1->m_nType = MPT_INT;
	pIntParam1->nValue = id;
	pCommand->AddParameter(pIntParam1);

	ZPostCommand(pCommand);
}

void ZPostRequestSpawnWorldItem(const MUID& uidChar, const int nItemID, const D3DXVECTOR3* pos, float fDropDelayTime)
{
	MCommand *pCommand = ZNewCmd(1544);

	MCommandParameterUID* pUID = MCommandParameterUIDF();
	pUID->VTable = (void*)OFFSET_MCommand_MCommandParameterUIDVTable;
	pUID->m_nType = MPT_UID;
	pUID->m_Value = uidChar;
	pCommand->AddParameter(pUID);

	MCommandParameterInt* pIntParam = MCommandParameterIntF();
	pIntParam->VTable = (void*)OFFSET_MCommand_MCommandParameterIntVTable;
	pIntParam->m_nType = MPT_INT;
	pIntParam->nValue = nItemID;
	pCommand->AddParameter(pIntParam);
	ZPostCommand(pCommand);

	MCommandParameterPos* pPosParam1 = MCommandParameterPosF();
	pPosParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterPosVTable;
	pPosParam1->m_nType = MPT_POS;
	pPosParam1->m_fX = pos->x;
	pPosParam1->m_fY = pos->y;
	pPosParam1->m_fZ = pos->z;
	pCommand->AddParameter(pPosParam1);

	MCommandParameterFloat* pFloatParam1 = MCommandParameterFloatF();
	pFloatParam1->VTable = (void*)OFFSET_MCommand_MCommandParameterFloatVTable;
	pFloatParam1->m_nType = MPT_FLOAT;
	pFloatParam1->fValue = fDropDelayTime;
	pFloatParam1->pCommandData = ZGameInterface::GetInstance()->GetGame()->GetCommandData();
	pCommand->AddParameter(pFloatParam1);

	ZPostCommand(pCommand);
}

//typedef int (__cdecl*tHook)(char *a1, unsigned __int8 a2); // 0x20EF000
typedef int(__cdecl* tHook)(PIP_ADAPTER_INFO pAdapterInfo, void* pOutBufLen);
tHook oHook; 


typedef int(__cdecl* tHook2)(LPHW_PROFILE_INFO lpHwProfileInfo);
tHook2 oHook2;

typedef int(__cdecl* tHook3)(LPCTSTR lpRootPathName,
	LPTSTR  lpVolumeNameBuffer,
	DWORD   nVolumeNameSize,
	 LPDWORD lpVolumeSerialNumber,
	 LPDWORD lpMaximumComponentLength,
	 LPDWORD lpFileSystemFlags,
	 LPTSTR  lpFileSystemNameBuffer,
	      DWORD   nFileSystemNameSize);
tHook3 oHook3;

int nHook(PIP_ADAPTER_INFO pAdapterInfo, void* pOutBufLen)
{
	/*int ret = oHook(pAdapterInfo, pOutBufLen);

	for (int offset = 0;; offset++)
	{
	DWORD dwStartAddress = (DWORD)pAdapterInfo;
	DWORD dwAdapterInfo = (dwStartAddress + (offset * 0x280));
	DWORD dwAdapterName = (dwAdapterInfo + 0x8);

	//((char*)(dwAdapterName))[1] = 'B';

	if (((char*)(dwAdapterName))[0] != '{')
	break;

	// Spoof name
	int nameSize = strlen((char*)(dwAdapterName));

	for (int i = 0; i < nameSize; i++)
	{
	if (i >= 37)
	break;

	if (i == 0 || i == 9 || i == 14 || i == 19 || i == 24 || i == 37)
	continue;

	((char*)(dwAdapterName))[i] = 'B';
	}

	_IP_ADAPTER_INFO* info = (_IP_ADAPTER_INFO*)(dwStartAddress + (offset * 0x280));

	// Spoof address
	for (int i = 0; i < info->AddressLength; i++)
	{
	info->Address[i] = 0x1;
	}

	for (int i = 0; i < strlen(info->Description); i++)
	{
	info->Description[i] = '1';
	}

	break;
	}

	return ret;*/

	pAdapterInfo->Address[0] = rand() % 10000 + 1;
	pAdapterInfo->Address[1] = rand() % 10000 + 1;
	pAdapterInfo->Address[2] = rand() % 10000 + 1;
	pAdapterInfo->Address[3] = rand() % 10000 + 1;
	pAdapterInfo->Address[4] = rand() % 10000 + 1;
	pAdapterInfo->Address[5] = rand() % 10000 + 1;
	pAdapterInfo->Address[6] = rand() % 10000 + 1;
	pAdapterInfo->Address[7] = rand() % 10000 + 1;

	return oHook(pAdapterInfo, pOutBufLen);
}

int nHook2(LPHW_PROFILE_INFO lpHwProfileInfo)
{
	//char c[1024];
	//sprintf_s(c, "lpcbData %d",  lpcbData);
	//Log("nHook2");
	//MessageBox(NULL, "TEST", NULL, NULL);
	for (int i = 0; i < 10; i++)
	{
		lpHwProfileInfo->szHwProfileGuid[i] = 'c';
		lpHwProfileInfo->szHwProfileName[i] = 'c';
	}

	return oHook2(lpHwProfileInfo);
}

int nHook3(LPCTSTR lpRootPathName,
	LPTSTR  lpVolumeNameBuffer,
	DWORD   nVolumeNameSize,
	LPDWORD lpVolumeSerialNumber,
	LPDWORD lpMaximumComponentLength,
	LPDWORD lpFileSystemFlags,
	LPTSTR  lpFileSystemNameBuffer,
	DWORD   nFileSystemNameSize)
{
	MessageBox(NULL, "GetVolumeInformation", NULL, NULL);

	return oHook3( lpRootPathName,
		  lpVolumeNameBuffer,
		   nVolumeNameSize,
		 lpVolumeSerialNumber,
		lpMaximumComponentLength,
		lpFileSystemFlags,
		 lpFileSystemNameBuffer,
		  nFileSystemNameSize);
}

#include "VTable.h"

Utils::VtableDetour* g_vtableDetour = NULL;
Utils::VtableHook* g_vtableHook = NULL;

void OnDamagedSkill(/*ZObject*/void* pAttacker, D3DXVECTOR3 srcPos, /*ZDAMAGETYPE*/int damageType, /*MMatchWeaponType*/int weaponType, float fDamage, float fPiercingRatio, int nMeleeType) // 28
{
	Console::print("OnDamagedSkill\n");
	((void(*)(void*, D3DXVECTOR3, int, int, float, float, int))g_vtableHook->GetOriginal(28))(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
}

IDirect3DDevice9* pDevice = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVB;    // Buffer to hold vertices
LPDIRECT3DINDEXBUFFER9  g_pIB;    // Buffer to hold indices

struct vertex
{
	FLOAT x, y, z, rhw;
	DWORD color;
};

void BoxFilled(float x, float y, float w, float h, DWORD color)
{
	vertex V[4];

	V[0].color = V[1].color = V[2].color = V[3].color = color;

	V[0].z = V[1].z = V[2].z = V[3].z = 0;
	V[0].rhw = V[1].rhw = V[2].rhw = V[3].rhw = 0;

	V[0].x = x;
	V[0].y = y;
	V[1].x = x + w;
	V[1].y = y;
	V[2].x = x + w;
	V[2].y = y + h;
	V[3].x = x;
	V[3].y = y + h;

	unsigned short indexes[] = { 0, 1, 3, 1, 2, 3 };

	pDevice->CreateVertexBuffer(4 * sizeof(vertex), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB, NULL);
	pDevice->CreateIndexBuffer(2 * sizeof(short), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB, NULL);

	VOID* pVertices;
	g_pVB->Lock(0, sizeof(V), (void**)&pVertices, 0);
	memcpy(pVertices, V, sizeof(V));
	g_pVB->Unlock();

	VOID* pIndex;
	g_pIB->Lock(0, sizeof(indexes), (void**)&pIndex, 0);
	memcpy(pIndex, indexes, sizeof(indexes));
	g_pIB->Unlock();

	pDevice->SetTexture(0, NULL);
	pDevice->SetPixelShader(NULL);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	pDevice->SetStreamSource(0, g_pVB, 0, sizeof(vertex));
	pDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->SetIndices(g_pIB);

	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);

	g_pVB->Release();
	g_pIB->Release();
}


void Box(float x, float y, float w, float h, float linewidth, DWORD color)
{
	if (linewidth == 0 || linewidth == 1)
	{
		BoxFilled(x, y, w, 1, color);             // Top
		BoxFilled(x, y + h - 1, w, 1, color);         // Bottom
		BoxFilled(x, y + 1, 1, h - 2 * 1, color);       // Left
		BoxFilled(x + w - 1, y + 1, 1, h - 2 * 1, color);   // Right
	}
	else
	{
		BoxFilled(x, y, w, linewidth, color);                                     // Top
		BoxFilled(x, y + h - linewidth, w, linewidth, color);                         // Bottom
		BoxFilled(x, y + linewidth, linewidth, h - 2 * linewidth, color);               // Left
		BoxFilled(x + w - linewidth, y + linewidth, linewidth, h - 2 * linewidth, color);   // Right
	}
}

void RDrawLine(D3DXVECTOR3 &v1, D3DXVECTOR3 &v2, DWORD dwColor)
{
	/*
	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	*/
	struct LVERTEX {
		float x, y, z;		// world position
		DWORD color;
	};

	LVERTEX ver[2] = { { v1.x,v1.y,v1.z,dwColor },{ v2.x,v2.y,v2.z,dwColor } };

	HRESULT hr = pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, ver, sizeof(LVERTEX));
}

class MDrawContext;
typedef void(__thiscall* TextRelativeT)(MDrawContext* pDC, float x, float y, const char *szText, bool bCenter);
TextRelativeT TextRelative = (TextRelativeT)0x5DCAE0;

D3DXVECTOR3& GetCameraPosition()
{
	return *(D3DXVECTOR3*)0xA1166C;
}

D3DXMATRIX& GetViewProjectionViewport()
{
	return *(D3DXMATRIX*)0xA117A8;
}

D3DXPLANE& GetViewFrustum()
{
	return *(D3DXPLANE*)0xA117EC; // MAYBE
}


D3DXVECTOR3 RGetTransformCoord(D3DXVECTOR3 &coord)
{
	D3DXVECTOR3 ret;
	D3DXVec3TransformCoord(&ret, &coord, &GetViewProjectionViewport());
	return ret;
}


struct rboundingbox
{
	union {
		struct {
			float minx, miny, minz, maxx, maxy, maxz;
		};
		struct {
			rvector vmin, vmax;
		};
		float m[2][3];
	};

	rvector Point(int i) const { return rvector((i & 1) ? vmin.x : vmax.x, (i & 2) ? vmin.y : vmax.y, (i & 4) ? vmin.z : vmax.z); }

	void Add(const rvector &kPoint)
	{
		if (vmin.x > kPoint.x)	vmin.x = kPoint.x;
		if (vmin.y > kPoint.y)	vmin.y = kPoint.y;
		if (vmin.z > kPoint.z)	vmin.z = kPoint.z;
		if (vmax.x < kPoint.x)	vmax.x = kPoint.x;
		if (vmax.y < kPoint.y)	vmax.y = kPoint.y;
		if (vmax.z < kPoint.z)	vmax.z = kPoint.z;
	}
};


float GetDistance(rboundingbox *bb, rplane *plane)		// Æò¸é¿¡¼­ boundingbox¿ÍÀÇ ÃÖ´ë°Å¸®
{
	float a, b, c;
	a = (plane->a>0) ? bb->m[1][0] : bb->m[0][0];
	b = (plane->b>0) ? bb->m[1][1] : bb->m[0][1];
	c = (plane->c>0) ? bb->m[1][2] : bb->m[0][2];
	return plane->a*a + plane->b*b + plane->c*c + plane->d;
}

bool isInPlane(rboundingbox *bb, rplane *plane)
{
	return (GetDistance(bb, plane) >= 0);
}

bool isInViewFrustum(rboundingbox *bb, rplane *plane)
{
	if (isInPlane(bb, plane) && isInPlane(bb, plane + 1) &&
		isInPlane(bb, plane + 2) && isInPlane(bb, plane + 3)) return true;
	return false;
}

bool RGetScreenLine(int sx, int sy, D3DXVECTOR3 *pos, D3DXVECTOR3 *dir)
{
	D3DXVECTOR3 scrpoint = D3DXVECTOR3((float)sx, (float)sy, 0.1f);

	D3DXMATRIX inv;
	float det;
	if (D3DXMatrixInverse(&inv, &det, &GetViewProjectionViewport()) == NULL)
		return false;

	D3DXVECTOR3 worldpoint;
	D3DXVec3TransformCoord(&worldpoint, &scrpoint, &inv);

	*pos = GetCameraPosition();
	*dir = worldpoint - GetCameraPosition();
	D3DXVec3Normalize(dir, dir);

	return true;
}


bool isInViewFrustum(const D3DXVECTOR3 &point, float radius, D3DXPLANE* plane)		// bounding sphere
{
	if ((D3DXPlaneDotCoord(&plane[0], &point)>-radius) &&
		(D3DXPlaneDotCoord(&plane[1], &point)>-radius) &&
		(D3DXPlaneDotCoord(&plane[2], &point)>-radius) &&
		(D3DXPlaneDotCoord(&plane[3], &point)>-radius) &&
		(D3DXPlaneDotCoord(&plane[5], &point)>-radius))
		return true;
	return false;
}

float radius = 1600.0f;

bool bESP = true;
bool bNameESP = false;
bool bAimHelper = false;
bool bExtraHack = false;
void onRender(Renderer *renderer)
{
	if (bExtraHack)
		renderer->DrawText(10, 10, "Extra hack enabled (F9)");

	if (!bESP)
		return;

	//renderer->DrawText(10, 10, "Toggle ESP: F8");
	//renderer->DrawText(10, 30, "Toggle ESP Look: F7");
	//renderer->DrawText(10, 50, "Toggle Name ESP: F6");
	//renderer->DrawBorder(0, 0, renderer->GetWidth(), renderer->GetHeight(), 1, Color(255, 0, 255, 0));

	int width = renderer->GetWidth();
	int height = renderer->GetHeight();

	D3D9Renderer* d3 = (D3D9Renderer*)renderer;

	ZGameInterface* pGameInterface = ZGameInterface::GetInstance();

	if (!pGameInterface || pGameInterface == nullptr)
		return;


	ZGame* pGame = pGameInterface->GetGame();

	if (!pGame || pGame == nullptr || *(DWORD*)pGame == NULL)
		return;

	ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*(DWORD*)pGame + 0x5C);

	if (!pChar || pChar == nullptr)
		return;

	ZCharacterOld* pCharOld = (ZCharacterOld*)*(DWORD*)(*(DWORD*)pGame + 0x5C);

	if (!pCharOld || pCharOld == nullptr)
		return;

	if (!((DWORD*)pCharOld + 0x81D))
		return;

	D3DXVECTOR3* pos = pCharOld->GetPosition();
	D3DXVECTOR3* dir = pCharOld->GetDirection();

	ZCharacterManager* m_CharacterManager = (ZCharacterManager*)(*(DWORD*)pGame + 0x70);

	if (!m_CharacterManager || m_CharacterManager == nullptr || m_CharacterManager->empty())
		return;

	MMatchTeam currentTeam = pChar->GetTeamID();

	ZGameInterfaceOld* pGameInterfaceOld = (ZGameInterfaceOld*)pGameInterface;
	ZCombatInterface* pCombatInterface = pGameInterfaceOld->GetCombatInterface();

	Color targetColor;
	Color green = Color(144, 0, 255, 0);
	Color yellow = Color(144, 255, 255, 0);
	Color red = Color(144, 255, 0, 0);
	Color lowBlue = Color(144, 63, 170, 180);
	Color highBlue = Color(144, 7, 126, 255);
	Color color;

	for (ZCharacterManager::iterator i = m_CharacterManager->begin(); i != m_CharacterManager->end(); i++)
	{
		ZCharacterOld* pCharacter = (ZCharacterOld*)i->second;
		ZCharacter* pTargetChar = (ZCharacter*)i->second;
		
		if (!pCharacter)
			continue;

		if (pCharacter == pCharOld)
			continue;

		D3DXVECTOR3* targetPos = pCharacter->GetPosition();
		//D3DXVECTOR3 vTargetPos(targetPos->x, targetPos->y, 0.0f);
		D3DXVECTOR3 vTargetPos(targetPos->x, targetPos->y, targetPos->z);
		//D3DXVECTOR3* targetdir = pCharacter->GetDirection();

		if (currentTeam != MMT_ALL && currentTeam == pTargetChar->GetTeamID())
			continue;

		float hp = pCharacter->m_pModule_HPAP->GetHP();
		float maxHp = pCharacter->m_pModule_HPAP->GetMaxHP();

		if (pTargetChar->IsDie())
			continue;

		float health = (hp / maxHp);


		// Draw the HP bar

		if (hp <= 40.0f)
			color = red;
		else if (hp <= 80.0f)
			color = yellow;
		else if (hp <= 105.0f)
			color = lowBlue;
		else
			color = highBlue;

		//D3DXVECTOR3 vPos, vDir;
		//bool ret = RGetScreenLine(width / 2, height / 2, &vPos, &vDir);

		/*if (!isInViewFrustum(vTargetPos, radius, &GetViewFrustum()))
		{
			continue;
		}*/

		rboundingbox box = { 0 };

		box.vmax = vTargetPos;// +rvector(50.f, 50.f, 190.f);
		box.vmin = vTargetPos;// +rvector(-50.f, -50.f, 0.f);

		//if (isInViewFrustum(&box, &GetViewFrustum()))
		//	continue;

		//targetPos->z = 0.0f;
		D3DXVECTOR3 screen = RGetTransformCoord(vTargetPos);

		if (screen.z >= 1.0f)
			continue;
		
		ZCharacter* pTargetCharacter = pCombatInterface->GetTargetCharacter();

		if (pTargetCharacter == pTargetChar)
			targetColor = red;
		else
			targetColor = green;

		float v = screen.z - ((int)(screen.z * 1000) / 1000) * 10000;

		//rvector headpos = vTargetPos + rvector(0, 0, 180);
		rvector headpos = vTargetPos + rvector(0, 0, 150);
		D3DXVECTOR3 headScreen = RGetTransformCoord(headpos);

		//renderer->DrawBorder(screen.x - 40, screen.y, 80 - v, 100 - v, 1, targetColor);
		int diff = screen.y - headScreen.y;
		float size = 0.3f;
		int x = screen.x - (diff * size);
		int y = screen.y - diff;
		int w = (diff * size * 2);
		renderer->DrawBorder(x, y, w, diff, bAimHelper ? 2 : 1, bAimHelper ? color : targetColor);

		const char* name = pTargetChar->GetName();
		bool bold = false;

		if (bNameESP)
		{
			if (!bold)
				d3->DrawText(screen.x - 25, screen.y, (char*)name);
			else
				d3->DrawText(screen.x - 25, screen.y, D3DXCOLOR(1.0f, 0.415f, 0.007f, 1.0f), true, (char*)name);
		}

		if (bAimHelper)
		{
			renderer->DrawRect(x, y, w, 5, green);
		}

		renderer->DrawRect(x, y, (health * w), 5, color);
	}

}

int WINAPI Render(LPVOID)
{
#ifndef _PUBLISH
	DWORD dwDevice = 0x9F424C;
	
	while (!(pDevice = *(IDirect3DDevice9**)dwDevice))
		Sleep(100);

	Console::print("D3D Device: %p\n", pDevice);
	D3DVIEWPORT9 viewport;
	pDevice->GetViewport(&viewport);

	Console::print("Viewport H: %d, W: %d\n", viewport.Width, viewport.Height);
#endif



	// NOTE: This hook not working with the lite injector in this project, use Xenos
	mHackBase = HackBase::Singleton();
	if (!mHackBase->Initialize(onRender))
	{
#ifndef _PUBLISH
		Console::print("Error hooking game. Maybe injected into wrong process...");
#endif
	}

	bool bFlipmower = false;

	while (true)
	{
		if (bShutdown)
			break;

		//MDrawContextR2* g_pDC = MDrawContextR2::GetInstance();

		if (bExtraHack && GetAsyncKeyState(VK_MBUTTON) & 0x8000)
		{
			//CALLASMAIN(ZPostSkill(2, 7)); // Flip
			ZGameInterface* pGameInterface = ZGameInterface::GetInstance();

			if (!pGameInterface)
				continue;

			ZGame* pGame = pGameInterface->GetGame();

			if (!pGame || !*(DWORD*)pGame + 0x5C)
				continue;

			ZMyCharacter* pChar = (ZMyCharacter*)*(DWORD*)(*(DWORD*)pGame + 0x5C);

			if (pChar->GetHP() <= 0)
				continue;

			DWORD* dwLastTime = (DWORD*)(*(DWORD*)pGame + 0x38);
			dwLastTime[3] = 1;
			ZPostReaction(fShotTime++, ZR_CHARGED);
			dwLastTime[3] = 1;
			pChar->ChargedShot();
			dwLastTime[3] = 1;
			Sleep(1250);
		}

		if (GetAsyncKeyState(VK_F9) & 0x8000)
		{
			bExtraHack = !bExtraHack;
			Sleep(250);
		}

		if (GetAsyncKeyState(VK_F8) & 0x8000)
		{
			bESP = !bESP;
			Sleep(250);
		}


		if (GetAsyncKeyState(VK_F7) & 0x8000)
		{
			bAimHelper = !bAimHelper;
			Sleep(250);
		}

		if (GetAsyncKeyState(VK_F6) & 0x8000)
		{
			bNameESP = !bNameESP;
			Sleep(250);
		}


		if (GetAsyncKeyState(VK_HOME) & 0x8000)
		{
			ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			ZGame* pGame = pGameInterface->GetGame();
			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*(DWORD*)pGame + 0x5C);
			pChar->InitModuleStatus();
			Sleep(250);
		}

#ifndef _PUBLISH
		/*if (GetAsyncKeyState(VK_F5) & 0x8000)
		{
			radius -= 10.0f;
			Console::print("radius: %f\n", radius);
			Sleep(250);
		}


		if (GetAsyncKeyState(VK_HOME) & 0x8000)
		{
			//bRender = !bRender;
			///Console::print("g_pDC: %X\n", g_pDC);
			//Console::print("bRender: %d\n", bRender);


			ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			//Console::print("pGameInterface= %X\n", pGameInterface);
			ZGame* pGame = pGameInterface->GetGame();
			//Console::print("pGame= %X\n", pGame);
			ZMyCharacter* pMyChar = pGame->GetMyCharacter();
			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*(DWORD*)pGame + 0x5C);
			ZCharacterOld* pCharOld = (ZCharacterOld*)*(DWORD*)(*(DWORD*)pGame + 0x5C);
			D3DXVECTOR3* pos = pCharOld->GetPosition();
			D3DXVECTOR3* dir = pCharOld->GetDirection();

			Console::print("pMyChar= %X\n", pMyChar);
			Console::print("pChar= %X", pChar);
			Console::print("pos= %f, %f, %f", pos->x, pos->y, pos->z);
			Console::print("dir= %f, %f, %f", dir->x, dir->y, dir->z);

			D3DXVECTOR3 vPos, vDir;
			bool ret = RGetScreenLine(viewport.Width / 2, viewport.Height / 2, &vPos, &vDir);
			Console::print("RGetScreenLine: ret %d\n", ret);
			Console::print("vpos= %f, %f, %f\n", vPos.x, vPos.y, vPos.z);
			Console::print("vdir= %f, %f, %f\n", vDir.x, vDir.y, vDir.z);

			ZCharacterManager* m_CharacterManager = (ZCharacterManager*)(*(DWORD*)pGame + 0x70);
			Console::print("m_CharacterManager %X", m_CharacterManager);

			D3DXPLANE plane = GetViewFrustum();
			Console::print("place: (%f, %f, %f, %f)\n", plane.a, plane.b, plane.c, plane.d);

			for (ZCharacterManager::iterator i = m_CharacterManager->begin(); i != m_CharacterManager->end(); i++)
			{
				ZCharacterOld* pCharacter = (ZCharacterOld*)i->second;

				D3DXVECTOR3* targetPos = pCharacter->GetPosition();
				D3DXVECTOR3* targetdir = pCharacter->GetDirection();

				D3DXVECTOR3 projection = GetViewProjectionViewport();

				Console::print("POS(%f, %f, %f)\tDIR(%f, %f, %f)\tprojection(%f, %f, %f)", targetPos->x, targetPos->y, targetPos->z,
					targetdir->x, targetdir->y, targetdir->z, projection.x, projection.y, projection.z);

				//targetPos->z = 0.0f;
				D3DXVECTOR3 screen = RGetTransformCoord(*targetPos);
				Console::print("Screen: %f/%f", screen.x, screen.y);

				float hp = pCharacter->m_pModule_HPAP->GetHP();
				float maxHp = pCharacter->m_pModule_HPAP->GetMaxHP();

				if (hp <= 0.0f)
					continue;

				float health = (hp / maxHp);

				bool inView = isInViewFrustum(*targetPos, 100.f, &GetViewFrustum());

				Console::print("HP: %f/%f (%f%%)", hp, maxHp, health * 100);
				Console::print("Team: %d (InView: %d)", pCharacter->GetTeamID(), inView);
				Console::print("Team2: %d (InView: %d)", pCharacter->GetTeamID() & 2, inView);
				Console::print("Team4: %d (InView: %d)", pCharacter->GetTeamID() & 4, inView);
				Console::print("Name: %s", pCharacter->GetUsername());
			}

			Sleep(250);
		}

			//TextRelative((MDrawContext*)g_pDC, 50.0f, 50.0f, "BLAH", false);
			//g_pDC->Text(50, 50, "BLAH");

		//Sleep(10);

		//g_pDC->Text(50, 50, "BLAH");

		//Box(100.0f, 100.0f, 100.0f, 100.0f, 4.0f, 0xFF0000);*/
#endif
	}

	return 0;
}

__declspec(naked) void OnDamagedSkill_Hook(ZCharacter* pChar, ZObject* pAttacker, D3DXVECTOR3 srcPos, /*ZDAMAGETYPE*/int damageType, /*MMatchWeaponType*/int weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	Console::print("OnDamagedSkill_Hook: %d %f", damageType, fDamage);
}

int WINAPI Background(LPVOID)
{
	int c = 0;
	int cc = 0;

	while (true)
	{
		if (bShutdown)
			break;

		if (GetAsyncKeyState(VK_END) && 0x8000)
		{
			ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			Console::print("pGameInterface= %X\n", pGameInterface);
			ZGame* pGame = pGameInterface->GetGame();
			Console::print("pGame= %X\n", pGame);
			DWORD* dwLastTime = (DWORD*)(*(DWORD*)pGame + 0x38);

			Console::print("pGame::m_nLastTime0x38 = %X\n", *(DWORD*)(*(DWORD*)pGame + 0x38));
			Console::print("pGame::m_nLastTime = %X\n", pGame->m_nLastTime);
			Console::print("pGame:: 0 = %d\n", dwLastTime[0]);
			Console::print("pGame:: 1 = %d\n", dwLastTime[1]);
			Console::print("pGame:: 2 = %d\n", dwLastTime[2]);
			Console::print("pGame:: ZLASTTIME_SYNC_REPORT = %d\n", dwLastTime[ZGAME_LASTTIME::ZLASTTIME_SYNC_REPORT]);

			if (!pGame || !(*(DWORD*)pGame) || !*(DWORD*)(*(DWORD*)pGame + 0x5C))
				continue;

			ZMyCharacter* pMyChar = pGame->GetMyCharacter();
			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*(DWORD*)pGame + 0x5C);
			ZCharacterOld* pCharOld = (ZCharacterOld*)*(DWORD*)(*(DWORD*)pGame + 0x5C);
			D3DXVECTOR3* pos = pCharOld->GetPosition();
			D3DXVECTOR3* dir = pCharOld->GetDirection();
			//ZPostShotSp(()
			Console::print("pMyChar= %X\n", pMyChar);
			//*(float*)(*(DWORD*)pChar + 0x95C) = 10.f;
			//Console::print("TEST= %f\n", (*(float*)(*(DWORD*)pChar + 0x95C))); // m_fTimeOffset maybe
			//Console::print("GetItems= %X\n", *(DWORD*)(*(DWORD*)pChar + 0x7F0));

			// Lawnmower
			//CALLASMAIN(ZPostShot(pos, dir, 7));
			//CALLASMAIN(ZPostShot(pos, dir, 1));
			//CALLASMAIN(ZPostShot(pos, dir, 0));

			//Console::print("pVMesh= %X\n", *(DWORD*)(*(DWORD*)pChar + 0x7F8));
			//RVisualMesh* pVMesh = pChar->m_pVMesh;
			//rvector wepPos = pVMesh->GetCurrentWeaponPosition(false);
			//char name[32] = "Test";
			////memcpy_s((void*)pChar->GetName(), 32, name, 32);
			//DWORD dwMask = *(DWORD*)(*(DWORD*)((DWORD)pCharOld->m_pModule_HPAP + 16) - ((DWORD)pCharOld->m_pModule_HPAP + 0x10) - 214);
			//memcpy_s((void*)pCharOld->m_pModule_HPAP->GetHP(), 4, (void*)&health, 4);
			//Console::print("name: %s", pChar->GetName());
			//Console::print("HP: %f", *(float*)((*(DWORD*)((DWORD)pCharOld->m_pModule_HPAP + 0x18) - ((DWORD)pCharOld->m_pModule_HPAP + 0x18) - 0xD6) - dwMask));
			//float val = 250.0f;
			//void* addr = ((DWORD*)((DWORD)pCharOld->m_pModule_HPAP + 0x18) - ((DWORD)pCharOld->m_pModule_HPAP + 0x18) - 0xD6) - dwMask;
			//*(float*)addr = 1000.f;
			//Console::print("addr: %X\n", addr);
			//Console::print("val: %f\n", *(float*)addr);
			//memcpy_s(addr, 4, &val, 4);
			//Console::print("name: %s", pCharOld->m_Property.nameCharClan);
			//rvector head = pVMesh->GetBipTypePosition(RMeshPartsPosInfoType::eq_parts_pos_info_Head);
			//Console::print("pos= %f, %f, %f\n", head.x, head.y, head.z);

			//Console::print("pos= %f, %f, %f\n", pos->x, pos->y, pos->z);
			//Console::print("dir= %f, %f, %f\n", dir->x, dir->y, dir->z);
			//Console::print("ZGameClient: %X", ZGameClient::GetInstance());
			Console::print("MUID: %d %d", ZGameClient::GetInstance()->m_uidPlayer.Low, ZGameClient::GetInstance()->m_uidPlayer.High);
			//Console::print("HP: %f\n", ZCharGetHP(pChar));
			//pCharOld->m_pModule_HPAP->SetHP(1000.f);
			//typedef void*(__thiscall* INIT_HP_TYPE)(ZCharacter* pChar);
			//INIT_HP_TYPE InitHPAP = (INIT_HP_TYPE)0x4F1480;
			//InitHPAP(pChar);

			Console::print("HP: %f/%f\n", pCharOld->m_pModule_HPAP->GetHP(), pCharOld->m_pModule_HPAP->GetMaxHP());
			Console::print("GetTeamID: %d\n", pChar->GetTeamID());
			Console::print("IsDie: %d\n", pChar->IsDie());
			//Console::print("IsDie_: %d\n", pCharOld->IsDie_());

			//char* name = (char*)(**(DWORD**)(*(DWORD*)(pChar) + 0x8B8) - *(DWORD*)(*(DWORD*)(pChar) + 0x8B8) - 0xB6);
			//Console::print("name: %s\n", name);

			float hp = pCharOld->m_pModule_HPAP->GetHP();
			float maxHp = pCharOld->m_pModule_HPAP->GetMaxHP();
			float health = (hp / maxHp);

			//typedef ZEffectManager* (__stdcall* Type)();
			//Type ZGetEffectManager = (Type)0x54D8D0;

			ZPICKINFO zpi;
			ZeroMemory(&zpi, sizeof(ZPICKINFO));
			Console::print("pos: (%f, %f, %f)", (*pos).x, (*pos).y, (*pos).z);
			Console::print("dir: (%f, %f, %f)", (*dir).x, (*dir).y, (*dir).z);
			pChar->OnBlast(rvector(0, 0, 0));
			//bool bPick = pGame->Pick(pChar, *pos, *dir, &zpi, RM_FLAG_ADDITIVE | RM_FLAG_HIDE, false);
			//bool ret = pGame->CheckWall(pChar, pChar, false);
			//Console::print("GetTime: %f\n", pGame->GetTime());
			//Console::print("bPick: %d", bPick);
			//Console::print("ret: %d", ret);

			/*DWORD dwEffectManager;
			DWORD dwGameInterface = 0x764818;
			DWORD dwGameInterfacePtr = 0x764818;

			__asm
			{
				push ebp
				mov eax, dwGameInterface
				mov eax, [eax]
				mov eax, [eax + 0x2E0]
				mov dwEffectManager, eax
				pop ebp
			}

			Console::print("health: %f\n", health);
			Console::print("dwGameInterfacePtr: %X\n", dwGameInterfacePtr);
			Console::print("0x764818 DWORD: %X\n", *(DWORD**)0x764818);
			Console::print("dwEffectManager: %X\n", *(DWORD*)(*(DWORD*)(DWORD)pGameInterface + 0x2E0));
			Console::print("ZGetEffectManager: %X\n", ZGetEffectManager());
			Console::print("ZEffectManager::GetInstance: %X\n", ZEffectManager::GetInstance());*/

			//ZEffectManager::GetInstance()->AddReBirthEffect(*pos);
			/*PLH::VTableSwap* vHook = new PLH::VTableSwap();
			vHook->SetupHook((uint8_t*)pChar, 28, (uint8_t*)OnDamagedSkill_Hook); // MClient::Post
			if (vHook->Hook())
			{
				Console::print("OnDamagedSkill_Hook successfully hooked!");
				//Console::print("OnDamagedSkill_Hook address: %p\n", vHook->GetOriginal<void*>());
			}*/

			//pGameInterface->GetEffectManager()->AddReBirthEffect(*pos);

			//Console::print("GetHP: %f\n", pCharOld->GetHP());
			//pos->z += 1.0f;
			/*pos->x += dir->x * 1000;
			pos->y += dir->y * 1000;
			pos->z += dir->z * 1000;*/
			//CALLASMAIN(ZPostRequestSpawnWorldItem(ZGameClient::GetInstance()->m_uidPlayer, item, pos, 0.f));
			//CALLASMAIN(ZPostShotSp(pos, dir, 2, 8));
			//CALLASMAIN(ZPostSkill(2, 7));
			//CALLASMAIN(ZPostShot(pos, dir, 1));
			//pChar->InitStatus();

			//ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*dwGame + 0x5C);

			/*DWORD dwThis = (DWORD)pChar;
			DWORD dwA = *(DWORD*)(dwThis + 0x8AC);
			DWORD dwB = DWORD(dwThis + 0x8AC);
			dwA -= dwB;*/

			/*DWORD dwThis = (DWORD)pChar;
			DWORD dwA = *(DWORD*)(dwThis + 0x22E);
			DWORD dwB = DWORD(dwThis + 0x22E);
			dwA -= dwB;
			dwA -= 0xB6;

			sprintf((char*)dwA, "%s", "TEST");*/
			//pChar->RenameTest((DWORD*)(dwThis + 0x22E));

			//Console::print("pChar::dwA = %X\n", dwA);
			//Console::print("pChar::dwA - 0xFF = %X\n", dwA - 0xFF);



			//pChar->UpdateSpeed();

			//RVisualMesh* pVMesh = pChar->m_pVMesh;
			//pVMesh->UpdateSpWeaponFire();

			//MTD_CharInfo* pCharInfo = pChar->GetCharInfo();
			//Console::print("pCharInfo nBP %d\n", pCharInfo->nBP);

			//ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			//ZGame* pGame = pGameInterface->GetGame();
			//ZCharacter* pChar = pGame->GetMyCharacter();

			//ZCharacterItem* pItems = pChar->GetItems();
			//pChar->InitStatus();


			// m_nWeaponType
			/*DWORD dwItems = (DWORD)pItems;
			DWORD dwA = *(DWORD*)(dwItems + 0xC);
			DWORD dwB = DWORD(dwItems + 0xC);
			dwA -= dwB;
			DWORD m_nWeaponType = (DWORD)((dwA - 0xD6) - 1);
			//DWORD nWeaponType = *(DWORD *)(*(DWORD *)(dwItems + 0xC) - (dwItems + 0xC) - 0xD6) - 1;

			dwA = *(DWORD*)(dwItems + 0x8C);
			dwB = DWORD(dwItems + 0x8C);
			dwA -= dwB;
			float fDamage = (float)((dwA - 0xD6));
			//float damage = (float)*(signed int *)(*(DWORD *)(dwItems + 0x8C) - (dwItems + 0x8C) - 0xD6) * 1.5;

			Console::print("pChar = %X\n", pChar);
			Console::print("pItems = %X\n", pItems);
			Console::print("m_nWeaponType = %d\n", m_nWeaponType);*/

			//ZGame_ApplyPotion((ZGame*)pGame, 2000, pChar, 10.0f);

			//Console::print("nWeaponType = %d\n", nWeaponType);

			//ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			//ZGame* pGame = pGameInterface->GetGame();

			//g_vtableHook = new Utils::VtableHook((VTABLE*)pChar);
			//g_vtableHook->HookIndex(28, (VFUNC)OnDamagedSkill);
		}

		if (GetAsyncKeyState(VK_DELETE) & 0x8000)
		{
			continue;
			CALLASMAIN(ZPostSkill(0, 7));
			CALLASMAIN(ZPostSkill(1, 7));
			CALLASMAIN(ZPostSkill(2, 7));
			CALLASMAIN(ZPostSkill(3, 7));
			CALLASMAIN(ZPostSkill(4, 7));
			CALLASMAIN(ZPostSkill(5, 7));
		}

		if (GetAsyncKeyState(VK_INSERT) & 0x8000)
		{
			//continue;
			/*CALLASMAIN(ZPostSkill(0, 7));
			CALLASMAIN(ZPostSkill(1, 7));
			CALLASMAIN(ZPostSkill(2, 7));
			CALLASMAIN(ZPostSkill(3, 7));
			CALLASMAIN(ZPostSkill(4, 7));
			CALLASMAIN(ZPostSkill(5, 7));*/

			//CALLASMAIN(ZPostSkill(3, 7));

			ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			ZGame* pGame = pGameInterface->GetGame();

			if (!pGame || !(*(DWORD*)pGame) || !*(DWORD*)(*(DWORD*)pGame + 0x5C))
				continue;

			/*ZMyCharacter* pMyChar = pGame->GetMyCharacter();
			ZCharacterOld* pCharOld = (ZCharacterOld*)*(DWORD*)(*(DWORD*)pGame + 0x5C);
			D3DXVECTOR3* pos = pCharOld->GetPosition();
			D3DXVECTOR3* dir = pCharOld->GetDirection();*/

			//CALLASMAIN(ZPostSkill(2, 7));
			//CALLASMAIN(ZPostShot(pos, dir, 2));

			if (pGame && *(DWORD*)pGame + 0x38)
			{
				ZMyCharacter* pChar = (ZMyCharacter*)*(DWORD*)(*(DWORD*)pGame + 0x5C);
				ZCharacterOld* pCharOld = (ZCharacterOld*)*(DWORD*)(*(DWORD*)pGame + 0x5C);

				//DWORD* dwLastTime = (DWORD*)(*(DWORD*)pGame + 0x38);
				//dwLastTime[3] = 1;
				//CALLASMAIN(ZPostShot(pos, dir, 0));
				/*CALLASMAIN(ZPostSkill(0, 7));
				CALLASMAIN(ZPostSkill(1, 7));
				CALLASMAIN(ZPostSkill(2, 7));
				CALLASMAIN(ZPostSkill(3, 7));
				CALLASMAIN(ZPostSkill(4, 7));
				CALLASMAIN(ZPostSkill(5, 7));*/
				//CALLASMAIN(ZPostShotSp(*pos, *dir, 6, 8, pCharOld->GetCommandInt()));

				//pChar->InitModuleStatus();
				//CALLASMAIN(ZPostShotMelee(pCharOld->GetPosition(), 1, pCharOld->GetCommandInt()));
				//Console::print("Get Time: %f", pGame->GetTime());
				//Console::print("Get Time: %f", *(float *)(*(DWORD*)&pGame->_pad[0x2C] - ((DWORD)pGame + 0x2C) - 0xD6));
				//pCharOld->SetShotTime(1000.0f);
				//Console::print("m_f1ShotTime: %f", pCharOld->GetShotTime());
				//pChar->SetStunType(ZSTUNTYPE::ZST_NONE);
				//Console::print("stun type: %d", pCharOld->GetStunType());
				//pChar->SetPosition(D3DXVECTOR3(1, 1, 1));
				//pChar->SetInvincibleTime(0xFFFFFF);
				//ZPostReaction(15.f, ZR_CHARGING);

				//ZPostReaction(fShotTime++, ZR_CHARGED);

				/*DWORD* dwLastTime = (DWORD*)(*(DWORD*)pGame + 0x38);
				dwLastTime[3] = 1;
				ZPostReaction(fShotTime++, ZR_CHARGED);
				dwLastTime[3] = 1;
				pChar->ChargedShot();
				CALLASMAIN(ZPostShotMelee(pCharOld->GetPosition(), 1, pCharOld->GetCommandInt()));
				dwLastTime[3] = 1;*/

				DWORD* dw = (DWORD*)&pChar->GetZCharacterStatusBitPacking();
				DWORD* dw2 = (DWORD*)&pChar->GetZMyCharacterStatusBitPacking();
				//pChar->ChargedShot();
				//Console::print(" m_fSpawnTime: %f", (float*)(*(DWORD*)pChar + 0x203));
				/*DWORD* dw = (DWORD*)&pChar->GetZCharacterStatusBitPacking();
				DWORD* dw2 = (DWORD*)&pChar->GetZMyCharacterStatusBitPacking();
				DWORD* dwChar = (DWORD*)pChar;
				Console::print(" dwChar: %X", dwChar);
				Console::print(" m_bCharged: %d", *(dwChar + 0x8DC));
				Console::print(" m_bCharged: %d", *(bool*)(*(DWORD*)pChar + 0x8DC));
				Console::print(" m_bCharging: %d", *(bool*)(*(DWORD*)pChar + 0x8E0));
				Console::print(" m_bCharged: %d", (*(DWORD*)pChar + 0x8DC));
				Console::print(" m_bCharged: %d", (bool*)(*(DWORD*)pChar + 0x8DC));*/
				Console::print("1: %X %X", pChar->GetZCharacterStatusBitPacking(), &pChar->GetZCharacterStatusBitPacking());
				Console::print("2: %X %X", pChar->GetZMyCharacterStatusBitPacking(), &pChar->GetZMyCharacterStatusBitPacking());

				ZGameInterfaceOld* pGameInterfaceOld = (ZGameInterfaceOld*)pGameInterface;
				ZCombatInterface* pCombatInterface = pGameInterfaceOld->GetCombatInterface();

				Color targetColor;
				Color green = Color(144, 0, 255, 0);
				Color yellow = Color(144, 255, 255, 0);
				Color red = Color(144, 255, 0, 0);
				Color lowBlue = Color(144, 63, 170, 180);
				Color highBlue = Color(144, 7, 126, 255);
				Color color;

				ZCharacterManager* m_CharacterManager = (ZCharacterManager*)(*(DWORD*)pGame + 0x70);
				for (ZCharacterManager::iterator i = m_CharacterManager->begin(); i != m_CharacterManager->end(); i++)
				{
					ZCharacter* pCharacter = (ZCharacter*)i->second;
					pCharacter->OnBlast(rvector(0, 0, 0));
				}

				//*dw |= 2;

				/*for (int i = 0; i < 30; i++)
				{
					Console::print("%d = %d", i, *(dw2 + i));
				}*/

				//Console::print("velocity: %f", pChar->m_pModule_Movable->m_Velocity.z);
				//pChar->JumpChargedShot();

				//CALLASMAIN(ZPostShot(pCharOld->GetPosition(), pCharOld->GetPosition(), 1));
				//pChar->AddDelayedWork(fShotTime + 0.25f, ZDW_SHOT);
				//pChar->AddDelayedWork(fShotTime + 0.3f, ZDW_SLASH);
				//dwLastTime[3] = 1;
			}
			else
				Console::print("Missing:dwLastTime");

			//CALLASMAIN(ZPostSkill(2, 7));
			//CALLASMAIN(ZPostShot(pos, dir, 2));
			//CALLASMAIN(ZPostShotMelee(pCharOld->GetPosition(), 1, pCharOld->GetCommandInt()));
			/*CALLASMAIN(ZPostSkill(0, 7));
			CALLASMAIN(ZPostSkill(1, 7));
			CALLASMAIN(ZPostSkill(2, 7));
			CALLASMAIN(ZPostSkill(3, 7));
			CALLASMAIN(ZPostSkill(4, 7));
			CALLASMAIN(ZPostSkill(5, 7));*/

			/*CALLASMAIN(ZPostShot(pos, dir, 7));
			CALLASMAIN(ZPostShot(pos, dir, 1));*/

			//CALLASMAIN(ZPostShot(pos, dir, 0));
			//pChar->AddDelayedWork(fShotTime + 0.25f, ZDW_SHOT);
			//pChar->AddDelayedWork(fShotTime + 0.3f, ZDW_SLASH);

			/*MDrawContextR2* g_pDC = MDrawContextR2::GetInstance();
			g_pDC->SetFont(MFontManager::Get("FONTa12_O1Org"));
			g_pDC->Text(50, 50, "BLAH");*/

			//Sleep(350);
			//CALLASMAIN(ZPostDamage(ZGameClient::GetInstance()->m_uidPlayer, 600.0f, pCharOld->GetCommandInt()));
			//CALLASMAIN(ZPostShotSp(*pos, *dir, 6, 8, pCharOld->GetCommandInt()));


			// Suicide
			//CALLASMAIN(ZPostDie(ZGameClient::GetInstance()->m_uidPlayer));

			//CALLASMAIN(ZPostRequestTimeSync(1532395900055000, pCharOld->GetCommandInt()));

			//pChar->AddDelayedWork(fShotTime + 0.2f, ZDW_SHOT);
			//Sleep(50);
		}

#ifdef _IGNORE
		if (GetAsyncKeyState(VK_F1) & 0x8000)
		{
			ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			ZGame* pGame = pGameInterface->GetGame();
			ZCombatInterface* pCombatInterface = pGameInterface->GetCombatInterface();

			Console::print("pCombatInterface = %X", pCombatInterface);


			//ZCharacter* pTargetChar = pCombatInterface->GetTargetCharacter();

			//Console::print("GetTargetCharacter = %X", pTargetChar);

			/*if (pTargetChar)
			{
			D3DXVECTOR3* pos = pTargetChar->GetPosition();
			Console::print("pTargetChar = %f, %f, %f", pos->x, pos->y, pos->z);
			}*/

			//ZCharacter* pMyCharacter = pGame->GetMyCharacter();
			DWORD* dwGameInterface = (DWORD*)ZGameInterface::GetInstance();
			DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);

			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*dwGame + 0x5C);

			Console::print("pChar = %X", pChar);


			D3DXVECTOR3* dir = pChar->GetDirection();
			Console::print("Dir = (%f, %f, %f)", dir->x, dir->y, dir->z);

			// Lawnmower
			//CALLASMAIN(ZPostShot(pChar->GetPosition(), dir, 7));
			//CALLASMAIN(ZPostShot(pChar->GetPosition(), dir, 1));

			// Massives
			CALLASMAIN(ZPostSkill(2, 7));
			//CALLASMAIN(ZPostShot(pChar->GetPosition(), dir, 1));

			//SetInvincibleTime(pChar, 900000);

			//UnknonwInsideOnPeerSpawn(pChar, NULL, pChar->GetPosition(), pChar); - Works, duuno what it is - should check


			// Works, dunno
			/*typedef int(__thiscall* Unknown_1T)(DWORD a);
			Unknown_1T Unknown_1 = (Unknown_1T)0x40A010;
			Unknown_1(0x12);*/

			/*CALLASMAIN(ZPostSkill(0, 7));
			CALLASMAIN(ZPostSkill(1, 7));
			CALLASMAIN(ZPostSkill(2, 7));
			CALLASMAIN(ZPostSkill(3, 7));
			CALLASMAIN(ZPostSkill(4, 7));
			CALLASMAIN(ZPostSkill(5, 7));*/

			//CALLASMAIN(ZPostShot(pChar->GetPosition(), dir, 1));

			/*ZCharacterManager* m_CharacterManager = (ZCharacterManager*)(*dwGame + 0x70);
			Console::print("m_CharacterManager %X", m_CharacterManager);

			for (ZCharacterManager::iterator i = m_CharacterManager->begin(); i != m_CharacterManager->end(); i++)
			{
			ZCharacter* pCharacter = i->second;

			//if (pCharacter->GetUID() == pChar->GetUID())
			//	continue;

			//D3DXVECTOR3* pos = pCharacter->GetPosition();
			//Console::print("[%s] %X, (%f, %f, %f) HP: %f/%f", pCharacter->GetUsername(), pCharacter, pos->x, pos->y, pos->z, ZCharGetHP(pCharacter), ZCharGetMaxHP(pCharacter));
			//pChar->SetPosition(pos);
			//CALLASMAIN(pChar->SetPosition(pos));
			//pCharacter->ForceDie();

			//pCombatInterface->SetPickTarget(true, pCharacter);

			//(*(void(**)(void))(*(DWORD*)pCharacter + 0xAC))();
			//pCharacter->InitStatusMaybe();

			if (GetAsyncKeyState(VK_MENU) & 1)
			{
			(*(void(**)(void))(*(DWORD*)pCharacter + 0xAC))();
			}

			if (GetAsyncKeyState(VK_CONTROL) & 1)
			{
			pCharacter->InitStatusMaybe();
			}
			}*/

			//pCombatInterface->ChangeWeapon(ZCWT_SECONDARY);

			//char a[256];
			//sprintf_s(a, "width: %d", Mint::GetInstance()[0x8A]);
			//ZChatOutput(a, 0, 0, 0xFF00FF00);
			//MessageBox(NULL, a, NULL, NULL);

			//ZPostSpMotion(ZC_SPMOTION_BOW);
			/*CALLASMAIN(ZPostSkill(0, 7));
			CALLASMAIN(ZPostSkill(1, 7));
			CALLASMAIN(ZPostSkill(2, 7));
			CALLASMAIN(ZPostSkill(3, 7));
			CALLASMAIN(ZPostSkill(4, 7));
			CALLASMAIN(ZPostSkill(5, 7));*/
			//oHook = (tHook)DetourFunction((PBYTE)0x20E4090, (PBYTE)nHook);


			/*DWORD* dwGameInterface = (DWORD*)ZGameInterface::GetInstance();
			DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);
			DWORD* dwChar = (DWORD*)*(DWORD*)(*dwGame + 0x5C);
			DWORD m_pModule_HPAP = (DWORD)*(DWORD*)(dwChar + 0x86C);
			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*dwGame + 0x5C);
			ZCombatInterface* pCombatInterface = (ZCombatInterface*)*(DWORD*)(*dwGame + 0x2E8);
			ZCharacterManager* m_CharacterManager = (ZCharacterManager*)(*dwGame + 0x70);

			char a[256];
			sprintf_s(a, "m_pModule_HPAP %X  /  %X", pChar->m_pModule_HPAP, m_pModule_HPAP);
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "[a] m_fMaxHP %X, m_fMaxHP %f", pChar->m_pModule_HPAP->m_fMaxHP, pChar->m_pModule_HPAP->m_fMaxHP.Ref());
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "[a] Mask %X", pChar->m_pModule_HPAP->m_fMask);
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "[a] Mask %X", pChar->m_pModule_HPAP->m_fMask.Ref());
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			//sprintf_s(a, "[x] Mask %X", (*(DWORD*)(m_pModule_HPAP + 16) - (m_pModule_HPAP + 0x10) - 214));
			//ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "[b] Mask %X", pChar->m_pModule_HPAP->GetMask());
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			if (GetAsyncKeyState(VK_CONTROL) & 1)
			{
			DWORD v2 = (DWORD)pChar->m_pModule_HPAP;
			//sprintf_s(a, "Float %f", (float)(*(float *)(*(DWORD *)(v2 + 8) - (v2 + 8) - 0xD6) - *(float *)(*(DWORD *)(v2 + 16) - (v2 + 0x10) - 214)));
			sprintf_s(a, "Float %f", pChar->m_pModule_HPAP->GetHP());
			ZChatOutput(a, 0, 0, 0xFF00FF00);
			sprintf_s(a, "Mask %X", pChar->m_pModule_HPAP->GetMask());
			ZChatOutput(a, 0, 0, 0xFF00FF00);
			}


			if (GetAsyncKeyState(VK_MENU) & 1)
			{

			}*/

			/*for (ZCharacterManager::iterator i = m_CharacterManager->begin(); i != m_CharacterManager->end(); i++)
			{
			ZCharacter* pCharacter = i->second;
			pCharacter->ForceDie();

			//pCombatInterface->SetPickTarget(true, pCharacter);

			//(*(void(**)(void))(*(DWORD*)pCharacter + 0xAC))();
			//pCharacter->InitStatusMaybe();
			}*/

			//pCombatInterface->m_CrossHair->m_bVisible = !pCombatInterface->m_CrossHair->m_bVisible;

			//if (pCombatInterface->m_bCrossHairVisible)
			//	pCombatInterface->m_bCrossHairVisible = false;

			//float fDir[3] = {1.0f, 1.0f, 1.0f};
			//D3DXVECTOR3* dir = (D3DXVECTOR3*)(fDir);

			//CALLASMAIN(ZPostShot(pChar->GetPosition(), dir, 1));
			//AddDeniedCommand(MC_REQUEST_GIVE_ONESELF_UP);
			//ZPostSkill(2, 7);
			//ZPostHPAPInfo(100.f, 100.f);
			//ZPostHacker(1);
		}

		if (GetAsyncKeyState(VK_MBUTTON) & 1)
		{
			ZPostReload();

			ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			ZGame* pGame = pGameInterface->GetGame();
			ZCombatInterface* pCombatInterface = pGameInterface->GetCombatInterface();
		}

		if (GetAsyncKeyState(VK_F2) & 0x8000)
		{
			ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			ZGame* pGame = pGameInterface->GetGame();
			ZCombatInterface* pCombatInterface = pGameInterface->GetCombatInterface();
			//ZCharacter* pMyCharacter = pGame->GetMyCharacter();
			DWORD* dwGameInterface = (DWORD*)ZGameInterface::GetInstance();
			DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);
			//DWORD pChar = *(DWORD*)(*dwGame + 0x5C);
			//DWORD* dwChar = (DWORD*)*(DWORD*)(*dwGame + 0x5C);
			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*dwGame + 0x5C);
			//ZObjectCollision* m_Collision = (ZObjectCollision*)*(DWORD*)(*dwChar + 0x820);

			DWORD* dwCombatInterface = (DWORD*)(*dwGameInterface + 0x2E8);
			ZCharacter* pTargetChar = (ZCharacter*)*(DWORD*)(*dwCombatInterface + 0x33C);
			//BonusHealth(pChar);
			//PenaltyHealth(pChar);

			//if (GetAsyncKeyState(VK_CONTROL) & 1)
			//	pChar->SetAnimationUpper(ZC_STATE_UPPER_GUARD_BLOCK1);

			char a[256];
			//sprintf_s(a, "ZGameInterface: %X,   ZGame: %X/%X,     ZCharacter :%X/%X", ZGameInterface::GetInstance(), dwGame, ZGameInterface::GetInstance()->GetGame(), pChar, 
			//	ZGameInterface::GetInstance()->GetGame()->GetMyCharacter());
			//ZChatOutput(a, 0, 0, 0xFF00FF00);
			//sprintf_s(a, "pChar->nSex: %d", (DWORD*)(pChar + 0x8C0));
			//ZChatOutput(a, 0, 0, 0xFF00FF00);

			D3DXVECTOR3* pos = pChar->GetPosition();
			Console::print("pChar->GetPosition(%f, %f, %f)", pos->x, pos->y, pos->z);

			ZCharacterManager* m_CharacterManager = (ZCharacterManager*)(*dwGame + 0x70);
			Console::print("m_CharacterManager %X", m_CharacterManager);

			Console::print("GetTeamID %d", pChar->GetTeamID());

			Console::print("MAIN_THREAD_ID %X / %X", *(DWORD*)MAIN_THREAD_ID, *(DWORD**)MAIN_THREAD_ID);

			_SetCurrentThreadId(*(DWORD*)MAIN_THREAD_ID);

			Console::print("GetCurrentThreadId %X", GetCurrentThreadId());

			if (GetAsyncKeyState(VK_CONTROL) & 1)
			{
				//sprintf_s(a, "m_CharacterManager2->size() %d", m_CharacterManager->size());
				//ZChatOutput(a, 0, 0, 0xFF00FF00);

				//pos->x += 100.f;
				//pos->y += 100.f;
				//pos->z += 100.f;
				//pChar->SetPosition(pos);

				//ZPostCommand(pC);

				//MCommand* pC = ((MCommand*(*)(int))g_dwZNewCmd)(iCmd)
				MCommand* pC = ZNewCmd(MC_PEER_RELOAD);
				//ZPostCommand(pC);

				Console::print("pC %X", pC);
				//ZPostFriendList(MC_PEER_RELOAD);

				DWORD dwZNewCmd = 0x559950;
				DWORD dwZPostCommand = 0x559D50;

				pCombatInterface->ChangeWeapon(ZCWT_SECONDARY);

				/*__asm
				{
				//mov ecx, MC_PEER_RELOAD
				push ecx
				mov ecx, MC_PEER_RELOAD
				call dwZNewCmd
				mov ecx, eax
				call dwZPostCommand
				pop ecx
				}*/
			}

			if (/*dwCombatInterface && pTargetChar && */GetAsyncKeyState(VK_MENU) & 1)
			{
				//for (ZCharacterManager::iterator i = pGame->m_CharacterManager->begin(); i != pGame->m_CharacterManager->end(); i++)

				//pChar->InitStatusMaybe();

				for (ZCharacterManager::iterator i = m_CharacterManager->begin(); i != m_CharacterManager->end(); i++)
				{
					ZCharacter* pCharacter = i->second;

					//if (pCharacter->GetUID() == pChar->GetUID())
					//	continue;

					D3DXVECTOR3* pos = pCharacter->GetPosition();
					//Console::print("%X, (%f, %f, %f) HP: %f/%f", pCharacter, pos->x, pos->y, pos->z, ZCharGetHP(pCharacter), ZCharGetMaxHP(pCharacter));
					Console::print("%X, (%f, %f, %f)", pCharacter, pos->x, pos->y, pos->z, ZCharGetHP(pCharacter));
					//pChar->SetPosition(pos);
					//CALLASMAIN(pChar->SetPosition(pos));
					//pCharacter->ForceDie();

					//pCombatInterface->SetPickTarget(true, pCharacter);

					//(*(void(**)(void))(*(DWORD*)pCharacter + 0xAC))();
					//pCharacter->InitStatusMaybe();
				}

				//pTargetChar->SetAnimationUpper(ZC_STATE_UPPER_GUARD_BLOCK1);
				//sprintf_s(a, "pTargetChar: %X", pTargetChar);

				//pChar->m_Collision->SetCollideable(false);


				//m_Collision->SetCollideable(false);
				//ZChatOutput(a, 0, 0, 0xFF00FF00);
			}
		}

		if (GetAsyncKeyState(VK_F3) & 0x8000)
		{
			//ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			//ZGame* pGame = pGameInterface->GetGame();
			//ZCharacter* pMyCharacter = pGame->GetMyCharacter();
			DWORD* dwGameInterface = (DWORD*)ZGameInterface::GetInstance();
			DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);
			//DWORD pChar = *(DWORD*)(*dwGame + 0x5C);
			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*dwGame + 0x5C);
			CALLASMAIN(ZPostShotMelee(pChar->GetPosition(), 0, pChar->GetCommandInt()));
			//CALLASMAIN(ZPostShotMelee(pChar->GetPosition(), 1, pChar->GetCommandInt()));
			//OnScream(pChar);

			if (GetAsyncKeyState(VK_MENU) & 1)
				ZPostSkill(2, 7);

			//ZPostSkill(2, 7);
		}

		/*if (GetAsyncKeyState(VK_F3) & 0x8000)
		{
		ZPostSkill(2, 7);
		ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
		ZGame* pGame = pGameInterface->GetGame();
		ZCharacter* pMyCharacter = pGame->GetMyCharacter();

		//PenaltyHealth(pMyCharacter);
		//pMyCharacter->SetAnimationUpper(ZC_STATE_UPPER_GUARD_IDLE);
		}*/

		if (GetAsyncKeyState(VK_F4) & 0x8000)
		{
			//ZPostReaction(c, c);
			_ZPostReaction(0.15f, ZR_CHARGED);
			/*c++;

			if (c >= 4)
			c = 0;

			if (++cc >= 20)
			{
			cc = 0;
			Sleep(50);
			}*/
		}

		/*if (GetAsyncKeyState(VK_F5) & 0x8000)
		{
		ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
		ZGame* pGame = pGameInterface->GetGame();
		ZCharacter* pMyCharacter = pGame->GetMyCharacter();

		//ZPostReaction(1, 0x3F800000);
		//ZPostReaction(15.f, ZR_CHARGED);
		}*/

		if (GetAsyncKeyState(VK_F5) & 0x8000)
		{
			/*ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			ZGame* pGame = pGameInterface->GetGame();
			ZCharacter* pMyCharacter = pGame->GetMyCharacter();

			if (pMyCharacter)
			pMyCharacter->Revival();*/

			DWORD* dwGameInterface = (DWORD*)ZGameInterface::GetInstance();
			DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);
			//DWORD pChar = *(DWORD*)(*dwGame + 0x5C);
			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*dwGame + 0x5C);

			//if (GetAsyncKeyState(VK_CONTROL) & 1)
			//	pChar->m_bVisible = !pChar->m_bVisible;

			pChar->Revival();
		}

		if (GetAsyncKeyState(VK_F6) & 0x8000)
		{
			DWORD* dwGameInterface = (DWORD*)ZGameInterface::GetInstance();
			DWORD* dwGame = (DWORD*)(*dwGameInterface + 0x2FC);
			//DWORD pChar = *(DWORD*)(*dwGame + 0x5C);
			ZCharacter* pChar = (ZCharacter*)*(DWORD*)(*dwGame + 0x5C);

			//if (GetAsyncKeyState(VK_CONTROL) & 1)
			//pChar->m_bVisible = !pChar->m_bVisible;

			onRevival(pChar);

			/*ZGameInterface* pGameInterface = ZGameInterface::GetInstance();
			ZGame* pGame = pGameInterface->GetGame();
			ZCharacter* pMyCharacter = pGame->GetMyCharacter();
			pMyCharacter->Revival();*/
			//pMyCharacter->SetAnimationLower(ZC_STATE_LOWER_UPPERCUT);

			//DWORD v25 = *(DWORD*)(*(DWORD*)((DWORD)pGame + 0x2C) - ((DWORD)pGame + 0x2C) - 0xD6);

			char a[256];
			//sprintf_s(a, "pChar->m_pVMesh->m_SelectWeaponMotionType : %d\n", pMyCharacter->m_pVMesh->m_SelectWeaponMotionType);
			//sprintf_s(a, "pMyCharacter: %X\n", pMyCharacter);
			//ZChatOutput(a, 0, 0, 0xFF00FF00);

			//ZPostReaction(4, 0x3F800000);

			//sprintf_s(a, "m_nLevel : %d, m_nBP : %d, m_nXP : %d\n", ZGetMyInfo()->m_nLevel, ZGetMyInfo()->m_nBP, ZGetMyInfo()->m_nXP);
			//float* pos = GetPosition((DWORD*)((*(DWORD*)0x6EADB0 + 0x2FC) + 0x5C));
			//float* pos = GetPosition((DWORD*)((DWORD*)(*(DWORD*)0x6EADB0 + 0x2FC) + 0x5C));
			//ZGame* game = (ZGame*)(*(DWORD*)0x6EADB0);
			//DWORD* dwGame = (DWORD*)(*(DWORD*)0x6EADB0);
			//ZMatch* match = (ZMatch*)(*(DWORD*)(*dwGame) + 0x2FC);
			//DWORD dwMatch = *(DWORD*)(*(DWORD*)(game)+0x2FC);
			//DWORD* character = (DWORD*)(*(DWORD*)match + 0x5C);
			//DWORD character1 = *(DWORD*)(*(DWORD*)match + 0x5C);
			//DWORD character2 = *(DWORD*)(*(DWORD*)dwMatch + 0x5C);
			//DWORD character3 = *(DWORD*)(*(DWORD*)dwMatch + 0x5C);

			//ZGame* game = (ZGame*)(*(DWORD*)0x6EADB0);
			//DWORD* dwGame = (DWORD*)(*(DWORD*)0x6EADB0);
			//ZMatch* match = (ZMatch*)(*(DWORD*)(*dwGame) + 0x2FC);
			//DWORD* dwMatch = (DWORD*)(*(DWORD*)(*dwGame) + 0x2FC);
			//DWORD dwChar = *(DWORD*)(dwMatch + 0x5C);
			//DWORD* pInstance = (DWORD*)(*(DWORD*)0x6EADB0);
			//DWORD* pInstance = (DWORD*)0x6EADB0;
			//DWORD* pMatch = (DWORD*)*(DWORD*)(*pInstance + 0x2FC);
			//DWORD pChar = *(DWORD*)(pMatch + 0x5C);
			//DWORD *pGame = (DWORD*)0x006EADB0;


			//float* pos = GetPosition2((*(DWORD*)pChar() + 0x5C));
			//sprintf_s(a, "Position %f, %f, %f\n", pos[0], pos[1], pos[2]);
			//ZChatOutput(a, 0, 0, 0xFF00FF00);
			//DWORD pGame = 0x006EADB0;
			//ShowWidget((DWORD*)(0x006EADB0), "Greeter", 1, 0);
		}

		if (GetAsyncKeyState(VK_F7) & 0x8000)
		{
			char a[256];
			//DWORD* pInstance = (DWORD*)(*(DWORD*)0x6EADB0);
			//DWORD* pInstance = (DWORD*)0x6EADB0;
			//DWORD pMatch = *(DWORD*)(*pInstance + 0x2FC);
			//DWORD pChar = *(DWORD*)(pMatch + 0x5C);

			//ZGame* game = (ZGame*)(*(DWORD*)0x6EADB0);
			//DWORD* dwGame = (DWORD*)0x6EADB0;
			//GunzState* state = (GunzState*)(*dwGame + 0x2430);
			//GunzState state = GunzState::GUNZ_NA;

			//DWORD* dwGameInterface1 = (DWORD*)ZGameInterface::GetInstance();
			//DWORD* dwGame1 = (DWORD*)(*dwGameInterface1 + 0x2FC);
			//DWORD pChar = *(DWORD*)(*dwGame + 0x5C);
			//ZCharacter* pMyChar = (ZCharacter*)*(DWORD*)(*dwGame1 + 0x5C);

			//pMyChar->m_bVisible = false;
			//ZPostReaction(3, ZR_BE_DASH_UPPERCUT);


			//ZMatch* match = (ZMatch*)(*(DWORD*)(*dwGame) + 0x2FC);
			//DWORD* dwMatch = (DWORD*)(*(DWORD*)(*dwGame) + 0x2FC);
			//DWORD dwChar = *(DWORD*)(dwMatch + 0x5C);
			//sprintf_s(a, "pInstance: %X     *pInstance: %X\n", pInstance, *pInstance);
			//ZChatOutput(a, 0, 0, 0xFF00FF00);
			//DWORD* m_pInstance = (DWORD*)0x006EADB0;
			//sprintf_s(a, "dwGame1 %X, %X, *game: %X, GetGame: %X\n", dwGame1, ZGameInterface::GetInstance()->m_pGame, *ZGameInterface::GetInstance()->m_pGame, ZGameInterface::GetInstance()->GetGame());
			//sprintf_s(a, "dwGame1 %X, GetGame: %X\n", dwGame1, ZGameInterface::GetInstance()->GetGame());
			//ZChatOutput(a, 0, 0, 0xFF00FF00);
			//sprintf_s(a, "HP: %f\n", ZCharGetHP(pMyChar));
			//ZChatOutput(a, 0, 0, 0xFF00FF00);
			//sprintf_s(a, "pChar %X\n", *(DWORD*)(*(DWORD*)(pGameInterface + 0x2FC) + 0x5C));
			//ZChatOutput(a, 0, 0, 0xFF00FF00);

			//sprintf_s(a, "%X   ||   %X\n", (GetMatch() + 0x5C), (*(DWORD*)GetMatch() + 0x5C));
			//ZChatOutput(a, 0, 0, 0xFF00FF00);
			//sprintf_s(a, "character: %X character1: %X character2: %X character2: %X: NOW: %X\n", character, character1, character2, character3, (DWORD*)(*(DWORD*)dwMatch + 0x5C));
			//ZChatOutput(a, 0, 0, 0xFF00FF00);
		}

		if (GetAsyncKeyState(VK_F8) & 0x8000)
		{
			//ZMyInfo* info = ZGetMyInfo();

			/*__asm
			{
			popad

			mov eax, info
			lea ecx, [eax + 0x1E0]
			call 0x4AECA0
			add eax, 0x64
			mov ecx, 0x6B2


			pushad
			}*/

			/*char a[256];
			sprintf_s(a, "GameInterface : %X\n", ZGameInterface::GetInstance());
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "*(DWORD*)0x6EADB0 : %X\n", *(DWORD*)0x6EADB0);
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "(DWORD*)0x6EADB0 : %X\n", (DWORD*)0x6EADB0);
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "*(DWORD**)0x6EADB0 : %X\n", *(DWORD**)0x6EADB0);
			ZChatOutput(a, 0, 0, 0xFF00FF00);*/

			ZPostReaction(4, ZR_DISCHARGED);

			//sprintf_s(a, "m_pGame : %X\n", ZGameInterface::GetInstance()->m_pGame);
			//ZChatOutput(a, 0, 0, 0xFF00FF00);

			//sprintf_s(a, "m_pMyCharacter : %X\n", ZGameInterface::GetInstance()->m_pGame->m_pMyCharacter);
			//ZChatOutput(a, 0, 0, 0xFF00FF00);

			//ZChatOutput(a, 0, 0, 0xFF00FF00);

			/*char a[256];
			sprintf_s(a, "ZGetMyInfo: %X\n", info);
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "ZGetMyInfo a: %X\n", info->a);
			ZChatOutput(a, 0, 0, 0xFF00FF00);

			sprintf_s(a, "ZGetMyInfo HP: %d\n", ZGetHP(info->a));
			ZChatOutput(a, 0, 0, 0xFF00FF00);*/
		}

		if (GetAsyncKeyState(VK_F9) & 0x8000)
		{
			//ZMyInfo* info = ZGetMyInfo();

			ZPostReaction(5, ZR_END);

			char a[256];
			//ZChatOutput(a, 0, 0, 0xFF00FF00);

			//SetHP(50);
			//char a[256];
			//float* pos = (float*)((*(DWORD*)((DWORD*)0x6EADB0) + 0x2E8) + 0x33C);
			//sprintf_s(a, "pos: %f, %f, %f\n", pos[0], pos[1], pos[2]);
			//ZChatOutput(a, 0, 0, 0xFF00FF00);


			/*A* a = *(A**)0x6EADB0;

			char aa[256];
			printf_s("a: %X\n", a);
			sprintf_s(aa, "a: %X\n", a);
			MessageBox(NULL, aa, NULL, NULL);

			if (a)
			{
			printf_s("ADDR: %X\n", a);
			sprintf_s(aa, "ADDR: %X\n", a->aa);
			MessageBox(NULL, aa, NULL, NULL);

			if (a->aa)
			{
			sprintf_s(aa, "a->aa: %X\n", a->aa->aaa);
			MessageBox(NULL, aa, NULL, NULL);
			printf_s("a->aa: %X\n", a->aa);

			if (a->aa->aaa)
			{
			printf_s("a->aa->aaa: %X\n", a->aa->aaa);

			if (a->aa->aaa->demage)
			{
			sprintf_s(aa, "a->aa->aaa->demage: %f\n", a->aa->aaa->demage);
			MessageBox(NULL, aa, NULL, NULL);
			printf_s("a->aa->aaa->demage: %f\n", a->aa->aaa->demage);
			}
			}
			}
			}
			*/
			//
		}

#endif

		if (GetAsyncKeyState(VK_PRIOR) & 0x8000)	// Page Up
		{
			//mHackBase->cleanUpD3DHooks();
			ShutdownDLL();
		}


		Sleep(50);
	}

	return 0;
}

DWORD jmpReturn = NULL;

//typedef void(*gDetourHookFcuntion)(int a);
//gDetourHookFcuntion pDetourHookFcuntion;

typedef char (*gDetourHookFcuntion)(int a1, unsigned int a2, BYTE *a3, DWORD *a4);
gDetourHookFcuntion pDetourHookFcuntion;

/*void __declspec(naked) nDetourHookFunction(int a)
{
	__asm pushad;

	//printf_s("DetourHookFunction()\n");
	//ZChatOutput("OnScream", 0, 0, 0xFF00FF00);
	//Log("nDetourHookFunction");

	__asm ret 0;
	__asm popad;
	//__asm jmp[pDetourHookFcuntion];

	//pDetourHookFcuntion(a);
}*/


char __cdecl nDetourHookFunction(int a1, unsigned int a2, BYTE *a3, DWORD *a4)
{
	__asm pushad;

	//printf_s("DetourHookFunction()\n");
	//ZChatOutput("OnScream", 0, 0, 0xFF00FF00);
	//Log("nDetourHookFunction");
	//MessageBox(NULL, "nDetourHookFunction", NULL, NULL);

	//__asm ret 0;
	__asm popad;
	//__asm jmp[pDetourHookFcuntion];
	return pDetourHookFcuntion(0, 0, 0, 0);
}

void __cdecl nMlog(const char* pFormat, ...)
{
	Console::print("mlog");
	Console::print(pFormat);
}

//#define NO_STDIO_REDIRECT

typedef int(__fastcall* ZNewCmdFT)(int nCmd);
ZNewCmdFT ZNewCmdF;
int __fastcall hkZNewCmd(int nCmd)
{
	Console::print("ZNewCmd: %d\n", nCmd);
	return ZNewCmdF(nCmd);
}

/*std::shared_ptr<PLH::Detour> Detour_Ex(new PLH::Detour);

typedef int(__stdcall* tVEH)(int intparam);
tVEH oVEHTest;

std::shared_ptr<PLH::VEHHook> VEHHook_Ex;
__declspec(noinline) int __stdcall hkVEHTest(int param)
{
	auto ProtectionObject = VEHHook_Ex->GetProtectionObject();
	Console::print("ZNewCmd: %d\n", param);
	return oVEHTest(param);
}*/

#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")

ULONG(WINAPI *Real_GetAdapterInfo)(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen) = GetAdaptersInfo;

static const char characters[] =
"0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int charactersLength = sizeof(characters) - 1;

#define RandomChar characters[rand() % charactersLength]

ULONG WINAPI Mine_GetAdapterInfo(PIP_ADAPTER_INFO pAdapterInfo, PULONG pOutBufLen)
{
	auto ret = Real_GetAdapterInfo(pAdapterInfo, pOutBufLen);

	DWORD dwStartAddress = (DWORD)pAdapterInfo;
	DWORD dwAdapterInfo = (dwStartAddress + (0 * 0x280));
	DWORD* dwComboIndex = (DWORD*)(dwAdapterInfo + 0x4);
	DWORD* dwAdapterName = (DWORD*)(dwAdapterInfo + 0x8);

	for (int offset = 0; ; offset++)
	{
		_IP_ADAPTER_INFO* info = (_IP_ADAPTER_INFO*)(dwStartAddress + (offset * 0x280));

		if ((char)info->AdapterName[0] != '{')
			break;

		info->ComboIndex = rand() % 50;

		for (int i = 0; i < 8; i++)
		{
			srand(time(NULL) * i);
			info->Address[i] = rand() % 256;
		}


		// Spoof name
		int nameSize = strlen(info->AdapterName);

		for (int i = 0; i < nameSize; i++)
		{
			if (i == 0 || i == 9 || i == 14 || i == 19 || i == 24 || i == 37)
				continue;

			srand(time(NULL) * i);
			info->AdapterName[i] = RandomChar;
		}
	}

	return ret;
}

typedef void(__stdcall* GetEncryptMD5HashValueT)(char* szEncryptMD5Value, char* szModule);
GetEncryptMD5HashValueT GetEncryptMD5HashValue = (GetEncryptMD5HashValueT)0x57ABC0;

void hkGetEncryptMD5HashValue(char* szEncryptMD5Value, char* szModule)
{
	Console::print("hkGetEncryptMD5HashValue: %s %s", szEncryptMD5Value, szModule);
	//GetEncryptMD5HashValue(szEncryptMD5Value, szModule);
}

void* (__cdecl* Real_memcpy)(_Out_writes_bytes_all_(_Size) void * _Dst, _In_reads_bytes_(_Size) const void * _Src, _In_ size_t _Size) = memcpy;

void* __cdecl Mine_memcpy(_Out_writes_bytes_all_(_Size) void * _Dst, _In_reads_bytes_(_Size) const void * _Src, _In_ size_t _Size)
{
	Console::print("Mine_memcpy: %X %X %d", _Dst, _Src, _Size);
	return Real_memcpy(_Dst, _Src, _Size);
}


#pragma comment(lib, "Winmm.lib")

typedef void(__stdcall* Exit_T)();
Exit_T ZApplicationExit = (Exit_T)0x524360;

void hkZApplicationExit()
{
	Console::print("hkZApplicationExit");
}

void *DetourFunction(BYTE *src, const BYTE *dst, const int len)
{
	BYTE *jmp = (BYTE*)malloc(len + 5);
	DWORD dwback;

	VirtualProtect(src, len, PAGE_READWRITE, &dwback);

	memcpy(jmp, src, len);    jmp += len;

	jmp[0] = 0xE9;
	*(DWORD*)(jmp + 1) = (DWORD)(src + len - jmp) - 5;

	src[0] = 0xE9;
	*(DWORD*)(src + 1) = (DWORD)(dst - src) - 5;

	VirtualProtect(src, len, dwback, &dwback);

	return (jmp - len);
}

DWORD WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpszRerversed)
{
	HMODULE hHandle = NULL;
	HMODULE hAdvapi = NULL;
	HMODULE hKernel32 = NULL;


	// Perform actions based on the reason for calling.
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		//CreateLog();
		//Log("Injected");
		//Log("Injected 2");

		g_hModule = hModule;

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID &)Real_GetAdapterInfo, Mine_GetAdapterInfo);
		DetourTransactionCommit();

		/*DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID &)ZApplicationExit, hkZApplicationExit);
		DetourTransactionCommit();*/

		/*DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID &)Real_PostMessage, Mine_PostMessage);
		DetourTransactionCommit();*/

		/*DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID &)Real_memcpy, Mine_memcpy);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID &)GetEncryptMD5HashValue, hkGetEncryptMD5HashValue);
		DetourTransactionCommit();*/

#ifndef _PUBLISH
		Console::CreateConsole("FGunz");
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Background, NULL, NULL, dwBackgroundThreadId);
#else
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Render, NULL, NULL, dwRenderThreadId);
#endif

		/*DWORD dwTimeGetTime = (DWORD)GetProcAddress(LoadLibrary("winmm.dll"), "timeGetTime");
		Console::print("dwTimeGetTime: %X", dwTimeGetTime);
		Console::print("timeGetTime: %X", timeGetTime);*/

		//mlog = (mlog_t)DetourFunction((PBYTE)mlog_offset, (PBYTE)nMlog);
		//Console::print("Hook mlog: %X/%X", mlog_offset, mlog);

		//ZNewCmdF = (ZNewCmdFT)DetourFunction((PBYTE)0x574430, (PBYTE)hkZNewCmd);

		/*Detour_Ex->SetupHook((BYTE*)0x574430, (BYTE*)&hkZNewCmd); //can cast to byte* to
		Detour_Ex->Hook();
		ZNewCmdF = Detour_Ex->GetOriginal<ZNewCmdFT>();*/

		/*Detour_Ex->SetupHook((BYTE*)mlog_offset, (BYTE*)&nMlog); //can cast to byte* to
		Detour_Ex->Hook();
		mlog = Detour_Ex->GetOriginal<mlog_t>();*/


		// 574430 MCommand__ZNewCmd

		//hHandle = GetModuleHandle("Iphlpapi.dll");
		//hAdvapi = GetModuleHandle("Advapi32.dll");
		//hKernel32 = GetModuleHandle("Kernel32.dll");

		/*if (hHandle || hAdvapi || hKernel32)
		{
		DWORD dwAddress = (DWORD)GetProcAddress(hHandle, "GetAdaptersInfo");
		//DWORD dwAdvapi = (DWORD)GetProcAddress(hAdvapi, "RegQueryValueEx");
		//DWORD dwAdvapi = (DWORD)GetProcAddress(hAdvapi, "GetCurrentHwProfile");
		//DWORD dwKernel32 = (DWORD)GetProcAddress(hKernel32, "GetVolumeInformation");

		//oHook = (tHook)DetourFunction((PBYTE)dwAddress, (PBYTE)nHook);
		//oHook2 = (tHook2)DetourFunction((PBYTE)dwAdvapi, (PBYTE)nHook2);
		//oHook3 = (tHook3)DetourFunction((PBYTE)dwKernel32, (PBYTE)nHook3);

		char _p[256];
		sprintf_s(_p, "Hook: %x", dwAddress);
		MessageBox(NULL, _p, "hook", NULL);
		}*/


		//oHook = (tHook)DetourFunction((PBYTE)0x20E4090, (PBYTE)nHook);
		//RedirectIOToConsole();
		//CreateConsole("FGunz");
		//pDetourHookFcuntion = (gDetourHookFcuntion)DetourFunction((PBYTE)0x20E5A6C, (PBYTE)nDetourHookFunction);
		//pDetourHookFcuntion = (gDetourHookFcuntion)DetourFunction((PBYTE)0x20E4090, (PBYTE)nDetourHookFunction);
		//oRevival = (tRevival)DetourFunction((PBYTE)0x4D58F0, (PBYTE)nRevival);
		//printf_s("FGunz.dll Injected\n");
		//cout << "BLAH" << endl;
		//fprintf(stdout, "BLAH stdout");
		//OutputDebugString("WUT");
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		break;
	}

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		//ShutdownDLL();
		//Detour_Ex->UnHook();
		break;
	}

	return TRUE;
}