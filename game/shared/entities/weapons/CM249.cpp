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
#include "CBasePlayer.h"

#include "gamerules/GameRules.h"

#include "CM249.h"

BEGIN_DATADESC( CM249 )
	DEFINE_FIELD( m_flReloadEnd, FIELD_FLOAT ),
	DEFINE_FIELD( m_bReloading, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iFire, FIELD_INTEGER ),
	DEFINE_FIELD( m_iSmoke, FIELD_INTEGER ),
	DEFINE_FIELD( m_iLink, FIELD_INTEGER ),
	DEFINE_FIELD( m_iShell, FIELD_INTEGER ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_m249, CM249 );

CM249::CM249()
	: BaseClass( WEAPON_M249 )
{
}

void CM249::Precache()
{
	BaseClass::Precache();

	PRECACHE_MODEL( "models/v_saw.mdl" );
	PRECACHE_MODEL( "models/w_saw.mdl" );
	PRECACHE_MODEL( "models/p_saw.mdl" );

	m_iShell = PRECACHE_MODEL( "models/saw_shell.mdl" );
	m_iLink = PRECACHE_MODEL( "models/saw_link.mdl" );
	m_iSmoke = PRECACHE_MODEL( "sprites/wep_smoke_01.spr" );
	m_iFire = PRECACHE_MODEL( "sprites/xfire.spr" );

	PRECACHE_SOUND( "weapons/saw_reload.wav" );
	PRECACHE_SOUND( "weapons/saw_reload2.wav" );
	PRECACHE_SOUND( "weapons/saw_fire1.wav" );

	m_usFireM249 = PRECACHE_EVENT( 1, "events/m249.sc" );
}

void CM249::Spawn()
{
	Precache();

	SetModel( "models/w_saw.mdl" );

	m_bAlternatingEject = false;

	FallInit(); // get ready to fall down.
}

bool CM249::AddDuplicate( CBasePlayerWeapon* pOriginal )
{
	const bool bResult = BaseClass::AddDuplicate( pOriginal );

	//If the weapon's clip has been refilled, the body will be wrong. Correct it. - Solokiller
	pOriginal->SetBody( RecalculateBody( pOriginal->m_iClip ) );

#ifndef CLIENT_DLL
	//Update the client so its body value matches it. - Solokiller
	MESSAGE_BEGIN( MSG_ONE, gmsgWpnBody, nullptr, pOriginal->m_pPlayer->edict() );
		WRITE_BYTE( pOriginal->m_iId );
		WRITE_BYTE( pOriginal->GetBody() );
	MESSAGE_END();
#endif

	return bResult;
}

bool CM249::AddToPlayer( CBasePlayer* pPlayer )
{
	if( BaseClass::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, nullptr, pPlayer->edict() );
			WRITE_BYTE( m_iId );
		MESSAGE_END();

		return true;
	}

	return false;
}

bool CM249::Deploy()
{
	return DefaultDeploy( "models/v_saw.mdl", "models/p_saw.mdl", M249_DRAW, "mp5" );
}

void CM249::Holster()
{
	SetThink( nullptr );

	SendWeaponAnim( M249_HOLSTER );

	m_bReloading = false;

	m_fInReload = false;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10.0, 15.0 );
}

void CM249::WeaponIdle()
{
	ResetEmptySound();

	if( m_bReloading && UTIL_WeaponTimeBase() >= m_flReloadEnd )
	{
		m_bReloading = false;

		SetBody( 0 );

		SendWeaponAnim( M249_RELOAD_END, GetBody() );
	}

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		const float flNextIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

		int iAnim;

		if( flNextIdle <= 0.95 )
		{
			iAnim = M249_SLOWIDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		}
		else
		{
			iAnim = M249_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.16;
		}

		SendWeaponAnim( iAnim, GetBody() );
	}
}

void CM249::PrimaryAttack()
{
	if( m_pPlayer->GetWaterLevel() == WATERLEVEL_HEAD )
	{
		PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if( m_iClip <= 0 )
	{
		if( !m_fInReload )
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		}

		return;
	}

	--m_iClip;

	SetBody( RecalculateBody( m_iClip  ) );

	m_bAlternatingEject = !m_bAlternatingEject;

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_pPlayer->GetEffects() |= EF_MUZZLEFLASH;

	m_flNextAnimTime = UTIL_WeaponTimeBase() + 0.2;

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	UTIL_MakeVectors( m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle() );

	Vector vecSrc = m_pPlayer->GetGunPosition();

	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	Vector vecSpread;

	if( bIsMultiplayer() )
	{
		if( m_pPlayer->GetButtons().Any( IN_DUCK ) )
		{
			vecSpread = VECTOR_CONE_3DEGREES;
		}
		else if( m_pPlayer->GetButtons().Any( IN_MOVERIGHT |
										   IN_MOVELEFT | 
										   IN_FORWARD | 
										   IN_BACK ) )
		{
			vecSpread = VECTOR_CONE_15DEGREES;
		}
		else
		{
			vecSpread = VECTOR_CONE_6DEGREES;
		}
	}
	else
	{
		if( m_pPlayer->GetButtons().Any( IN_DUCK ) )
		{
			vecSpread = VECTOR_CONE_4DEGREES;
		}
		else if( m_pPlayer->GetButtons().Any( IN_MOVERIGHT |
										   IN_MOVELEFT |
										   IN_FORWARD |
										   IN_BACK ) )
		{
			vecSpread = VECTOR_CONE_10DEGREES;
		}
		else
		{
			vecSpread = VECTOR_CONE_2DEGREES;
		}
	}

	Vector vecDir = m_pPlayer->FireBulletsPlayer( 
		1, 
		vecSrc, vecAiming, vecSpread, 
		8192.0, BULLET_PLAYER_556, 2, 0, 
		m_pPlayer, m_pPlayer->random_seed );

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( 
		flags, m_pPlayer->edict(), m_usFireM249, 0, 
		g_vecZero, g_vecZero, 
		vecDir.x, vecDir.y, 
		GetBody(), 0, 
		m_bAlternatingEject ? 1 : 0, 0 );

	if( !m_iClip )
	{
		if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] <= 0 )
		{
			m_pPlayer->SetSuitUpdate( "!HEV_AMO0", SUIT_SENTENCE, SUIT_REPEAT_OK );
		}
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.067;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2;

#ifndef CLIENT_DLL
	Vector vecPunchAngle = m_pPlayer->GetPunchAngle();

	vecPunchAngle.x = UTIL_RandomFloat( -2, 2 );

	vecPunchAngle.y = UTIL_RandomFloat( -1, 1 );

	m_pPlayer->SetPunchAngle( vecPunchAngle );

	UTIL_MakeVectors( m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle() );

	const Vector& vecVelocity = m_pPlayer->GetAbsVelocity();

	const float flZVel = vecVelocity.z;

	//TODO: magic number - Solokiller
	Vector vecInvPushDir = gpGlobals->v_forward * 35.0;

	float flNewZVel = CVAR_GET_FLOAT( "sv_maxspeed" );

	if( vecInvPushDir.z >= 10.0 )
		flNewZVel = vecInvPushDir.z;

	if( !g_pGameRules->IsDeathmatch() )
	{
		Vector vecNewVel = vecVelocity - vecInvPushDir;

		//Restore Z velocity to make deathmatch easier.
		vecNewVel.z = flZVel;

		m_pPlayer->SetAbsVelocity( vecNewVel );
	}
	else
	{
		Vector vecNewVel = vecVelocity;

		const float flZTreshold = -( flNewZVel + 100.0 );

		if( vecVelocity.x > flZTreshold )
		{
			vecNewVel.x -= vecInvPushDir.x;
		}

		if( vecVelocity.y > flZTreshold )
		{
			vecNewVel.y -= vecInvPushDir.y;
		}

		vecNewVel.z -= vecInvPushDir.z;
	}
#endif
}

void CM249::Reload()
{
	if( DefaultReload( M249_RELOAD_START, 1.0, GetBody() ) )
	{
		m_bReloading = true;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.78;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.78;

		m_flReloadEnd = UTIL_WeaponTimeBase() + 1.33;
	}
}

void CM249::GetWeaponData( weapon_data_t& data )
{
	BaseClass::GetWeaponData( data );

	//Needed because the client may not get a chance to process the reload end if ping is low enough. - Solokiller
	data.iuser2 = GetBody();
	data.fuser2 = m_flReloadEnd;
}

void CM249::SetWeaponData( const weapon_data_t& data )
{
	BaseClass::SetWeaponData( data );

	SetBody( data.iuser2 );
	m_flReloadEnd = data.fuser2;
}

void CM249::DecrementTimers( float flTime )
{
	BaseClass::DecrementTimers( flTime );

	m_flReloadEnd = max( m_flReloadEnd - flTime, -0.001f );
}

int CM249::RecalculateBody( int iClip )
{
	if( iClip == 0 )
	{
		return 8;
	}
	else if( iClip >= 0 && iClip <= 7 )
	{
		return 9 - iClip;
	}
	else
	{
		return 0;
	}
}

#endif // GAME_OPFOR
