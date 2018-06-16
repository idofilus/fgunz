#pragma once

#define ZGetGame() (ZApplication::GetGameInterface() ? ZApplication::GetGameInterface()->GetGame() : NULL)

class ZApplication
{
public:

public:
	static ZGameInterface* GetGameInterface()
	{
		return *(ZGameInterface**)OFFSET_ZGameInterface;
	}
};