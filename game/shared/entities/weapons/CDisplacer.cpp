#ifdef GAME_OPFOR

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
#include "Weapons.h"
#include "CBasePlayer.h"

#ifndef CLIENT_DLL
#include "entities/rope/CRope.h"
#include "entities/spawnpoints/CBaseSpawnPoint.h"
#include "entities/weapons/CDisplacerBall.h"

#include "gamerules/GameRules.h"
#endif

#include "CDisplacer.h"

extern CBaseEntity* g_pLastSpawn;

#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS( info_displacer_xen_target, CPointEntity );
#endif

BEGIN_DATADESC( CDisplacer )
	DEFINE_THINKFUNC( SpinupThink ),
	DEFINE_THINKFUNC( AltSpinupThink ),
	DEFINE_THINKFUNC( FireThink ),
	DEFINE_THINKFUNC( AltFireThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_displacer, CDisplacer );

CDisplacer::CDisplacer()
	: BaseClass( WEAPON_DISPLACER )
{
}

void CDisplacer::Precache()
{
	BaseClass::Precache();

	PRECACHE_MODEL( "models/v_displacer.mdl" );
	PRECACHE_MODEL( "models/w_displacer.mdl" );
	PRECACHE_MODEL( "models/p_displacer.mdl" );

	PRECACHE_SOUND( "weapons/displacer_fire.wav" );
	PRECACHE_SOUND( "weapons/displacer_self.wav" );
	PRECACHE_SOUND( "weapons/displacer_spin.wav" );
	PRECACHE_SOUND( "weapons/displacer_spin2.wav" );

	PRECACHE_SOUND( "buttons/button11.wav" );

	m_iSpriteTexture = PRECACHE_MODEL( "sprites/shockwave.spr" );

	UTIL_PrecacheOther( "displacer_ball" );

	m_usFireDisplacer = PRECACHE_EVENT( 1, "events/displacer.sc" );
}

void CDisplacer::Spawn()
{
	Precache();

	SetModel( "models/w_displacer.mdl" );

	FallInit();
}

bool CDisplacer::AddToPlayer( CBasePlayer* pPlayer )
{
	if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return true;
	}
	return false;
}

bool CDisplacer::Deploy()
{
	return DefaultDeploy( 
		"models/v_displacer.mdl", "models/p_displacer.mdl", 
		DISPLACER_DRAW, "egon" );
}

void CDisplacer::Holster()
{
	m_fInReload = false;

	EMIT_SOUND_DYN( 
		m_pPlayer, 
		CHAN_WEAPON, "weapons/displacer_spin.wav", 
		0, ATTN_NONE, SND_STOP, PITCH_NORM );

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 5.0 );

	SendWeaponAnim( DISPLACER_HOLSTER1 );

	if( m_pfnThink == &CDisplacer::SpinupThink )
	{
		m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] -= 20;
		SetThink( nullptr );
	}
	else if( m_pfnThink == &CDisplacer::AltSpinupThink )
	{
		m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] -= 60;
		SetThink( nullptr );
	}
}

void CDisplacer::WeaponIdle()
{
	ResetEmptySound();

	if( m_flSoundDelay != 0 && gpGlobals->time >= m_flSoundDelay )
		m_flSoundDelay = 0;

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		const float flNextIdle = UTIL_RandomFloat( 0, 1 );

		int iAnim;

		if( flNextIdle <= 0.5 )
		{
			iAnim = DISPLACER_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}
		else
		{
			iAnim = DISPLACER_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}

		SendWeaponAnim( iAnim );
	}
}

void CDisplacer::PrimaryAttack()
{
	if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] >= 20 )
	{
		SetThink( &CDisplacer::SpinupThink );

		SetNextThink( gpGlobals->time );

		m_Mode = Mode::STARTED;

		EMIT_SOUND_DYN( 
			m_pPlayer, 
			CHAN_WEAPON, "weapons/displacer_spin.wav", 
			UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM, 0, PITCH_NORM );

		m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.5;
	}
	else
	{
		EMIT_SOUND_DYN(
			m_pPlayer,
			CHAN_WEAPON, "buttons/button11.wav",
			UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM, 0, PITCH_NORM );

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CDisplacer::SecondaryAttack()
{
	if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] >= 60 )
	{
		SetThink( &CDisplacer::AltSpinupThink );

		SetNextThink( gpGlobals->time );

		m_Mode = Mode::STARTED;

		EMIT_SOUND_DYN(
			m_pPlayer,
			CHAN_WEAPON, "weapons/displacer_spin2.wav",
			UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM, 0, PITCH_NORM );

		m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5;
	}
	else
	{
		EMIT_SOUND_DYN(
			m_pPlayer,
			CHAN_WEAPON, "buttons/button11.wav",
			UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM, 0, PITCH_NORM );

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CDisplacer::Reload()
{
	//Nothing
}

void CDisplacer::SpinupThink()
{
	if( m_Mode == Mode::STARTED )
	{
		SendWeaponAnim( DISPLACER_SPINUP );

		m_Mode = Mode::SPINNING_UP;

		int flags;

#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(
			flags,
			m_pPlayer->edict(),
			m_usFireDisplacer,
			0,
			g_vecZero, g_vecZero,
			0, 0,
			static_cast<int>( m_Mode ), 0,
			0, 0 );

		m_flStartTime = gpGlobals->time;
		m_iSoundState = 0;
	}

	if( m_Mode <= Mode::SPINNING_UP )
	{
		if( gpGlobals->time > m_flStartTime + 0.9 )
		{
			m_Mode = Mode::SPINNING;

			SetThink( &CDisplacer::FireThink );

			SetNextThink( gpGlobals->time + 0.1 );
		}

		m_iImplodeCounter = static_cast<int>( floorf( ( gpGlobals->time - m_flStartTime ) * 100.0 + 50.0 ) );
	}

	if( m_iImplodeCounter > 250 )
		m_iImplodeCounter = 250;

	m_iSoundState = 128;

	SetNextThink( gpGlobals->time + 0.1 );
}

void CDisplacer::AltSpinupThink()
{
	if( m_Mode == Mode::STARTED )
	{
		SendWeaponAnim( DISPLACER_SPINUP );

		m_Mode = Mode::SPINNING_UP;

		int flags;

#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL( 
			flags, 
			m_pPlayer->edict(), 
			m_usFireDisplacer, 
			0, 
			g_vecZero, g_vecZero, 
			0, 0, 
			static_cast<int>( m_Mode ), 0, 
			0, 0 );

		m_flStartTime = gpGlobals->time;
		m_iSoundState = 0;
	}

	if( m_Mode <= Mode::SPINNING_UP )
	{
		if( gpGlobals->time > m_flStartTime + 0.9 )
		{
			m_Mode = Mode::SPINNING;

			SetThink( &CDisplacer::AltFireThink );

			SetNextThink( gpGlobals->time + 0.1 );
		}

		m_iImplodeCounter = static_cast<int>( floorf( ( gpGlobals->time - m_flStartTime ) * 100.0 + 50.0 ) );
	}

	if( m_iImplodeCounter > 250 )
		m_iImplodeCounter = 250;

	m_iSoundState = 128;

	SetNextThink( gpGlobals->time + 0.1 );
}

void CDisplacer::FireThink()
{
	m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] -= 20;

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->GetEffects() |= EF_MUZZLEFLASH;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(
		flags,
		m_pPlayer->edict(),
		m_usFireDisplacer,
		0,
		g_vecZero, g_vecZero,
		0, 0,
		static_cast<int>( Mode::FIRED ), 0,
		0, 0 );

#ifndef CLIENT_DLL
	const Vector vecAnglesAim = m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle();

	UTIL_MakeVectors( vecAnglesAim );

	Vector vecSrc = m_pPlayer->GetGunPosition();

	Vector vecAngles = m_pPlayer->GetAutoaimVectorFromPoint( vecSrc, AUTOAIM_10DEGREES );

	CDisplacerBall::CreateDisplacerBall( vecSrc, vecAnglesAim + vecAngles, m_pPlayer );

	if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] == 0 )
	{
		m_pPlayer->SetSuitUpdate( "!HEV_AMO0", SUIT_SENTENCE,SUIT_REPEAT_OK );
	}
#endif

	SetThink( nullptr );
}

void CDisplacer::AltFireThink()
{
#ifndef CLIENT_DLL
	if( m_pPlayer->IsOnRope() )
	{
		m_pPlayer->SetMoveType( MOVETYPE_WALK );
		m_pPlayer->SetSolidType( SOLID_SLIDEBOX );
		m_pPlayer->SetOnRopeState( false );
		m_pPlayer->GetRope()->DetachObject();
		m_pPlayer->SetRope( nullptr );
	}
#endif

	STOP_SOUND( m_pPlayer, CHAN_WEAPON, "weapons/displacer_spin.wav" );

	m_pPlayer->SetDisplacerReturn( m_pPlayer->GetAbsOrigin() );

	m_pPlayer->SetDisplacerSndRoomtype( m_pPlayer->m_flSndRoomtype );

	//TODO: CTF support - Solokiller
#if 0
	if( g_pGameRules->IsCTF() && m_pPlayer->m_hFlag )
	{
		CCTFFlag* pFlag = EHANDLE_cast<CCTFFlag*>( m_pPlayer->m_hFlag );

		pFlag->DropFlag( m_pPlayer );
	}
#endif

#ifndef CLIENT_DLL
	CBaseEntity* pDestination;

	if( !g_pGameRules->IsMultiplayer() || g_pGameRules->IsCoOp() )
	{
		pDestination = UTIL_FindEntityByClassname( nullptr, "info_displacer_xen_target" );
	}
	else
	{
		pDestination = g_pGameRules->GetPlayerSpawnSpot( m_pPlayer );

		if( !pDestination )
			pDestination = g_pLastSpawn;

		Vector vecEnd = pDestination->GetAbsOrigin();

		vecEnd.z -= 100;

		TraceResult tr;

		UTIL_TraceLine( pDestination->GetAbsOrigin(), vecEnd, ignore_monsters, edict(), &tr );

		m_pPlayer->SetAbsOrigin( tr.vecEndPos + Vector( 0, 0, 37 ) );
	}

	if( pDestination && pDestination != CWorld::GetInstance() )
	{
		m_pPlayer->GetFlags().ClearFlags( FL_SKIPLOCALHOST );

		Vector vecNewOrigin = pDestination->GetAbsOrigin();

		if( !g_pGameRules->IsMultiplayer() || g_pGameRules->IsCoOp() )
		{
			vecNewOrigin.z += 37;
		}

		m_pPlayer->SetAbsOrigin( vecNewOrigin );

		m_pPlayer->SetAbsAngles( pDestination->GetAbsAngles() );

		m_pPlayer->SetViewAngle( pDestination->GetAbsAngles() );

		m_pPlayer->SetFixAngleMode( FIXANGLE_SET );

		m_pPlayer->SetBaseVelocity( g_vecZero );
		m_pPlayer->SetAbsVelocity( g_vecZero );
#endif

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2;

		SetThink( nullptr );

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		//Must always be handled on the server side in order to play the right sounds and effects. - Solokiller
		int flags = 0;

		PLAYBACK_EVENT_FULL(
			flags,
			m_pPlayer->edict(),
			m_usFireDisplacer,
			0,
			g_vecZero, g_vecZero,
			0, 0,
			static_cast<int>( Mode::FIRED ), 0,
			1, 0 );

		m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] -= 60;

		CDisplacerBall::CreateDisplacerBall( m_pPlayer->GetDisplacerReturn(), Vector( 90, 0, 0 ), m_pPlayer );

		if( !m_iClip )
		{
			if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] <= 0 )
				m_pPlayer->SetSuitUpdate( "!HEV_AMO0", SUIT_SENTENCE, SUIT_REPEAT_OK );
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase();

		if( !bIsMultiplayer() )
			m_pPlayer->SetGravity( 0.6 );
	}
	else
	{
		EMIT_SOUND_DYN(
			m_pPlayer,
			CHAN_WEAPON, "buttons/button11.wav",
			UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM, 0, PITCH_NORM );
	}
#endif
}

#endif // GAME_OPFOR
