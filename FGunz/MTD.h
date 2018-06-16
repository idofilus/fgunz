#pragma once

// Clan
#define CLAN_NAME_LENGTH				16

// Character
#define MIN_CHARNAME					3
#define MAX_CHARNAME					12
#define MATCHOBJECT_NAME_LENGTH			32
#define MAX_CHARNAME_LENGTH				24

enum MMatchClanGrade
{
	MCG_NONE = 0,		// 클랜원이 아님
	MCG_MASTER = 1,		// 클랜 마스터
	MCG_ADMIN = 2,		// 클랜 운영자

	MCG_MEMBER = 9,		// 일반 클랜원
	MCG_END
};

enum MMatchCharItemParts
{
	MMCIP_HEAD = 0,
	MMCIP_CHEST = 1,
	MMCIP_HANDS = 2,
	MMCIP_LEGS = 3,
	MMCIP_FEET = 4,
	MMCIP_FINGERL = 5,
	MMCIP_FINGERR = 6,
	MMCIP_MELEE = 7,
	MMCIP_PRIMARY = 8,
	MMCIP_SECONDARY = 9,
	MMCIP_CUSTOM1 = 10,
	MMCIP_CUSTOM2 = 11,
	MMCIP_AVATAR = 12,
	MMCIP_COMMUNITY1 = 13,
	MMCIP_COMMUNITY2 = 14,
	MMCIP_LONGBUFF1 = 15,
	MMCIP_LONGBUFF2 = 16,
	MMCIP_END
};

using i32 = int32_t;

enum MMatchUserGradeID : i32
{
	MMUG_FREE = 0,
	MMUG_REGULAR = 1,
	MMUG_STAR = 2,

	MMUG_CRIMINAL = 100,
	MMUG_WARNING_1 = 101,
	MMUG_WARNING_2 = 102,
	MMUG_WARNING_3 = 103,
	MMUG_CHAT_LIMITED = 104,
	MMUG_PENALTY = 105,

	MMUG_VIP = 251,
	MMUG_EVENTMASTER = 252,
	MMUG_BLOCKED = 253,
	MMUG_DEVELOPER = 254,
	MMUG_ADMIN = 255
};

struct MTD_CharInfo
{
	char szName[MATCHOBJECT_NAME_LENGTH];
	char szClanName[CLAN_NAME_LENGTH];
	MMatchClanGrade	nClanGrade;
	unsigned short nClanContPoint;
	char nCharNum;
	unsigned short nLevel;
	char nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;

	unsigned long int	nEquipedItemDesc[MMCIP_END];

	MMatchUserGradeID	nUGradeID;

	// ClanCLID
	unsigned int		nClanCLID;

	int					nDTLastWeekGrade;

	MUID				uidEquipedItem[MMCIP_END];
	unsigned long int	nEquipedItemCount[MMCIP_END];
};