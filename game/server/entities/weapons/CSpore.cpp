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
#include "Skill.h"
#include "entities/CSoundEnt.h"
#include "Decals.h"

#include "CSpore.h"

BEGIN_DATADESC( CSpore )
	DEFINE_FIELD( m_flIgniteTime, FIELD_TIME ),
	DEFINE_THINKFUNC( IgniteThink ),
	DEFINE_THINKFUNC( FlyThink ),
	DEFINE_THINKFUNC( GibThink ),
	DEFINE_TOUCHFUNC( RocketTouch ),
	DEFINE_TOUCHFUNC( MyBounceTouch ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( spore, CSpore );

void CSpore::Precache()
{
	PRECACHE_MODEL( "models/spore.mdl" );
	PRECACHE_MODEL( "sprites/glow01.spr" );

	m_iBlow = PRECACHE_MODEL( "sprites/spore_exp_01.spr" );
	m_iBlowSmall = PRECACHE_MODEL( "sprites/spore_exp_c_01.spr" );
	m_iSpitSprite = m_iTrail = PRECACHE_MODEL( "sprites/tinyspit.spr" );

	PRECACHE_SOUND( "weapons/splauncher_impact.wav" );
	PRECACHE_SOUND( "weapons/splauncher_bounce.wav" );
}

void CSpore::Spawn()
{
	Precache();

	if( m_SporeType == SporeType::GRENADE )
		SetMoveType( MOVETYPE_BOUNCE );
	else
		SetMoveType( MOVETYPE_FLY );

	SetSolidType( SOLID_BBOX );

	SetModel( "models/spore.mdl" );

	SetSize( g_vecZero, g_vecZero );

	SetAbsOrigin( GetAbsOrigin() );

	SetThink( &CSpore::FlyThink );

	if( m_SporeType == SporeType::GRENADE )
	{
		SetTouch( &CSpore::MyBounceTouch );

		if( !m_bPuked )
		{
			Vector vecAngles = GetAbsAngles();

			vecAngles.x -= ( UTIL_RandomLong( -5, 5 ) + 30 );
		}
	}
	else
	{
		SetTouch( &CSpore::RocketTouch );
	}

	UTIL_MakeVectors( GetAbsAngles() );

	if( !m_bIsAI )
	{
		if( m_SporeType != SporeType::GRENADE )
		{
			SetAbsVelocity( gpGlobals->v_forward * 1200 );
		}
	}
	else
	{
		SetGravity( 0.5 );
		SetFriction( 0.7 );
	}

	SetDamage( gSkillData.GetPlrDmgSpore() );

	m_flIgniteTime = gpGlobals->time;

	SetNextThink( gpGlobals->time + 0.01 );

	m_pSprite = CSprite::SpriteCreate( "sprites/glow01.spr", GetAbsOrigin(), false );

	m_pSprite->SetRenderMode( kRenderTransAdd );

	m_pSprite->SetRenderColor( Vector( 180, 180, 40 ) );

	m_pSprite->SetRenderAmount( 100 );

	m_pSprite->SetRenderFX( kRenderFxDistort );

	m_pSprite->SetScale( 0.8 );

	m_pSprite->SetAttachment( this, 0 );

	m_fRegisteredSound = false;

	m_flSoundDelay = gpGlobals->time;
}

void CSpore::BounceSound()
{
	//Nothing
}

void CSpore::IgniteThink()
{
	SetThink( nullptr );
	SetTouch( nullptr );

	if( m_pSprite )
	{
		UTIL_Remove( m_pSprite );
		m_pSprite = nullptr;
	}

	EMIT_SOUND_DYN(
		this,
		CHAN_WEAPON, "weapons/splauncher_impact.wav", 
		VOL_NORM, ATTN_NORM, 0, PITCH_NORM );

	const Vector vecDir = GetAbsVelocity().Normalize();

	TraceResult tr;

	UTIL_TraceLine( 
		GetAbsOrigin(), GetAbsOrigin() + vecDir * ( m_SporeType == SporeType::GRENADE ? 64 : 32 ),
		dont_ignore_monsters, edict(), &tr );

	UTIL_DecalTrace( &tr, DECAL_SPR_SPLT1 + UTIL_RandomLong( 0, 2 ) );

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
		WRITE_BYTE( TE_SPRITE_SPRAY );
		WRITE_COORD_VECTOR( GetAbsOrigin() );
		WRITE_COORD_VECTOR( tr.vecPlaneNormal );
		WRITE_SHORT( m_iSpitSprite );
		WRITE_BYTE( 100 );
		WRITE_BYTE( 40 );
		WRITE_BYTE( 180 );
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
		WRITE_BYTE( TE_DLIGHT );
		WRITE_COORD_VECTOR( GetAbsOrigin() );
		WRITE_BYTE( 10 );
		WRITE_BYTE( 15 );
		WRITE_BYTE( 220 );
		WRITE_BYTE( 40 );
		WRITE_BYTE( 5 );
		WRITE_BYTE( 10 );
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
		WRITE_BYTE( TE_SPRITE );
		WRITE_COORD_VECTOR( GetAbsOrigin() );
		WRITE_SHORT( UTIL_RandomLong( 0, 1 ) ? m_iBlow : m_iBlowSmall );
		WRITE_BYTE( 20 );
		WRITE_BYTE( 128 );
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
		WRITE_BYTE( TE_SPRITE_SPRAY );
		WRITE_COORD_VECTOR( GetAbsOrigin() );
		WRITE_COORD( UTIL_RandomFloat( -1, 1 ) );
		WRITE_COORD( 1 );
		WRITE_COORD( UTIL_RandomFloat( -1, 1 ) );
		WRITE_SHORT( m_iTrail );
		WRITE_BYTE( 2 );
		WRITE_BYTE( 20 );
		WRITE_BYTE( 80 );
	MESSAGE_END();

	::RadiusDamage( 
		GetAbsOrigin(), 
		CTakeDamageInfo( 
			this, 
			GetOwner(), 
			GetDamage(), 
			DMG_ALWAYSGIB | DMG_BLAST ), 
		200, 0 );

	SetThink( &CSpore::SUB_Remove );

	SetNextThink( gpGlobals->time );
}

void CSpore::FlyThink()
{
	const float flDelay = m_bIsAI ? 4.0 : 2.0;

	if( m_SporeType != SporeType::GRENADE || ( gpGlobals->time <= m_flIgniteTime + flDelay ) )
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
		WRITE_BYTE( TE_SPRITE_SPRAY );
			WRITE_COORD_VECTOR( GetAbsOrigin() );
			WRITE_COORD_VECTOR( GetAbsVelocity().Normalize() );
			WRITE_SHORT( m_iTrail );
			WRITE_BYTE( 2 );
			WRITE_BYTE( 20 );
			WRITE_BYTE( 80 );
		MESSAGE_END();
	}
	else
	{
		SetThink( &CSpore::IgniteThink );
	}

	SetNextThink( gpGlobals->time + 0.03 );
}

void CSpore::GibThink()
{
	//Nothing
}

void CSpore::RocketTouch( CBaseEntity* pOther )
{
	if( pOther->GetTakeDamageMode() != DAMAGE_NO )
	{
		pOther->TakeDamage( CTakeDamageInfo( this, GetOwner(), gSkillData.GetPlrDmgSpore(), DMG_GENERIC ) );
	}

	IgniteThink();
}

void CSpore::MyBounceTouch( CBaseEntity* pOther )
{
	if( pOther->GetTakeDamageMode() == DAMAGE_NO )
	{
		if( pOther != GetOwner() )
		{
			if( gpGlobals->time > m_flSoundDelay )
			{
				CSoundEnt::InsertSound( 
					bits_SOUND_DANGER, 
					GetAbsOrigin(),
					static_cast<int>( floor( GetDamage() / 0.4 ) ),
					0.3 );

				m_flSoundDelay = gpGlobals->time + 1.0;
			}

			if( GetFlags().Any( FL_SWIM ) )
			{
				SetAbsVelocity( GetAbsVelocity() * 0.5 );

				SetSequence( SPORE_IDLE );
			}
			else
			{
				EMIT_SOUND_DYN( 
					this, 
					CHAN_VOICE, "weapons/splauncher_bounce.wav", 
					0.25, ATTN_NORM, 0, PITCH_NORM );
			}
		}
	}
	else
	{
		pOther->TakeDamage( CTakeDamageInfo( this, GetOwner(), gSkillData.GetPlrDmgSpore(), DMG_GENERIC ) );

		IgniteThink();
	}
}

CSpore* CSpore::CreateSpore( 
	const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner,
	SporeType sporeType, bool bIsAI, bool bPuked )
{
	CSpore* pSpore = static_cast<CSpore*>( UTIL_CreateNamedEntity( "spore" ) );

	pSpore->SetAbsOrigin( vecOrigin );

	pSpore->m_SporeType = sporeType;

	if( bIsAI )
	{
		pSpore->SetAbsVelocity( vecAngles );

		pSpore->SetAbsAngles( UTIL_VecToAngles( vecAngles ) );
	}
	else
	{
		pSpore->SetAbsAngles( vecAngles );
	}

	pSpore->m_bIsAI = bIsAI;

	pSpore->m_bPuked = bPuked;

	pSpore->Spawn();

	pSpore->SetOwner( pOwner );

	return pSpore;
}

#endif // GAME_OPFOR
