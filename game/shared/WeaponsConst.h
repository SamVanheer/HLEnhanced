/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef GAME_SHARED_WEAPONSCONST_H
#define GAME_SHARED_WEAPONSCONST_H

/**
*	@file
*	Constants for the weapons code.
*/

/**
*	Constant items. These are indices into CBasePlayer::m_rgItems.
*	@see MAX_ITEMS
*	@see CBasePlayer::m_rgItems
*/
enum ConstantItem
{
	ITEM_HEALTHKIT = 1,
	ITEM_ANTIDOTE = 2,
	ITEM_SECURITY = 3,
	ITEM_BATTERY = 4,
};

enum WeaponId
{
	WEAPON_NONE = 0,
	WEAPON_CROWBAR = 1,
	WEAPON_GLOCK = 2,
	WEAPON_PYTHON = 3,
	WEAPON_MP5 = 4,
	WEAPON_CHAINGUN = 5,
	WEAPON_CROSSBOW = 6,
	WEAPON_SHOTGUN = 7,
	WEAPON_RPG = 8,
	WEAPON_GAUSS = 9,
	WEAPON_EGON = 10,
	WEAPON_HORNETGUN = 11,
	WEAPON_HANDGRENADE = 12,
	WEAPON_TRIPMINE = 13,
	WEAPON_SATCHEL = 14,
	WEAPON_SNARK = 15,
#ifdef GAME_OPFOR
	WEAPON_BARNACLE_GRAPPLE	= 16,
	WEAPON_DESERT_EAGLE		= 17,
	WEAPON_PIPEWRENCH		= 18,
	WEAPON_M249				= 19,
	WEAPON_DISPLACER		= 20,
	WEAPON_SHOCKRIFLE		= 22,
	WEAPON_SPORELAUNCHER	= 23,
	WEAPON_SNIPERRIFLE		= 24,
	WEAPON_KNIFE			= 25,
	WEAPON_PENGUIN			= 26,
#endif
};

#define WEAPON_ALLWEAPONS	( ~( 1 << WEAPON_SUIT ) )

#define WEAPON_SUIT			31	// ?????

#define MAX_NORMAL_BATTERY	100

// weapon clip/carry ammo capacities
#define URANIUM_MAX_CARRY		100
#define	_9MM_MAX_CARRY			250
#define _357_MAX_CARRY			36
#define BUCKSHOT_MAX_CARRY		125
#define BOLT_MAX_CARRY			50
#define ROCKET_MAX_CARRY		5
#define HANDGRENADE_MAX_CARRY	10
#define SATCHEL_MAX_CARRY		5
#define TRIPMINE_MAX_CARRY		5
#define SNARK_MAX_CARRY			15
#define HORNET_MAX_CARRY		8
#define M203_GRENADE_MAX_CARRY	10
#ifdef GAME_OPFOR
#define NATO762_MAX_CARRY		15
#define NATO556_MAX_CARRY		200
#define SHOCK_ROUNDS_MAX_CARRY	10
#define SPORES_MAX_CARRY		20
#define PENGUINS_MAX_CARRY		9
#endif

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP			-1

//#define CROWBAR_MAX_CLIP		WEAPON_NOCLIP
#define GLOCK_MAX_CLIP			17
#define PYTHON_MAX_CLIP			6
#define MP5_MAX_CLIP			50
#define CROSSBOW_MAX_CLIP		5
#define RPG_MAX_CLIP			1
#define GAUSS_MAX_CLIP			WEAPON_NOCLIP
#define EGON_MAX_CLIP			WEAPON_NOCLIP
#define HORNETGUN_MAX_CLIP		WEAPON_NOCLIP
#define HANDGRENADE_MAX_CLIP	WEAPON_NOCLIP
#define SATCHEL_MAX_CLIP		WEAPON_NOCLIP
#define TRIPMINE_MAX_CLIP		WEAPON_NOCLIP
#define SNARK_MAX_CLIP			WEAPON_NOCLIP
#ifdef GAME_OPFOR
#define SNIPERRIFLE_MAX_CLIP	5
#endif

// The amount of ammo given to a player by an ammo item.
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_GLOCKCLIP_GIVE		GLOCK_MAX_CLIP
#define AMMO_357BOX_GIVE		PYTHON_MAX_CLIP
#define AMMO_MP5CLIP_GIVE		MP5_MAX_CLIP
#define AMMO_CHAINBOX_GIVE		200
#define AMMO_M203BOX_GIVE		2
#define AMMO_BUCKSHOTBOX_GIVE	12
#define AMMO_CROSSBOWCLIP_GIVE	CROSSBOW_MAX_CLIP
#define AMMO_RPGCLIP_GIVE		RPG_MAX_CLIP
#define AMMO_URANIUMBOX_GIVE	20
#define AMMO_SNARKBOX_GIVE		5
#ifdef GAME_OPFOR
#define AMMO_SNIPERRIFLE_GIVE	5
#define AMMO_556_GIVE			50
#endif

// bullet types
enum Bullet
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, // glock
	BULLET_PLAYER_MP5, // mp5
	BULLET_PLAYER_357, // python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR, // crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,

#ifdef GAME_OPFOR
	BULLET_PLAYER_556		= 9,
	BULLET_PLAYER_762		= 10,
	BULLET_PLAYER_DEAGLE	= 11,
#endif
};

#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define	BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define	DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define	WEAPON_ACTIVITY_VOLUME	64

#define	MELEE_BODYHIT_VOLUME 128
#define	MELEE_WALLHIT_VOLUME 512

#define VECTOR_CONE_1DEGREES	Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES	Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES	Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES	Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES	Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES	Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES	Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES	Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES	Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES	Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES	Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES	Vector( 0.17365, 0.17365, 0.17365 )

// special deathmatch shotgun spreads
#define VECTOR_CONE_DM_SHOTGUN	Vector( 0.08716, 0.04362, 0.00  )// 10 degrees by 5 degrees
#define VECTOR_CONE_DM_DOUBLESHOTGUN Vector( 0.17365, 0.04362, 0.00 ) // 20 degrees by 5 degrees

/**
*	Flag values for CWeaponInfo::m_iFlags
*	@see CWeaponData
*/
enum ItemInfoFlag
{
	ITEM_FLAG_NONE = 0,
	/**
	*	Used on the client to allow selection of empty weapons.
	*/
	ITEM_FLAG_SELECTONEMPTY = 1,

	/**
	*	Don't automatically reload this weapon.
	*/
	ITEM_FLAG_NOAUTORELOAD = 2,

	/**
	*	Don't automatically switch to another weapon if this weapon is empty.
	*/
	ITEM_FLAG_NOAUTOSWITCHEMPTY = 4,

	/**
	*	If the entity count is nearing the maximum, this entity's respawn will be delayed.
	*	Multiplayer only.
	*/
	ITEM_FLAG_LIMITINWORLD = 8,

	/**
	*	A player can totally exhaust their ammo supply and lose this weapon.
	*/
	ITEM_FLAG_EXHAUSTIBLE = 16,
};

/**
*	Converts the given item info flag to a string representation.
*/
const char* ItemInfoFlagToString( const ItemInfoFlag flag );

/**
*	Converts the given string to an item info flag.
*	@return Item info flag, or ITEM_FLAG_NONE if the string didn't match any other constant.
*/
ItemInfoFlag StringToItemInfoFlag( const char* const pszString );

enum class WpnOnTargetState
{
	NOT_ACTIVE_WEAPON	= 0,
	ACTIVE_WEAPON		= 1,
	ACTIVE_IS_ONTARGET	= 0x40
};

#endif //GAME_SHARED_WEAPONSCONST_H