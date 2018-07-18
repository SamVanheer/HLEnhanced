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
#include "entities/weapons/CShockBeam.h"
#include "gamerules/GameRules.h"
#endif

#include "CShockRifle.h"

BEGIN_DATADESC( CShockRifle )
	DEFINE_FIELD( m_flRechargeTime, FIELD_FLOAT ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_shockrifle, CShockRifle );
LINK_ENTITY_TO_CLASS( weapon_shockroach, CShockRifle );

CShockRifle::CShockRifle()
	: BaseClass( WEAPON_SHOCKRIFLE )
{
}

void CShockRifle::Precache()
{
	BaseClass::Precache();

	PRECACHE_MODEL( "models/v_shock.mdl" );
	PRECACHE_MODEL( "models/w_shock_rifle.mdl" );
	PRECACHE_MODEL( "models/p_shock.mdl" );
	m_iSpriteTexture = PRECACHE_MODEL( "sprites/shockwave.spr" );
	PRECACHE_MODEL( "sprites/lgtning.spr" );

	PRECACHE_SOUND( "weapons/shock_fire.wav" );
	PRECACHE_SOUND( "weapons/shock_draw.wav" );
	PRECACHE_SOUND( "weapons/shock_recharge.wav" );
	PRECACHE_SOUND( "weapons/shock_discharge.wav" );

	UTIL_PrecacheOther( "shock_beam" );

	m_usShockRifle = PRECACHE_EVENT( 1, "events/shock.sc" );
}

void CShockRifle::Spawn()
{
	// Hack to allow for old names.
	SetClassname( "weapon_shockrifle" );

	Precache();

	SetModel( "models/w_shock_rifle.mdl" );

	FallInit();

	SetSequence( 0 );
	SetAnimTime( gpGlobals->time );
	SetFrameRate( 1 );
}

bool CShockRifle::AddToPlayer( CBasePlayer* pPlayer )
{
	if( BaseClass::AddToPlayer( pPlayer ) )
	{
#ifndef CLIENT_DLL
		pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] = GetWeaponInfo()->GetDefaultAmmo();
#endif

		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, nullptr, pPlayer );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return true;
	}

	return false;
}

bool CShockRifle::CanDeploy() const
{
	return true;
}

bool CShockRifle::Deploy()
{
	if( bIsMultiplayer() )
	{
		m_flRechargeTime = gpGlobals->time + 0.25;
	}
	else
	{
		m_flRechargeTime = gpGlobals->time + 0.5;
	}

	return DefaultDeploy( 
		"models/v_shock.mdl", "models/p_shock.mdl", 
		SHOCKRIFLE_DRAW, "bow" );
}

void CShockRifle::Holster()
{
	m_fInReload = false;

	SetThink( nullptr );

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;

	SendWeaponAnim( SHOCKRIFLE_HOLSTER );

	//TODO: unnecessary if the shock rifle can regen while inactive. - Solokiller
	if( !m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] )
	{
		m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] = 1;
	}
}

void CShockRifle::WeaponIdle()
{
	if( m_flSoundDelay != 0 && gpGlobals->time >= m_flSoundDelay )
	{
		m_flSoundDelay = 0;
	}

	//This used to be completely broken. It used the current game time instead of the weapon time base, which froze the idle animation.
	//It also never handled IDLE3, so it only ever played IDLE1, and then only animated it when you held down secondary fire.
	//This is now fixed. - Solokiller
	if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;

	const float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );

	if( flRand <= 0.75 )
	{
		iAnim = SHOCKRIFLE_IDLE3;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 51.0 / 15.0;
	}
	else
	{
		iAnim = SHOCKRIFLE_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 101.0 / 30.0;
	}

	SendWeaponAnim( iAnim );
}

void CShockRifle::PrimaryAttack()
{
	if( m_pPlayer->GetWaterLevel() == WATERLEVEL_HEAD )
	{
		//Water goes zap.
		const float flVolume = UTIL_RandomFloat( 0.8, 0.9 );

		EMIT_SOUND_DYN( 
			m_pPlayer, CHAN_ITEM, 
			"weapons/shock_discharge.wav", flVolume, ATTN_NONE, 0, PITCH_NORM );

		RadiusDamage(
			GetAbsOrigin(), 
			CTakeDamageInfo( 
				m_pPlayer, 
				m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] * 100.0, 
				DMG_ALWAYSGIB | DMG_BLAST ), 
			m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] * 150.0, 
			0 );

		m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] = 0;

		return;
	}

	if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] <= 0 )
	{
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	--m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ];

	m_flRechargeTime = gpGlobals->time + 1.0;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usShockRifle );

	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->GetEffects() |= EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
	const Vector vecAnglesAim = m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle();

	UTIL_MakeVectors( vecAnglesAim );

	Vector vecSrc = 
		m_pPlayer->GetGunPosition() + 
		gpGlobals->v_forward * 16 + 
		gpGlobals->v_right * 9 + 
		gpGlobals->v_up * -7;

	Vector vecAngles = m_pPlayer->GetAutoaimVectorFromPoint( vecSrc, AUTOAIM_10DEGREES );

	/*CShockBeam* pBeam = */CShockBeam::CreateShockBeam( vecSrc, vecAnglesAim + vecAngles, m_pPlayer );
#endif

	if( bIsMultiplayer() )
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	}
	else
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33;
}

void CShockRifle::SecondaryAttack()
{
	//Nothing
}

void CShockRifle::Reload()
{
	RechargeAmmo( true );
}

void CShockRifle::WeaponHolsterPreFrame()
{
	BaseClass::WeaponHolsterPreFrame();

	//Silently recharge in the background. - Solokiller
	RechargeAmmo( false );
}

void CShockRifle::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	Reload();
}

void CShockRifle::RechargeAmmo( bool bLoud )
{
	const int iMax = GetWeaponInfo()->GetDefaultAmmo();

	if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] >= iMax )
	{
		return;
	}

	while( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] < iMax )
	{
		if( m_flRechargeTime >= gpGlobals->time )
			break;

		++m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ];

		if( bLoud )
		{
			EMIT_SOUND_DYN(
				m_pPlayer, CHAN_WEAPON,
				"weapons/shock_recharge.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM );
		}

		if( bIsMultiplayer() )
		{
			m_flRechargeTime += 0.25;
		}
		else
		{
			m_flRechargeTime += 0.5;
		}
	}
}

#endif // GAME_OPFOR
