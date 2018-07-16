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
#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "entities/CEntityDictionary.h"

#ifdef CLIENT_DLL
#include "hl/CClientPrediction.h"
#endif

#include "Weapons.h"

#include "CWeaponInfoCache.h"
#include "CWeaponInfo.h"

void RegisterAmmoTypes()
{
	g_AmmoTypes.Clear();

	g_AmmoTypes.SetCanAddAmmoTypes( true );

	// Precaches the ammo and queues the ammo info for sending to clients
	g_AmmoTypes.AddAmmoType( "buckshot", BUCKSHOT_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "9mm", _9MM_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "ARgrenades", M203_GRENADE_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "357", _357_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "uranium", URANIUM_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "rockets", ROCKET_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "bolts", BOLT_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "Trip Mine", TRIPMINE_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "Satchel Charge", SATCHEL_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "Hand Grenade", HANDGRENADE_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "Snarks", SNARK_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "Hornets", HORNET_MAX_CARRY );
#ifdef GAME_OPFOR
	g_AmmoTypes.AddAmmoType( "762", NATO762_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "556", NATO556_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "shock_rounds", SHOCK_ROUNDS_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "spores", SPORES_MAX_CARRY );
	g_AmmoTypes.AddAmmoType( "Penguins", PENGUINS_MAX_CARRY );
#endif

	//No more letting weapons define ammo types. - Solokiller
	g_AmmoTypes.SetCanAddAmmoTypes( false );
}

// Precaches the weapon and queues the weapon info for sending to clients
void UTIL_PrecacheOtherWeapon( const char* const pszClassname )
{
#ifdef SERVER_DLL
	CBaseEntity* pEntity = UTIL_CreateNamedEntity( pszClassname );

	if( !pEntity )
	{
		ALERT( at_console, "NULL Ent \"%s\" in UTIL_PrecacheOtherWeapon\n", pszClassname );
		return;
	}

	pEntity->Precache();

	UTIL_RemoveNow( pEntity );
#else
	auto pReg = GetEntityDict().FindEntityClassByEntityName( pszClassname );

	if( !pReg )
	{
		ALERT( at_console, "NULL Ent \"%s\" in UTIL_PrecacheOtherWeapon\n", pszClassname );
		return;
	}

	auto pev = g_Prediction.CreateEntity();

	CBaseEntity* pEntity = pReg->CreateInstance( pev );

	if( !pEntity )
	{
		//We're in deep shit if this happens because we can't free the entvars instance. - Solokiller
		ALERT( at_console, "NULL Ent \"%s\" in UTIL_PrecacheOtherWeapon\n", pszClassname );
		return;
	}

	CBasePlayerWeapon* pWeapon = dynamic_cast<CBasePlayerWeapon*>( pEntity );

	if( !pWeapon )
	{
		ASSERTSZ( false, "Check your class inheritance!" );
		ALERT( at_console, "UTIL_PrecacheOtherWeapon: \"%s\" is not a weapon!\n", pszClassname );
		return;
	}

	pWeapon->Spawn();

	g_Prediction.AddWeapon( pWeapon );
#endif
}

void PrecacheWeapons()
{
	g_WeaponInfoCache.ClearInfos();

	// shotgun
	UTIL_PrecacheOtherWeapon( "weapon_shotgun" );

	// crowbar
	UTIL_PrecacheOtherWeapon( "weapon_crowbar" );

	// glock
	UTIL_PrecacheOtherWeapon( "weapon_9mmhandgun" );

	// mp5
	UTIL_PrecacheOtherWeapon( "weapon_9mmAR" );

	// python
	UTIL_PrecacheOtherWeapon( "weapon_357" );

	// gauss
	UTIL_PrecacheOtherWeapon( "weapon_gauss" );

	// rpg
	UTIL_PrecacheOtherWeapon( "weapon_rpg" );

	// crossbow
	UTIL_PrecacheOtherWeapon( "weapon_crossbow" );

	// egon
	UTIL_PrecacheOtherWeapon( "weapon_egon" );

	// tripmine
	UTIL_PrecacheOtherWeapon( "weapon_tripmine" );

	// satchel charge
	UTIL_PrecacheOtherWeapon( "weapon_satchel" );

	// hand grenade
	UTIL_PrecacheOtherWeapon( "weapon_handgrenade" );

	// squeak grenade
	UTIL_PrecacheOtherWeapon( "weapon_snark" );

	// hornetgun
	UTIL_PrecacheOtherWeapon( "weapon_hornetgun" );

#ifdef GAME_OPFOR
	// Knife
	UTIL_PrecacheOtherWeapon( "weapon_knife" );

	// Pipe wrench
	UTIL_PrecacheOtherWeapon("weapon_pipewrench" );

	// Barnacle grapple
	UTIL_PrecacheOtherWeapon( "weapon_grapple" );

	// Desert Eagle
	UTIL_PrecacheOtherWeapon( "weapon_eagle" );

	// Penguin Bomb
	UTIL_PrecacheOtherWeapon( "weapon_penguin" );

	// M249
	UTIL_PrecacheOtherWeapon( "weapon_m249" );

	// Displacer Cannon
	UTIL_PrecacheOtherWeapon( "weapon_displacer" );

	// Sniper rifle
	UTIL_PrecacheOtherWeapon( "weapon_sniperrifle" );

	// Spore Launcher
	UTIL_PrecacheOtherWeapon( "weapon_sporelauncher" );

	// Shock Rifle
	UTIL_PrecacheOtherWeapon( "weapon_shockrifle" );
#endif
}

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, const Vector& vecMins, const Vector& vecMaxs, CBaseEntity* pEntity )
{
	int			i, j, k;
	float		distance;
	const float* const minmaxs[2] = { vecMins, vecMaxs };
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity->edict(), &tmpTrace );
	if ( tmpTrace.flFraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity->edict(), &tmpTrace );
				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}
