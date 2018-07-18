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
//
//  cdll_dll.h

// this file is included by both the game-dll and the client-dll,

#ifndef GAME_SHARED_CDLL_DLL_H
#define GAME_SHARED_CDLL_DLL_H

/**
*	The maximum number of weapons that the engine can network. This is defined as 32 in the SDK, but can handle up to 64.
*	The weapons bitmask can only handle 32. Either use an iuser or another means of sending the weapon list.
*	 - Solokiller
*/
#define MAX_WEAPONS				64

/**
*	Hud item selection slots.
*	Now up to 10.
*/
#ifdef GAME_OPFOR
#define MAX_WEAPON_SLOTS		10
#else
#define MAX_WEAPON_SLOTS		5
#endif

/**
*	Hard coded item types.
*/
#define MAX_ITEMS				5

#define MAX_PLAYER_NAME_LENGTH	32

/**
*	Bit flags for hiding parts of the HUD.
*/
enum HideHUD
{
	/**
	*	Hide weapons info (ammo).
	*/
	HIDEHUD_WEAPONS		= 1 << 0,

	/**
	*	Hide flashlight icon.
	*/
	HIDEHUD_FLASHLIGHT	= 1 << 1,

	/**
	*	Hide all HUD elements.
	*/
	HIDEHUD_ALL			= 1 << 2,

	/**
	*	Hide health info.
	*/
	HIDEHUD_HEALTH		= 1 << 3,
};

/**
*	ClientPrint destinations.
*/
enum ClientPrintDest
{
	/**
	*	Print to notify area (top left).
	*/
	HUD_PRINTNOTIFY		= 1,

	/**
	*	Print to console.
	*/
	HUD_PRINTCONSOLE	= 2,

	/**
	*	Print to chat.
	*/
	HUD_PRINTTALK		= 3,

	/**
	*	Print to center of screen.
	*/
	HUD_PRINTCENTER		= 4,
};

/**
*	Maximum world coordinate for any axis. Negate for the opposite end of the axis.
*	TODO: If the maximum map size were ever increased, this should be updated as well. Editor and compiler hacks can allow for up to 16384, so account for that.
*/
#define WORLD_BOUNDARY 16384

/**
*	Maximum velocity for any axis. Used for validation only.
*	TODO: probably not the best way to handle this, seeing as sv_maxvelocity can be higher. - Solokiller
*/
#define MAX_VELOCITY 2000

/**
*	Maximum size of a single chunk of an MOTD to send to clients in one message.
*/
#define MAX_MOTD_CHUNK	  60

/**
*	The largest amount of data in an MOTD that can be sent to clients. Anything beyond this much is discarded.
*/
#define MAX_MOTD_LENGTH   1536 // (MAX_MOTD_CHUNK * 4)

#endif //GAME_SHARED_CDLL_DLL_H
