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
#include "Skill.h"

#include "gamerules/GameRules.h"

#include "CKnife.h"

BEGIN_DATADESC( CKnife )
	DEFINE_THINKFUNC( SwingAgain ),
	DEFINE_THINKFUNC( Smack ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_knife, CKnife );

CKnife::CKnife()
	: BaseClass( WEAPON_KNIFE )
{
}

void CKnife::Precache()
{
	BaseClass::Precache();

	PRECACHE_MODEL( "models/v_knife.mdl" );
	PRECACHE_MODEL( "models/w_knife.mdl" );
	PRECACHE_MODEL( "models/p_knife.mdl" );

	PRECACHE_SOUND( "weapons/knife1.wav" );
	PRECACHE_SOUND( "weapons/knife2.wav" );
	PRECACHE_SOUND( "weapons/knife3.wav" );
	PRECACHE_SOUND( "weapons/knife_hit_flesh1.wav" );
	PRECACHE_SOUND( "weapons/knife_hit_flesh2.wav" );
	PRECACHE_SOUND( "weapons/knife_hit_wall1.wav" );
	PRECACHE_SOUND( "weapons/knife_hit_wall2.wav" );

	m_usKnife = PRECACHE_EVENT( 1, "events/knife.sc" );
}

void CKnife::Spawn()
{
	Precache();

	SetModel( "models/w_knife.mdl" );

	m_iClip = WEAPON_NOCLIP;

	FallInit();
}

bool CKnife::Deploy()
{
	return DefaultDeploy( 
		"models/v_knife.mdl", "models/p_knife.mdl", 
		KNIFE_DRAW, "crowbar" );
}

void CKnife::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim( KNIFE_HOLSTER );
}

void CKnife::PrimaryAttack()
{
	if( !Swing( true ) )
	{
#ifndef CLIENT_DLL
		SetThink( &CKnife::SwingAgain );
		SetNextThink( gpGlobals->time + 0.1 );
#endif
	}
}

bool CKnife::Swing( const bool bFirst )
{
	bool bDidHit = false;

	TraceResult tr;

	UTIL_MakeVectors( m_pPlayer->GetViewAngle() );
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

#ifndef CLIENT_DLL
	if( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, Hull::HEAD, ENT( m_pPlayer->pev ), &tr );
		if( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			if( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer );
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	if( bFirst )
	{
		PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usKnife,
							 0.0, g_vecZero, g_vecZero, 0, 0, 0,
							 0.0, 0, 0.0 );
	}


	if( tr.flFraction >= 1.0 )
	{
		if( bFirst )
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay( 0.5 );

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		switch( ( ( m_iSwing++ ) % 2 ) + 1 )
		{
		case 0:
			SendWeaponAnim( KNIFE_ATTACK1 ); break;
		case 1:
			SendWeaponAnim( KNIFE_ATTACK2HIT ); break;
		case 2:
			SendWeaponAnim( KNIFE_ATTACK3HIT ); break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

#ifndef CLIENT_DLL

		// hit
		bDidHit = true;
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		if( pEntity )
		{
			g_MultiDamage.Clear();

			if( ( m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase() ) || g_pGameRules->IsMultiplayer() )
			{
				// first swing does full damage
				pEntity->TraceAttack( CTakeDamageInfo( m_pPlayer, gSkillData.GetPlrDmgKnife(), DMG_CLUB ), gpGlobals->v_forward, tr );
			}
			else
			{
				// subsequent swings do half
				pEntity->TraceAttack( CTakeDamageInfo( m_pPlayer, gSkillData.GetPlrDmgKnife() / 2, DMG_CLUB ), gpGlobals->v_forward, tr );
			}
			g_MultiDamage.ApplyMultiDamage( m_pPlayer, m_pPlayer );
		}

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool bHitWorld = true;

		if( pEntity )
		{
			if( pEntity->Classify() != EntityClassifications().GetNoneId() && pEntity->Classify() != EntityClassifications().GetClassificationId( classify::MACHINE ) )
			{
				// play thwack or smack sound
				switch( RANDOM_LONG( 0, 1 ) )
				{
				case 0:
					EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/knife_hit_flesh1.wav", 1, ATTN_NORM ); break;
				case 1:
					EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/knife_hit_flesh2.wav", 1, ATTN_NORM ); break;
				}
				m_pPlayer->m_iWeaponVolume = MELEE_BODYHIT_VOLUME;
				if( !pEntity->IsAlive() )
					return true;
				else
					flVol = 0.1;

				bHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if( bHitWorld )
		{
			float fvolbar = TEXTURETYPE_PlaySound( tr, vecSrc, vecSrc + ( vecEnd - vecSrc ) * 2, BULLET_PLAYER_CROWBAR );

			if( g_pGameRules->IsMultiplayer() )
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch( RANDOM_LONG( 0, 1 ) )
			{
			case 0:
				EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/knife_hit_wall1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
				break;
			case 1:
				EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/knife_hit_wall2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG( 0, 3 ) );
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * MELEE_WALLHIT_VOLUME;

		SetThink( &CKnife::Smack );
		SetNextThink( UTIL_WeaponTimeBase() + 0.2 );
#endif
		m_flNextPrimaryAttack = GetNextAttackDelay( 0.25 );
	}
	return bDidHit;
}

void CKnife::SwingAgain()
{
	Swing( false );
}

void CKnife::Smack()
{
	DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
}

#endif // GAME_OPFOR
