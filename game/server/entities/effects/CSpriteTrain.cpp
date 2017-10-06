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

#include "entities/plats/CFuncTrain.h"
#include "entities/plats/CPathCorner.h"
#include "CSpriteTrain.h"

BEGIN_DATADESC( CSpriteTrain )
	DEFINE_FIELD( m_sounds, FIELD_INTEGER ),
	DEFINE_FIELD( m_pevCurrentTarget, FIELD_EVARS ),
	DEFINE_FIELD( m_activated, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_maxFrame, FIELD_FLOAT ),
	DEFINE_FIELD( m_lastTime, FIELD_TIME ),
	DEFINE_FIELD( m_waiting, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nexting, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_nextTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_waitTime, FIELD_FLOAT ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( env_spritetrain, CSpriteTrain );

void CSpriteTrain::Precache()
{
	PRECACHE_MODEL( GetModelName() );
	BaseClass::Precache();
}

void CSpriteTrain::Spawn()
{
	Precache();
	if( GetSpeed() == 0 )
		SetSpeed( 100 );

	SetRenderMode( kRenderTransAdd );
	SetRenderAmount( 255 );

	if( !HasTarget() )
		ALERT( at_console, "FuncTrain with no target" );

	if( GetDamage() == 0 )
		SetDamage( 2 );

	SetMoveType( MOVETYPE_PUSH );
	SetSolidType( SOLID_NOT );

	SetModel( GetModelName() );
	SetSize( GetRelMin(), GetRelMax() );
	SetAbsOrigin( GetAbsOrigin() );

	m_activated = false;

	if( m_volume == 0 )
		m_volume = 0.85;

	m_maxFrame = MODEL_FRAMES( GetModelIndex() ) - 1;

	m_lastTime = GetLastThink();

	SetNextThink( GetLastThink() + 0.1 );

	m_waiting = false;
	m_nexting = false;

	m_waitTime = GetLastThink();
	m_nextTime = GetLastThink();

	m_stopSprite = false;
}

void CSpriteTrain::Activate()
{
	// Not yet active, so teleport to first target
	if( !m_activated )
	{
		m_activated = true;

		CBaseEntity* pTarg = UTIL_FindEntityByTargetname( nullptr, GetTarget() );

		//Use the world if no target was found. Replicates SDK behavior. - Solokiller
		if( !pTarg )
		{
			pTarg = CWorld::GetInstance();
		}

		SetTarget( MAKE_STRING( pTarg->GetTarget() ) );
		//TODO change to EHANDLE - Solokiller
		m_pevCurrentTarget = pTarg->pev;// keep track of this since path corners change our target for us.

		SetAbsOrigin( pTarg->GetAbsOrigin() - ( GetRelMin() + GetRelMax() ) * 0.5 );

		if( !HasTargetname() )
		{	// not triggered, so start immediately
			m_nexting = true;
			m_nextTime = GetLastThink() + 0.1;
			SetNextThink( GetLastThink() + 0.1 );
			Alert( at_console, "time=%f, nexttime=%f\n", GetLastThink(), m_nextTime );
		}
		else
			GetSpawnFlags().AddFlags( SF_TRAIN_WAIT_RETRIGGER );
	}
}

void CSpriteTrain::OverrideReset()
{
	// Are we moving?
	if( GetAbsVelocity() != g_vecZero && GetNextThink() != 0 )
	{
		SetTarget( GetMessage() );
		// now find our next target
		CBaseEntity* pTarg = GetNextTarget();
		if( !pTarg )
		{
			//TODO: not quite right? - Solokiller
			SetNextThink( 0.1 );
			SetAbsVelocity( g_vecZero );
		}
		else	// Keep moving for 0.1 secs, then find path_corner again and restart
		{
			m_nextTime = GetLastThink() + 0.1;
			m_nexting = true;
		}
	}
}

void CSpriteTrain::Blocked( CBaseEntity* pOther )
{
	if( GetLastThink() < m_flActivateFinished )
		return;

	m_flActivateFinished = GetLastThink() + 0.5;

	pOther->TakeDamage( this, this, GetDamage(), DMG_CRUSH );
}

void CSpriteTrain::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue )
{
	if( GetSpawnFlags().Any( SF_TRAIN_WAIT_RETRIGGER ) )
	{
		// Move toward my target
		GetSpawnFlags().ClearFlags( SF_TRAIN_WAIT_RETRIGGER );
		Next();
	}
	else
	{
		GetSpawnFlags().AddFlags( SF_TRAIN_WAIT_RETRIGGER );
		// Pop back to last target if it's available
		if( pev->enemy )
			SetTarget( pev->enemy->v.targetname );
		SetAbsVelocity( g_vecZero );
		if( pev->noiseStopMoving )
			EMIT_SOUND( this, CHAN_VOICE, ( char* ) STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );

		m_nexting = true;
		m_nextTime = GetLastThink() + m_flWait;
	}
}

void CSpriteTrain::Think()
{
	Animate( GetFrameRate() * ( GetLastThink() - m_lastTime ) );

	if( m_flWait != -1.0 )
	{
		if( m_waiting && GetLastThink() >= m_waitTime )
		{
			if( m_stopSprite )
			{
				SetAbsVelocity( g_vecZero );
				m_stopSprite = false;
			}
			m_waiting = false;
			Wait();
		}
	}

	if( m_flWait != -1.0 )
	{
		if( m_nexting && GetLastThink() >= m_nextTime )
		{
			if( m_stopSprite )
			{
				SetAbsVelocity( g_vecZero );
				m_stopSprite = false;
			}
			m_nexting = false;
			Next();
		}
	}

	SetNextThink( GetLastThink() + 0.1 );
	m_lastTime = GetLastThink();
}

void CSpriteTrain::Wait()
{
	// Fire the pass target if there is one
	if( m_pevCurrentTarget->message )
	{
		FireTargets( STRING( m_pevCurrentTarget->message ), this, this, USE_TOGGLE, 0 );
		if( FBitSet( m_pevCurrentTarget->spawnflags, SF_CORNER_FIREONCE ) )
			m_pevCurrentTarget->message = iStringNull;
	}

	// need pointer to LAST target.
	if( FBitSet( m_pevCurrentTarget->spawnflags, SF_TRAIN_WAIT_RETRIGGER ) || ( GetSpawnFlags().Any( SF_TRAIN_WAIT_RETRIGGER ) ) )
	{
		GetSpawnFlags().AddFlags( SF_TRAIN_WAIT_RETRIGGER );
		// clear the sound channel.
		if( pev->noiseMovement )
			STOP_SOUND( this, CHAN_STATIC, ( char* ) STRING( pev->noiseMovement ) );
		if( pev->noiseStopMoving )
			EMIT_SOUND( this, CHAN_VOICE, ( char* ) STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );
		return;
	}

	// ALERT ( at_console, "%f\n", m_flWait );

	if( m_flWait != 0 )
	{	
		if( pev->noiseMovement )
			STOP_SOUND( this, CHAN_STATIC, ( char* ) STRING( pev->noiseMovement ) );
		if( pev->noiseStopMoving )
			EMIT_SOUND( this, CHAN_VOICE, ( char* ) STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );

		m_nexting = true;
		// -1 wait will wait forever!	
		m_nextTime = GetLastThink() + m_flWait;
	}
	else
	{
		Next();// do it RIGHT now!
	}
}

//
// Train next - path corner needs to change to next target 
//
void CSpriteTrain::Next()
{
	// now find our next target
	//TODO: this entity is supposed to work with path_corner only. Other entities will work, but will probably misbehave. - Solokiller
	//Check for classname and ignore others?
	CBaseEntity* pTarg = GetNextTarget();

	if( !pTarg )
	{
		if( pev->noiseMovement )
			STOP_SOUND( this, CHAN_STATIC, ( char* ) STRING( pev->noiseMovement ) );
		// Play stop sound
		if( pev->noiseStopMoving )
			EMIT_SOUND( this, CHAN_VOICE, ( char* ) STRING( pev->noiseStopMoving ), m_volume, ATTN_NORM );
		return;
	}

	// Save last target in case we need to find it again
	SetMessage( GetTarget() );

	SetTarget( pTarg->GetTarget() );
	m_flWait = pTarg->GetDelay();

	if( m_pevCurrentTarget && m_pevCurrentTarget->speed != 0 )
	{// don't copy speed from target if it is 0 (uninitialized)
		SetSpeed( m_pevCurrentTarget->speed );
		ALERT( at_aiconsole, "Train %s speed to %4.2f\n", GetTargetname(), GetSpeed() );
	}
	m_pevCurrentTarget = pTarg->pev;// keep track of this since path corners change our target for us.

	//TODO: replace with EHANDLE member - Solokiller
	//TODO Also change CFuncTrain
	pev->enemy = pTarg->edict();//hack

	if( FBitSet( m_pevCurrentTarget->spawnflags, SF_CORNER_TELEPORT ) )
	{
		// Path corner has indicated a teleport to the next corner.
		GetEffects().AddFlags( EF_NOINTERP );
		SetAbsOrigin( pTarg->GetAbsOrigin() - ( GetRelMin() + GetRelMax() )* 0.5 );
		Wait(); // Get on with doing the next path corner.
	}
	else
	{
		// Normal linear move.

		// CHANGED this from CHAN_VOICE to CHAN_STATIC around OEM beta time because trains should
		// use CHAN_STATIC for their movement sounds to prevent sound field problems.
		// this is not a hack or temporary fix, this is how things should be. (sjb).
		if( pev->noiseMovement )
		{
			STOP_SOUND( this, CHAN_STATIC, ( char* ) STRING( pev->noiseMovement ) );
			EMIT_SOUND( this, CHAN_STATIC, ( char* ) STRING( pev->noiseMovement ), m_volume, ATTN_NORM );
		}
		GetEffects().ClearFlags( EF_NOINTERP );

		LinearMove( pTarg->GetAbsOrigin() - ( GetRelMin() + GetRelMax() )* 0.5, GetSpeed() );
	}
}

void CSpriteTrain::LinearMove( Vector vecDest, float flSpeed )
{
	ASSERTSZ( flSpeed != 0, "LinearMove:  no speed is defined!" );
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "LinearMove: no post-move function defined");

	m_vecFinalDest = vecDest;

	// Already there?
	if( vecDest == GetAbsOrigin() )
	{
		Wait();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - GetAbsOrigin();

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	m_waiting = true;
	m_stopSprite = true;
	m_waitTime = GetLastThink() + flTravelTime;

	// scale the destdelta vector by the time spent traveling to get velocity
	SetAbsVelocity( vecDestDelta / flTravelTime );
}

void CSpriteTrain::Animate( float frames )
{
	if( m_maxFrame > 1.0 )
	{
		if( GetFrameRate() == 0.0 )
		{
			SetFrameRate( 10 );
		}

		SetFrame( fmod( frames + GetFrame(), m_maxFrame ) );
	}
}
