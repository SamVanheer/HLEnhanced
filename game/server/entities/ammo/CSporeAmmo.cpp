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

#include "entities/weapons/CSpore.h"

#include "CSporeAmmo.h"

BEGIN_DATADESC( CSporeAmmo )
	DEFINE_THINKFUNC( Idling ),
	DEFINE_TOUCHFUNC( SporeTouch ),
END_DATADESC()

//TODO: Add an ammo entity to give it to players. - Solokiller
LINK_ENTITY_TO_CLASS( ammo_spore, CSporeAmmo );

void CSporeAmmo::Precache()
{
	PRECACHE_MODEL( "models/spore_ammo.mdl" );
	PRECACHE_SOUND( "weapons/spore_ammo.wav" );
}

void CSporeAmmo::Spawn()
{
	Precache();

	SetModel( "models/spore_ammo.mdl" );

	SetMoveType( MOVETYPE_FLY );

	SetSize( Vector( -16, -16, -16 ), Vector( 16, 16, 16 ) );

	Vector vecOrigin = GetAbsOrigin();

	vecOrigin.z += 16;

	SetAbsOrigin( vecOrigin );

	Vector vecAngles = GetAbsAngles();

	vecAngles.x -= 90;

	SetAbsAngles( vecAngles );

	SetSequence( SPOREAMMO_SPAWNDN );

	SetAnimTime( gpGlobals->time );

	SetNextThink( gpGlobals->time + 4 );

	SetFrame( 0 );

	SetFrameRate( 1 );

	SetHealth( 1 );

	SetBody( SPOREAMMOBODY_FULL );

	SetTakeDamageMode( DAMAGE_AIM );

	SetSolidType( SOLID_BBOX );

	SetTouch( &CSporeAmmo::SporeTouch );
	SetThink( &CSporeAmmo::Idling );
}

void CSporeAmmo::OnTakeDamage( const CTakeDamageInfo& info )
{
	if( GetBody() == SPOREAMMOBODY_EMPTY )
	{
		return;
	}

	SetBody( SPOREAMMOBODY_EMPTY );

	SetSequence( SPOREAMMO_SNATCHDN );

	SetAnimTime( gpGlobals->time );
	SetFrame( 0 );
	SetNextThink( gpGlobals->time + 0.66 );

	Vector vecAngles = GetAbsAngles();

	vecAngles.x -= 90;
	//Rotate it so spores that aren't rotated in Hammer point in the right direction. - Solokiller
	vecAngles.y += 180;

	Vector vecLaunchDir = vecAngles;

	vecLaunchDir.x += UTIL_RandomFloat( -20, 20 );
	vecLaunchDir.y += UTIL_RandomFloat( -20, 20 );
	vecLaunchDir.z += UTIL_RandomFloat( -20, 20 );

	CSpore* pSpore = CSpore::CreateSpore( GetAbsOrigin(), vecLaunchDir, this, CSpore::SporeType::GRENADE, false, true );

	UTIL_MakeVectors( vecLaunchDir );

	pSpore->SetAbsVelocity( gpGlobals->v_forward * 800 );
}

bool CSporeAmmo::AddAmmo( CBaseEntity* pOther )
{
	return UTIL_GiveAmmoToPlayer( this, pOther, 1, "spores", "weapons/spore_ammo.wav" );
}

void CSporeAmmo::Idling()
{
	switch( GetSequence() )
	{
	case SPOREAMMO_SPAWNDN:
		{
			SetSequence( SPOREAMMO_IDLE1 );
			SetAnimTime( gpGlobals->time );
			SetFrame( 0 );
			break;
		}

	case SPOREAMMO_SNATCHDN:
		{
			SetSequence( SPOREAMMO_IDLE );
			SetAnimTime( gpGlobals->time );
			SetFrame( 0 );
			SetNextThink( gpGlobals->time + 10 );
			break;
		}

	case SPOREAMMO_IDLE:
		{
			SetBody( SPOREAMMOBODY_FULL );
			SetSequence( SPOREAMMO_SPAWNDN );
			SetAnimTime( gpGlobals->time );
			SetFrame( 0 );
			SetNextThink( gpGlobals->time + 4 );
			break;
		}

	default: break;
	}
}

void CSporeAmmo::SporeTouch( CBaseEntity* pOther )
{
	if( !pOther->IsPlayer() || GetBody() == SPOREAMMOBODY_EMPTY )
		return;

	if( AddAmmo( pOther ) )
	{
		SetBody( SPOREAMMOBODY_EMPTY );

		SetSequence( SPOREAMMO_SNATCHDN );

		SetAnimTime( gpGlobals->time );
		SetFrame( 0 );
		SetNextThink( gpGlobals->time + 0.66 );
	}
}

#endif // GAME_OPFOR
