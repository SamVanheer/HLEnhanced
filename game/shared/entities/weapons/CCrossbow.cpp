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
#include "entities/NPCs/Monsters.h"
#include "Weapons.h"
#include "CCrossbow.h"
#include "nodes/Nodes.h"
#include "CBasePlayer.h"
#include "gamerules/GameRules.h"

#ifndef CLIENT_DLL
#include "entities/weapons/CCrossbowBolt.h"
#endif

LINK_ENTITY_TO_CLASS( weapon_crossbow, CCrossbow );

CCrossbow::CCrossbow()
	: BaseClass( WEAPON_CROSSBOW )
{
}

void CCrossbow::Spawn( )
{
	Precache( );
	SetModel( "models/w_crossbow.mdl");

	FallInit();// get ready to fall down.
}

bool CCrossbow::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return true;
	}
	return false;
}

void CCrossbow::Precache( void )
{
	BaseClass::Precache();

	PRECACHE_MODEL("models/w_crossbow.mdl");
	PRECACHE_MODEL("models/v_crossbow.mdl");
	PRECACHE_MODEL("models/p_crossbow.mdl");

	PRECACHE_SOUND("weapons/xbow_fire1.wav");
	PRECACHE_SOUND("weapons/xbow_reload1.wav");

	UTIL_PrecacheOther( "crossbow_bolt" );

	m_usCrossbow = PRECACHE_EVENT( 1, "events/crossbow1.sc" );
	m_usCrossbow2 = PRECACHE_EVENT( 1, "events/crossbow2.sc" );
}

bool CCrossbow::Deploy()
{
	if (m_iClip)
		return DefaultDeploy( "models/v_crossbow.mdl", "models/p_crossbow.mdl", CROSSBOW_DRAW1, "bow" );
	return DefaultDeploy( "models/v_crossbow.mdl", "models/p_crossbow.mdl", CROSSBOW_DRAW2, "bow" );
}

void CCrossbow::Holster()
{
	m_fInReload = false;// cancel any reload in progress.

	if ( m_fInZoom )
	{
		SecondaryAttack( );
	}

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	if (m_iClip)
		SendWeaponAnim( CROSSBOW_HOLSTER1 );
	else
		SendWeaponAnim( CROSSBOW_HOLSTER2 );
}

void CCrossbow::PrimaryAttack( void )
{
	if ( m_fInZoom && bIsMultiplayer() )
	{
		FireSniperBolt();
		return;
	}

	FireBolt();
}

// this function only gets called in multiplayer
void CCrossbow::FireSniperBolt()
{
	m_flNextPrimaryAttack = GetNextAttackDelay(0.75);

	if (m_iClip == 0)
	{
		PlayEmptySound( );
		return;
	}

	TraceResult tr;

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usCrossbow2, 0.0, g_vecZero, g_vecZero, 0, 0, m_iClip, m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ], 0, 0 );

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	
	Vector anglesAim = m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle();
	UTIL_MakeVectors( anglesAim );
	Vector vecSrc = m_pPlayer->GetGunPosition( ) - gpGlobals->v_up * 2;
	Vector vecDir = gpGlobals->v_forward;

	UTIL_TraceLine(vecSrc, vecSrc + vecDir * 8192, dont_ignore_monsters, m_pPlayer->edict(), &tr);

#ifndef CLIENT_DLL
	if ( tr.pHit->v.takedamage )
	{
		g_MultiDamage.Clear( );
		CBaseEntity::Instance(tr.pHit)->TraceAttack( CTakeDamageInfo( m_pPlayer, 120, DMG_BULLET | DMG_NEVERGIB ), vecDir, tr ); 
		g_MultiDamage.ApplyMultiDamage( this, m_pPlayer );
	}
#endif
}

void CCrossbow::FireBolt()
{
	TraceResult tr;

	if (m_iClip == 0)
	{
		PlayEmptySound( );
		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	m_iClip--;

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usCrossbow, 0.0, g_vecZero, g_vecZero, 0, 0, m_iClip, m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ], 0, 0 );

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector anglesAim = m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle();
	UTIL_MakeVectors( anglesAim );
	
	anglesAim.x		= -anglesAim.x;

#ifndef CLIENT_DLL
	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	Vector vecDir = gpGlobals->v_forward;

	CCrossbowBolt *pBolt = CCrossbowBolt::BoltCreate();
	pBolt->SetAbsOrigin( vecSrc );
	pBolt->SetAbsAngles( anglesAim );
	pBolt->SetOwner( m_pPlayer );

	if (m_pPlayer->GetWaterLevel() == WATERLEVEL_HEAD)
	{
		pBolt->SetAbsVelocity( vecDir * BOLT_WATER_VELOCITY );
		pBolt->SetSpeed( BOLT_WATER_VELOCITY );
	}
	else
	{
		pBolt->SetAbsVelocity( vecDir * BOLT_AIR_VELOCITY );
		pBolt->SetSpeed( BOLT_AIR_VELOCITY );
	}

	Vector vecAVelocity = pBolt->GetAngularVelocity();
	vecAVelocity.z = 10;
	pBolt->SetAngularVelocity( vecAVelocity );
#endif

	if (!m_iClip && m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", SUIT_SENTENCE, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.75);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75;

	if (m_iClip != 0)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
}


void CCrossbow::SecondaryAttack()
{
	if ( m_pPlayer->GetFOV() != 0 )
	{
		//TODO: figure out if these can be merged - Solokiller
		m_pPlayer->SetFOV( 0 );
		m_pPlayer->m_iFOV = 0; // 0 means reset to default fov
		m_fInZoom = false;
	}
	else if ( m_pPlayer->GetFOV() != 20 )
	{
		m_pPlayer->SetFOV( 20 );
		m_pPlayer->m_iFOV = 20;
		m_fInZoom = true;
	}
	
	SetNextThink( UTIL_WeaponTimeBase() + 0.1 );
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
}


void CCrossbow::Reload( void )
{
	if ( m_pPlayer->GetAmmoCountByID( PrimaryAmmoIndex() ) <= 0 )
		return;

	if ( m_pPlayer->GetFOV() != 0 )
	{
		SecondaryAttack();
	}

	if ( DefaultReload( CROSSBOW_RELOAD, 4.5 ) )
	{
		EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/xbow_reload1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 93 + RANDOM_LONG(0,0xF));
	}
}


void CCrossbow::WeaponIdle( void )
{
	m_pPlayer->GetAutoaimVector( AUTOAIM_2DEGREES );  // get the autoaim vector but ignore it;  used for autoaim crosshair in DM

	ResetEmptySound( );
	
	if ( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
	{
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
		if (flRand <= 0.75)
		{
			if (m_iClip)
			{
				SendWeaponAnim( CROSSBOW_IDLE1 );
			}
			else
			{
				SendWeaponAnim( CROSSBOW_IDLE2 );
			}
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		}
		else
		{
			if (m_iClip)
			{
				SendWeaponAnim( CROSSBOW_FIDGET1 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 30.0;
			}
			else
			{
				SendWeaponAnim( CROSSBOW_FIDGET2 );
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 30.0;
			}
		}
	}
}



class CCrossbowAmmo : public CBasePlayerAmmo
{
public:
	DECLARE_CLASS( CCrossbowAmmo, CBasePlayerAmmo );

	void Spawn( void ) override
	{ 
		Precache( );
		SetModel( "models/w_crossbow_clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void ) override
	{
		PRECACHE_MODEL ("models/w_crossbow_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo( CBaseEntity *pOther ) override
	{ 
		return UTIL_GiveAmmoToPlayer( this, pOther, AMMO_CROSSBOWCLIP_GIVE, "bolts" );
	}
};
LINK_ENTITY_TO_CLASS( ammo_crossbow, CCrossbowAmmo );