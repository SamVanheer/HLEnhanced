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
#include "Skill.h"
#include "Decals.h"
#include "cbase.h"
#include "Weapons.h"

/*
================
TraceAttack
================
*/
void CBaseEntity::TraceAttack( const CTakeDamageInfo& info, Vector vecDir, TraceResult *ptr )
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	if( pev->takedamage )
	{
		g_MultiDamage.AddMultiDamage( info, this );

		int blood = BloodColor();

		if( blood != DONT_BLEED )
		{
			SpawnBlood( vecOrigin, blood, info.GetDamage() );// a little surface blood.
			TraceBleed( info, vecDir, ptr );
		}
	}
}

void CBaseEntity::TraceBleed( const CTakeDamageInfo& info, Vector vecDir, TraceResult *ptr )
{
	if( BloodColor() == DONT_BLEED )
		return;

	if( info.GetDamage() == 0 )
		return;

	if( !( info.GetDamageTypes() & ( DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_MORTAR ) ) )
		return;

	// make blood decal on the wall! 
	TraceResult Bloodtr;
	Vector vecTraceDir;
	float flNoise;
	int cCount;
	int i;

	/*
	if ( !IsAlive() )
	{
	// dealing with a dead monster.
	if ( pev->max_health <= 0 )
	{
	// no blood decal for a monster that has already decalled its limit.
	return;
	}
	else
	{
	pev->max_health--;
	}
	}
	*/

	if( info.GetDamage() < 10 )
	{
		flNoise = 0.1;
		cCount = 1;
	}
	else if( info.GetDamage() < 25 )
	{
		flNoise = 0.2;
		cCount = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount = 4;
	}

	for( i = 0; i < cCount; i++ )
	{
		vecTraceDir = vecDir * -1;// trace in the opposite direction the shot came from (the direction the shot is going)

		vecTraceDir.x += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.y += RANDOM_FLOAT( -flNoise, flNoise );
		vecTraceDir.z += RANDOM_FLOAT( -flNoise, flNoise );

		UTIL_TraceLine( ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT( pev ), &Bloodtr );

		if( Bloodtr.flFraction != 1.0 )
		{
			UTIL_BloodDecalTrace( &Bloodtr, BloodColor() );
		}
	}
}

void CBaseEntity::SUB_FadeOut( void )
{
	if( pev->renderamt > 7 )
	{
		pev->renderamt -= 7;
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		pev->renderamt = 0;
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink( &CBaseEntity::SUB_Remove );
	}
}

//
// fade out - slowly fades a entity out, then removes it.
//
// DON'T USE ME FOR GIBS AND STUFF IN MULTIPLAYER! 
// SET A FUTURE THINK AND A RENDERMODE!!
void CBaseEntity::SUB_StartFadeOut( void )
{
	if( pev->rendermode == kRenderNormal )
	{
		pev->renderamt = 255;
		pev->rendermode = kRenderTransTexture;
	}

	pev->solid = SOLID_NOT;
	pev->avelocity = g_vecZero;

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink( &CBaseEntity::SUB_FadeOut );
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Monsters.
================
*/
void CBaseEntity::FireBullets( const unsigned int cShots,
							   Vector vecSrc, Vector vecDirShooting, Vector vecSpread,
							   float flDistance, int iBulletType,
							   int iTracerFreq, int iDamage, CBaseEntity* pAttacker )
{
	static int tracerCount;
	int tracer;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;

	if( pAttacker == nullptr )
		pAttacker = this;  // the default attacker is ourselves

	g_MultiDamage.Clear();
	g_MultiDamage.SetDamageTypes( DMG_BULLET | DMG_NEVERGIB );

	for( unsigned int iShot = 1; iShot <= cShots; iShot++ )
	{
		// get circular gaussian spread
		float x, y;

		UTIL_GetCircularGaussianSpread( x, y );

		Vector vecDir = vecDirShooting +
			x * vecSpread.x * vecRight +
			y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( pev )/*pentIgnore*/, &tr );

		tracer = 0;
		if( iTracerFreq != 0 && ( tracerCount++ % iTracerFreq ) == 0 )
		{
			Vector vecTracerSrc;

			if( IsPlayer() )
			{// adjust tracer position for player
				vecTracerSrc = vecSrc + Vector( 0, 0, -4 ) + gpGlobals->v_right * 2 + gpGlobals->v_forward * 16;
			}
			else
			{
				vecTracerSrc = vecSrc;
			}

			if( iTracerFreq != 1 )		// guns that always trace also always decal
				tracer = 1;
			switch( iBulletType )
			{
			case BULLET_MONSTER_MP5:
			case BULLET_MONSTER_9MM:
			case BULLET_MONSTER_12MM:
			default:
				MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, vecTracerSrc );
				WRITE_BYTE( TE_TRACER );
				WRITE_COORD( vecTracerSrc.x );
				WRITE_COORD( vecTracerSrc.y );
				WRITE_COORD( vecTracerSrc.z );
				WRITE_COORD( tr.vecEndPos.x );
				WRITE_COORD( tr.vecEndPos.y );
				WRITE_COORD( tr.vecEndPos.z );
				MESSAGE_END();
				break;
			}
		}
		// do damage, paint decals
		if( tr.flFraction != 1.0 )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

			if( iDamage )
			{
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, iDamage, DMG_BULLET | ( ( iDamage > 16 ) ? DMG_ALWAYSGIB : DMG_NEVERGIB ) ), vecDir, &tr );

				TEXTURETYPE_PlaySound( &tr, vecSrc, vecEnd, iBulletType );
				DecalGunshot( &tr, iBulletType );
			}
			else switch( iBulletType )
			{
			default:
			case BULLET_MONSTER_9MM:
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, gSkillData.monDmg9MM, DMG_BULLET ), vecDir, &tr );

				TEXTURETYPE_PlaySound( &tr, vecSrc, vecEnd, iBulletType );
				DecalGunshot( &tr, iBulletType );

				break;

			case BULLET_MONSTER_MP5:
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, gSkillData.monDmgMP5, DMG_BULLET ), vecDir, &tr );

				TEXTURETYPE_PlaySound( &tr, vecSrc, vecEnd, iBulletType );
				DecalGunshot( &tr, iBulletType );

				break;

			case BULLET_MONSTER_12MM:
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, gSkillData.monDmg12MM, DMG_BULLET ), vecDir, &tr );
				if( !tracer )
				{
					TEXTURETYPE_PlaySound( &tr, vecSrc, vecEnd, iBulletType );
					DecalGunshot( &tr, iBulletType );
				}
				break;

			case BULLET_NONE: // FIX 
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, 50, DMG_CLUB ), vecDir, &tr );
				TEXTURETYPE_PlaySound( &tr, vecSrc, vecEnd, iBulletType );
				// only decal glass
				if( !FNullEnt( tr.pHit ) && GET_PRIVATE( tr.pHit )->GetRenderMode() != kRenderNormal )
				{
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG( 0, 2 ) );
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, ( flDistance * tr.flFraction ) / 64.0 );
	}
	g_MultiDamage.ApplyMultiDamage( this, pAttacker );
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.

This version is used by Players, uses the random seed generator to sync client and server side shots.
================
*/
Vector CBaseEntity::FireBulletsPlayer( const unsigned int cShots,
									   Vector vecSrc, Vector vecDirShooting, Vector vecSpread,
									   float flDistance, int iBulletType,
									   int iTracerFreq, int iDamage, CBaseEntity* pAttacker, int shared_rand )
{
	static int tracerCount;
	TraceResult tr;
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	float x, y;

	if( pAttacker == nullptr )
		pAttacker = this;  // the default attacker is ourselves

	g_MultiDamage.Clear();
	g_MultiDamage.SetDamageTypes( DMG_BULLET | DMG_NEVERGIB );

	for( unsigned int iShot = 1; iShot <= cShots; iShot++ )
	{
		//Use player's random seed.
		// get circular gaussian spread
		UTIL_GetSharedCircularGaussianSpread( shared_rand, iShot, x, y );

		Vector vecDir = vecDirShooting +
			x * vecSpread.x * vecRight +
			y * vecSpread.y * vecUp;
		Vector vecEnd;

		vecEnd = vecSrc + vecDir * flDistance;
		UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( pev )/*pentIgnore*/, &tr );

		// do damage, paint decals
		if( tr.flFraction != 1.0 )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

			if( iDamage )
			{
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, iDamage, DMG_BULLET | ( ( iDamage > 16 ) ? DMG_ALWAYSGIB : DMG_NEVERGIB ) ), vecDir, &tr );

				TEXTURETYPE_PlaySound( &tr, vecSrc, vecEnd, iBulletType );
				DecalGunshot( &tr, iBulletType );
			}
			else switch( iBulletType )
			{
			default:
			case BULLET_PLAYER_9MM:
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, gSkillData.plrDmg9MM, DMG_BULLET ), vecDir, &tr );
				break;

			case BULLET_PLAYER_MP5:
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, gSkillData.plrDmgMP5, DMG_BULLET ), vecDir, &tr );
				break;

			case BULLET_PLAYER_BUCKSHOT:
				// make distance based!
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, gSkillData.plrDmgBuckshot, DMG_BULLET ), vecDir, &tr );
				break;

			case BULLET_PLAYER_357:
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, gSkillData.plrDmg357, DMG_BULLET ), vecDir, &tr );
				break;

			case BULLET_NONE: // FIX 
				pEntity->TraceAttack( CTakeDamageInfo( pAttacker, 50, DMG_CLUB ), vecDir, &tr );
				TEXTURETYPE_PlaySound( &tr, vecSrc, vecEnd, iBulletType );
				// only decal glass
				if( !FNullEnt( tr.pHit ) && GET_PRIVATE( tr.pHit )->GetRenderMode() != kRenderNormal )
				{
					UTIL_DecalTrace( &tr, DECAL_GLASSBREAK1 + RANDOM_LONG( 0, 2 ) );
				}

				break;
			}
		}
		// make bullet trails
		UTIL_BubbleTrail( vecSrc, tr.vecEndPos, ( flDistance * tr.flFraction ) / 64.0 );
	}
	g_MultiDamage.ApplyMultiDamage( this, pAttacker );

	return Vector( x * vecSpread.x, y * vecSpread.y, 0.0 );
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target
//=========================================================
bool CBaseEntity::FVisible( const CBaseEntity *pEntity ) const
{
	TraceResult tr;
	Vector		vecLookerOrigin;
	Vector		vecTargetOrigin;

	if( FBitSet( pEntity->pev->flags, FL_NOTARGET ) )
		return false;

	// don't look through water
	if( ( GetWaterLevel() != WATERLEVEL_HEAD && pEntity->GetWaterLevel() == WATERLEVEL_HEAD )
		|| ( GetWaterLevel() == WATERLEVEL_HEAD && pEntity->GetWaterLevel() == WATERLEVEL_DRY ) )
		return false;

	vecLookerOrigin = GetAbsOrigin() + pev->view_ofs;//look through the caller's 'eyes'
	vecTargetOrigin = pEntity->EyePosition();

	UTIL_TraceLine( vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT( pev )/*pentIgnore*/, &tr );

	if( tr.flFraction != 1.0 )
	{
		return false;// Line of sight is not established
	}
	else
	{
		return true;// line of sight is valid.
	}
}

//=========================================================
// FVisible - returns true if a line can be traced from
// the caller's eyes to the target vector
//=========================================================
bool CBaseEntity::FVisible( const Vector &vecOrigin ) const
{
	TraceResult tr;
	Vector		vecLookerOrigin;

	vecLookerOrigin = EyePosition();//look through the caller's 'eyes'

	UTIL_TraceLine( vecLookerOrigin, vecOrigin, ignore_monsters, ignore_glass, ENT( pev )/*pentIgnore*/, &tr );

	if( tr.flFraction != 1.0 )
	{
		return false;// Line of sight is not established
	}
	else
	{
		return true;// line of sight is valid.
	}
}