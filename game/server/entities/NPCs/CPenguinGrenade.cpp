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
#include "Monsters.h"
#include "Weapons.h"
#include "CBasePlayer.h"
#include "Skill.h"

#include "entities/CSoundEnt.h"
#include "gamerules/GameRules.h"

#include "CPenguinGrenade.h"

float CPenguinGrenade::m_flNextBounceSoundTime = 0;

BEGIN_DATADESC( CPenguinGrenade )
	DEFINE_FIELD( m_flDie, FIELD_TIME ),
	DEFINE_FIELD( m_vecTarget, FIELD_VECTOR ),
	DEFINE_FIELD( m_flNextHunt, FIELD_TIME ),
	DEFINE_FIELD( m_flNextHit, FIELD_TIME ),
	DEFINE_FIELD( m_posPrev, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),
	DEFINE_TOUCHFUNC( SuperBounceTouch ),
	DEFINE_THINKFUNC( HuntThink ),
	DEFINE_THINKFUNC( Smoke ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( monster_penguin, CPenguinGrenade );

void CPenguinGrenade::Precache()
{
	PRECACHE_MODEL( "models/w_penguin.mdl" );
	PRECACHE_SOUND( "squeek/sqk_blast1.wav" );
	PRECACHE_SOUND( "common/bodysplat.wav" );
	PRECACHE_SOUND( "squeek/sqk_die1.wav" );
	PRECACHE_SOUND( "squeek/sqk_hunt1.wav" );
	PRECACHE_SOUND( "squeek/sqk_hunt2.wav" );
	PRECACHE_SOUND( "squeek/sqk_hunt3.wav" );
	PRECACHE_SOUND( "squeek/sqk_deploy1.wav" );
}

void CPenguinGrenade::Spawn()
{
	Precache();
	// motor
	SetMoveType( MOVETYPE_BOUNCE );
	SetSolidType( SOLID_BBOX );

	SetModel( "models/w_penguin.mdl" );
	SetSize( Vector( -4, -4, 0 ), Vector( 4, 4, 8 ) );
	SetAbsOrigin( GetAbsOrigin() );

	SetTouch( &CPenguinGrenade::SuperBounceTouch );
	SetThink( &CPenguinGrenade::HuntThink );
	SetNextThink( gpGlobals->time + 0.1 );
	m_flNextHunt = gpGlobals->time + 1E6;

	GetFlags() |= FL_MONSTER;
	SetTakeDamageMode( DAMAGE_AIM );
	SetHealth(  gSkillData.GetSnarkHealth() );
	SetGravity( 0.5 );
	SetFriction( 0.5 );

	SetDamage( gSkillData.GetPlrDmgHandGrenade() );

	m_flDie = gpGlobals->time + PENGUIN_DETONATE_DELAY;

	m_flFieldOfView = 0; // 180 degrees

	if( CBaseEntity* pOwner = GetOwner() )
		m_hOwner = pOwner;

	m_flNextBounceSoundTime = gpGlobals->time;// reset each time a snark is spawned.

	SetSequence( PENGUINGRENADE_RUN );
	ResetSequenceInfo();
}

EntityClassification_t CPenguinGrenade::GetClassification()
{
	if( m_iMyClass != EntityClassifications().GetNoneId() )
		return m_iMyClass; // protect against recursion

	if( CBaseEntity* pEnemy = m_hEnemy )
	{
		m_iMyClass = EntityClassifications().GetClassificationId( classify::INSECT ); // no one cares about it

		const auto classId = pEnemy->Classify();

		if( classId == EntityClassifications().GetClassificationId( classify::PLAYER ) ||
			classId == EntityClassifications().GetClassificationId( classify::HUMAN_PASSIVE ) ||
			classId == EntityClassifications().GetClassificationId( classify::HUMAN_MILITARY ) )
		{
			m_iMyClass = EntityClassifications().GetNoneId();
			return EntityClassifications().GetClassificationId( classify::ALIEN_MILITARY ); // barney's get mad, grunts get mad at it
		}

		m_iMyClass = EntityClassifications().GetNoneId();
	}

	return EntityClassifications().GetClassificationId( classify::ALIEN_BIOWEAPON );
}

void CPenguinGrenade::Killed( const CTakeDamageInfo& info, GibAction gibAction )
{
	CBaseEntity* pOwner = m_hOwner;

	//Set owner if it was changed. - Solokiller
	if( pOwner )
	{
		SetOwner( pOwner );
	}

	Detonate();

	UTIL_BloodDrips( GetAbsOrigin(), g_vecZero, BloodColor(), 80 );

	//Detonate clears the owner, so set it again. - Solokiller
	if( pOwner )
	{
		SetOwner( pOwner );
	}
}

void CPenguinGrenade::GibMonster()
{
	EMIT_SOUND_DYN( this, CHAN_VOICE, "common/bodysplat.wav", 0.75, ATTN_NORM, 0, 200 );
}

void CPenguinGrenade::SuperBounceTouch( CBaseEntity *pOther )
{
	TraceResult tr = UTIL_GetGlobalTrace();

	CBaseEntity* pRealOwner = m_hOwner;

	{
		CBaseEntity* pOwner = GetOwner();
		// don't hit the guy that launched this grenade
		if( pOwner && pOther == pOwner )
			return;
	}

	// at least until we've bounced once
	SetOwner( nullptr );

	{
		Vector vecAngles = GetAbsAngles();

		vecAngles.x = 0;
		vecAngles.z = 0;

		SetAbsAngles( vecAngles );
	}

	// avoid bouncing too much
	if( m_flNextHit > gpGlobals->time )
		return;

	// higher pitch as squeeker gets closer to detonation time
	const float flpitch = 155.0 - 60.0 * ( ( m_flDie - gpGlobals->time ) / PENGUIN_DETONATE_DELAY );

	if( pOther->GetTakeDamageMode() != DAMAGE_NO && m_flNextAttack < gpGlobals->time )
	{
		// attack!

		bool bIsEnemy = true;

		//Check if this is an ally. Used for teamplay/CTF. - Solokiller
		if( g_pGameRules->IsMultiplayer() )
		{
			CBaseEntity* pOwner = pRealOwner;

			if( !pOwner )
				pOwner = CWorld::GetInstance();

			if( pOwner->IsPlayer() && pOther->IsPlayer() )
			{
				bIsEnemy = g_pGameRules->FPlayerCanTakeDamage(
					static_cast<CBasePlayer*>( pOther ), 
					CTakeDamageInfo( pOwner, gSkillData.GetSnarkDmgBite(), DMG_SLASH ) );
			}
		}

		CBaseEntity* pHit = Instance( tr.pHit );

		// make sure it's me who has touched them
		if( pHit == pOther )
		{
			// and it's not another squeakgrenade
			if( pHit->GetModelIndex() != GetModelIndex() )
			{
				// ALERT( at_console, "hit enemy\n");
				g_MultiDamage.Clear();
				pOther->TraceAttack( CTakeDamageInfo( this, gSkillData.GetSnarkDmgBite(), DMG_SLASH ), gpGlobals->v_forward, tr );
				if( pRealOwner )
					g_MultiDamage.ApplyMultiDamage( this, pRealOwner );
				else
					g_MultiDamage.ApplyMultiDamage( this, this );

				// add more explosion damage
				// m_flDie += 2.0; // add more life
				//Friendly players cause explosive damage to increase at a lower rate. - Solokiller
				SetDamage( GetDamage() + ( bIsEnemy ? gSkillData.GetPlrDmgHandGrenade() : gSkillData.GetPlrDmgHandGrenade() / 5.0 ) );

				if( GetDamage() > 500 )
					SetDamage( 500 );

				// make bite sound
				EMIT_SOUND_DYN( 
					this, 
					CHAN_WEAPON, "squeek/sqk_deploy1.wav", 
					VOL_NORM, ATTN_NORM, 0, ( int ) flpitch );
				m_flNextAttack = gpGlobals->time + 0.5;
			}
		}
	}

	m_flNextHit = gpGlobals->time + 0.1;
	m_flNextHunt = gpGlobals->time;

	if( g_pGameRules->IsMultiplayer() )
	{
		// in multiplayer, we limit how often snarks can make their bounce sounds to prevent overflows.
		if( gpGlobals->time < m_flNextBounceSoundTime )
		{
			// too soon!
			return;
		}
	}

	if( !( GetFlags().Any( FL_SWIM ) ) )
	{
		// play bounce sound
		float flRndSound = RANDOM_FLOAT( 0, 1 );

		if( flRndSound <= 0.33 )
			EMIT_SOUND_DYN( this, CHAN_VOICE, "squeek/sqk_hunt1.wav", VOL_NORM, ATTN_NORM, 0, ( int ) flpitch );
		else if( flRndSound <= 0.66 )
			EMIT_SOUND_DYN( this, CHAN_VOICE, "squeek/sqk_hunt2.wav", VOL_NORM, ATTN_NORM, 0, ( int ) flpitch );
		else
			EMIT_SOUND_DYN( this, CHAN_VOICE, "squeek/sqk_hunt3.wav", VOL_NORM, ATTN_NORM, 0, ( int ) flpitch );
		CSoundEnt::InsertSound( bits_SOUND_COMBAT, GetAbsOrigin(), 256, 0.25 );
	}
	else
	{
		// skittering sound
		CSoundEnt::InsertSound( bits_SOUND_COMBAT, GetAbsOrigin(), 100, 0.1 );
	}

	m_flNextBounceSoundTime = gpGlobals->time + 0.5;// half second.
}

void CPenguinGrenade::HuntThink()
{
	// ALERT( at_console, "think\n" );

	if( !IsInWorld() )
	{
		SetTouch( nullptr );
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance();
	SetNextThink( gpGlobals->time + 0.1 );

	// explode when ready
	if( gpGlobals->time >= m_flDie )
	{
		g_vecAttackDir = GetAbsVelocity().Normalize();
		SetHealth( -1 );
		Killed( CTakeDamageInfo( this, 0, 0 ), GIB_NORMAL );
		return;
	}

	// float
	if( GetWaterLevel() != WATERLEVEL_DRY )
	{
		if( GetMoveType() == MOVETYPE_BOUNCE )
		{
			SetMoveType( MOVETYPE_FLY );
		}
		Vector vecVelocity = GetAbsVelocity() * 0.9;
		vecVelocity.z += 8.0;

		SetAbsVelocity( vecVelocity );
	}
	else if( GetMoveType() == MOVETYPE_FLY )
	{
		SetMoveType( MOVETYPE_BOUNCE );
	}

	// return if not time to hunt
	if( m_flNextHunt > gpGlobals->time )
		return;

	m_flNextHunt = gpGlobals->time + 2.0;

	Vector vecFlat = GetAbsVelocity();
	vecFlat.z = 0;
	vecFlat = vecFlat.Normalize();

	UTIL_MakeVectors( GetAbsAngles() );

	if( m_hEnemy == nullptr || !m_hEnemy->IsAlive() )
	{
		// find target, bounce a bit towards it.
		Look( 512 );
		m_hEnemy = BestVisibleEnemy();
	}

	// squeek if it's about time blow up
	if( ( m_flDie - gpGlobals->time <= 0.5 ) && ( m_flDie - gpGlobals->time >= 0.3 ) )
	{
		EMIT_SOUND_DYN( 
			this, 
			CHAN_VOICE, "squeek/sqk_die1.wav", 
			VOL_NORM, ATTN_NORM, 0, PITCH_NORM + RANDOM_LONG( 0, 0x3F ) );
		CSoundEnt::InsertSound( bits_SOUND_COMBAT, GetAbsOrigin(), 256, 0.25 );
	}

	// higher pitch as squeeker gets closer to detonation time
	float flpitch = 155.0 - 60.0 * ( ( m_flDie - gpGlobals->time ) / PENGUIN_DETONATE_DELAY );
	if( flpitch < 80 )
		flpitch = 80;

	if( m_hEnemy != nullptr )
	{
		if( FVisible( m_hEnemy ) )
		{
			Vector vecDir = m_hEnemy->EyePosition() - GetAbsOrigin();
			m_vecTarget = vecDir.Normalize();
		}

		float flVel = GetAbsVelocity().Length();
		float flAdj = 50.0 / ( flVel + 10.0 );

		if( flAdj > 1.2 )
			flAdj = 1.2;

		// ALERT( at_console, "think : enemy\n");

		// ALERT( at_console, "%.0f %.2f %.2f %.2f\n", flVel, m_vecTarget.x, m_vecTarget.y, m_vecTarget.z );

		SetAbsVelocity( GetAbsVelocity() * flAdj + m_vecTarget * 300 );
	}

	if( GetFlags().Any( FL_SWIM ) )
	{
		SetAngularVelocity( g_vecZero );
	}
	else
	{
		if( GetAngularVelocity() == g_vecZero )
		{
			Vector vecAVel;
			vecAVel.x = RANDOM_FLOAT( -100, 100 );
			vecAVel.y = GetAngularVelocity().y;
			vecAVel.z = RANDOM_FLOAT( -100, 100 );

			SetAngularVelocity( vecAVel );
		}
	}

	if( ( GetAbsOrigin() - m_posPrev ).Length() < 1.0 )
	{
		Vector vecVel;
		vecVel.x = RANDOM_FLOAT( -100, 100 );
		vecVel.y = RANDOM_FLOAT( -100, 100 );
		vecVel.z = GetAbsVelocity().z;

		SetAbsVelocity( vecVel );
	}
	m_posPrev = GetAbsOrigin();

	Vector vecAngles = UTIL_VecToAngles( GetAbsVelocity() );

	vecAngles.x = 0;
	vecAngles.z = 0;

	SetAbsAngles( vecAngles );
}

void CPenguinGrenade::Smoke()
{
	if( UTIL_PointContents( GetAbsOrigin() ) == CONTENTS_WATER )
	{
		UTIL_Bubbles( 
			GetAbsOrigin() - Vector( 64, 64, 64 ), 
			GetAbsOrigin() + Vector( 64, 64, 64 ), 
			100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD_VECTOR( GetAbsOrigin() );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( ( int ) ( ( GetDamage() - 50.0 ) * 0.8 ) );
			WRITE_BYTE( 12 );
		MESSAGE_END();
	}

	UTIL_Remove( this );
}

#endif // GAME_OPFOR
