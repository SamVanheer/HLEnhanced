#include <new>

#include "hud.h"
#include "cl_util.h"

#include "com_weapons.h"
#include "usercmd.h"
#include "entity_state.h"
#include "hl_weapons.h"
#include "parsemsg.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "Weapons.h"
#include "CBasePlayer.h"

#include "entities/weapons/CRpg.h"

#include "entities/CEntityDictionary.h"
#include "entities/CEntityRegistry.h"

#include "CClientPrediction.h"

//TODO: move - Solokiller
Vector g_vPlayerVelocity;

CClientPrediction g_Prediction;

void CClientPrediction::MsgFunc_WpnBody( const char* pszName, int iSize, void* pBuf )
{
	CBufferReader reader( pBuf, iSize );

	const int iWeaponID = reader.ReadByte();
	const int iBody = reader.ReadByte();

	if( CBasePlayerWeapon* pWeapon = GetWeapon( iWeaponID ) )
	{
		pWeapon->SetBody( iBody );

		if( m_pPlayer->m_pActiveItem == pWeapon )
		{
			//This will reset some engine data, but it's better than going out of sync. - Solokiller
			HUD_SendWeaponAnim( HUD_GetWeaponAnim(), iBody, true );
		}
	}
}

void CClientPrediction::Initialize()
{
	HOOK_GLOBAL_MESSAGE( *this, WpnBody );
}

void CClientPrediction::NewMapStarted()
{
	for( auto pWeapon : m_pWeapons )
	{
		if( pWeapon )
			UTIL_RemoveNow( pWeapon );
	}

	memset( m_pWeapons, 0, sizeof( m_pWeapons ) );

	UTIL_RemoveNow( m_pPlayer );

	m_pPlayer = nullptr;

	memset( m_Entvars, 0, sizeof( m_Entvars ) );

	m_uiNumAllocatedEnts = 0;

	if( auto pPlayer = GetEntityDict().FindEntityClassByEntityName( "player" ) )
	{
		m_pPlayer = static_cast<CBasePlayer*>( pPlayer->CreateInstance( CreateEntity() ) );

		m_pPlayer->Precache();
		m_pPlayer->InitialSpawn();
		m_pPlayer->Spawn();
	}
	else
		Alert( at_error, "CClientPrediction::MapInit: Couldn't find player class!\n" );
}

void CClientPrediction::MapInit()
{
}

entvars_t* CClientPrediction::CreateEntity()
{
	ASSERT( static_cast<size_t>( m_uiNumAllocatedEnts ) < ARRAYSIZE( m_Entvars ) );

	if( m_uiNumAllocatedEnts >= ARRAYSIZE( m_Entvars ) )
		return nullptr;

	memset( &m_Entvars[ m_uiNumAllocatedEnts ], 0, sizeof( entvars_t ) );
	return &m_Entvars[ m_uiNumAllocatedEnts++ ];
}

void CClientPrediction::AddWeapon( CBasePlayerWeapon* pWeapon )
{
	ASSERT( pWeapon );

	if( !pWeapon )
		return;

	if( m_pWeapons[ pWeapon->m_iId ] )
	{
		Alert( at_warning, "HUD_AddWeapon: Weapon \"%s\" already in slot %d, overwriting!\n", pWeapon->GetClassname(), pWeapon->m_iId );
	}

	m_pWeapons[ pWeapon->m_iId ] = pWeapon;
}

void CClientPrediction::SetupWeapons()
{
	for( auto pWeapon : m_pWeapons )
	{
		if( pWeapon )
		{
			pWeapon->m_pPlayer = m_pPlayer;
		}
	}
}

/*
=====================
HUD_WeaponsPostThink

Run Weapon firing code on client
=====================
*/
void CClientPrediction::WeaponsPostThink( local_state_t *from, local_state_t *to, usercmd_t *cmd, double time, unsigned int random_seed )
{
	int i;
	int buttonsChanged;
	CBasePlayerWeapon *pWeapon = NULL;
	CBasePlayerWeapon *pCurrent;
	weapon_data_t nulldata, *pfrom, *pto;
	static int lasthealth;

	memset( &nulldata, 0, sizeof( nulldata ) );

	// Get current clock
	gpGlobals->time = time;

	// Fill in data based on selected weapon
	// FIXME, make this a method in each weapon?  where you pass in an entity_state_t *?
	//Just pull the weapon from the list. Does the same thing as the switch that used to be here, without needing an update every time you add a weapon - Solokiller
	pWeapon = m_pWeapons[ from->client.m_iId ];

	//Non-predicted weapons should be null here. - Solokiller
	if( pWeapon && !pWeapon->IsPredicted() )
		pWeapon = nullptr;

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	g_finalstate = to;

	// If we are running events/etc. go ahead and see if we
	//  managed to die between last frame and this one
	// If so, run the appropriate player killed or spawn function
	if( g_runfuncs )
	{
		if( to->client.health <= 0 && lasthealth > 0 )
		{
			m_pPlayer->Killed( CTakeDamageInfo(), GIB_NORMAL );

		}
		else if( to->client.health > 0 && lasthealth <= 0 )
		{
			m_pPlayer->Spawn();
		}

		lasthealth = to->client.health;
	}

	// We are not predicting the current weapon, just bow out here.
	if( !pWeapon )
		return;

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		pCurrent = m_pWeapons[ i ];
		if( !pCurrent )
		{
			continue;
		}

		pfrom = &from->weapondata[ i ];

		pCurrent->SetWeaponData( *pfrom );

		//TODO: this doesn't look right. What if pCurrent isn't the current weapon? - Solokiller
		if( pCurrent->PrimaryAmmoIndex() != WEAPON_NOCLIP )
			m_pPlayer->m_rgAmmo[ pCurrent->PrimaryAmmoIndex() ] = ( int ) from->client.vuser4[ 1 ];

		if( pCurrent->SecondaryAmmoIndex() != WEAPON_NOCLIP )
			m_pPlayer->m_rgAmmo[ pCurrent->SecondaryAmmoIndex() ] = ( int ) from->client.vuser4[ 2 ];
	}

	// For random weapon events, use this seed to seed random # generator
	m_pPlayer->random_seed = random_seed;

	// Get old buttons from previous state.
	m_pPlayer->m_afButtonLast = from->playerstate.oldbuttons;

	// Which buttsons chave changed
	buttonsChanged = ( m_pPlayer->m_afButtonLast ^ cmd->buttons );	// These buttons have changed this frame

																// Debounced button codes for pressed/released
																// The changed ones still down are "pressed"
	m_pPlayer->m_afButtonPressed = buttonsChanged & cmd->buttons;
	// The ones not down are "released"
	m_pPlayer->m_afButtonReleased = buttonsChanged & ( ~cmd->buttons );

	// Set player variables that weapons code might check/alter
	m_pPlayer->GetButtons().Set( cmd->buttons );

	m_pPlayer->SetAbsVelocity( from->client.velocity );
	m_pPlayer->GetFlags().Set( from->client.flags );

	m_pPlayer->SetDeadFlag( static_cast<DeadFlag>( from->client.deadflag ) );
	m_pPlayer->SetWaterLevel( static_cast<WaterLevel>( from->client.waterlevel ) );
	m_pPlayer->SetMaxSpeed( from->client.maxspeed );
	m_pPlayer->SetFOV( from->client.fov );
	m_pPlayer->SetWeaponAnim( from->client.weaponanim );
	m_pPlayer->SetViewModelIndex( from->client.viewmodel );
	m_pPlayer->m_flNextAttack = from->client.m_flNextAttack;
	m_pPlayer->m_flNextAmmoBurn = from->client.fuser2;
	m_pPlayer->m_flAmmoStartCharge = from->client.fuser3;

	g_vPlayerVelocity = m_pPlayer->GetAbsVelocity();

	//Stores all our ammo info, so the client side weapons can use them.
	m_pPlayer->SetAmmoCount( "9mm", ( int ) from->client.vuser1[ 0 ] );
	m_pPlayer->SetAmmoCount( "357", ( int ) from->client.vuser1[ 1 ] );
	m_pPlayer->SetAmmoCount( "ARgrenades", ( int ) from->client.vuser1[ 2 ] );
	m_pPlayer->SetAmmoCount( "bolts", ( int ) from->client.ammo_nails ); //is an int anyways...
	m_pPlayer->SetAmmoCount( "buckshot",  ( int ) from->client.ammo_shells );
	m_pPlayer->SetAmmoCount( "uranium", ( int ) from->client.ammo_cells );
	m_pPlayer->SetAmmoCount( "Hornets", ( int ) from->client.vuser2[ 0 ] );
	m_pPlayer->SetAmmoCount( "rockets", ( int ) from->client.ammo_rockets );
#ifdef GAME_OPFOR
	m_pPlayer->SetAmmoCount( "762", ( int ) from->client.vuser3[ 0 ] );
	m_pPlayer->SetAmmoCount( "556", ( int ) from->client.vuser3[ 1 ] );
	m_pPlayer->SetAmmoCount( "shock_rounds", ( int ) from->client.vuser3[ 2 ] );
	m_pPlayer->SetAmmoCount( "spores", ( int ) from->client.vuser4[ 0 ] );
#endif

	// Point to current weapon object
	if( from->client.m_iId )
	{
		m_pPlayer->m_pActiveItem = m_pWeapons[ from->client.m_iId ];
	}

	if( m_pPlayer->m_pActiveItem->m_iId == WEAPON_RPG )
	{
		if( !cl_lw->value )
		{
			( ( CRpg * ) m_pPlayer->m_pActiveItem )->m_bSpotActive = from->client.vuser2[ 1 ] != 0;
		}

		( ( CRpg * ) m_pPlayer->m_pActiveItem )->m_cActiveRockets = ( int ) from->client.vuser2[ 2 ];
	}

	// Don't go firing anything if we have died or are spectating
	// Or if we don't have a weapon model deployed
	if( ( m_pPlayer->GetDeadFlag() != ( DEAD_DISCARDBODY + 1 ) ) &&
		!CL_IsDead() && m_pPlayer->GetViewModelIndex() && !g_iUser1 )
	{
		if( pWeapon->GetNextThink() > 0 && pWeapon->GetNextThink() <= gpGlobals->time )
		{
			pWeapon->SetNextThink( 0 );

			pWeapon->Think();
		}

		if( m_pPlayer->m_flNextAttack <= 0 )
		{
			pWeapon->ItemPostFrame();
		}
	}

	// Assume that we are not going to switch weapons
	to->client.m_iId = from->client.m_iId;

	// Now see if we issued a changeweapon command ( and we're not dead )
	if( cmd->weaponselect && ( m_pPlayer->GetDeadFlag() != ( DEAD_DISCARDBODY + 1 ) ) )
	{
		// Switched to a different weapon?
		if( from->weapondata[ cmd->weaponselect ].m_iId == cmd->weaponselect )
		{
			CBasePlayerWeapon *pNew = m_pWeapons[ cmd->weaponselect ];
			if( pNew && ( pNew != pWeapon ) )
			{
				// Put away old weapon
				if( m_pPlayer->m_pActiveItem )
					m_pPlayer->m_pActiveItem->Holster();

				m_pPlayer->m_pLastItem = m_pPlayer->m_pActiveItem;
				m_pPlayer->m_pActiveItem = pNew;

				// Deploy new weapon
				if( m_pPlayer->m_pActiveItem )
				{
					m_pPlayer->m_pActiveItem->Deploy();
				}

				// Update weapon id so we can predict things correctly.
				to->client.m_iId = cmd->weaponselect;
			}
		}
	}

	// Copy in results of prediction code
	to->client.viewmodel		= m_pPlayer->GetViewModelIndex();
	to->client.fov				= m_pPlayer->GetFOV();
	to->client.weaponanim		= m_pPlayer->GetWeaponAnim();
	to->client.m_flNextAttack	= m_pPlayer->m_flNextAttack;
	to->client.fuser2			= m_pPlayer->m_flNextAmmoBurn;
	to->client.fuser3			= m_pPlayer->m_flAmmoStartCharge;
	to->client.maxspeed			= m_pPlayer->GetMaxSpeed();

	//HL Weapons
	to->client.vuser1[ 0 ] = m_pPlayer->GetAmmoCount( "9mm" );
	to->client.vuser1[ 1 ] = m_pPlayer->GetAmmoCount( "357" );
	to->client.vuser1[ 2 ] = m_pPlayer->GetAmmoCount( "ARgrenades" );

	to->client.ammo_nails		= m_pPlayer->GetAmmoCount( "bolts" );
	to->client.ammo_shells		= m_pPlayer->GetAmmoCount( "buckshot" );
	to->client.ammo_cells		= m_pPlayer->GetAmmoCount( "uranium" );
	to->client.vuser2[ 0 ]		= m_pPlayer->GetAmmoCount( "Hornets" );
	to->client.ammo_rockets		= m_pPlayer->GetAmmoCount( "rockets" );
#ifdef GAME_OPFOR
	to->client.vuser3[ 0 ]		= m_pPlayer->GetAmmoCount( "762" );
	to->client.vuser3[ 1 ]		= m_pPlayer->GetAmmoCount( "556" );
	to->client.vuser3[ 2 ]		= m_pPlayer->GetAmmoCount( "shock_rounds" );
	to->client.vuser4[ 0 ]		= m_pPlayer->GetAmmoCount( "spores" );
#endif

	//TODO: why isn't this in the weapon's user variables? - Solokiller
	if( m_pPlayer->m_pActiveItem->m_iId == WEAPON_RPG )
	{
		if( !cl_lw->value )
		{
			from->client.vuser2[ 1 ] = ( ( CRpg * ) m_pPlayer->m_pActiveItem )->m_bSpotActive;
		}

		from->client.vuser2[ 2 ] = ( ( CRpg * ) m_pPlayer->m_pActiveItem )->m_cActiveRockets;
	}

	// Make sure that weapon animation matches what the game .dll is telling us
	//  over the wire ( fixes some animation glitches )
	if( g_runfuncs && ( HUD_GetWeaponAnim() != to->client.weaponanim ) )
	{
		// Force a fixed anim down to viewmodel
		//Now uses the body that the weapon is currently using. - Solokiller
		HUD_SendWeaponAnim( to->client.weaponanim, pWeapon->GetBody(), 1 );
	}

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		pCurrent = m_pWeapons[ i ];

		pto = &to->weapondata[ i ];

		if( !pCurrent )
		{
			memset( pto, 0, sizeof( weapon_data_t ) );
			continue;
		}

		pCurrent->GetWeaponData( *pto );

		// Decrement weapon counters, server does this at same time ( during post think, after doing everything else )
		pto->m_flNextReload -= cmd->msec / 1000.0;
		pto->m_fNextAimBonus -= cmd->msec / 1000.0;
		pto->m_flNextPrimaryAttack -= cmd->msec / 1000.0;
		pto->m_flNextSecondaryAttack -= cmd->msec / 1000.0;
		pto->m_flTimeWeaponIdle -= cmd->msec / 1000.0;
		pto->fuser1 -= cmd->msec / 1000.0;

		pCurrent->DecrementTimers( cmd->msec / 1000.0 );

		to->client.vuser4[ 1 ] = pCurrent->PrimaryAmmoIndex() != WEAPON_NOCLIP ? m_pPlayer->m_rgAmmo[ pCurrent->PrimaryAmmoIndex() ] : 0;
		to->client.vuser4[ 2 ] = pCurrent->SecondaryAmmoIndex() != WEAPON_NOCLIP ? m_pPlayer->m_rgAmmo[ pCurrent->SecondaryAmmoIndex() ] : 0;

		/*		if ( pto->m_flPumpTime != -9999 )
		{
		pto->m_flPumpTime -= cmd->msec / 1000.0;
		if ( pto->m_flPumpTime < -0.001 )
		pto->m_flPumpTime = -0.001;
		}*/

		if( pto->m_fNextAimBonus < -1.0 )
		{
			pto->m_fNextAimBonus = -1.0;
		}

		if( pto->m_flNextPrimaryAttack < -1.0 )
		{
			pto->m_flNextPrimaryAttack = -1.0;
		}

		if( pto->m_flNextSecondaryAttack < -0.001 )
		{
			pto->m_flNextSecondaryAttack = -0.001;
		}

		if( pto->m_flTimeWeaponIdle < -0.001 )
		{
			pto->m_flTimeWeaponIdle = -0.001;
		}

		if( pto->m_flNextReload < -0.001 )
		{
			pto->m_flNextReload = -0.001;
		}

		if( pto->fuser1 < -0.001 )
		{
			pto->fuser1 = -0.001;
		}
	}

	// m_flNextAttack is now part of the weapons, but is part of the player instead
	to->client.m_flNextAttack -= cmd->msec / 1000.0;
	if( to->client.m_flNextAttack < -0.001 )
	{
		to->client.m_flNextAttack = -0.001;
	}

	to->client.fuser2 -= cmd->msec / 1000.0;
	if( to->client.fuser2 < -0.001 )
	{
		to->client.fuser2 = -0.001;
	}

	to->client.fuser3 -= cmd->msec / 1000.0;
	if( to->client.fuser3 < -0.001 )
	{
		to->client.fuser3 = -0.001;
	}

	// Store off the last position from the predicted state.
	HUD_SetLastOrg();

	// Wipe it so we can't use it after this frame
	g_finalstate = NULL;
}