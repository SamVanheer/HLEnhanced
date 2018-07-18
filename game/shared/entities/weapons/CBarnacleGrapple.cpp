/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#ifdef GAME_OPFOR

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "Weapons.h"
#include "CBasePlayer.h"
#include "Skill.h"
#include "customentity.h"

#include "entities/weapons/CBarnacleGrappleTip.h"

#ifndef CLIENT_DLL
#include "gamerules/GameRules.h"
#endif

#include "CBarnacleGrapple.h"

BEGIN_DATADESC( CBarnacleGrapple )
	DEFINE_FIELD( m_pBeam, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_flShootTime, FIELD_TIME ),
	DEFINE_FIELD( m_FireState, FIELD_INTEGER ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( weapon_grapple, CBarnacleGrapple );

CBarnacleGrapple::CBarnacleGrapple()
	: BaseClass( WEAPON_BARNACLE_GRAPPLE )
{
}

void CBarnacleGrapple::Precache()
{
	BaseClass::Precache();

	PRECACHE_MODEL( "models/v_bgrap.mdl" );
	PRECACHE_MODEL( "models/w_bgrap.mdl" );
	PRECACHE_MODEL( "models/p_bgrap.mdl" );

	PRECACHE_SOUND( "weapons/bgrapple_release.wav" );
	PRECACHE_SOUND( "weapons/bgrapple_impact.wav" );
	PRECACHE_SOUND( "weapons/bgrapple_fire.wav" );
	PRECACHE_SOUND( "weapons/bgrapple_cough.wav" );
	PRECACHE_SOUND( "weapons/bgrapple_pull.wav" );
	PRECACHE_SOUND( "weapons/bgrapple_wait.wav" );
	PRECACHE_SOUND( "weapons/alienweap_draw.wav" );
	PRECACHE_SOUND( "barnacle/bcl_chew1.wav" );
	PRECACHE_SOUND( "barnacle/bcl_chew2.wav" );
	PRECACHE_SOUND( "barnacle/bcl_chew3.wav" );

	PRECACHE_MODEL( "sprites/tongue.spr" );

	UTIL_PrecacheOther( "grapple_tip" );
}

void CBarnacleGrapple::Spawn()
{
	Precache();

	SetModel( "models/w_bgrap.mdl" );

	m_iClip = WEAPON_NOCLIP;

	m_pTip = nullptr;

	m_bGrappling = false;

	FallInit();
}

bool CBarnacleGrapple::AddToPlayer( CBasePlayer* pPlayer )
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

bool CBarnacleGrapple::Deploy()
{
	return DefaultDeploy(
		"models/v_bgrap.mdl", "models/p_bgrap.mdl",
		BGRAPPLE_UP, "gauss" );
}

void CBarnacleGrapple::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim( BGRAPPLE_DOWN );

	if( m_FireState != FireState::OFF )
	{
		EndAttack();
	}
}

void CBarnacleGrapple::WeaponIdle()
{
	ResetEmptySound();

	if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
	{
		if( m_FireState != FireState::OFF )
			EndAttack();

		m_bMissed = false;

		const float flNextIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

		int iAnim;

		if( flNextIdle <= 0.5 )
		{
			iAnim = BGRAPPLE_LONGIDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10.0;
		}
		else if( flNextIdle > 0.95 )
		{
			EMIT_SOUND_DYN( 
				m_pPlayer, CHAN_STATIC, "weapons/bgrapple_cough.wav",
				VOL_NORM, ATTN_NORM, 0, PITCH_NORM );

			iAnim = BGRAPPLE_COUGH;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4.6;
		}
		else
		{
			iAnim = BGRAPPLE_BREATHE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.566;
		}

		SendWeaponAnim( iAnim );
	}
}

void CBarnacleGrapple::PrimaryAttack()
{
	if( m_bMissed )
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
		return;
	}

	UTIL_MakeVectors( m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle() );

	if( m_pTip )
	{
		if( m_pTip->IsStuck() )
		{
			CBaseEntity* pTarget = m_pTip->GetGrappleTarget();

			if( !pTarget )
			{
				EndAttack();
				return;
			}

			if( m_pTip->GetGrappleType() > CBarnacleGrappleTip::TargetClass::SMALL )
			{
				m_pPlayer->SetMoveType( MOVETYPE_FLY );
				//Tells the physics code that the player is not on a ladder - Solokiller
				m_pPlayer->GetFlags() |= FL_IMMUNE_LAVA;
			}

			if( m_bMomentaryStuck )
			{
				SendWeaponAnim( BGRAPPLE_FIRETRAVEL );

				EMIT_SOUND_DYN( 
					m_pPlayer, CHAN_STATIC,
					"weapons/bgrapple_impact.wav", 0.98, ATTN_NORM, 0, 125 );

				if( pTarget->IsPlayer() )
				{
					EMIT_SOUND_DYN( 
						pTarget, CHAN_STATIC, 
						"weapons/bgrapple_impact.wav", 0.98, ATTN_NORM, 0, 125 );
				}

				m_bMomentaryStuck = false;
			}

			switch( m_pTip->GetGrappleType() )
			{
			case CBarnacleGrappleTip::TargetClass::NOT_A_TARGET: break;

			case CBarnacleGrappleTip::TargetClass::SMALL:
				pTarget->BarnacleVictimGrabbed( this );
				m_pTip->SetAbsOrigin( pTarget->Center() );

				pTarget->SetAbsVelocity( 
					pTarget->GetAbsVelocity() + ( m_pPlayer->GetAbsOrigin() - pTarget->GetAbsOrigin() ) );
				
				if( pTarget->GetAbsVelocity().Length() > 450.0 )
				{
					pTarget->SetAbsVelocity( pTarget->GetAbsVelocity().Normalize() * 450.0 );
				}

				break;

			case CBarnacleGrappleTip::TargetClass::MEDIUM:
			case CBarnacleGrappleTip::TargetClass::LARGE:
			case CBarnacleGrappleTip::TargetClass::FIXED:
				pTarget->BarnacleVictimGrabbed( this );

				if( m_pTip->GetGrappleType() != CBarnacleGrappleTip::TargetClass::FIXED )
					m_pTip->SetAbsOrigin( pTarget->Center() );

				m_pPlayer->SetAbsVelocity(
					m_pPlayer->GetAbsVelocity() + ( m_pTip->GetAbsOrigin() - m_pPlayer->GetAbsOrigin() ) );

				if( m_pPlayer->GetAbsVelocity().Length() > 450.0 )
				{
					m_pPlayer->SetAbsVelocity( m_pPlayer->GetAbsVelocity().Normalize() * 450.0 );
					
					Vector vecPitch = UTIL_VecToAngles( m_pPlayer->GetAbsVelocity() );

					if( (vecPitch.x > 55.0 && vecPitch.x < 205.0) || vecPitch.x < -55.0 )
					{
						m_bGrappling = false;
						m_pPlayer->SetAnimation( PLAYER_IDLE );
					}
					else
					{
						m_bGrappling = true;
						m_pPlayer->SetAnimation( PLAYER_GRAPPLE );
					}
				}
				else
				{
					m_bGrappling = false;
					m_pPlayer->SetAnimation( PLAYER_IDLE );
				}

				break;
			}
		}

		if( m_pTip->HasMissed() )
		{
			EMIT_SOUND_DYN(
				m_pPlayer, CHAN_STATIC,
				"weapons/bgrapple_release.wav", 0.98, ATTN_NORM, 0, 125 );

			EndAttack();
			return;
		}
	}

	if( m_FireState != FireState::OFF )
	{
		m_pPlayer->m_iWeaponVolume = 450;

		if( m_flShootTime != 0.0 && gpGlobals->time > m_flShootTime )
		{
			SendWeaponAnim( BGRAPPLE_FIREWAITING );

			Vector vecPunchAngle = m_pPlayer->GetPunchAngle();

			vecPunchAngle.x += 2.0;

			m_pPlayer->SetPunchAngle( vecPunchAngle );

			Fire( m_pPlayer->GetGunPosition(), gpGlobals->v_forward );

			EMIT_SOUND_DYN( 
				m_pPlayer, CHAN_STATIC, 
				"weapons/bgrapple_pull.wav", 0.98, ATTN_NORM, 0, 125 );

			m_flShootTime = 0;
		}
	}
	else
	{
		m_bMomentaryStuck = true;

		SendWeaponAnim( BGRAPPLE_FIRE );

		m_pPlayer->m_iWeaponVolume = 450;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;

		if( bIsMultiplayer() )
		{
			m_flShootTime = gpGlobals->time;
		}
		else
		{
			m_flShootTime = gpGlobals->time + 0.35;
		}

		EMIT_SOUND_DYN( 
			m_pPlayer, CHAN_WEAPON, 
			"weapons/bgrapple_fire.wav", 0.98, ATTN_NORM, 0, 125 );

		m_FireState = FireState::CHARGE;
	}

	if( !m_pTip )
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.01;
		return;
	}

	if( m_pTip->GetGrappleType() != CBarnacleGrappleTip::TargetClass::FIXED && m_pTip->IsStuck() )
	{
		UTIL_MakeVectors( m_pPlayer->GetViewAngle() );

		Vector vecSrc = m_pPlayer->GetGunPosition();

		Vector vecEnd = vecSrc + gpGlobals->v_forward * 16.0;

		TraceResult tr;

		UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr );

		if( tr.flFraction >= 1.0 )
		{
			UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, Hull::HEAD, m_pPlayer->edict(), &tr );
		
			if( tr.flFraction < 1.0 )
			{
				CBaseEntity* pHit = Instance( tr.pHit );

				if( !pHit )
					pHit = CWorld::GetInstance();

				if( !pHit )
				{
					FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer );
				}
			}
		}

		if( tr.flFraction < 1.0 )
		{
			CBaseEntity* pHit = Instance( tr.pHit );

			if( !pHit )
				pHit = CWorld::GetInstance();

			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

			if( pHit )
			{
				if( m_pTip )
				{
					bool bValidTarget = false;
					if( pHit->IsPlayer() )
					{
						m_pTip->SetGrappleTarget( pHit );

						bValidTarget = true;
					}
					else if( m_pTip->ClassifyTarget( pHit ) != CBarnacleGrappleTip::TargetClass::NOT_A_TARGET )
					{
						bValidTarget = true;
					}

					if( bValidTarget )
					{
						if( m_flDamageTime + 0.5 < gpGlobals->time )
						{
#ifndef CLIENT_DLL
							g_MultiDamage.Clear();

							float flDamage = gSkillData.GetPlrDmgGrapple();

							if( g_pGameRules->IsMultiplayer() )
							{
								flDamage *= 2;
							}

							pHit->TraceAttack( 
								CTakeDamageInfo( this, m_pPlayer, flDamage, DMG_ALWAYSGIB | DMG_CLUB ), 
								gpGlobals->v_forward, tr );

							g_MultiDamage.ApplyMultiDamage( m_pPlayer, m_pPlayer );
#endif

							m_flDamageTime = gpGlobals->time;

							const char* pszSample;

							switch( UTIL_RandomLong( 0, 2 ) )
							{
							default:
							case 0: pszSample = "barnacle/bcl_chew1.wav"; break;
							case 1: pszSample = "barnacle/bcl_chew2.wav"; break;
							case 2: pszSample = "barnacle/bcl_chew3.wav"; break;
							}

							EMIT_SOUND_DYN( 
								m_pPlayer, CHAN_VOICE, 
								pszSample, VOL_NORM, ATTN_NORM, 0, 125 );
						}
					}
				}
			}
		}
	}

	//TODO: CTF support - Solokiller
	/*
	if( g_pGameRules->IsMultiplayer() && g_pGameRules->IsCTF() )
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
	}
	else
	*/
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.01;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
	}
}

void CBarnacleGrapple::SecondaryAttack()
{
	if( m_pTip && m_pTip->IsStuck() && 
		( !m_pTip->GetGrappleTarget() || m_pTip->GetGrappleTarget()->IsPlayer() ) )
	{
		EndAttack();
	}
	else
	{
		UTIL_MakeVectors( m_pPlayer->GetViewAngle() + m_pPlayer->GetPunchAngle() );

		m_pPlayer->SetMoveType( MOVETYPE_WALK );
		m_pPlayer->GetFlags().ClearFlags( FL_IMMUNE_LAVA );
	}
}

void CBarnacleGrapple::Fire( const Vector& vecOrigin, const Vector& vecDir )
{
#ifndef CLIENT_DLL
	Vector vecSrc = vecOrigin;

	Vector vecEnd = vecSrc + vecDir * 2048.0;

	TraceResult tr;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr );

	if( !tr.fAllSolid )
	{
		CBaseEntity* pHit = Instance( tr.pHit );

		if( !pHit )
			pHit = CWorld::GetInstance();

		if( pHit )
		{
			UpdateEffect();

			m_flDamageTime = gpGlobals->time;
		}
	}
#endif
}

void CBarnacleGrapple::EndAttack()
{
	m_FireState = FireState::OFF;

	SendWeaponAnim( BGRAPPLE_FIREREACHED );

	EMIT_SOUND_DYN(
		m_pPlayer, CHAN_STATIC,
		"weapons/bgrapple_release.wav", 0.98, ATTN_NORM, 0, 125 );

	EMIT_SOUND_DYN(
		m_pPlayer, CHAN_STATIC,
		"weapons/bgrapple_pull.wav", 0.0, ATTN_NONE, SND_STOP, 100 );

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase();

	//TODO: CTF support - Solokiller
	/*
	if( bIsMultiplayer() && g_pGameRules->IsCTF() )
	{
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
	}
	else
	*/
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
	}

	DestroyEffect();

	if( m_bGrappling && m_pPlayer->IsAlive() )
	{
		m_pPlayer->SetAnimation( PLAYER_IDLE );
	}

	m_pPlayer->SetMoveType( MOVETYPE_WALK );
	m_pPlayer->GetFlags().ClearFlags( FL_IMMUNE_LAVA );
}

void CBarnacleGrapple::CreateEffect()
{
#ifndef CLIENT_DLL
	DestroyEffect();

	m_pTip = static_cast<CBarnacleGrappleTip*>( UTIL_CreateNamedEntity( "grapple_tip" ) );

	m_pTip->Spawn();

	UTIL_MakeVectors( m_pPlayer->GetViewAngle() );

	Vector vecOrigin = 
		m_pPlayer->GetGunPosition() + 
		gpGlobals->v_forward * 16.0 + 
		gpGlobals->v_right * 8.0 + 
		gpGlobals->v_up * -8.0;

	Vector vecAngles = m_pPlayer->GetViewAngle();

	vecAngles.x = -vecAngles.x;

	m_pTip->SetPosition( vecOrigin, vecAngles, m_pPlayer );

	if( !m_pBeam )
	{
		m_pBeam = CBeam::BeamCreate( "sprites/tongue.spr", 16 );

		m_pBeam->EntsInit( m_pTip->entindex(), m_pPlayer->entindex() );

		m_pBeam->SetBeamFlags( BEAM_FSOLID );

		m_pBeam->SetBrightness( 100.0 );

		m_pBeam->SetEndAttachment( 1 );

		m_pBeam->GetSpawnFlags() |= SF_BEAM_TEMPORARY;
	}
#endif
}

void CBarnacleGrapple::UpdateEffect()
{
#ifndef CLIENT_DLL
	if( !m_pBeam || !m_pTip )
		CreateEffect();
#endif
}

void CBarnacleGrapple::DestroyEffect()
{
	if( m_pBeam )
	{
		UTIL_Remove( m_pBeam );
		m_pBeam = nullptr;
	}

	if( m_pTip )
	{
		m_pTip->Killed( CTakeDamageInfo( nullptr, 0, 0 ), GIB_NEVER );
		m_pTip = nullptr;
	}
}

#endif // GAME_OPFOR
