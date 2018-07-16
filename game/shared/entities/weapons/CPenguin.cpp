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

#include "CPenguin.h"

LINK_ENTITY_TO_CLASS( weapon_penguin, CPenguin );

CPenguin::CPenguin()
	: BaseClass( WEAPON_PENGUIN )
{
}

void CPenguin::Precache()
{
	BaseClass::Precache();

	PRECACHE_MODEL( "models/w_penguinnest.mdl" );
	PRECACHE_MODEL( "models/v_penguin.mdl" );
	PRECACHE_MODEL( "models/p_penguin.mdl" );

	PRECACHE_SOUND( "squeek/sqk_hunt2.wav" );
	PRECACHE_SOUND( "squeek/sqk_hunt3.wav" );

	UTIL_PrecacheOther( "monster_penguin" );

	m_usPenguinFire = PRECACHE_EVENT( 1, "events/penguinfire.sc" );
}

void CPenguin::Spawn()
{
	Precache();

	SetModel( "models/w_penguinnest.mdl" );

	FallInit();

	SetSequence( PENGUINNEST_IDLE );

	SetAnimTime( gpGlobals->time );
	SetFrameRate( 1 );
}

bool CPenguin::Deploy()
{
	// play hunt sound
	const float flRndSound = RANDOM_FLOAT( 0, 1 );

	if( flRndSound <= 0.5 )
		EMIT_SOUND_DYN( this, CHAN_VOICE, "squeek/sqk_hunt2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM );
	else
		EMIT_SOUND_DYN( this, CHAN_VOICE, "squeek/sqk_hunt3.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM );

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	//Opposing Force uses the penguin animation set, which doesn't exist. - Solokiller
	return DefaultDeploy( 
		"models/v_penguin.mdl", "models/p_penguin.mdl", 
		PENGUIN_UP, "squeak" );
}

void CPenguin::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if( !m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] )
	{
		m_pPlayer->GetWeapons().ClearFlags( 1 << m_iId );
		SetThink( &CPenguin::DestroyItem );
		SetNextThink( gpGlobals->time + 0.1 );
		return;
	}

	SendWeaponAnim( PENGUIN_DOWN );
	EMIT_SOUND( m_pPlayer, CHAN_WEAPON, "common/null.wav", VOL_NORM, ATTN_NORM );
}

void CPenguin::WeaponIdle()
{
	if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if( m_bJustThrown )
	{
		m_bJustThrown = false;

		if( !m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] )
		{
			RetireWeapon();
			return;
		}

		SendWeaponAnim( PENGUIN_UP );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		return;
	}

	int iAnim;
	float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
	if( flRand <= 0.75 )
	{
		iAnim = PENGUIN_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * ( 2 );
	}
	else if( flRand <= 0.875 )
	{
		iAnim = PENGUIN_FIDGETFIT;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0 / 16.0;
	}
	else
	{
		iAnim = PENGUIN_FIDGETNIP;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 16.0;
	}
	SendWeaponAnim( iAnim );
}

void CPenguin::PrimaryAttack()
{
	if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] )
	{
		UTIL_MakeVectors( m_pPlayer->GetViewAngle() );
		TraceResult tr;

		// HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players
		// Move origin up if crouched and start trace a bit outside of body ( 20 units instead of 16 )
		Vector trace_origin = m_pPlayer->GetAbsOrigin();
		if( m_pPlayer->GetFlags().Any( FL_DUCKING ) )
		{
			trace_origin = trace_origin - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
		}

		// find place to toss monster
		UTIL_TraceLine( trace_origin + gpGlobals->v_forward * 20, trace_origin + gpGlobals->v_forward * 64, dont_ignore_monsters, NULL, &tr );

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usPenguinFire, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );

		if( tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction > 0.25 )
		{
			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

#ifndef CLIENT_DLL
			CBaseEntity *pSqueak = CBaseEntity::Create( "monster_penguin", tr.vecEndPos, m_pPlayer->GetViewAngle(), m_pPlayer->edict() );
			pSqueak->SetAbsVelocity( gpGlobals->v_forward * 200 + m_pPlayer->GetAbsVelocity() );
#endif

			// play hunt sound
			float flRndSound = RANDOM_FLOAT( 0, 1 );

			if( flRndSound <= 0.5 )
				EMIT_SOUND_DYN( this, CHAN_VOICE, "squeek/sqk_hunt2.wav", 1, ATTN_NORM, 0, 105 );
			else
				EMIT_SOUND_DYN( this, CHAN_VOICE, "squeek/sqk_hunt3.wav", 1, ATTN_NORM, 0, 105 );

			m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

			--m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ];

			m_bJustThrown = true;

			m_flNextPrimaryAttack = GetNextAttackDelay( 1.9 );
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8;
		}
	}
}

void CPenguin::SecondaryAttack()
{
	//Nothing.
}

#endif // GAME_OPFOR
