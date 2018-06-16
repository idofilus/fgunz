#pragma once

class ZEffectManager
{
public:

public:
	static ZEffectManager* GetInstance()
	{
		typedef ZEffectManager* (__stdcall* Type)();
		Type ZGetEffectManager = (Type)0x54D8D0;
		return ZGetEffectManager();
	}

	void AddReBirthEffect(const D3DXVECTOR3& Target)
	{
		typedef void (__thiscall* Type)(ZEffectManager* thisPtr, const D3DXVECTOR3& Target);
		Type function = (Type)OFFSET_ZEffectManager_AddReBirthEffect;
		function(this, Target);
	}
};