/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
// pm_shared.h
//
#if !defined( PM_SHAREDH )
#define PM_SHAREDH
#pragma once

void PM_Init( playermove_t *ppmove );

/**
*	@brief Called after game initialization, lets the physics code do post-init stuff that needs interfaces set up during init time
*/
void PM_PostInit();

void PM_Move ( playermove_t *ppmove, int server );

/**
*	Spectator Movement modes (stored in pev->iuser1, so the physics code can get at them)
*/
enum ObserverMode
{
	OBS_NONE			= 0,
	OBS_CHASE_LOCKED	= 1,
	OBS_CHASE_FREE		= 2,
	OBS_ROAMING			= 3,		
	OBS_IN_EYE			= 4,
	OBS_MAP_FREE		= 5,
	OBS_MAP_CHASE		= 6,
};

#endif
