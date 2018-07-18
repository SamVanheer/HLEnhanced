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
#include "CBasePlayer.h"
#include "Weapons.h"
#include "entities/CSoundEnt.h"
#include "entities/plats/CFuncTrackTrain.h"
#include "entities/CSprayCan.h"
#include "entities/effects/CGib.h"
#include "entities/CBloodSplat.h"
#include "entities/rope/CRope.h"

#include "nodes/Nodes.h"

#include "gamerules/GameRules.h"
#include "Server.h"

#include "com_model.h"

#if USE_ANGELSCRIPT
#include "Angelscript/ScriptAPI/ASEvents.h"
#endif

extern DLL_GLOBAL bool			g_fGameOver;
extern DLL_GLOBAL unsigned int	g_ulModelIndexPlayer;
extern DLL_GLOBAL bool			gEvilImpulse101;

bool g_bPrecacheGrunt = false;

/*
================
CheckPowerups

Check for turning off powerups

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
================
*/
static void CheckPowerups( CBaseEntity* pEntity )
{
	if( pEntity->GetHealth() <= 0 )
		return;

	pEntity->SetModelIndex( g_ulModelIndexPlayer );    // don't use eyes
}

void CBasePlayer::PreThink()
{
	const int buttonsChanged = ( m_afButtonLast ^ GetButtons().Get() );	// These buttons have changed this frame

	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed = buttonsChanged & GetButtons().Get();		// The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & ( ~GetButtons().Get() );	// The ones not down are "released"

	g_pGameRules->PlayerThink( this );

	bool bCheckVehicles = true;

#if USE_ANGELSCRIPT
	uint32_t uiFlags = PreThinkFlag::NONE;

	CallGlobalEvent( g_PlayerPreThinkEvent, CallFlag::NONE, this, &uiFlags );

	bCheckVehicles = !( uiFlags & PreThinkFlag::SKIP_VEHICLES );
#endif

	if( g_fGameOver )
		return;         // intermission or finale

	UTIL_MakeVectors( GetViewAngle() );             // is this still used?

	ItemPreFrame();
	WaterMove();

	if( g_pGameRules && g_pGameRules->FAllowFlashlight() )
		m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
	else
		m_iHideHUD |= HIDEHUD_FLASHLIGHT;


	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();

	CheckTimeBasedDamage();

	CheckSuitUpdate();

	// Observer Button Handling
	if( IsObserver() )
	{
		Observer_HandleButtons();
		Observer_CheckTarget();
		Observer_CheckProperties();
		SetImpulse( 0 );
		return;
	}

	if( GetDeadFlag() >= DEAD_DYING )
	{
		PlayerDeathThink();
		return;
	}

	// So the correct flags get sent to client asap.
	//
	if( m_afPhysicsFlags & PFLAG_ONTRAIN )
		GetFlags() |= FL_ONTRAIN;
	else
		GetFlags().ClearFlags( FL_ONTRAIN );

	if( bCheckVehicles )
	{
#ifdef GAME_OPFOR
	//We're on a rope. - Solokiller
	if( m_afPhysicsFlags & PFLAG_ONROPE && m_pRope )
	{
		SetAbsVelocity( g_vecZero );

		const Vector vecAttachPos = m_pRope->GetAttachedObjectsPosition();

		SetAbsOrigin( vecAttachPos );

		Vector vecForce;

		/*
		//TODO: This causes sideways acceleration that doesn't occur in Op4. - Solokiller
		//TODO: should be IN_MOVERIGHT and IN_MOVELEFT - Solokiller
		if( GetButtons().Any( IN_DUCK ) )
		{
			vecForce.x = gpGlobals->v_right.x;
			vecForce.y = gpGlobals->v_right.y;
			vecForce.z = 0;
			
			m_pRope->ApplyForceFromPlayer( vecForce );
		}

		if( GetButtons().Any( IN_JUMP ) )
		{
			vecForce.x = -gpGlobals->v_right.x;
			vecForce.y = -gpGlobals->v_right.y;
			vecForce.z = 0;
			m_pRope->ApplyForceFromPlayer( vecForce );
		}
		*/

		//Determine if any force should be applied to the rope, or if we should move around. - Solokiller
		if( GetButtons().Any( IN_BACK | IN_FORWARD ) )
		{
			if( ( gpGlobals->v_forward.x * gpGlobals->v_forward.x + 
				gpGlobals->v_forward.y * gpGlobals->v_forward.y - 
				gpGlobals->v_forward.z * gpGlobals->v_forward.z ) <= 0.0 )
			{
				if( m_bIsClimbing )
				{
					const float flDelta = gpGlobals->time - m_flLastClimbTime;
					m_flLastClimbTime = gpGlobals->time;
					if( GetButtons().Any( IN_FORWARD ) )
					{
						if( gpGlobals->v_forward.z < 0.0 )
						{
							if( !m_pRope->MoveDown( flDelta ) )
							{
								//Let go of the rope, detach. - Solokiller
								SetMoveType( MOVETYPE_WALK );
								SetSolidType( SOLID_SLIDEBOX );

								m_afPhysicsFlags &= ~PFLAG_ONROPE;
								m_pRope->DetachObject();
								m_pRope = nullptr;
								m_bIsClimbing = false;
							}
						}
						else
						{
							m_pRope->MoveUp( flDelta );
						}
					}
					if( GetButtons().Any( IN_BACK ) )
					{
						if( gpGlobals->v_forward.z < 0.0 )
						{
							m_pRope->MoveUp( flDelta );
						}
						else if( !m_pRope->MoveDown( flDelta ) )
						{
							//Let go of the rope, detach. - Solokiller
							SetMoveType( MOVETYPE_WALK );
							SetSolidType( SOLID_SLIDEBOX );
							m_afPhysicsFlags &= ~PFLAG_ONROPE;
							m_pRope->DetachObject();
							m_pRope = nullptr;
							m_bIsClimbing = false;
						}
					}
				}
				else
				{
					m_bIsClimbing = true;
					m_flLastClimbTime = gpGlobals->time;
				}
			}
			else
			{
				vecForce.x = gpGlobals->v_forward.x;
				vecForce.y = gpGlobals->v_forward.y;
				vecForce.z = 0.0;
				if( GetButtons().Any( IN_BACK ) )
				{
					vecForce.x = -gpGlobals->v_forward.x;
					vecForce.y = -gpGlobals->v_forward.y;
					vecForce.z = 0;
				}
				m_pRope->ApplyForceFromPlayer( vecForce );
				m_bIsClimbing = false;
			}
		}
		else
		{
			m_bIsClimbing = false;
		}

		if( m_afButtonPressed & IN_JUMP )
		{
			//We've jumped off the rope, give us some momentum - Solokiller
			SetMoveType( MOVETYPE_WALK );
			SetSolidType( SOLID_SLIDEBOX );
			this->m_afPhysicsFlags &= ~PFLAG_ONROPE;

			Vector vecDir = gpGlobals->v_up * 165.0 + gpGlobals->v_forward * 150.0;

			Vector vecVelocity = m_pRope->GetAttachedObjectsVelocity() * 2;

			vecVelocity.NormalizeInPlace();

			vecVelocity = vecVelocity * 200;

			SetAbsVelocity( vecVelocity + vecDir );

			m_pRope->DetachObject();
			m_pRope = nullptr;
			m_bIsClimbing = false;
		}
		return;
	}
#endif

		// Train speed control
		if( m_afPhysicsFlags & PFLAG_ONTRAIN )
		{
			CBaseEntity *pTrain = GetGroundEntity();

			//To match original behavior, Instance returns the world if entity is null - Solokiller
			if( !pTrain )
				pTrain = CWorld::GetInstance();

			float vel;

			if( !pTrain )
			{
				TraceResult trainTrace;
				// Maybe this is on the other side of a level transition
				UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -38 ), ignore_monsters, ENT( pev ), &trainTrace );

				// HACKHACK - Just look for the func_tracktrain classname
				if( trainTrace.flFraction != 1.0 && trainTrace.pHit )
					pTrain = CBaseEntity::Instance( trainTrace.pHit );


				if( !pTrain || !( pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE ) || !pTrain->OnControls( this ) )
				{
					//ALERT( at_error, "In train mode with no train!\n" );
					m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
					m_iTrain = TRAIN_NEW | TRAIN_OFF;
					return;
				}
			}
			else if( !GetFlags().Any( FL_ONGROUND ) || pTrain->GetSpawnFlags().Any( SF_TRACKTRAIN_NOCONTROL ) || ( GetButtons().Any( IN_MOVELEFT | IN_MOVERIGHT ) ) )
			{
				// Turn off the train if you jump, strafe, or the train controls go dead
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW | TRAIN_OFF;
				return;
			}

			SetAbsVelocity( g_vecZero );
			vel = 0;
			if( m_afButtonPressed & IN_FORWARD )
			{
				vel = 1;
				pTrain->Use( this, this, USE_SET, ( float ) vel );
			}
			else if( m_afButtonPressed & IN_BACK )
			{
				vel = -1;
				pTrain->Use( this, this, USE_SET, ( float ) vel );
			}

			if( vel )
			{
				m_iTrain = TrainSpeed( pTrain->GetSpeed(), pTrain->GetImpulse() );
				m_iTrain |= TRAIN_ACTIVE | TRAIN_NEW;
			}

		}
		else if( m_iTrain & TRAIN_ACTIVE )
			m_iTrain = TRAIN_NEW; // turn off train
	}

	if( GetButtons().Any( IN_JUMP ) )
	{
		// If on a ladder, jump off the ladder
		// else Jump
		Jump();
	}


	// If trying to duck, already ducked, or in the process of ducking
	if( GetButtons().Any( IN_DUCK ) || GetFlags().Any( FL_DUCKING ) || ( m_afPhysicsFlags & PFLAG_DUCKING ) )
		Duck();

	if( !GetFlags().Any( FL_ONGROUND ) )
	{
		m_flFallVelocity = -GetAbsVelocity().z;
	}

	// StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	// Clear out ladder pointer
	m_hEnemy = NULL;

	if( m_afPhysicsFlags & PFLAG_ONBARNACLE )
	{
		SetAbsVelocity( g_vecZero );
	}
}

void CBasePlayer::PostThink()
{
	if( g_fGameOver )
		goto pt_end;         // intermission or finale

	if( !IsAlive() )
		goto pt_end;

	// Handle Tank controlling
	if( m_pTank != NULL )
	{
		// if they've moved too far from the gun,  or selected a weapon, unuse the gun
		if( m_pTank->OnControls( this ) && !HasWeaponModelName() )
		{
			m_pTank->Use( this, this, USE_SET, 2 );	// try fire the gun
		}
		else
		{  // they've moved off the platform
			m_pTank->Use( this, this, USE_OFF, 0 );
			m_pTank = NULL;
		}
	}

	// do weapon stuff
	ItemPostFrame();

	// check to see if player landed hard enough to make a sound
	// falling farther than half of the maximum safe distance, but not as far a max safe distance will
	// play a bootscrape sound, and no damage will be inflicted. Fallling a distance shorter than half
	// of maximum safe distance will make no sound. Falling farther than max safe distance will play a 
	// fallpain sound, and damage will be inflicted based on how far the player fell

	if( GetFlags().Any( FL_ONGROUND ) && ( GetHealth() > 0 ) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
	{
		// ALERT ( at_console, "%f\n", m_flFallVelocity );

		if( GetWaterType() == CONTENTS_WATER )
		{
			// Did he hit the world or a non-moving entity?
			// BUG - this happens all the time in water, especially when 
			// BUG - water has current force
			//CBaseEntity* pEntity = GetGroundEntity();
			//if ( !pEntity || pEntity->GetAbsVelocity().z == 0 )
			//	EMIT_SOUND( this, CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM);
		}
		else if( m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
		{
			// after this point, we start doing damage

			float flFallDamage = g_pGameRules->FlPlayerFallDamage( this );

			if( flFallDamage > GetHealth() )
			{//splat
			 // note: play on item channel because we play footstep landing on body channel
				EMIT_SOUND( this, CHAN_ITEM, "common/bodysplat.wav", 1, ATTN_NORM );
			}

			if( flFallDamage > 0 )
			{
				TakeDamage( CWorld::GetInstance(), CWorld::GetInstance(), flFallDamage, DMG_FALL );
				Vector vecPunchAngle = GetPunchAngle();
				vecPunchAngle.x = 0;
				SetPunchAngle( vecPunchAngle );
			}
		}

		if( IsAlive() )
		{
			SetAnimation( PLAYER_WALK );
		}
	}

	if( GetFlags().Any( FL_ONGROUND ) )
	{
		if( m_flFallVelocity > 64 && !g_pGameRules->IsMultiplayer() )
		{
			CSoundEnt::InsertSound( bits_SOUND_PLAYER, GetAbsOrigin(), m_flFallVelocity, 0.2 );
			// ALERT( at_console, "fall %f\n", m_flFallVelocity );
		}
		m_flFallVelocity = 0;
	}

	// select the proper animation for the player character	
	if( IsAlive() )
	{
		if( !GetAbsVelocity().x && !GetAbsVelocity().y )
			SetAnimation( PLAYER_IDLE );
		else if( ( GetAbsVelocity().x || GetAbsVelocity().y ) && ( GetFlags().Any( FL_ONGROUND ) ) )
			SetAnimation( PLAYER_WALK );
		else if( GetWaterLevel() > WATERLEVEL_FEET )
			SetAnimation( PLAYER_WALK );
	}

	StudioFrameAdvance();
	CheckPowerups( this );

	UpdatePlayerSound();

pt_end:
#if defined( CLIENT_WEAPONS )
	// Decay timers on weapons
	// go through all of the weapons and make a list of the ones to pack
	for( int i = 0; i < MAX_WEAPON_SLOTS; i++ )
	{
		if( m_rgpPlayerItems[ i ] )
		{
			CBasePlayerWeapon *pPlayerItem = m_rgpPlayerItems[ i ];

			while( pPlayerItem )
			{
				if( pPlayerItem->IsPredicted() )
				{
					pPlayerItem->m_flNextPrimaryAttack = max( pPlayerItem->m_flNextPrimaryAttack - gpGlobals->frametime, -1.0f );
					pPlayerItem->m_flNextSecondaryAttack = max( pPlayerItem->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001f );

					if( pPlayerItem->m_flTimeWeaponIdle != 1000 )
					{
						pPlayerItem->m_flTimeWeaponIdle = max( pPlayerItem->m_flTimeWeaponIdle - gpGlobals->frametime, -0.001f );
					}

					if( pPlayerItem->pev->fuser1 != 1000 )
					{
						pPlayerItem->pev->fuser1 = max( pPlayerItem->pev->fuser1 - gpGlobals->frametime, -0.001f );
					}

					pPlayerItem->DecrementTimers( gpGlobals->frametime );

					// Only decrement if not flagged as NO_DECREMENT
					//					if ( gun->m_flPumpTime != 1000 )
					//	{
					//		gun->m_flPumpTime	= max( gun->m_flPumpTime - gpGlobals->frametime, -0.001 );
					//	}

				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}

	m_flNextAttack -= gpGlobals->frametime;
	if( m_flNextAttack < -0.001 )
		m_flNextAttack = -0.001;

	if( m_flNextAmmoBurn != 1000 )
	{
		m_flNextAmmoBurn -= gpGlobals->frametime;

		if( m_flNextAmmoBurn < -0.001 )
			m_flNextAmmoBurn = -0.001;
	}

	if( m_flAmmoStartCharge != 1000 )
	{
		m_flAmmoStartCharge -= gpGlobals->frametime;

		if( m_flAmmoStartCharge < -0.001 )
			m_flAmmoStartCharge = -0.001;
	}
#endif

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = GetButtons().Get();
}

void CBasePlayer::PlayerDeathThink()
{
	if( GetFlags().Any( FL_ONGROUND ) )
	{
		const float flForward = GetAbsVelocity().Length() - 20;
		if( flForward <= 0 )
			SetAbsVelocity( g_vecZero );
		else
			SetAbsVelocity( flForward * GetAbsVelocity().Normalize() );
	}

	if( HasWeapons() )
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}


	if( GetModelIndex() && ( !m_fSequenceFinished ) && ( GetDeadFlag() == DEAD_DYING ) )
	{
		StudioFrameAdvance();

		m_iRespawnFrames++;				// Note, these aren't necessarily real "frames", so behavior is dependent on # of client movement commands
		if( m_iRespawnFrames < 120 )   // Animations should be no longer than this
			return;
	}

	// once we're done animating our death and we're on the ground, we want to set movetype to None so our dead body won't do collisions and stuff anymore
	// this prevents a bug where the dead body would go to a player's head if he walked over it while the dead player was clicking their button to respawn
	if( GetMoveType() != MOVETYPE_NONE && GetFlags().Any( FL_ONGROUND ) )
		SetMoveType( MOVETYPE_NONE );

	if( GetDeadFlag() == DEAD_DYING )
		SetDeadFlag( DEAD_DEAD );

	StopAnimation();

	GetEffects() |= EF_NOINTERP;
	SetFrameRate( 0.0 );

	const bool fAnyButtonDown = ( GetButtons().Any( ~IN_SCORE ) ) != 0;

	// wait for all buttons released
	if( GetDeadFlag() == DEAD_DEAD )
	{
		if( fAnyButtonDown )
			return;

		if( g_pGameRules->FPlayerCanRespawn( this ) )
		{
			m_fDeadTime = gpGlobals->time;
			SetDeadFlag( DEAD_RESPAWNABLE );
		}

		return;
	}

	// if the player has been dead for one second longer than allowed by forcerespawn, 
	// forcerespawn isn't on. Send the player off to an intermission camera until they 
	// choose to respawn.
	if( g_pGameRules->IsMultiplayer() && ( gpGlobals->time > ( m_fDeadTime + 6 ) ) && !( m_afPhysicsFlags & PFLAG_OBSERVER ) )
	{
		// go to dead camera. 
		StartDeathCam();
	}

	if( IsObserver() )	// player is in spectator mode
		return;

	// wait for any button down,  or mp_forcerespawn is set and the respawn time is up
	if( !fAnyButtonDown
		&& !( g_pGameRules->IsMultiplayer() && forcerespawn.value > 0 && ( gpGlobals->time > ( m_fDeadTime + 5 ) ) ) )
		return;

	GetButtons().ClearAll();
	m_iRespawnFrames = 0;

	//ALERT(at_console, "Respawn\n");

	g_pGameRules->PlayerRespawn( this, !( m_afPhysicsFlags & PFLAG_OBSERVER ) );// don't copy a corpse if we're in deathcam.
	SetNextThink( -1 );
}

//=========================================================
// UpdatePlayerSound - updates the position of the player's
// reserved sound slot in the sound list.
//=========================================================
void CBasePlayer::UpdatePlayerSound()
{
	int iBodyVolume;
	int iVolume;
	CSound *pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( this ) );

	if( !pSound )
	{
		ALERT( at_console, "Client lost reserved sound!\n" );
		return;
	}

	pSound->m_iType = bits_SOUND_NONE;

	// now calculate the best target volume for the sound. If the player's weapon
	// is louder than his body/movement, use the weapon volume, else, use the body volume.

	if( GetFlags().Any( FL_ONGROUND ) )
	{
		iBodyVolume = GetAbsVelocity().Length();

		// clamp the noise that can be made by the body, in case a push trigger,
		// weapon recoil, or anything shoves the player abnormally fast. 
		if( iBodyVolume > 512 )
		{
			iBodyVolume = 512;
		}
	}
	else
	{
		iBodyVolume = 0;
	}

	if( GetButtons().Any( IN_JUMP ) )
	{
		iBodyVolume += 100;
	}

	// convert player move speed and actions into sound audible by monsters.
	if( m_iWeaponVolume > iBodyVolume )
	{
		m_iTargetVolume = m_iWeaponVolume;

		// OR in the bits for COMBAT sound if the weapon is being louder than the player. 
		pSound->m_iType |= bits_SOUND_COMBAT;
	}
	else
	{
		m_iTargetVolume = iBodyVolume;
	}

	// decay weapon volume over time so bits_SOUND_COMBAT stays set for a while
	m_iWeaponVolume -= 250 * gpGlobals->frametime;

	// if target volume is greater than the player sound's current volume, we paste the new volume in 
	// immediately. If target is less than the current volume, current volume is not set immediately to the
	// lower volume, rather works itself towards target volume over time. This gives monsters a much better chance
	// to hear a sound, especially if they don't listen every frame.
	iVolume = pSound->m_iVolume;

	if( m_iTargetVolume > iVolume )
	{
		iVolume = m_iTargetVolume;
	}
	else if( iVolume > m_iTargetVolume )
	{
		iVolume -= 250 * gpGlobals->frametime;

		if( iVolume < m_iTargetVolume )
		{
			iVolume = 0;
		}
	}

	if( m_fNoPlayerSound )
	{
		// debugging flag, lets players move around and shoot without monsters hearing.
		iVolume = 0;
	}

	if( gpGlobals->time > m_flStopExtraSoundTime )
	{
		// since the extra sound that a weapon emits only lasts for one client frame, we keep that sound around for a server frame or two 
		// after actual emission to make sure it gets heard.
		m_iExtraSoundTypes = 0;
	}

	if( pSound )
	{
		pSound->m_vecOrigin = GetAbsOrigin();
		pSound->m_iType |= ( bits_SOUND_PLAYER | m_iExtraSoundTypes );
		pSound->m_iVolume = iVolume;
	}

	// keep track of virtual muzzle flash
	m_iWeaponFlash -= 256 * gpGlobals->frametime;
	if( m_iWeaponFlash < 0 )
		m_iWeaponFlash = 0;

	//UTIL_MakeVectors ( GetAbsAngles() );
	//gpGlobals->v_forward.z = 0;

	// Below are a couple of useful little bits that make it easier to determine just how much noise the 
	// player is making. 
	// UTIL_ParticleEffect ( GetAbsOrigin() + gpGlobals->v_forward * iVolume, g_vecZero, 255, 25 );
	//ALERT ( at_console, "%d/%d\n", iVolume, m_iTargetVolume );
}

/*
===========
WaterMove
============
*/
void CBasePlayer::WaterMove()
{
	if( GetMoveType() == MOVETYPE_NOCLIP )
		return;

	if( GetHealth() < 0 )
		return;

	if( GetWaterLevel() != WATERLEVEL_HEAD )
	{
		// not underwater

		// play 'up for air' sound
		if( pev->air_finished < gpGlobals->time )
			EMIT_SOUND( this, CHAN_VOICE, "player/pl_wade1.wav", 1, ATTN_NORM );
		else if( pev->air_finished < gpGlobals->time + 9 )
			EMIT_SOUND( this, CHAN_VOICE, "player/pl_wade2.wav", 1, ATTN_NORM );

		pev->air_finished = gpGlobals->time + PLAYER_SWIM_AIRTIME;
		SetDamage( 2 );

		// if we took drowning damage, give it back slowly
		if( m_idrowndmg > m_idrownrestored )
		{
			// set drowning damage bit.  hack - dmg_drownrecover actually
			// makes the time based damage code 'give back' health over time.
			// make sure counter is cleared so we start count correctly.

			// NOTE: this actually causes the count to continue restarting
			// until all drowning damage is healed.

			m_bitsDamageType |= DMG_DROWNRECOVER;
			m_bitsDamageType &= ~DMG_DROWN;
			m_rgbTimeBasedDamage[ itbd_DrownRecover ] = 0;
		}

	}
	else
	{	// fully under water
		// stop restoring damage while underwater
		m_bitsDamageType &= ~DMG_DROWNRECOVER;
		m_rgbTimeBasedDamage[ itbd_DrownRecover ] = 0;

		if( pev->air_finished < gpGlobals->time )		// drown!
		{
			if( pev->pain_finished < gpGlobals->time )
			{
				// take drowning damage
				SetDamage( GetDamage() + 1 );
				if( GetDamage() > 5 )
					SetDamage( 5 );
				TakeDamage( CWorld::GetInstance(), CWorld::GetInstance(), GetDamage(), DMG_DROWN );
				pev->pain_finished = gpGlobals->time + 1;

				// track drowning damage, give it back when
				// player finally takes a breath

				m_idrowndmg += GetDamage();
			}
		}
		else
		{
			m_bitsDamageType &= ~DMG_DROWN;
		}
	}

	if( !GetWaterLevel() )
	{
		if( GetFlags().Any( FL_INWATER ) )
		{
			GetFlags().ClearFlags( FL_INWATER );
		}
		return;
	}

	// make bubbles

	const int air = ( int ) ( pev->air_finished - gpGlobals->time );
	if( !RANDOM_LONG( 0, 0x1f ) && RANDOM_LONG( 0, PLAYER_SWIM_AIRTIME - 1 ) >= air )
	{
		switch( RANDOM_LONG( 0, 3 ) )
		{
		case 0:	EMIT_SOUND( this, CHAN_BODY, "player/pl_swim1.wav", 0.8, ATTN_NORM ); break;
		case 1:	EMIT_SOUND( this, CHAN_BODY, "player/pl_swim2.wav", 0.8, ATTN_NORM ); break;
		case 2:	EMIT_SOUND( this, CHAN_BODY, "player/pl_swim3.wav", 0.8, ATTN_NORM ); break;
		case 3:	EMIT_SOUND( this, CHAN_BODY, "player/pl_swim4.wav", 0.8, ATTN_NORM ); break;
		}
	}

	if( GetWaterType() == CONTENTS_LAVA )		// do damage
	{
		if( GetDamageTime() < gpGlobals->time )
			TakeDamage( CWorld::GetInstance(), CWorld::GetInstance(), 10 * GetWaterLevel(), DMG_BURN );
	}
	else if( GetWaterType() == CONTENTS_SLIME )		// do damage
	{
		SetDamageTime( gpGlobals->time + 1 );
		TakeDamage( CWorld::GetInstance(), CWorld::GetInstance(), 4 * GetWaterLevel(), DMG_ACID );
	}

	if( !GetFlags().Any( FL_INWATER ) )
	{
		GetFlags() |= FL_INWATER;
		SetDamageTime( 0 );
	}
}

/*
============
ImpulseCommands
============
*/
void CBasePlayer::ImpulseCommands()
{
	TraceResult	tr;// UNDONE: kill me! This is temporary for PreAlpha CDs

				   // Handle use events
	PlayerUse();

	int iImpulse = GetImpulse();
	switch( iImpulse )
	{
	case 99:
		{

			bool bOn;

			if( !gmsgLogo )
			{
				bOn = true;
				//TODO: figure out why this is done here. - Solokiller
				gmsgLogo = REG_USER_MSG( "Logo", 1 );
			}
			else
			{
				bOn = false;
			}

			ASSERT( gmsgLogo > 0 );
			// send "health" update message
			MESSAGE_BEGIN( MSG_ONE, gmsgLogo, NULL, this );
			WRITE_BYTE( bOn );
			MESSAGE_END();

			if( !bOn )
				gmsgLogo = 0;
			break;
		}
	case 100:
		// temporary flashlight for level designers
		if( FlashlightIsOn() )
		{
			FlashlightTurnOff();
		}
		else
		{
			FlashlightTurnOn();
		}
		break;

	case	201:// paint decal

		if( gpGlobals->time < m_flNextDecalTime )
		{
			// too early!
			break;
		}

		UTIL_MakeVectors( GetViewAngle() );
		UTIL_TraceLine( GetAbsOrigin() + GetViewOffset(), GetAbsOrigin() + GetViewOffset() + gpGlobals->v_forward * 128, ignore_monsters, ENT( pev ), &tr );

		if( tr.flFraction != 1.0 )
		{// line hit something, so paint a decal
			m_flNextDecalTime = gpGlobals->time + decalfrequency.value;
			auto pCan = static_cast<CSprayCan*>( UTIL_CreateNamedEntity( "spray_can" ) );
			pCan->CreateSprayCan( this );
		}

		break;

	default:
		// check all of the cheat impulse commands now
		CheatImpulseCommands( iImpulse );
		break;
	}

	SetImpulse( 0 );
}

//=========================================================
//=========================================================
void CBasePlayer::CheatImpulseCommands( int iImpulse )
{
	if( !UTIL_CheatsAllowed() )
	{
		return;
	}

	switch( iImpulse )
	{
	case 76:
		{
			if( !g_bPrecacheGrunt )
			{
				g_bPrecacheGrunt = true;
				ALERT( at_console, "You must now restart to use Grunt-o-matic.\n" );
			}
			else
			{
				UTIL_MakeVectors( Vector( 0, GetViewAngle().y, 0 ) );
				Create( "monster_human_grunt", GetAbsOrigin() + gpGlobals->v_forward * 128, GetAbsAngles() );
			}
			break;
		}

	case 101:
		{
			gEvilImpulse101 = true;
			GiveNamedItem( "item_suit" );
			GiveNamedItem( "item_battery" );
			GiveNamedItem( "weapon_crowbar" );
			GiveNamedItem( "weapon_9mmhandgun" );
			GiveNamedItem( "ammo_9mmclip" );
			GiveNamedItem( "weapon_shotgun" );
			GiveNamedItem( "ammo_buckshot" );
			GiveNamedItem( "weapon_9mmAR" );
			GiveNamedItem( "ammo_9mmAR" );
			GiveNamedItem( "ammo_ARgrenades" );
			GiveNamedItem( "weapon_handgrenade" );
			GiveNamedItem( "weapon_tripmine" );

			GiveNamedItem( "weapon_357" );
			GiveNamedItem( "ammo_357" );
			GiveNamedItem( "weapon_crossbow" );
			GiveNamedItem( "ammo_crossbow" );
			GiveNamedItem( "weapon_egon" );
			GiveNamedItem( "weapon_gauss" );
			GiveNamedItem( "ammo_gaussclip" );
			GiveNamedItem( "weapon_rpg" );
			GiveNamedItem( "ammo_rpgclip" );
			GiveNamedItem( "weapon_satchel" );
			GiveNamedItem( "weapon_snark" );
			GiveNamedItem( "weapon_hornetgun" );

#ifdef GAME_OPFOR
			GiveNamedItem( "weapon_knife" );
			GiveNamedItem( "weapon_pipewrench" );
			GiveNamedItem( "weapon_grapple" );
			GiveNamedItem( "weapon_eagle" );
			GiveNamedItem( "weapon_m249" );
			GiveNamedItem( "weapon_displacer" );
			GiveNamedItem( "weapon_sniperrifle" );
			GiveNamedItem( "weapon_sporelauncher" );
			GiveNamedItem( "weapon_shockrifle" );
			GiveNamedItem( "ammo_556" );
			GiveNamedItem( "ammo_762" );
#endif

			gEvilImpulse101 = false;
			break;
		}

	case 102:
		{
			// Gibbage!!!
			CGib::SpawnRandomGibs( this, 1, 1 );
			break;
		}

	case 103:
		{
			// What the hell are you doing?
			auto pEntity = UTIL_FindEntityForward( this );
			if( pEntity )
			{
				CBaseMonster *pMonster = pEntity->MyMonsterPointer();
				if( pMonster )
					pMonster->ReportAIState();
			}
			break;
		}

	case 104:
		{
			// Dump all of the global state varaibles (and global entity names)
			gGlobalState.DumpGlobals();
			break;
		}

	case	105:// player makes no sound for monsters to hear.
		{
			if( m_fNoPlayerSound )
			{
				ALERT( at_console, "Player is audible\n" );
				m_fNoPlayerSound = false;
			}
			else
			{
				ALERT( at_console, "Player is silent\n" );
				m_fNoPlayerSound = true;
			}
			break;
		}

	case 106:
		{
			// Give me the classname and targetname of this entity.
			auto pEntity = UTIL_FindEntityForward( this );
			if( pEntity )
			{
				ALERT( at_console, "Classname: %s", pEntity->GetClassname() );

				if( pEntity->HasTargetname() )
				{
					ALERT( at_console, " - Targetname: %s\n", pEntity->GetTargetname() );
				}
				else
				{
					ALERT( at_console, " - TargetName: No Targetname\n" );
				}

				ALERT( at_console, "Model: %s\n", pEntity->GetModelName() );
				if( HasGlobalName() )
					ALERT( at_console, "Globalname: %s\n", pEntity->GetGlobalName() );
			}

			break;
		}

	case 107:
		{
			TraceResult tr;

			CBaseEntity* pWorld = CWorld::GetInstance();

			Vector start = GetAbsOrigin() + GetViewOffset();
			Vector end = start + gpGlobals->v_forward * 1024;
			UTIL_TraceLine( start, end, ignore_monsters, edict(), &tr );
			if( tr.pHit )
				pWorld = CBaseEntity::Instance( tr.pHit );
			const texture_t* pTexture = UTIL_TraceTexture( pWorld, start, end );
			if( pTexture )
				ALERT( at_console, "Texture: %s\n", pTexture->name );
			break;
		}

	case	195:// show shortest paths for entire level to nearest node
		{
			Create( "node_viewer_fly", GetAbsOrigin(), GetAbsAngles() );
			break;
		}

	case	196:// show shortest paths for entire level to nearest node
		{
			Create( "node_viewer_large", GetAbsOrigin(), GetAbsAngles() );
			break;
		}

	case	197:// show shortest paths for entire level to nearest node
		{
			Create( "node_viewer_human", GetAbsOrigin(), GetAbsAngles() );
			break;
		}

	case	199:// show nearest node and all connections
		{
			ALERT( at_console, "%d\n", WorldGraph.FindNearestNode( GetAbsOrigin(), bits_NODE_GROUP_REALM ) );
			WorldGraph.ShowNodeConnections( WorldGraph.FindNearestNode( GetAbsOrigin(), bits_NODE_GROUP_REALM ) );

			break;
		}
		
	case	202:// Random blood splatter
		{
			UTIL_MakeVectors( GetViewAngle() );
			TraceResult tr;
			UTIL_TraceLine( GetAbsOrigin() + GetViewOffset(), GetAbsOrigin() + GetViewOffset() + gpGlobals->v_forward * 128, ignore_monsters, ENT( pev ), &tr );

			if( tr.flFraction != 1.0 )
			{// line hit something, so paint a decal
				auto pBlood = static_cast< CBloodSplat* >( UTIL_CreateNamedEntity( "blood_splat" ) );
				pBlood->CreateSplat( this );
			}
			break;
		}

	case	203:// remove creature.
		{
			auto pEntity = UTIL_FindEntityForward( this );
			if( pEntity )
			{
				if( pEntity->GetTakeDamageMode() != DAMAGE_NO )
					pEntity->SetThink( &CBaseEntity::SUB_Remove );
			}
			break;
		}
	}
}

/*
================
CheckSuitUpdate

Play suit update if it's time
================
*/
void CBasePlayer::CheckSuitUpdate()
{
	int isentence = 0;
	int isearch = m_iSuitPlayNext;

	//TODO: remove direct references to this flag, use members for checking/setting - Solokiller
	// Ignore suit updates if no suit
	if( !( GetWeapons().Any( 1 << WEAPON_SUIT ) ) )
		return;

	// if in range of radiation source, ping geiger counter
	UpdateGeigerCounter();

	if( g_pGameRules->IsMultiplayer() )
	{
		// don't bother updating HEV voice in multiplayer.
		return;
	}

	if( gpGlobals->time >= m_flSuitUpdate && m_flSuitUpdate > 0 )
	{
		// play a sentence off of the end of the queue
		for( int i = 0; i < CSUITPLAYLIST; ++i )
		{
			if( ( isentence = m_rgSuitPlayList[ isearch ] ) != 0 )
				break;

			if( ++isearch == CSUITPLAYLIST )
				isearch = 0;
		}

		if( isentence )
		{
			m_rgSuitPlayList[ isearch ] = 0;
			if( isentence > 0 )
			{
				// play sentence number

				char sentence[ CBSENTENCENAME_MAX + 1 ];
				strcpy( sentence, "!" );
				strcat( sentence, g_Sentences.GetSentenceName( isentence ) );
				EMIT_SOUND_SUIT( this, sentence );
			}
			else
			{
				// play sentence group
				EMIT_GROUPID_SUIT( this, -isentence );
			}
			m_flSuitUpdate = gpGlobals->time + PLAYER_SUITUPDATETIME;
		}
		else
		{
			// queue is empty, don't check 
			m_flSuitUpdate = 0;
		}
	}
}

// add sentence to suit playlist queue. if fgroup is true, then
// name is a sentence group (HEV_AA), otherwise name is a specific
// sentence name ie: !HEV_AA0.  If iNoRepeat is specified in
// seconds, then we won't repeat playback of this word or sentence
// for at least that number of seconds.
void CBasePlayer::SetSuitUpdate( const char* const pszName, const SuitUpdateType updateType, int iNoRepeatTime )
{
	int isentence;
	int iempty = -1;


	// Ignore suit updates if no suit
	if( !( GetWeapons().Any( 1 << WEAPON_SUIT ) ) )
		return;

	if( g_pGameRules->IsMultiplayer() )
	{
		// due to static channel design, etc. We don't play HEV sounds in multiplayer right now.
		return;
	}

	// if name == NULL, then clear out the queue

	if( !pszName )
	{
		for( int i = 0; i < CSUITPLAYLIST; ++i )
			m_rgSuitPlayList[ i ] = 0;
		return;
	}
	// get sentence or group number
	if( updateType == SUIT_SENTENCE )
	{
		isentence = g_Sentences.Lookup( pszName, NULL );
		if( isentence < 0 )
			return;
	}
	else
		// mark group number as negative
		isentence = -g_Sentences.GetIndex( pszName );

	// check norepeat list - this list lets us cancel
	// the playback of words or sentences that have already
	// been played within a certain time.

	for( int i = 0; i < CSUITNOREPEAT; ++i )
	{
		if( isentence == m_rgiSuitNoRepeat[ i ] )
		{
			// this sentence or group is already in 
			// the norepeat list

			if( m_rgflSuitNoRepeatTime[ i ] < gpGlobals->time )
			{
				// norepeat time has expired, clear it out
				m_rgiSuitNoRepeat[ i ] = 0;
				m_rgflSuitNoRepeatTime[ i ] = 0.0;
				iempty = i;
				break;
			}
			else
			{
				// don't play, still marked as norepeat
				return;
			}
		}
		// keep track of empty slot
		if( !m_rgiSuitNoRepeat[ i ] )
			iempty = i;
	}

	// sentence is not in norepeat list, save if norepeat time was given

	if( iNoRepeatTime )
	{
		if( iempty < 0 )
			iempty = RANDOM_LONG( 0, CSUITNOREPEAT - 1 ); // pick random slot to take over
		m_rgiSuitNoRepeat[ iempty ] = isentence;
		m_rgflSuitNoRepeatTime[ iempty ] = iNoRepeatTime + gpGlobals->time;
	}

	// find empty spot in queue, or overwrite last spot

	m_rgSuitPlayList[ m_iSuitPlayNext++ ] = isentence;
	if( m_iSuitPlayNext == CSUITPLAYLIST )
		m_iSuitPlayNext = 0;

	if( m_flSuitUpdate <= gpGlobals->time )
	{
		if( m_flSuitUpdate == 0 )
			// play queue is empty, don't delay too long before playback
			m_flSuitUpdate = gpGlobals->time + PLAYER_SUITFIRSTUPDATETIME;
		else
			m_flSuitUpdate = gpGlobals->time + PLAYER_SUITUPDATETIME;
	}
}

// if in range of radiation source, ping geiger counter
void CBasePlayer::UpdateGeigerCounter()
{
	// delay per update ie: don't flood net with these msgs
	if( gpGlobals->time < m_flgeigerDelay )
		return;

	m_flgeigerDelay = gpGlobals->time + PLAYER_GEIGERDELAY;

	// send range to radition source to client

	const byte range = ( byte ) ( m_flgeigerRange / 4 );

	if( range != m_igeigerRangePrev )
	{
		m_igeigerRangePrev = range;

		MESSAGE_BEGIN( MSG_ONE, gmsgGeigerRange, NULL, this );
		WRITE_BYTE( range );
		MESSAGE_END();
	}

	// reset counter and semaphore
	if( !RANDOM_LONG( 0, 3 ) )
		m_flgeigerRange = 1000;

}

void CBasePlayer::CheckTimeBasedDamage()
{
	byte bDuration = 0;

	if( !( m_bitsDamageType & DMG_TIMEBASED ) )
		return;

	// only check for time based damage approx. every 2 seconds
	if( fabs( gpGlobals->time - m_tbdPrev ) < 2.0 )
		return;

	m_tbdPrev = gpGlobals->time;

	for( int i = 0; i < CDMG_TIMEBASED; ++i )
	{
		// make sure bit is set for damage type
		if( m_bitsDamageType & ( DMG_PARALYZE << i ) )
		{
			switch( i )
			{
			case itbd_Paralyze:
				// UNDONE - flag movement as half-speed
				bDuration = PARALYZE_DURATION;
				break;
			case itbd_NerveGas:
				//TakeDamage( this, this, NERVEGAS_DAMAGE, DMG_GENERIC );	
				bDuration = NERVEGAS_DURATION;
				break;
			case itbd_Poison:
				TakeDamage( this, this, POISON_DAMAGE, DMG_GENERIC );
				bDuration = POISON_DURATION;
				break;
			case itbd_Radiation:
				//TakeDamage( this, this, RADIATION_DAMAGE, DMG_GENERIC );
				bDuration = RADIATION_DURATION;
				break;
			case itbd_DrownRecover:
				// NOTE: this hack is actually used to RESTORE health
				// after the player has been drowning and finally takes a breath
				if( m_idrowndmg > m_idrownrestored )
				{
					int idif = min( m_idrowndmg - m_idrownrestored, 10 );

					GiveHealth( idif, DMG_GENERIC );
					m_idrownrestored += idif;
				}
				bDuration = 4;	// get up to 5*10 = 50 points back
				break;
			case itbd_Acid:
				//TakeDamage(pev, pev, ACID_DAMAGE, DMG_GENERIC);
				bDuration = ACID_DURATION;
				break;
			case itbd_SlowBurn:
				//TakeDamage(pev, pev, SLOWBURN_DAMAGE, DMG_GENERIC);
				bDuration = SLOWBURN_DURATION;
				break;
			case itbd_SlowFreeze:
				//TakeDamage(pev, pev, SLOWFREEZE_DAMAGE, DMG_GENERIC);
				bDuration = SLOWFREEZE_DURATION;
				break;
			default:
				bDuration = 0;
			}

			if( m_rgbTimeBasedDamage[ i ] )
			{
				// use up an antitoxin on poison or nervegas after a few seconds of damage					
				if( ( ( i == itbd_NerveGas ) && ( m_rgbTimeBasedDamage[ i ] < NERVEGAS_DURATION ) ) ||
					( ( i == itbd_Poison ) && ( m_rgbTimeBasedDamage[ i ] < POISON_DURATION ) ) )
				{
					if( m_rgItems[ ITEM_ANTIDOTE ] )
					{
						m_rgbTimeBasedDamage[ i ] = 0;
						m_rgItems[ ITEM_ANTIDOTE ]--;
						SetSuitUpdate( "!HEV_HEAL4", SUIT_SENTENCE, SUIT_REPEAT_OK );
					}
				}


				// decrement damage duration, detect when done.
				if( !m_rgbTimeBasedDamage[ i ] || --m_rgbTimeBasedDamage[ i ] == 0 )
				{
					m_rgbTimeBasedDamage[ i ] = 0;
					// if we're done, clear damage bits
					m_bitsDamageType &= ~( DMG_PARALYZE << i );
				}
			}
			else
				// first time taking this damage type - init damage duration
				m_rgbTimeBasedDamage[ i ] = bDuration;
		}
	}
}
