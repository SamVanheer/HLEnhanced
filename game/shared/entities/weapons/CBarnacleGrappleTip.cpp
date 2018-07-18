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

#ifndef CLIENT_DLL
#include "gamerules/GameRules.h"
#endif

#include "com_model.h"

#include "CBarnacleGrappleTip.h"

BEGIN_DATADESC( CBarnacleGrappleTip )
	DEFINE_THINKFUNC( FlyThink ),
	DEFINE_THINKFUNC( OffsetThink ),
	DEFINE_TOUCHFUNC( TongueTouch ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( grapple_tip, CBarnacleGrappleTip );

namespace
{
//TODO: this should be handled differently. A method that returns an overall size, another whether it's fixed, etc. - Solokiller
const char* const grapple_small[] = 
{
	"monster_bloater",
	"monster_snark",
	"monster_shockroach",
	"monster_rat",
	"monster_alien_babyvoltigore",
	"monster_babycrab",
	"monster_cockroach",
	"monster_flyer_flock",
	"monster_headcrab",
	"monster_leech",
	"monster_penguin"
};

const char* const grapple_medium[] = 
{
	"monster_alien_controller",
	"monster_alien_slave",
	"monster_barney",
	"monster_bullchicken",
	"monster_cleansuit_scientist",
	"monster_houndeye",
	"monster_human_assassin",
	"monster_human_grunt",
	"monster_human_grunt_ally",
	"monster_human_medic_ally",
	"monster_human_torch_ally",
	"monster_male_assassin",
	"monster_otis",
	"monster_pitdrone",
	"monster_scientist",
	"monster_zombie",
	"monster_zombie_barney",
	"monster_zombie_soldier"
};

const char* const grapple_large[] = 
{
	"monster_alien_grunt",
	"monster_alien_voltigore",
	"monster_assassin_repel",
	"monster_grunt_ally_repel",
	"monster_bigmomma",
	"monster_gargantua",
	"monster_geneworm",
	"monster_gonome",
	"monster_grunt_repel",
	"monster_ichthyosaur",
	"monster_nihilanth",
	"monster_pitworm",
	"monster_pitworm_up",
	"monster_shocktrooper"
};

const char* const grapple_fixed[] = 
{
	"monster_barnacle",
	"monster_sitting_cleansuit_scientist",
	"monster_sitting_scientist",
	"monster_tentacle",
	"ammo_spore"
};
}

void CBarnacleGrappleTip::Precache()
{
	PRECACHE_MODEL( "models/shock_effect.mdl" );
}

void CBarnacleGrappleTip::Spawn()
{
	Precache();

	SetMoveType( MOVETYPE_FLY );
	SetSolidType( SOLID_BBOX );

	SetModel( "models/shock_effect.mdl" );

	SetSize( g_vecZero, g_vecZero );

	SetAbsOrigin( GetAbsOrigin() );

	SetThink( &CBarnacleGrappleTip::FlyThink );
	SetTouch( &CBarnacleGrappleTip::TongueTouch );

	Vector vecAngles = GetAbsAngles();

	vecAngles.x -= 30.0;

	SetAbsAngles( vecAngles );

	UTIL_MakeVectors( GetAbsAngles() );

	vecAngles.x = -( 30.0 + vecAngles.x );

	SetAbsVelocity( g_vecZero );

	SetGravity( 1.0 );

	SetNextThink( gpGlobals->time + 0.02 );

	m_bIsStuck = false;
	m_bMissed = false;
}

void CBarnacleGrappleTip::FlyThink()
{
	UTIL_MakeAimVectors( GetAbsAngles() );

	SetAbsAngles( UTIL_VecToAngles( gpGlobals->v_forward ) );

	const float flNewVel = ( ( GetAbsVelocity().Length() * 0.8 ) + 400.0 );

	SetAbsVelocity( GetAbsVelocity() * 0.2 + ( flNewVel * gpGlobals->v_forward ) );

	if( !bIsMultiplayer() )
	{
		//Note: the old grapple had a maximum velocity of 1600. - Solokiller
		if( GetAbsVelocity().Length() > 750.0 )
		{
			SetAbsVelocity( GetAbsVelocity().Normalize() * 750.0 );
		}
	}
	else
	{
		//TODO: should probably clamp at sv_maxvelocity to prevent the tip from going off course. - Solokiller
		if( GetAbsVelocity().Length() > 2000.0 )
		{
			SetAbsVelocity( GetAbsVelocity().Normalize() * 2000.0 );
		}
	}

	SetNextThink( gpGlobals->time + 0.02 );
}

void CBarnacleGrappleTip::OffsetThink()
{
	//Nothing
}

void CBarnacleGrappleTip::TongueTouch( CBaseEntity* pOther )
{
	TargetClass targetClass;

	if( !pOther )
	{
		targetClass = TargetClass::NOT_A_TARGET;
		m_bMissed = true;
	}
	else
	{
		if( pOther->IsPlayer() )
		{
			targetClass = TargetClass::MEDIUM;

			m_hGrappleTarget = pOther;

			m_bIsStuck = true;
		}
		else
		{
			targetClass = ClassifyTarget( pOther );

			if( targetClass != TargetClass::NOT_A_TARGET )
			{
				m_bIsStuck = true;
			}
			else
			{
				m_bMissed = true;
			}
		}
	}

	SetAbsVelocity( g_vecZero );

	m_GrappleType = targetClass;

	SetThink( &CBarnacleGrappleTip::OffsetThink );
	SetNextThink( gpGlobals->time + 0.02 );

	SetTouch( nullptr );
}

CBarnacleGrappleTip::TargetClass CBarnacleGrappleTip::ClassifyTarget( CBaseEntity* pTarget )
{
	if( !pTarget )
		return TargetClass::NOT_A_TARGET;

	if( pTarget->IsPlayer() )
	{
		m_hGrappleTarget = pTarget;

		return TargetClass::MEDIUM;
	}

	const Vector vecStart = GetAbsOrigin();
	const Vector vecEnd = GetAbsOrigin() + GetAbsVelocity() * 1024.0;

	TraceResult tr;

	UTIL_TraceLine( vecStart, vecEnd, ignore_monsters, edict(), &tr );

	CBaseEntity* pHit = Instance( tr.pHit );

	if( !pHit )
		pHit = CWorld::GetInstance();

	const texture_t* pTexture = UTIL_TraceTexture( pHit, vecStart, vecEnd );

	bool bIsFixed = false;

	if( pTexture && strnicmp( pTexture->name, "xeno_grapple", 12 ) == 0 )
	{
		bIsFixed = true;
	}
	else
	{
		for( size_t uiIndex = 0; uiIndex < ARRAYSIZE( grapple_small ); ++uiIndex )
		{
			if( strcmp( pTarget->GetClassname(), grapple_small[ uiIndex ] ) == 0 )
			{
				m_hGrappleTarget = pTarget;
				m_vecOriginOffset = GetAbsOrigin() - pTarget->GetAbsOrigin();

				return TargetClass::SMALL;
			}
		}

		for( size_t uiIndex = 0; uiIndex < ARRAYSIZE( grapple_medium ); ++uiIndex )
		{
			if( strcmp( pTarget->GetClassname(), grapple_medium[ uiIndex ] ) == 0 )
			{
				m_hGrappleTarget = pTarget;
				m_vecOriginOffset = GetAbsOrigin() - pTarget->GetAbsOrigin();

				return TargetClass::MEDIUM;
			}
		}

		for( size_t uiIndex = 0; uiIndex < ARRAYSIZE( grapple_large ); ++uiIndex )
		{
			if( strcmp( pTarget->GetClassname(), grapple_large[ uiIndex ] ) == 0 )
			{
				m_hGrappleTarget = pTarget;
				m_vecOriginOffset = GetAbsOrigin() - pTarget->GetAbsOrigin();

				return TargetClass::LARGE;
			}
		}

		for( size_t uiIndex = 0; uiIndex < ARRAYSIZE( grapple_fixed ); ++uiIndex )
		{
			if( strcmp( pTarget->GetClassname(), grapple_fixed[ uiIndex ] ) == 0 )
			{
				bIsFixed = true;
				break;
			}
		}
	}

	if( bIsFixed )
	{
		m_hGrappleTarget = pTarget;
		m_vecOriginOffset = g_vecZero;

		return TargetClass::FIXED;
	}

	return TargetClass::NOT_A_TARGET;
}

void CBarnacleGrappleTip::SetPosition( const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner )
{
	SetAbsOrigin( vecOrigin );
	SetAbsAngles( vecAngles );
	SetOwner( pOwner );
}

#endif // GAME_OPFOR
