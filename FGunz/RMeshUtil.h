#pragma once

enum RWeaponMotionType
{
	eq_weapon_etc = 0,	// weapon single,double

	eq_wd_katana,
	eq_ws_pistol,
	eq_wd_pistol,
	eq_wd_shotgun,
	eq_wd_rifle,
	eq_wd_grenade,
	eq_ws_dagger,
	eq_wd_item,
	eq_wd_rlauncher,// eq_wd_rocket_launcher
	eq_ws_smg,		// 10
	eq_wd_smg,		// 11
	eq_wd_sword,	// 12
	eq_wd_blade,	// 13
	eq_wd_dagger,	// 14

	eq_weapon_end,
};

typedef enum _RMeshPartsPosInfoType {
	eq_parts_pos_info_etc = 0,

	eq_parts_pos_info_Root,
	eq_parts_pos_info_Head,
	eq_parts_pos_info_HeadNub,
	eq_parts_pos_info_Neck,
	eq_parts_pos_info_Pelvis,
	eq_parts_pos_info_Spine,
	eq_parts_pos_info_Spine1,
	eq_parts_pos_info_Spine2,

	eq_parts_pos_info_LCalf,
	eq_parts_pos_info_LClavicle,
	eq_parts_pos_info_LFinger0,
	eq_parts_pos_info_LFingerNub,
	eq_parts_pos_info_LFoot,
	eq_parts_pos_info_LForeArm,
	eq_parts_pos_info_LHand,
	eq_parts_pos_info_LThigh,
	eq_parts_pos_info_LToe0,
	eq_parts_pos_info_LToe0Nub,
	eq_parts_pos_info_LUpperArm,

	eq_parts_pos_info_RCalf,
	eq_parts_pos_info_RClavicle,
	eq_parts_pos_info_RFinger0,
	eq_parts_pos_info_RFingerNub,
	eq_parts_pos_info_RFoot,
	eq_parts_pos_info_RForeArm,
	eq_parts_pos_info_RHand,
	eq_parts_pos_info_RThigh,
	eq_parts_pos_info_RToe0,
	eq_parts_pos_info_RToe0Nub,
	eq_parts_pos_info_RUpperArm,

	// 위치 추가 더미들

	//	eq_parts_pos_info_Lesser_Healing,
	//	eq_parts_pos_info_Healing,
	eq_parts_pos_info_Effect,

	eq_parts_pos_info_end
} RMeshPartsPosInfoType;