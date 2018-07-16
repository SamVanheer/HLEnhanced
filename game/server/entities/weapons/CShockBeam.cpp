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
#include "customentity.h"
#include "Skill.h"
#include "Decals.h"

#include "CShockBeam.h"

BEGIN_DATADESC( CShockBeam )
	DEFINE_THINKFUNC( FlyThink ),
	DEFINE_THINKFUNC( ExplodeThink ),
	DEFINE_THINKFUNC( WaterExplodeThink ),
	DEFINE_TOUCHFUNC( BallTouch ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( shock_beam, CShockBeam );

void CShockBeam::Precache()
{
	PRECACHE_MODEL( "sprites/flare3.spr" );
	PRECACHE_MODEL( "sprites/lgtning.spr" );
	PRECACHE_MODEL( "sprites/glow01.spr" );
	PRECACHE_MODEL( "models/shock_effect.mdl" );
	PRECACHE_SOUND( "weapons/shock_impact.wav" );
}

void CShockBeam::Spawn()
{
	Precache();

	SetMoveType( MOVETYPE_FLY );
	SetSolidType( SOLID_BBOX );

	SetModel( "models/shock_effect.mdl" );

	SetAbsOrigin( GetAbsOrigin() );

	SetSize( Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );

	SetTouch( &CShockBeam::BallTouch );
	SetThink( &CShockBeam::FlyThink );

	m_pSprite = CSprite::SpriteCreate( 
		"sprites/flare3.spr", 
		GetAbsOrigin(), 
		false );

	m_pSprite->SetRenderMode( kRenderTransAdd );
	m_pSprite->SetRenderColor( Vector( 255, 255, 255 ) );
	m_pSprite->SetRenderAmount( 255 );
	m_pSprite->SetRenderFX( kRenderFxDistort );

	m_pSprite->SetScale( 0.35 );

	m_pSprite->SetAttachment( this, 0 );

	m_pBeam1 = CBeam::BeamCreate( "sprites/lgtning.spr", 60 );

	if( m_pBeam1 )
	{
		m_pBeam1->SetAbsOrigin( GetAbsOrigin() );

		m_pBeam1->EntsInit( entindex(), entindex() );

		m_pBeam1->SetStartAttachment( 1 );
		m_pBeam1->SetEndAttachment( 2 );

		m_pBeam1->SetRenderColor( Vector( 0, 253, 253 ) );

		m_pBeam1->SetBeamFlags( BEAM_FSHADEOUT );
		m_pBeam1->SetBrightness( 180 );
		m_pBeam1->SetNoise( 0 );

		m_pBeam1->SetScrollRate( 10 );

		if( bIsMultiplayer() )
		{
			SetNextThink( gpGlobals->time + 0.01 );
			return;
		}

		m_pBeam2 = CBeam::BeamCreate( "sprites/lgtning.spr", 20 );

		if( m_pBeam2 )
		{
			m_pBeam2->SetAbsOrigin( GetAbsOrigin() );

			m_pBeam2->EntsInit( entindex(), entindex() );

			m_pBeam2->SetStartAttachment( 1 );
			m_pBeam2->SetEndAttachment( 2 );

			m_pBeam2->SetRenderColor( Vector( 255, 255, 157 ) );

			m_pBeam2->SetBeamFlags( BEAM_FSHADEOUT );
			m_pBeam2->SetBrightness( 180 );
			m_pBeam2->SetNoise( 30 );

			m_pBeam2->SetScrollRate( 30 );

			SetNextThink( gpGlobals->time + 0.01 );
		}
	}
}

EntityClassification_t CShockBeam::GetClassification()
{
	return EntityClassifications().GetNoneId();
}

void CShockBeam::FlyThink()
{
	if( GetWaterLevel() == WATERLEVEL_HEAD )
	{
		SetThink( &CShockBeam::WaterExplodeThink );
	}

	SetNextThink( gpGlobals->time + 0.01 );
}

void CShockBeam::ExplodeThink()
{
	Explode();
	UTIL_Remove( this );
}

void CShockBeam::WaterExplodeThink()
{
	CBaseEntity* pOwner = GetOwner();

	Explode();

	::RadiusDamage( 
		GetAbsOrigin(), 
		CTakeDamageInfo( this, pOwner, 100.0, DMG_ALWAYSGIB | DMG_BLAST ), 
		150.0, 0 );

	UTIL_Remove( this );
}

void CShockBeam::BallTouch( CBaseEntity* pOther )
{
	if( pOther->GetTakeDamageMode() != DAMAGE_NO )
	{
		TraceResult tr = UTIL_GetGlobalTrace();

		CBaseEntity* pOwner = GetOwner();

		g_MultiDamage.Clear();

		int bitsDamageTypes = DMG_ALWAYSGIB | DMG_SHOCK;

		if( CBaseMonster* pMonster = pOther->MyMonsterPointer() )
		{
			bitsDamageTypes = 64;
			if( pMonster->m_flShockDuration > 1.0 )
			{
				bitsDamageTypes = 8192;
			}

			pMonster->AddShockEffect( 63, 152, 208, 16, 0.5 );
		}

		pOther->TraceAttack( 
			CTakeDamageInfo( 
				pOwner, 
				bIsMultiplayer() ? gSkillData.GetPlrDmgShockRoachM() : gSkillData.GetPlrDmgShockRoachS(),
				bitsDamageTypes ), 
			GetAbsVelocity().Normalize(),
			tr );

		g_MultiDamage.ApplyMultiDamage( this, pOwner );

		SetAbsVelocity( g_vecZero );
	}

	SetThink( &CShockBeam::ExplodeThink );
	SetNextThink( gpGlobals->time + 0.01 );

	if( pOther->GetTakeDamageMode() == DAMAGE_NO )
	{
		TraceResult tr;

		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 10, dont_ignore_monsters, edict(), &tr );
	
		UTIL_DecalTrace( &tr, DECAL_OFSCORCH1 + UTIL_RandomLong( 0, 2 ) );

		MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, GetAbsOrigin() );
			WRITE_BYTE( TE_SPARKS );
			WRITE_COORD_VECTOR( GetAbsOrigin() );
		MESSAGE_END();
	}
}

void CShockBeam::Explode()
{
	if( m_pSprite )
	{
		UTIL_Remove( m_pSprite );
		m_pSprite = nullptr;
	}

	if( m_pBeam1 )
	{
		UTIL_Remove( m_pBeam1 );
		m_pBeam1 = nullptr;
	}

	if( m_pBeam2 )
	{
		UTIL_Remove( m_pBeam2 );
		m_pBeam2 = nullptr;
	}

	SetDamage( 40 );

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
	WRITE_BYTE( TE_DLIGHT );
	WRITE_COORD_VECTOR( GetAbsOrigin() );
	WRITE_BYTE( 8 );
	WRITE_BYTE( 0 );
	WRITE_BYTE( 253 );
	WRITE_BYTE( 253 );
	WRITE_BYTE( 5 );
	WRITE_BYTE( 10 );
	MESSAGE_END();

	SetOwner( nullptr );

	EMIT_SOUND_DYN(
		this, CHAN_WEAPON,
		"weapons/shock_impact.wav",
		UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM, 0, PITCH_NORM );
}

CShockBeam* CShockBeam::CreateShockBeam( const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner )
{
	Vector vecNewAngles = vecAngles;

	vecNewAngles.x = -vecNewAngles.x;

	CShockBeam* pBeam = static_cast<CShockBeam*>( Create( "shock_beam", vecOrigin, vecNewAngles, pOwner->edict(), false ) );

	pBeam->SetAbsOrigin( vecOrigin );

	UTIL_MakeVectors( vecAngles );

	pBeam->SetAbsVelocity( gpGlobals->v_forward * 2000.0 );

	pBeam->Spawn();

	return pBeam;
}

#endif // GAME_OPFOR
