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
#include "Effects.h"
#include "Skill.h"
#include "Decals.h"
#include "gamerules/GameRules.h"

#include "CDisplacerBall.h"

extern CBaseEntity* g_pLastSpawn;

namespace
{
//TODO: can probably be smarter - Solokiller
const char* const displace[] = 
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
	"monster_zombie_soldier",
	"monster_alien_grunt",
	"monster_alien_voltigore",
	"monster_assassin_repel",
	"monster_grunt_ally_repel",
	"monster_gonome",
	"monster_grunt_repel",
	"monster_ichthyosaur",
	"monster_shocktrooper"
};
}

BEGIN_DATADESC( CDisplacerBall )
	DEFINE_TOUCHFUNC( BallTouch ),
	DEFINE_THINKFUNC( FlyThink ),
	DEFINE_THINKFUNC( FlyThink2 ),
	DEFINE_THINKFUNC( FizzleThink ),
	DEFINE_THINKFUNC( ExplodeThink ),
	DEFINE_THINKFUNC( KillThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( displacer_ball, CDisplacerBall );

void CDisplacerBall::Precache()
{
	PRECACHE_MODEL( "sprites/exit1.spr" );
	PRECACHE_MODEL( "sprites/lgtning.spr" );
	m_iTrail = PRECACHE_MODEL( "sprites/disp_ring.spr" );

	PRECACHE_SOUND( "weapons/displacer_impact.wav" );
	PRECACHE_SOUND( "weapons/displacer_teleport.wav" );
}

void CDisplacerBall::Spawn()
{
	Precache();

	SetMoveType( MOVETYPE_FLY );
	SetSolidType( SOLID_BBOX );

	SetModel( "sprites/exit1.spr" );

	SetAbsOrigin( GetAbsOrigin() );

	SetSize( g_vecZero, g_vecZero );

	SetRenderMode( kRenderTransAdd );

	SetRenderAmount( 255 );

	SetScale( 0.75 );

	SetTouch( &CDisplacerBall::BallTouch );
	SetThink( &CDisplacerBall::FlyThink );

	SetNextThink( gpGlobals->time + 0.2 );

	InitBeams();
}

EntityClassification_t CDisplacerBall::GetClassification()
{
	return EntityClassifications().GetNoneId();
}

void CDisplacerBall::BallTouch( CBaseEntity* pOther )
{
	SetAbsVelocity( g_vecZero );

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, GetAbsOrigin() );
		WRITE_BYTE( TE_BEAMCYLINDER );
		WRITE_COORD_VECTOR( GetAbsOrigin() );		// coord coord coord (center position) 
		WRITE_COORD( GetAbsOrigin().x );			// coord coord coord (axis and radius) 
		WRITE_COORD( GetAbsOrigin().y );			
		WRITE_COORD( GetAbsOrigin().z + 800.0 );	
		WRITE_SHORT( m_iTrail );					 // short (sprite index) 
		WRITE_BYTE( 0 );							 // byte (starting frame) 
		WRITE_BYTE( 0 );							 // byte (frame rate in 0.1's) 
		WRITE_BYTE( 3 );							 // byte (life in 0.1's) 
		WRITE_BYTE( 16 );							 // byte (line width in 0.1's) 
		WRITE_BYTE( 0 );							 // byte (noise amplitude in 0.01's) 
		WRITE_BYTE( 255 );							 // byte,byte,byte (color)
		WRITE_BYTE( 255 );							 
		WRITE_BYTE( 255 );							 
		WRITE_BYTE( 255 );							 // byte (brightness)
		WRITE_BYTE( 0 );							 // byte (scroll speed in 0.1's)
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
		WRITE_BYTE( TE_DLIGHT );
		WRITE_COORD_VECTOR( GetAbsOrigin() );		// coord, coord, coord (pos) 
		WRITE_BYTE( 16 );							// byte (radius in 10's) 
		WRITE_BYTE( 255 );							// byte byte byte (color)
		WRITE_BYTE( 180 );							
		WRITE_BYTE( 96 );							
		WRITE_BYTE( 10 );							// byte (brightness)
		WRITE_BYTE( 10 );							// byte (life in 10's)
	MESSAGE_END();

	m_hDisplacedTarget = nullptr;

	SetTouch( nullptr );
	SetThink( nullptr );

	TraceResult tr;

	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 10, dont_ignore_monsters, edict(), &tr );

	UTIL_DecalTrace( &tr, DECAL_SCORCH1 + UTIL_RandomLong( 0, 1 ) );

	if( pOther->IsPlayer() )
	{
		//TODO: what is this for? - Solokiller
		//pOther->AddFlags( FL_CLIENT );

		CBasePlayer* pPlayer = static_cast<CBasePlayer*>( pOther );

		//TODO: CTF support - Solokiller

#if 0
		if( g_pGameRules->IsCTF() && pPlayer->m_hFlag )
		{
			CCTFGoalFlag* pFlag = pPlayer->m_hFlag;

			pFlag->DropFlag( pOther );

			CBaseEntity* pOwner = GetOwner();

			if( !pOwner )
				pOwner = CWorld::GetInstance();

			if( pOwner->IsPlayer() && pOwner->m_iTeamNum != pOther->m_iTeamNum )
			{
				MESSAGE_BEGIN( MSG_ALL, gmsgCTFScore );
					WRITE_BYTE( pPlayer->entindex() );
					WRITE_BYTE( pPlayer->m_iCTFScore );
				MESSAGE_END();

				ClientPrint( pPlayer, HUD_PRINTTALK, "#CTFScorePoint" );
				UTIL_ClientPrintAll( HUD_PRINTNOTIFY, UTIL_VarArgs( "%s", pPlayer->GetNetName() ) );

				//TODO: team IDs - Solokiller
				if( pPlayer->m_iTeamNum == 1 )
				{
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "#CTFFlagDisplacedBM" );
				}
				else if( pPlayer->m_iTeamNum == 2 )
				{
					UTIL_ClientPrintAll( HUD_PRINTNOTIFY, "#CTFFlagDisplacedOF" );
				}
			}
		}
#endif

		CBaseEntity* pSpawnSpot = g_pGameRules->GetPlayerSpawnSpot( pPlayer );

		if( !pSpawnSpot )
			pSpawnSpot = g_pLastSpawn;

		Vector vecEnd = pSpawnSpot->GetAbsOrigin();

		vecEnd.z -= 100;

		UTIL_TraceLine( pSpawnSpot->GetAbsOrigin(), vecEnd, ignore_monsters, edict(), &tr );
	
		pPlayer->SetAbsOrigin( tr.vecEndPos + Vector( 0, 0, 37 ) );

		pPlayer->SetSequence( pPlayer->LookupActivity( ACT_IDLE ) );

		pPlayer->SetIsClimbing( false );

		//TODO: enable if needed. - Solokiller
#if 0
		pPlayer->m_flLastX = 0;
		pPlayer->m_flLastY = 0;
#endif

		pPlayer->m_fNoPlayerSound = false;
	}

	if( ClassifyTarget( pOther ) )
	{
		tr = UTIL_GetGlobalTrace();

		g_MultiDamage.Clear();

		m_hDisplacedTarget = pOther;

		pOther->Killed( CTakeDamageInfo( this, 0, DMG_GENERIC ), GIB_NEVER );
	}

	SetBaseVelocity( g_vecZero );

	SetAbsVelocity( g_vecZero );

	SetSolidType( SOLID_NOT );

	SetAbsOrigin( GetAbsOrigin() );

	SetThink( &CDisplacerBall::KillThink );

	SetNextThink( gpGlobals->time + ( g_pGameRules->IsMultiplayer() ? 0.2 : 0.5 ) );
}

void CDisplacerBall::FlyThink()
{
	ArmBeam( -1 );
	ArmBeam( 1 );
	SetNextThink( gpGlobals->time + 0.05 );
}

void CDisplacerBall::FlyThink2()
{
	SetSize( Vector( -8, -8, -8 ), Vector( 8, 8, 8 ) );

	ArmBeam( -1 );
	ArmBeam( 1 );

	SetNextThink( gpGlobals->time + 0.05 );
}

void CDisplacerBall::FizzleThink()
{
	ClearBeams();

	SetDamage( gSkillData.GetPlrDmgDisplacerOther() );

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, GetAbsOrigin() );
		WRITE_BYTE( TE_DLIGHT );
		WRITE_COORD_VECTOR( GetAbsOrigin() );
		WRITE_BYTE( 16 );
		WRITE_BYTE( 255 );
		WRITE_BYTE( 180 );
		WRITE_BYTE( 96 );
		WRITE_BYTE( 10 );
		WRITE_BYTE( 10 );
	MESSAGE_END();

	CBaseEntity* pOwner = GetOwner();

	SetOwner( nullptr );
	
	RadiusDamage( 
		GetAbsOrigin(), 
		CTakeDamageInfo( 
			this, 
			pOwner, 
			GetDamage(), 
			DMG_ALWAYSGIB | DMG_BLAST ), 
		128.0, 0 );

	EMIT_SOUND_DYN( 
		this, 
		CHAN_WEAPON, "weapons/displacer_impact.wav", 
		UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM, 0, PITCH_NORM );

	UTIL_Remove( this );
}

void CDisplacerBall::ExplodeThink()
{
	ClearBeams();

	SetDamage( gSkillData.GetPlrDmgDisplacerOther() );

	CBaseEntity* pOwner = GetOwner();

	SetOwner( nullptr );

	RadiusDamage(
		GetAbsOrigin(),
		CTakeDamageInfo(
			this,
			pOwner,
			GetDamage(),
			DMG_ALWAYSGIB | DMG_BLAST ),
		gSkillData.GetPlrRadiusDisplacer(), 0 );

	EMIT_SOUND_DYN(
		this,
		CHAN_WEAPON, "weapons/displacer_teleport.wav",
		UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM, 0, PITCH_NORM );

	UTIL_Remove( this );
}

void CDisplacerBall::KillThink()
{
	if( CBaseEntity* pTarget = m_hDisplacedTarget )
	{
		pTarget->SetThink( &CBaseEntity::SUB_Remove );

		//TODO: no next think? - Solokiller
	}

	SetThink( &CDisplacerBall::ExplodeThink );
	SetNextThink( gpGlobals->time + 0.2 );
}

void CDisplacerBall::InitBeams()
{
	memset( m_pBeam, 0, sizeof( m_pBeam ) );

	m_uiBeams = 0;

	SetSkin( 0 );
}

void CDisplacerBall::ClearBeams()
{
	for( auto& pBeam : m_pBeam )
	{
		if( pBeam )
		{
			UTIL_Remove( pBeam );
			pBeam = nullptr;
		}
	}

	m_uiBeams = 0;

	SetSkin( 0 );
}

void CDisplacerBall::ArmBeam( int iSide )
{
	//This method is identical to the Alien Slave's ArmBeam, except it treats m_pBeam as a circular buffer.
	if( m_uiBeams >= NUM_BEAMS )
		m_uiBeams = 0;

	TraceResult tr;
	float flDist = 1.0;

	UTIL_MakeAimVectors( GetAbsAngles() );
	Vector vecSrc = GetAbsOrigin() + gpGlobals->v_up * 36 + gpGlobals->v_right * iSide * 16 + gpGlobals->v_forward * 32;

	for( int i = 0; i < 3; i++ )
	{
		Vector vecAim = gpGlobals->v_right * iSide * RANDOM_FLOAT( 0, 1 ) + gpGlobals->v_up * RANDOM_FLOAT( -1, 1 );
		TraceResult tr1;
		UTIL_TraceLine( vecSrc, vecSrc + vecAim * 512, dont_ignore_monsters, ENT( pev ), &tr1 );
		if( flDist > tr1.flFraction )
		{
			tr = tr1;
			flDist = tr.flFraction;
		}
	}

	// Couldn't find anything close enough
	if( flDist == 1.0 )
		return;

	//The beam might already exist if we've created all beams before. - Solokiller
	if( !m_pBeam[ m_uiBeams ] )
		m_pBeam[ m_uiBeams ] = CBeam::BeamCreate( "sprites/lgtning.spr", 30 );

	if( !m_pBeam[ m_uiBeams ] )
		return;

	CBaseEntity* pHit = Instance( tr.pHit );

	if( !pHit )
		pHit = CWorld::GetInstance();

	if( pHit && pHit->GetTakeDamageMode() != DAMAGE_NO )
	{
		//Beam hit something, deal radius damage to it. - Solokiller
		m_pBeam[ m_uiBeams ]->EntsInit( pHit->entindex(), entindex() );

		m_pBeam[ m_uiBeams ]->SetRenderColor( Vector( 255, 255, 255 ) );

		m_pBeam[ m_uiBeams ]->SetBrightness( 255 );

		RadiusDamage( 
			tr.vecEndPos, 
			CTakeDamageInfo( 
				this, 
				GetOwner(),
				25.0, 
				DMG_ENERGYBEAM ), 
			15.0, 0 );
	}
	else
	{
		m_pBeam[ m_uiBeams ]->PointEntInit( tr.vecEndPos, entindex() );
		m_pBeam[ m_uiBeams ]->SetEndAttachment( iSide < 0 ? 2 : 1 );
		// m_pBeam[ m_uiBeams ]->SetColor( 180, 255, 96 );
		m_pBeam[ m_uiBeams ]->SetColor( 96, 128, 16 );
		m_pBeam[ m_uiBeams ]->SetBrightness( 64 );
		m_pBeam[ m_uiBeams ]->SetNoise( 80 );
	}

	++m_uiBeams;
}

bool CDisplacerBall::ClassifyTarget( CBaseEntity* pTarget )
{
	if( !pTarget )
		return false;

	if( pTarget->IsPlayer() )
		return false;

	for( size_t uiIndex = 0; uiIndex < ARRAYSIZE( displace ); ++uiIndex )
	{
		if( strcmp( pTarget->GetClassname(), displace[ uiIndex ] ) == 0 )
			return true;
	}

	return false;
}

CDisplacerBall* CDisplacerBall::CreateDisplacerBall( const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner )
{
	CDisplacerBall* pBall = static_cast<CDisplacerBall*>( UTIL_CreateNamedEntity( "displacer_ball" ) );

	pBall->SetAbsOrigin( vecOrigin );

	Vector vecNewAngles = vecAngles;

	vecNewAngles.x = -vecNewAngles.x;

	pBall->SetAbsAngles( vecNewAngles );

	UTIL_MakeVectors( vecAngles );

	pBall->SetAbsVelocity( gpGlobals->v_forward * 500 );

	pBall->SetOwner( pOwner );

	pBall->Spawn();

	return pBall;
}

#endif // GAME_OPFOR
