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
#include "entities/NPCs/Monsters.h"
#include "Weapons.h"
#include "CPipewrench.h"
#include "nodes/Nodes.h"
#include "CBasePlayer.h"
#include "gamerules/GameRules.h"

#ifdef SERVER_DLL
BEGIN_DATADESC( CPipewrench )
	DEFINE_FIELD( m_flBigSwingStart, FIELD_TIME ),
	DEFINE_FIELD( m_iSwingMode, FIELD_INTEGER),
	DEFINE_THINKFUNC( SwingAgain ),
	DEFINE_THINKFUNC( Smack ),
END_DATADESC()
#endif

LINK_ENTITY_TO_CLASS( weapon_pipewrench, CPipewrench );

CPipewrench::CPipewrench()
	: BaseClass( WEAPON_PIPEWRENCH )
{
}

void CPipewrench::Spawn( )
{
	Precache( );
	SetModel( "models/w_pipe_wrench.mdl");
	m_iClip = -1;
	m_iSwingMode = 0;

	FallInit();// get ready to fall down.
}

void CPipewrench::Precache( void )
{
	BaseClass::Precache();

	PRECACHE_MODEL("models/v_pipe_wrench.mdl");
	PRECACHE_MODEL("models/w_pipe_wrench.mdl");
	PRECACHE_MODEL("models/p_pipe_wrench.mdl");
	// Shepard - The commented sounds below are unused
	// in Opposing Force, if you wish to use them,
	// uncomment all the appropriate lines.
	/*PRECACHE_SOUND("weapons/pwrench_big_hit1.wav");
	PRECACHE_SOUND("weapons/pwrench_big_hit2.wav");*/
	PRECACHE_SOUND("weapons/pwrench_big_hitbod1.wav");
	PRECACHE_SOUND("weapons/pwrench_big_hitbod2.wav");
	PRECACHE_SOUND("weapons/pwrench_big_miss.wav");
	PRECACHE_SOUND("weapons/pwrench_hit1.wav");
	PRECACHE_SOUND("weapons/pwrench_hit2.wav");
	PRECACHE_SOUND("weapons/pwrench_hitbod1.wav");
	PRECACHE_SOUND("weapons/pwrench_hitbod2.wav");
	PRECACHE_SOUND("weapons/pwrench_hitbod3.wav");
	PRECACHE_SOUND("weapons/pwrench_miss1.wav");
	PRECACHE_SOUND("weapons/pwrench_miss2.wav");

	m_usPipewrench = PRECACHE_EVENT ( 1, "events/pipewrench.sc" );
}

bool CPipewrench::Deploy()
{
	return DefaultDeploy( "models/v_pipe_wrench.mdl", "models/p_pipe_wrench.mdl", PIPEWRENCH_DRAW, "crowbar" );
}

void CPipewrench::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim( PIPEWRENCH_HOLSTER );
}

void CPipewrench::PrimaryAttack()
{
	if( !m_iSwingMode && !Swing( true ) )
	{
#ifndef CLIENT_DLL
		SetThink( &CPipewrench::SwingAgain );
		SetNextThink( gpGlobals->time + 0.1 );
#endif
	}
}

void CPipewrench::SecondaryAttack()
{
	if ( m_iSwingMode != 1 )
	{
		SendWeaponAnim( PIPEWRENCH_BIG_SWING_START );
		m_flBigSwingStart = gpGlobals->time;
		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = GetNextAttackDelay(1.0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
		m_iSwingMode = 1;
	}
	m_iSwingMode = 1;
}

void CPipewrench::Smack( )
{
	DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
}

void CPipewrench::SwingAgain( void )
{
	Swing( false );
}

bool CPipewrench::Swing( const bool bFirst )
{
	bool bDidHit = false;

	TraceResult tr;

	UTIL_MakeVectors( m_pPlayer->GetViewAngle() );
	Vector vecSrc	= m_pPlayer->GetGunPosition( );
	Vector vecEnd	= vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

#ifndef CLIENT_DLL
	if ( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, Hull::HEAD, ENT( m_pPlayer->pev ), &tr );
		if ( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer );
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	if( bFirst )
	{
		PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usPipewrench, 
							 0.0, g_vecZero, g_vecZero, 0, 0, 0,
							 0.0, 0, 0.0 );
	}


	if ( tr.flFraction >= 1.0 )
	{
		if( bFirst )
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(0.75);
			m_flNextSecondaryAttack = GetNextAttackDelay(0.75);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;

			// Shepard - In Opposing Force, the "miss" sound is
			// played twice (maybe it's a mistake from Gearbox or
			// an intended feature), if you only want a single
			// sound, comment this "switch" or the one in the
			// event (EV_Pipewrench)
			switch ( ((m_iSwing++) % 1) )
			{
			case 0: EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/pwrench_miss1.wav", 1, ATTN_NORM); break;
			case 1: EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/pwrench_miss2.wav", 1, ATTN_NORM); break;
			}

			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		switch( ((m_iSwing++) % 2) + 1 )
		{
		case 0:
			SendWeaponAnim( PIPEWRENCH_ATTACK1HIT ); break;
		case 1:
			SendWeaponAnim( PIPEWRENCH_ATTACK2HIT ); break;
		case 2:
			SendWeaponAnim( PIPEWRENCH_ATTACK3HIT ); break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		
#ifndef CLIENT_DLL

		// hit
		bDidHit = true;
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		if( pEntity )
		{
			g_MultiDamage.Clear( );

			if ( (m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase() ) || g_pGameRules->IsMultiplayer() )
			{
				// first swing does full damage
				pEntity->TraceAttack( CTakeDamageInfo( m_pPlayer, gSkillData.GetPlrDmgPipewrench(), DMG_CLUB ), gpGlobals->v_forward, tr );
			}
			else
			{
				// subsequent swings do half
				pEntity->TraceAttack( CTakeDamageInfo( m_pPlayer, gSkillData.GetPlrDmgPipewrench() / 2, DMG_CLUB ), gpGlobals->v_forward, tr );
			}	
			g_MultiDamage.ApplyMultiDamage( m_pPlayer, m_pPlayer );
		}

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool bHitWorld = true;

		if (pEntity)
		{
			if ( pEntity->Classify() != EntityClassifications().GetNoneId() && pEntity->Classify() != EntityClassifications().GetClassificationId( classify::MACHINE ) )
			{
				// play thwack or smack sound
				switch( RANDOM_LONG(0,2) )
				{
				case 0:
					EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/pwrench_hitbod1.wav", 1, ATTN_NORM); break;
				case 1:
					EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/pwrench_hitbod2.wav", 1, ATTN_NORM); break;
				case 2:
					EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/pwrench_hitbod3.wav", 1, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = MELEE_BODYHIT_VOLUME;
				if ( !pEntity->IsAlive() )
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
			float fvolbar = TEXTURETYPE_PlaySound(tr, vecSrc, vecSrc + (vecEnd-vecSrc)*2, BULLET_PLAYER_CROWBAR );

			if ( g_pGameRules->IsMultiplayer() )
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play pipe wrench strike
			switch( RANDOM_LONG(0,1) )
			{
			case 0:
				EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/pwrench_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			case 1:
				EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/pwrench_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * MELEE_WALLHIT_VOLUME;

		SetThink( &CPipewrench::Smack );
		SetNextThink( UTIL_WeaponTimeBase() + 0.2 );
#endif
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flNextSecondaryAttack = GetNextAttackDelay(0.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;	
	}
	return bDidHit;
}

void CPipewrench::BigSwing()
{
	TraceResult tr;

	UTIL_MakeVectors( m_pPlayer->GetViewAngle() );
	Vector vecSrc	= m_pPlayer->GetGunPosition( );
	Vector vecEnd	= vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, ENT( m_pPlayer->pev ), &tr );

#ifndef CLIENT_DLL
	if ( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, Hull::HEAD, ENT( m_pPlayer->pev ), &tr );
		if ( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer );
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}
#endif

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, m_pPlayer->edict(), m_usPipewrench, 
	0.0, g_vecZero, g_vecZero, 0, 0, 0,
	0.0, 1, 0.0 );


	if ( tr.flFraction >= 1.0 )
	{
		// miss
		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = GetNextAttackDelay(1.0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
	}
	else
	{
		SendWeaponAnim( PIPEWRENCH_BIG_SWING_HIT );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		
#ifndef CLIENT_DLL

		// hit
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		if( pEntity )
		{
			g_MultiDamage.Clear( );

			float flDamage = (gpGlobals->time - m_flBigSwingStart) * gSkillData.GetPlrDmgPipewrench() + 25.0f;
			if ( (m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase() ) || g_pGameRules->IsMultiplayer() )
			{
				// first swing does full damage
				pEntity->TraceAttack( CTakeDamageInfo( m_pPlayer, flDamage, DMG_CLUB ), gpGlobals->v_forward, tr ); 
			}
			else
			{
				// subsequent swings do half
				pEntity->TraceAttack( CTakeDamageInfo( m_pPlayer, flDamage / 2, DMG_CLUB ), gpGlobals->v_forward, tr ); 
			}	
			g_MultiDamage.ApplyMultiDamage( m_pPlayer, m_pPlayer );
		}

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool bHitWorld = true;

		if (pEntity)
		{
			if ( pEntity->Classify() != EntityClassifications().GetNoneId() && pEntity->Classify() != EntityClassifications().GetClassificationId( classify::MACHINE ) )
			{
				// play thwack or smack sound
				switch( RANDOM_LONG(0,1) )
				{
				case 0:
					EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/pwrench_big_hitbod1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND( m_pPlayer, CHAN_ITEM, "weapons/pwrench_big_hitbod2.wav", 1, ATTN_NORM);
					break;
				}
				m_pPlayer->m_iWeaponVolume = MELEE_BODYHIT_VOLUME;
				if ( !pEntity->IsAlive() )
					  return;
				else
					  flVol = 0.1;

				bHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if( bHitWorld )
		{
			float fvolbar = TEXTURETYPE_PlaySound(tr, vecSrc, vecSrc + (vecEnd-vecSrc)*2, BULLET_PLAYER_CROWBAR );

			if ( g_pGameRules->IsMultiplayer() )
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play pipe wrench strike
			// Shepard - The commented sounds below are unused
			// in Opposing Force, if you wish to use them,
			// uncomment all the appropriate lines.
			switch( RANDOM_LONG(0,1) )
			{
			case 0:
				EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/pwrench_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3));
				//EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/pwrench_big_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			case 1:
				EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/pwrench_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3));
				//EMIT_SOUND_DYN( m_pPlayer, CHAN_ITEM, "weapons/pwrench_big_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * MELEE_WALLHIT_VOLUME;

		// Shepard - The original Opposing Force's pipe wrench
		// doesn't make a bullet hole decal when making a big
		// swing. If you want that decal, just uncomment the
		// 2 lines below.
		/*SetThink( &CPipewrench::Smack );
		SetNextThink( UTIL_WeaponTimeBase() + 0.2 );*/
#endif
		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = GetNextAttackDelay(1.0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	}
}

void CPipewrench::WeaponIdle()
{
	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_iSwingMode > 0 )
	{
		if ( m_iSwingMode == 1 )
		{
			BigSwing();
			m_iSwingMode = 2;
		}
		else
			m_iSwingMode = 0;
	}

	if ( !m_iSwingMode )
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

		if ( flRand <= 0.3 + 0 * 0.75 )
		{
			iAnim = PIPEWRENCH_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
		}
		else if ( flRand <= 0.6 + 0 * 0.875 )
		{
			iAnim = PIPEWRENCH_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}
		else
		{
			iAnim = PIPEWRENCH_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}
		SendWeaponAnim( iAnim );
	}
}

void CPipewrench::GetWeaponData( weapon_data_t& data )
{
	BaseClass::GetWeaponData( data );

	data.m_fInSpecialReload = static_cast<int>( m_iSwingMode );
}

void CPipewrench::SetWeaponData( const weapon_data_t& data )
{
	BaseClass::SetWeaponData( data );

	m_iSwingMode = data.m_fInSpecialReload;
}

#endif // GAME_OPFOR
