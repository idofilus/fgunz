#pragma once

class ZGame;
class ZEffectManager;

class ZGameInterface
{
public:
	char _pad[0x2E0];						// 0x000
	ZEffectManager** m_pEffectManager;		// 0x2E0
	//char _pad[0x2FC];	// 0x000
	//ZGame* pGame;		// 0x2FC


	unsigned long int m_nDrawCount; // 0x2484

public:
	ZGame* GetGame()
	{
		return (ZGame*)(*(DWORD*)this + 0x2FC);
	}

	static ZGameInterface* GetInstance()
	{
		return *(ZGameInterface**)OFFSET_ZGameInterface;
	}
};