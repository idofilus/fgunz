#pragma once

class ZSoundEngine
{
public:

public:
	static ZSoundEngine* GetInstance()
	{
		return *(ZSoundEngine**)OFFSET_ZSoundEngine;
	}

	// only 2D sound
	// TODO: This not worked
	int PlaySound(char* Name, bool bLoop = false, DWORD dwDelay = 0)
	{
		typedef int(__thiscall* PlaySoundT)(ZSoundEngine* thisPtr, char* Name, bool bLoop, DWORD dwDelay);
		PlaySoundT PlaySoundF = (PlaySoundT)OFFSET_ZSoundEngine_PlaySound;
		return PlaySoundF(this, Name, bLoop, dwDelay);
	}
};