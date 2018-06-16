#pragma once

#define OFFSET_ZGameInterface							0x764818

// ZCharacter
#define OFFSET_GetHP									0x4F0620		 // 8B 91 ? ? ? ? 8B 42 18 8D 4A 18 F3 0F 10 1D ? ? ? ? 2B C1 F3 0F 10 15 ? ? ? ? F3 0F 10 25 ? ? ? ? F3 0F 10 A8 ? ? ? ? 0F 54 DD 0F 28 C5 0F 57 C3 0F 28 CD F3 0F C2 C2 ? 0F 54 D0 0F 56 D3 F3 0F 58 CA F3 0F 5C CA 0F 28 C1 F3 0F 5C C5 F3 0F C2 C3 ? 0F 54 C4 F3 0F 5C C8 0F 57 C0 0F 2F C1 72 03 0F 28 E8 8B 4A 10 8D 42 10 2B C8 F3 0F 5C A9 ? ? ? ? 0F 28 C5
#define OFFSET_GetAP									0x4F06A0		 // 8B 91 ? ? ? ? 8B 42 24 8D 4A 24 F3 0F 10 1D ? ? ? ? 2B C1 F3 0F 10 15 ? ? ? ? F3 0F 10 25 ? ? ? ? F3 0F 10 A8 ? ? ? ? 0F 54 DD 0F 28 C5 0F 57 C3 0F 28 CD F3 0F C2 C2 ? 0F 54 D0 0F 56 D3 F3 0F 58 CA F3 0F 5C CA 0F 28 C1 F3 0F 5C C5 F3 0F C2 C3 ? 0F 54 C4 F3 0F 5C C8 0F 57 C0 0F 2F C1 72 03 0F 28 E8 8B 4A 10 8D 42 10 2B C8 F3 0F 5C A9 ? ? ? ? 0F 28 C5

#define OFFSET_ZPostReaction							0x4245D0		 // 83 EC 0C 56 57 8B F9 F3 0F 11 44 24

// MCommand
#define OFFSET_MCommand_ZNewCmd							0x574430		 // 55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 0C 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B C1
#define OFFSET_MCommand_MCommandParameterFloat			0x424580		 // 55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 0C 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B C1
#define OFFSET_MCommand_MCommandParameterFloatVTable	0x712264
#define OFFSET_MCommand_MCommandParameterPos			0x4F7800		 // 56 68 ? ? ? ? FF 15 ? ? ? ? 8B 35 ? ? ? ? 85 F6 74 17 8B 46 14 68 ? ? ? ? A3 ? ? ? ? FF 15 ? ? ? ? 8B C6 5E C3 6A 18 E8 ? ? ? ? FF 05 ? ? ? ? 83 C4 04 8B F0 68 ? ? ? ? FF 15 ? ? ? ? 8B C6 5E C3
#define OFFSET_MCommand_MCommandParameterPosVTable		0x70A844
#define OFFSET_MCommand_MCommandParameterVector			0x64EAC1		 // 55 8B EC 56 57 EB 1B
#define OFFSET_MCommand_MCommandParameterVectorVTable	0x712214
#define OFFSET_MCommand_MCommandParameterInt			0x41E1A0		 // 56 68 ? ? ? ? FF 15 ? ? ? ? 8B 35 ? ? ? ? 85 F6 74 17 8B 46 08 68 ? ? ? ? A3 ? ? ? ? FF 15 ? ? ? ? 8B C6 5E C3 6A 10 E8 ? ? ? ? FF 05 ? ? ? ? 83 C4 04 8B F0 68 ? ? ? ? FF 15 ? ? ? ? 8B C6 5E C3
#define OFFSET_MCommand_MCommandParameterIntVTable		0x7122B4
#define OFFSET_MCommand_MCommandParameterBlob			0x5A0B70		 // 55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 51 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F1 89 75 F0 C7 46
#define OFFSET_MCommand_MCommandParameterBlobVTable		0x7121C4
#define OFFSET_MCommand_MCommandParameterUID			0x424320		 // 56 68 ? ? ? ? FF 15 ? ? ? ? 8B 35 ? ? ? ? 85 F6 74 17 8B 46 08 68 ? ? ? ? A3 ? ? ? ? FF 15 ? ? ? ? 8B C6 5E C3 6A 14 E8 ? ? ? ? FF 05 ? ? ? ? 83 C4 04 8B F0 68 ? ? ? ? FF 15 ? ? ? ? 8B C6 5E C3
#define OFFSET_MCommand_MCommandParameterUIDVTable		0x7121EC
#define OFFSET_MCommand_MCommandAddParameter			0x59F700		 // 56 8D 71 1C C1 FA 02
#define OFFSET_MCommand_MCommandZPostCommand			0x574830		 // 55 8B EC 83 E4 F8 8B 15 ? ? ? ? 53

// MCommand Constructors
// 56 68 ? ? ? ? FF 15 ? ? ? ? 8B 35 ? ? ? ? 85 F6 74 17 8B 46 08 68 ? ? ? ? A3 ? ? ? ? FF 15 ? ? ? ? 8B C6 5E C3 6A 10 E8 ? ? ? ? FF 05 ? ? ? ? 83 C4 04 8B F0 68 ? ? ? ? FF 15 ? ? ? ? 8B C6 5E C3

// ZSoundEngine
#define OFFSET_ZSoundEngine								0x70BDE0		 // B9 ? ? ? ? E8 ? ? ? ? 8B 8C 24 ? ? ? ? 5F 5E 5D
#define OFFSET_ZSoundEngine_PlaySound					0x51C500		 // 83 EC 30 A1 ? ? ? ? 33 C4 89 44 24 2C 8B 44 24 34 56 8B F1

// ZEffectManager
#define OFFSET_ZEffectManager							0x54D8D0		 // A1 ? ? ? ? 85 C0 75 07 8B 80 ? ? ? ? C3 8B 00 8B 80 ? ? ? ? C3
#define OFFSET_ZEffectManager_AddReBirthEffect			0x4DA720		 // 55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 83 EC 14 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F9 8B 35

// ZGame
#define OFFSET_ZGame_OnExplosionGrenade					0x536E90		 // 55 8B EC 83 E4 F8 83 EC 58 A1 ? ? ? ? 33 C4 89 44 24 54 F3 0F 10 45
#define OFFSET_ZGame_PickHistory						0x541640		 // 83 EC 68 A1 ? ? ? ? 33 C4 89 44 24 64 8B 44 24 74
#define OFFSET_ZGame_Pick								0x541590		 // 83 EC 14 A1 ? ? ? ? 33 C4 89 44 24 10 8B 44 24 20
#define OFFSET_ZGame_CheckWall							0x536B40		 // 83 EC 58 A1 ? ? ? ? 33 C4 89 44 24 54 8B 54 24 60