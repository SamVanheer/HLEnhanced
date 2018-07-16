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

#ifdef GAME_OPFOR

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "Weapons.h"

#include "CAmmo556.h"

LINK_ENTITY_TO_CLASS( ammo_556, CAmmo556 );

void CAmmo556::Precache()
{
	PRECACHE_MODEL( "models/w_saw_clip.mdl" );
	PRECACHE_SOUND( "items/9mmclip1.wav" );
}

void CAmmo556::Spawn()
{
	Precache();

	SetModel( "models/w_saw_clip.mdl" );

	BaseClass::Spawn();
}

bool CAmmo556::AddAmmo( CBaseEntity* pOther )
{
	return UTIL_GiveAmmoToPlayer( this, pOther, AMMO_556_GIVE, "556" );
}

#endif // GAME_OPFOR
