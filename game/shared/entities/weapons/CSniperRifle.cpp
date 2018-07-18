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
#include "CBasePlayer.h"
#include "Weapons.h"
#include "gamerules/GameRules.h"

#include "CSniperRifle.h"

BEGIN_DATADESC( CSniperRifle )
	DEFINE_FIELD( m_flReloadStart, FIELD_TIME ),
	DEFINE_FIELD( m_bReloading, FIELD_BOOLEAN ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_sniperrifle, CSniperRifle );

CSniperRifle::CSniperRifle()
	: BaseClass( WEAPON_SNIPERRIFLE )
{
}

void CSniperRifle::Precache()
{
	BaseClass::Precache();

	PRECACHE_MODEL( "models/w_m40a1.mdl" );
	PRECACHE_MODEL( "models/v_m40a1.mdl" );
	PRECACHE_MODEL( "models/p_m40a1.mdl" );

	PRECACHE_SOUND( "weapons/sniper_fire.wav" );
	PRECACHE_SOUND( "weapons/sniper_zoom.wav" );
	PRECACHE_SOUND( "weapons/sniper_reload_first_seq.wav" );
	PRECACHE_SOUND( "weapons/sniper_reload_second_seq.wav" );
	PRECACHE_SOUND( "weapons/sniper_miss.wav" );
	PRECACHE_SOUND( "weapons/sniper_bolt1.wav" );
	PRECACHE_SOUND( "weapons/sniper_bolt2.wav" );

	m_usSniper = PRECACHE_EVENT( 1, "events/sniper.sc" );
}

void CSniperRifle::Spawn()
{
	Precache();

	SetModel( "models/w_m40a1.mdl" );

	FallInit(); // get ready to fall down.
}

bool CSniperRifle::AddToPlayer( CBasePlayer* pPlayer )
{
	if( BaseClass::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer );
		WRITE_BYTE( m_iId );
		MESSAGE_END();
		return true;
	}
	return false;
}

bool CSniperRifle::Deploy()
{
	return BaseClass::DefaultDeploy( 
		"models/v_m40a1.mdl", "models/p_m40a1.mdl", 
		SNIPERRIFLE_DRAW, "bow" );
}

void CSniperRifle::Holster()
{
	m_fInReload = false;// cancel any reload in progress.

	if( m_bInZoom )
		SecondaryAttack();

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.25;

	SendWeaponAnim( SNIPERRIFLE_HOLSTER );
}

void CSniperRifle::WeaponIdle()
{
	/*Vector vecAiming = */m_pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES );

	ResetEmptySound();

	if( m_bReloading && gpGlobals->time >= m_flReloadStart + 2.3239999 )
	{
		SendWeaponAnim( SNIPERRIFLE_RELOAD2 );
		m_bReloading = false;
	}

	if( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
	{
		if( m_iClip )
			SendWeaponAnim( SNIPERRIFLE_SLOWIDLE );
		else
			SendWeaponAnim( SNIPERRIFLE_SLOWIDLE2 );

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.348;
	}
}

void CSniperRifle::PrimaryAttack()
{
	if( m_pPlayer->GetWaterLevel() == WATERLEVEL_HEAD )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0f;
		return;
	}

	if( !m_iClip )
	{
		PlayEmptySound();
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	--m_iClip;

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecAngles = m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle();

	UTIL_MakeVectors( vecAngles );

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES );

	//TODO: 8192 constant should be defined somewhere - Solokiller
	Vector vecShot = m_pPlayer->FireBulletsPlayer( 1,
									vecSrc, vecAiming, g_vecZero, 
									8192, BULLET_PLAYER_762, 0, 0,
									m_pPlayer, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, 
							m_pPlayer->edict(), m_usSniper, 0, 
							g_vecZero, g_vecZero, 
							vecShot.x, vecShot.y, 
							m_iClip, m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ],
							0, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.0f;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0f;
}

void CSniperRifle::SecondaryAttack()
{
	EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/sniper_zoom.wav", 
					VOL_NORM, ATTN_NORM, 0, PITCH_NORM );

	m_bInZoom = !m_bInZoom;

	ToggleZoom();

	//TODO: use UTIL_WeaponTimeBase() - Solokiller
	SetNextThink( 0.0 + 0.1 );

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1 + 0.5;
}

void CSniperRifle::Reload()
{
	if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] > 0 )
	{
		if( m_bInZoom )
		{
			ToggleZoom();
		}

		if( m_iClip )
		{
			if( DefaultReload( SNIPERRIFLE_RELOAD3, 2.324, 1 ) )
			{
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.324;
			}
		}
		else if( DefaultReload( SNIPERRIFLE_RELOAD1, 2.324, 1 ) )
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 4.102;
			m_flReloadStart = gpGlobals->time;
			m_bReloading = true;
		}
		else
		{
			m_bReloading = false;
		}
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.102;
}

void CSniperRifle::ToggleZoom()
{
	if( m_pPlayer->GetFOV() == 0 )
	{
		m_pPlayer->SetFOV( 18 );
		m_pPlayer->m_iFOV = 18;

		m_bInZoom = true;
	}
	else
	{
		m_pPlayer->SetFOV( 0 );
		m_pPlayer->m_iFOV = 0;

		m_bInZoom = false;
	}
}

class CSniperRifleAmmo : public CBasePlayerAmmo
{
public:
	DECLARE_CLASS( CSniperRifleAmmo, CBasePlayerAmmo );

	void Spawn( void ) override
	{
		Precache();
		SetModel( "models/w_m40a1clip.mdl" );
		CBasePlayerAmmo::Spawn();
	}

	void Precache( void ) override
	{
		PRECACHE_MODEL( "models/w_m40a1clip.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}

	bool AddAmmo( CBaseEntity *pOther ) override
	{
		return UTIL_GiveAmmoToPlayer( this, pOther, AMMO_SNIPERRIFLE_GIVE, "762" );
	}
};

LINK_ENTITY_TO_CLASS( ammo_762, CSniperRifleAmmo );
#endif // GAME_OPFOR
