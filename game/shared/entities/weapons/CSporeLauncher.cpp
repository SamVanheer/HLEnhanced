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

#ifndef CLIENT_DLL
#include "entities/weapons/CSpore.h"
#endif

#include "CSporeLauncher.h"

BEGIN_DATADESC( CSporeLauncher )
	DEFINE_FIELD( m_ReloadState, FIELD_INTEGER ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_sporelauncher, CSporeLauncher );

CSporeLauncher::CSporeLauncher()
	: BaseClass( WEAPON_SPORELAUNCHER )
{
}

void CSporeLauncher::Precache()
{
	BaseClass::Precache();

	PRECACHE_MODEL( "models/w_spore_launcher.mdl" );
	PRECACHE_MODEL( "models/v_spore_launcher.mdl" );
	PRECACHE_MODEL( "models/p_spore_launcher.mdl" );

	PRECACHE_SOUND( "weapons/splauncher_fire.wav" );
	PRECACHE_SOUND( "weapons/splauncher_altfire.wav" );
	PRECACHE_SOUND( "weapons/splauncher_bounce.wav" );
	PRECACHE_SOUND( "weapons/splauncher_reload.wav" );
	PRECACHE_SOUND( "weapons/splauncher_pet.wav" );

	UTIL_PrecacheOther( "spore" );

	m_usFireSpore = PRECACHE_EVENT( 1, "events/spore.sc" );
}

void CSporeLauncher::Spawn()
{
	Precache();

	SetModel( "models/w_spore_launcher.mdl" );

	FallInit();

	SetSequence( 0 );

	SetAnimTime( gpGlobals->time );

	SetFrameRate( 1 );
}

bool CSporeLauncher::AddToPlayer( CBasePlayer* pPlayer )
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

bool CSporeLauncher::Deploy()
{
	return DefaultDeploy( 
		"models/v_spore_launcher.mdl", "models/p_spore_launcher.mdl", 
		SPLAUNCHER_DRAW1, "rpg" );
}

void CSporeLauncher::Holster()
{
	m_ReloadState = ReloadState::NOT_RELOADING;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim( SPLAUNCHER_HOLSTER1 );
}

bool CSporeLauncher::ShouldWeaponIdle()
{
	return true;
}

void CSporeLauncher::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
	{
		if( m_iClip == 0 && m_ReloadState == ReloadState::NOT_RELOADING && m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] )
		{
			Reload();
		}
		else if( m_ReloadState != ReloadState::NOT_RELOADING )
		{
			if( m_iClip != GetWeaponInfo()->GetMaxMagazine() && m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] )
			{
				Reload();
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim( SPLAUNCHER_AIM );

				m_ReloadState = ReloadState::NOT_RELOADING;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.83;
			}
		}
		else
		{
			int iAnim;
			float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
			if( flRand <= 0.75 )
			{
				iAnim = SPLAUNCHER_IDLE;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;
			}
			else if( flRand <= 0.95 )
			{
				iAnim = SPLAUNCHER_IDLE2;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4;
			}
			else
			{
				iAnim = SPLAUNCHER_FIDGET;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4;

				EMIT_SOUND_DYN( 
					m_pPlayer,
					CHAN_ITEM, "weapons/splauncher_pet.wav",
					0.7, ATTN_NORM, 0, PITCH_NORM );
			}
			SendWeaponAnim( iAnim );
		}
	}
}

void CSporeLauncher::PrimaryAttack()
{
	if( m_iClip )
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		Vector vecAngles = m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle();

		UTIL_MakeVectors( vecAngles );

		const Vector vecSrc = 
			m_pPlayer->GetGunPosition() + 
			gpGlobals->v_forward * 16 +
			gpGlobals->v_right * 8 + 
			gpGlobals->v_up * -8;

		vecAngles = vecAngles + m_pPlayer->GetAutoaimVectorFromPoint( vecSrc, AUTOAIM_10DEGREES );

		CSpore* pSpore = CSpore::CreateSpore( 
			vecSrc, vecAngles,
			m_pPlayer, 
			CSpore::SporeType::ROCKET, false, false );

		UTIL_MakeVectors( vecAngles );

		pSpore->SetAbsVelocity( pSpore->GetAbsVelocity().Length() * gpGlobals->v_forward );
#endif

		int flags;

#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usFireSpore );

		--m_iClip;
	}

	m_flNextPrimaryAttack =
		m_flNextSecondaryAttack = 
		m_flTimeWeaponIdle =
		UTIL_WeaponTimeBase() + 0.5;

	m_ReloadState = ReloadState::NOT_RELOADING;
}

void CSporeLauncher::SecondaryAttack()
{
	if( m_iClip )
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		Vector vecAngles = m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle();

		UTIL_MakeVectors( vecAngles );

		const Vector vecSrc =
			m_pPlayer->GetGunPosition() +
			gpGlobals->v_forward * 16 +
			gpGlobals->v_right * 8 +
			gpGlobals->v_up * -8;

		vecAngles = vecAngles + m_pPlayer->GetAutoaimVectorFromPoint( vecSrc, AUTOAIM_10DEGREES );

		CSpore* pSpore = CSpore::CreateSpore(
			vecSrc, vecAngles,
			m_pPlayer,
			CSpore::SporeType::GRENADE, false, false );

		UTIL_MakeVectors( vecAngles );

		pSpore->SetAbsVelocity( m_pPlayer->GetAbsVelocity() + 800 * gpGlobals->v_forward );
#endif

		int flags;

#if defined( CLIENT_WEAPONS )
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usFireSpore );

		--m_iClip;
	}

	m_flNextPrimaryAttack =
		m_flNextSecondaryAttack =
		m_flTimeWeaponIdle =
		UTIL_WeaponTimeBase() + 0.5;

	m_ReloadState = ReloadState::NOT_RELOADING;
}

void CSporeLauncher::Reload()
{
	if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] <= 0 || m_iClip == GetWeaponInfo()->GetMaxMagazine() )
		return;

	// don't reload until recoil is done
	if( m_flNextPrimaryAttack > UTIL_WeaponTimeBase() )
		return;

	// check to see if we're ready to reload
	if( m_ReloadState == ReloadState::NOT_RELOADING )
	{
		SendWeaponAnim( SPLAUNCHER_RELOAD_REACH );
		m_ReloadState = ReloadState::DO_RELOAD_EFFECTS;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.66;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.66;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.66;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.66;
		return;
	}
	else if( m_ReloadState == ReloadState::DO_RELOAD_EFFECTS )
	{
		if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
			return;
		// was waiting for gun to move to side
		m_ReloadState = ReloadState::RELOAD_ONE;

		EMIT_SOUND_DYN( 
			m_pPlayer, 
			CHAN_ITEM, "weapons/splauncher_reload.wav", 
			0.7, ATTN_NORM, 0, 100 );

		SendWeaponAnim( SPLAUNCHER_RELOAD );

		m_flNextReload = UTIL_WeaponTimeBase() + 1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] -= 1;
		m_ReloadState = ReloadState::DO_RELOAD_EFFECTS;
	}
}

void CSporeLauncher::GetWeaponData( weapon_data_t& data )
{
	BaseClass::GetWeaponData( data );

	//m_ReloadState is called m_fInSpecialReload in Op4. - Solokiller
	data.m_fInSpecialReload = static_cast<int>( m_ReloadState );
}

void CSporeLauncher::SetWeaponData( const weapon_data_t& data )
{
	BaseClass::SetWeaponData( data );

	m_ReloadState = static_cast<ReloadState>( data.m_fInSpecialReload );
}
#endif // GAME_OPFOR
