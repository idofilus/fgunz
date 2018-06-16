#pragma once

#include "Offset.h"

#define MC_PEER_RELOAD 10033

enum MCommandParameterType
{
	MPT_INT = 0,
	MPT_UINT = 1,
	MPT_FLOAT = 2,
	MPT_BOOL = 3,
	MPT_STR = 4,
	MPT_VECTOR = 5,
	MPT_POS = 6,
	MPT_DIR = 7,
	MPT_COLOR = 8,
	MPT_UID = 9,
	MPT_BLOB = 10,

	MPT_CHAR = 11,
	MPT_UCHAR = 12,
	MPT_SHORT = 13,
	MPT_USHORT = 14,
	MPT_INT64 = 15,
	MPT_UINT64 = 16,

	MPT_SVECTOR = 17,
	MPT_END = 18,
};

class MCommandParameter
{
public:
	void *VTable;							// 0x00
	MCommandParameterType m_nType;			// 0x04

	void *operator new(unsigned int nSize)
	{
		return ((void*(*)(int))OFFSET_MCommand_ZNewCmd)(nSize);
	}
}; // 0x08

class MCommandParameterVector : public MCommandParameter
{
public:
	float	m_fX;		// 0x08
	float	m_fY;		// 0x0C
	float	m_fZ;		// 0x10

	MCommandParameterVector(void)
	{
		m_fX = m_fY = m_fZ = 0;
		m_nType = MPT_VECTOR;
	}

	MCommandParameterVector(float x, float y, float z)
	{
		m_fX = x;
		m_fY = y;
		m_fZ = z;
		m_nType = MPT_VECTOR;
	}
};

class MCommandParameterPos : public MCommandParameterVector
{
public:
	MCommandParameterPos(void) : MCommandParameterVector() 
	{
		VTable = (void*)0x712264;
		m_nType = MPT_POS; 
	}

	MCommandParameterPos(float x, float y, float z) : MCommandParameterVector(x, y, z) 
	{
		VTable = (void*)0x712264;
		m_nType = MPT_POS; 
	}
};

class MCommand
{
public:
	void AddParameter(MCommandParameter *pCmdParam)
	{
		typedef void(__thiscall* AddParameterT)(MCommand* thisptr, MCommandParameter *pCmdParam);
		AddParameterT AddParameter = (AddParameterT)OFFSET_MCommand_MCommandAddParameter;
		return AddParameter(this, pCmdParam);
	}
};