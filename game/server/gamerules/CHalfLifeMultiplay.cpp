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
//
// CHalfLifeMultiplay.cpp
//
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "Weapons.h"
#include "CHalfLifeMultiplay.h"
 
#include "Skill.h"
#include "Server.h"
#include "entities/items/CItem.h"
#include "entities/spawnpoints/CBaseSpawnPoint.h"
#include "voice_gamemgr.h"
#include "hltv.h"

#include "CWeaponInfoCache.h"

#include "MapCycle.h"

extern DLL_GLOBAL CGameRules	*g_pGameRules;
extern DLL_GLOBAL bool	g_fGameOver;

extern int g_teamplay;

float g_flIntermissionStartTime = 0;

CVoiceGameMgr	g_VoiceGameMgr;

class CMultiplayGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker)
	{
		if ( g_teamplay )
		{
			if ( g_pGameRules->PlayerRelationship( pListener, pTalker ) != GR_TEAMMATE )
			{
				return false;
			}
		}

		return true;
	}
};
static CMultiplayGameMgrHelper g_GameMgrHelper;

//*********************************************************
// Rules for the half-life multiplayer game.
//*********************************************************

CHalfLifeMultiplay :: CHalfLifeMultiplay()
{
	g_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	m_flIntermissionEndTime = 0;
	g_flIntermissionStartTime = 0;
}

void CHalfLifeMultiplay::OnCreate()
{
	CGameRules::OnCreate();

	// 11/8/98
	// Modified by YWB:  Server .cfg file is now a cvar, so that 
	//  server ops can run multiple game servers, with different server .cfg files,
	//  from a single installed directory.
	// Mapcyclefile is already a cvar.

	// 3/31/99
	// Added lservercfg file cvar, since listen and dedicated servers should not
	// share a single config file. (sjb)
	if( IS_DEDICATED_SERVER() )
	{
		// this code has been moved into engine, to only run server.cfg once
	}
	else
	{
		// listen server
		char *lservercfgfile = ( char * ) CVAR_GET_STRING( "lservercfgfile" );

		if( lservercfgfile && lservercfgfile[ 0 ] )
		{
			char szCommand[ 256 ];

			ALERT( at_console, "Executing listen server config file\n" );
			sprintf( szCommand, "exec %s\n", lservercfgfile );
			SERVER_COMMAND( szCommand );
		}
	}
}

bool CHalfLifeMultiplay::ClientCommand( CBasePlayer *pPlayer, const char *pcmd )
{
	if(g_VoiceGameMgr.ClientCommand(pPlayer, pcmd))
		return true;

	return CGameRules::ClientCommand(pPlayer, pcmd);
}

cvar_t* CHalfLifeMultiplay::GetSkillCvar( const skilldata_t& skillData, const char* pszSkillCvarName )
{
	//These cvars have overrides for multiplayer. - Solokiller
	if( strcmp( pszSkillCvarName, "sk_suitcharger" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_crowbar" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_9mm_bullet" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_357_bullet" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_9mmAR_bullet" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_9mmAR_grenade" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_buckshot" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_xbow_bolt_client" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_rpg" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_egon_wide" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_egon_narrow" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_hand_grenade" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_satchel" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_tripmine" ) == 0 ||
		strcmp( pszSkillCvarName, "sk_plr_hornet_dmg" ) == 0 )
	{
		char szBuffer[ 64 ];

		//All overrides have _mp appended to their base name. - Solokiller
		/*const int Result = */snprintf( szBuffer, sizeof( szBuffer ), "%s_mp", pszSkillCvarName );

		return skilldata_t::GetSkillCvar( szBuffer, skillData.GetSkillLevel() );
	}

	return CGameRules::GetSkillCvar( skillData, pszSkillCvarName );
}

extern cvar_t timeleft, fragsleft;

extern cvar_t mp_chattime;

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: Think ( void )
{
	g_VoiceGameMgr.Update(gpGlobals->frametime);

	///// Check game rules /////
	static int last_frags;
	static int last_time;

	int frags_remaining = 0;
	int time_remaining = 0;

	if ( g_fGameOver )   // someone else quit the game already
	{
		// bounds check
		int time = (int)CVAR_GET_FLOAT( "mp_chattime" );
		if ( time < 1 )
			CVAR_SET_STRING( "mp_chattime", "1" );
		else if ( time > MAX_INTERMISSION_TIME )
			CVAR_SET_STRING( "mp_chattime", UTIL_dtos( MAX_INTERMISSION_TIME ) );

		m_flIntermissionEndTime = g_flIntermissionStartTime + mp_chattime.value;

		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->time )
		{
			if ( m_bEndIntermissionButtonHit  // check that someone has pressed a key, or the max intermission time is over
				|| ( ( g_flIntermissionStartTime + MAX_INTERMISSION_TIME ) < gpGlobals->time) ) 
				ChangeLevel(); // intermission is over
		}

		return;
	}

	float flTimeLimit = timelimit.value * 60;
	float flFragLimit = fraglimit.value;

	time_remaining = (int)(flTimeLimit ? ( flTimeLimit - gpGlobals->time ) : 0);
	
	if ( flTimeLimit != 0 && gpGlobals->time >= flTimeLimit )
	{
		GoToIntermission();
		return;
	}

	if ( flFragLimit )
	{
		int bestfrags = 9999;
		int remain;

		// check if any player is over the frag limit
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

			if ( pPlayer && pPlayer->GetFrags() >= flFragLimit )
			{
				GoToIntermission();
				return;
			}


			if ( pPlayer )
			{
				remain = flFragLimit - pPlayer->GetFrags();
				if ( remain < bestfrags )
				{
					bestfrags = remain;
				}
			}

		}
		frags_remaining = bestfrags;
	}

	// Updates when frags change
	if ( frags_remaining != last_frags )
	{
		g_engfuncs.pfnCvar_DirectSet( &fragsleft, UTIL_VarArgs( "%i", frags_remaining ) );
	}

	// Updates once per second
	if ( timeleft.value != last_time )
	{
		g_engfuncs.pfnCvar_DirectSet( &timeleft, UTIL_VarArgs( "%i", time_remaining ) );
	}

	last_frags = frags_remaining;
	last_time  = time_remaining;
}


//=========================================================
//=========================================================
bool CHalfLifeMultiplay::IsMultiplayer() const
{
	return true;
}

//=========================================================
//=========================================================
bool CHalfLifeMultiplay::IsDeathmatch() const
{
	return true;
}

//=========================================================
//=========================================================
bool CHalfLifeMultiplay::IsCoOp() const
{
	return gpGlobals->coop != 0;
}

//=========================================================
//=========================================================
bool CHalfLifeMultiplay::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerWeapon *pWeapon )
{
	if ( !pWeapon->CanDeploy() )
	{
		// that weapon can't deploy anyway.
		return false;
	}

	if ( !pPlayer->m_pActiveItem )
	{
		// player doesn't have an active item!
		return true;
	}

	if ( !pPlayer->m_pActiveItem->CanHolster() )
	{
		// can't put away the active item.
		return false;
	}

	if ( pWeapon->iWeight() > pPlayer->m_pActiveItem->iWeight() )
	{
		return true;
	}

	return false;
}

bool CHalfLifeMultiplay::GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerWeapon *pCurrentWeapon )
{
	CBasePlayerWeapon *pCheck;
	CBasePlayerWeapon *pBest;// this will be used in the event that we don't find a weapon in the same category.
	int iBestWeight;
	int i;

	iBestWeight = -1;// no weapon lower than -1 can be autoswitched to
	pBest = NULL;

	if ( !pCurrentWeapon->CanHolster() )
	{
		// can't put this gun away right now, so can't switch.
		return false;
	}

	for ( i = 0 ; i < MAX_WEAPON_SLOTS; i++ )
	{
		pCheck = pPlayer->m_rgpPlayerItems[ i ];

		while ( pCheck )
		{
			if ( pCheck->iWeight() > -1 && pCheck->iWeight() == pCurrentWeapon->iWeight() && pCheck != pCurrentWeapon )
			{
				// this weapon is from the same category. 
				if ( pCheck->CanDeploy() )
				{
					if ( pPlayer->SwitchWeapon( pCheck ) )
					{
						return true;
					}
				}
			}
			else if ( pCheck->iWeight() > iBestWeight && pCheck != pCurrentWeapon )// don't reselect the weapon we're trying to get rid of
			{
				//ALERT ( at_console, "Considering %s\n", pCheck->GetClassname() );
				// we keep updating the 'best' weapon just in case we can't find a weapon of the same weight
				// that the player was using. This will end up leaving the player with his heaviest-weighted 
				// weapon. 
				if ( pCheck->CanDeploy() )
				{
					// if this weapon is useable, flag it as the best
					iBestWeight = pCheck->iWeight();
					pBest = pCheck;
				}
			}

			pCheck = pCheck->m_pNext;
		}
	}

	// if we make it here, we've checked all the weapons and found no useable 
	// weapon in the same catagory as the current weapon. 
	
	// if pBest is null, we didn't find ANYTHING. Shouldn't be possible- should always 
	// at least get the crowbar, but ya never know.
	if ( !pBest )
	{
		return false;
	}

	pPlayer->SwitchWeapon( pBest );

	return true;
}

//=========================================================
//=========================================================
bool CHalfLifeMultiplay :: ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ CCONNECT_REJECT_REASON_SIZE ] )
{
	g_VoiceGameMgr.ClientConnected(pEntity);
	return true;
}

void CHalfLifeMultiplay :: UpdateGameMode( CBasePlayer *pPlayer )
{
	MESSAGE_BEGIN( MSG_ONE, gmsgGameMode, NULL, pPlayer );
		WRITE_BYTE( 0 );  // game mode none
	MESSAGE_END();
}

void CHalfLifeMultiplay :: InitHUD( CBasePlayer *pl )
{
	// notify other clients of player joining the game
	UTIL_ClientPrintAll( HUD_PRINTNOTIFY, UTIL_VarArgs( "%s has joined the game\n", 
		pl->HasNetName() ? pl->GetNetName() : "unconnected" ) );

	// team match?
	if ( g_teamplay )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" entered the game\n",  
			pl->GetNetName(),
			UTIL_GetPlayerUserId( pl ),
			UTIL_GetPlayerAuthId( pl ),
			g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pl->edict() ), "model" ) );
	}
	else
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%i>\" entered the game\n",  
			pl->GetNetName(),
			UTIL_GetPlayerUserId( pl ),
			UTIL_GetPlayerAuthId( pl ),
			UTIL_GetPlayerUserId( pl ) );
	}

	UpdateGameMode( pl );

	// sending just one score makes the hud scoreboard active;  otherwise
	// it is just disabled for single play
	MESSAGE_BEGIN( MSG_ONE, gmsgScoreInfo, NULL, pl );
		WRITE_BYTE( pl->entindex() );
		WRITE_SHORT( 0 );
		WRITE_SHORT( 0 );
		WRITE_SHORT( 0 );
		WRITE_SHORT( 0 );
	MESSAGE_END();

	SendMOTDToClient( pl );

	// loop through all active players and send their score info to the new client
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		// FIXME:  Probably don't need to cast this just to read m_iDeaths
		CBasePlayer *plr = UTIL_PlayerByIndex( i );

		if ( plr )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgScoreInfo, NULL, pl );
				WRITE_BYTE( i );	// client number
				WRITE_SHORT( plr->GetFrags() );
				WRITE_SHORT( plr->m_iDeaths );
				WRITE_SHORT( 0 );
				WRITE_SHORT( GetTeamIndex( plr->m_szTeamName ) + 1 );
			MESSAGE_END();
		}
	}

	if ( g_fGameOver )
	{
		MESSAGE_BEGIN( MSG_ONE, SVC_INTERMISSION, NULL, pl );
		MESSAGE_END();
	}
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: ClientDisconnected( edict_t *pClient )
{
	if ( pClient )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );

		if ( pPlayer )
		{
			FireTargets( "game_playerleave", pPlayer, pPlayer, USE_TOGGLE, 0 );

			// team match?
			if ( g_teamplay )
			{
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" disconnected\n",  
					pPlayer->GetNetName(),
					UTIL_GetPlayerUserId( pPlayer ),
					UTIL_GetPlayerAuthId( pPlayer ),
					g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "model" ) );
			}
			else
			{
				UTIL_LogPrintf( "\"%s<%i><%s><%i>\" disconnected\n",  
					pPlayer->GetNetName(),
					UTIL_GetPlayerUserId( pPlayer ),
					UTIL_GetPlayerAuthId( pPlayer ),
					UTIL_GetPlayerUserId( pPlayer ) );
			}

			pPlayer->RemoveAllItems( true );// destroy all of the players weapons and items
		}
	}
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay :: FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	int iFallDamage = (int)falldamage.value;

	switch ( iFallDamage )
	{
	case 1://progressive
		pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
		return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
		break;
	default:
	case 0:// fixed
		return 10;
		break;
	}
} 

//=========================================================
//=========================================================
bool CHalfLifeMultiplay::FPlayerCanTakeDamage( CBasePlayer *pPlayer, const CTakeDamageInfo& info )
{
	return true;
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: PlayerThink( CBasePlayer *pPlayer )
{
	if ( g_fGameOver )
	{
		// check for button presses
		if ( pPlayer->m_afButtonPressed & ( IN_DUCK | IN_ATTACK | IN_ATTACK2 | IN_USE | IN_JUMP ) )
			m_bEndIntermissionButtonHit = true;

		// clear attack/use commands from player
		pPlayer->m_afButtonPressed = 0;
		pPlayer->GetButtons().Set( 0 );
		pPlayer->m_afButtonReleased = 0;
	}
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: PlayerSpawn( CBasePlayer *pPlayer )
{
	bool addDefault = true;
	CBaseEntity	*pWeaponEntity = nullptr;

	pPlayer->GetWeapons().AddFlags( 1 << WEAPON_SUIT );

	while( ( pWeaponEntity = UTIL_FindEntityByClassname( pWeaponEntity, "game_player_equip" ) ) != nullptr )
	{
		pWeaponEntity->Touch( pPlayer );
		addDefault = false;
	}

	if ( addDefault )
	{
		pPlayer->GiveNamedItem( "weapon_crowbar" );
		pPlayer->GiveNamedItem( "weapon_9mmhandgun" );

		//Now looks up the value so it auto-corrects. Change the weapon type if you change the weapon being given. - Solokiller
		if( auto pInfo = g_WeaponInfoCache.FindWeaponInfo( "weapon_9mmhandgun" ) )
			pPlayer->GiveAmmo( pInfo->GetMaxMagazine() * 4, "9mm" );// 4 full reloads
	}
}

//=========================================================
//=========================================================
bool CHalfLifeMultiplay::FPlayerCanRespawn( CBasePlayer *pPlayer )
{
	return true;
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay :: FlPlayerSpawnTime( CBasePlayer *pPlayer )
{
	return gpGlobals->time;//now!
}

bool CHalfLifeMultiplay::AllowAutoTargetCrosshair()
{
	return ( aimcrosshair.value != 0 );
}

//=========================================================
// IPointsForKill - how many points awarded to anyone
// that kills this player?
//=========================================================
int CHalfLifeMultiplay :: IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled )
{
	return 1;
}


//=========================================================
// PlayerKilled - someone/something killed this player
//=========================================================
void CHalfLifeMultiplay::PlayerKilled( CBasePlayer* pVictim, const CTakeDamageInfo& info )
{
	auto pKiller = info.GetAttacker();
	//auto pInflictor = info.GetInflictor();

	ASSERT( pKiller );
	ASSERT( info.GetInflictor() );

	DeathNotice( pVictim, info );

	pVictim->m_iDeaths += 1;

	FireTargets( "game_playerdie", pVictim, pVictim, USE_TOGGLE, 0 );

	CBasePlayer* const peKiller = pKiller && pKiller->IsPlayer() ? ( CBasePlayer* ) pKiller : nullptr;

	if( pVictim == pKiller )  
	{
		// killed self
		pKiller->SetFrags( pKiller->GetFrags() - 1 );
	}
	else if( peKiller )
	{
		// if a player dies in a deathmatch game and the killer is a client, award the killer some points
		peKiller->SetFrags( peKiller->GetFrags() + IPointsForKill( peKiller, pVictim ) );
		
		FireTargets( "game_playerkill", peKiller, peKiller, USE_TOGGLE, 0 );
	}
	else
	{
		// killed by the world
		pKiller->SetFrags( pKiller->GetFrags() - 1 );
	}

	// update the scores
	// killed scores
	MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( pVictim->entindex() );
		WRITE_SHORT( pVictim->GetFrags() );
		WRITE_SHORT( pVictim->m_iDeaths );
		WRITE_SHORT( 0 );
		WRITE_SHORT( GetTeamIndex( pVictim->m_szTeamName ) + 1 );
	MESSAGE_END();

	// killers score, if it's a player
	if( peKiller )
	{
		MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
			WRITE_BYTE( peKiller->entindex() );
			WRITE_SHORT( peKiller->GetFrags() );
			WRITE_SHORT( peKiller->m_iDeaths );
			WRITE_SHORT( 0 );
			WRITE_SHORT( GetTeamIndex( peKiller->m_szTeamName) + 1 );
		MESSAGE_END();

		// let the killer paint another decal as soon as he'd like.
		peKiller->m_flNextDecalTime = gpGlobals->time;
	}

	if( pVictim->HasNamedPlayerItem("weapon_satchel") )
	{
		DeactivateSatchels( pVictim, SatchelAction::RELEASE );
	}
}

//=========================================================
// Deathnotice. 
//=========================================================
void CHalfLifeMultiplay::DeathNotice( CBasePlayer* pVictim, const CTakeDamageInfo& info )
{
	auto pKiller = info.GetAttacker();
	auto pInflictor = info.GetInflictor();

	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_index = 0;

	//TODO clean this up - Solokiller
	
	// Hack to fix name change
	const char* const tau = "tau_cannon";
	const char* const gluon = "gluon gun";

	if ( pKiller->GetFlags().Any( FL_CLIENT ) )
	{
		killer_index = pKiller->entindex();
		
		if ( pInflictor )
		{
			if ( pInflictor == pKiller )
			{
				// If the inflictor is the killer,  then it must be their current weapon doing the damage
				CBasePlayer *pPlayer = (CBasePlayer*) pKiller;
				
				if ( pPlayer->m_pActiveItem )
				{
					killer_weapon_name = pPlayer->m_pActiveItem->pszName();
				}
			}
			else
			{
				killer_weapon_name = pInflictor->GetClassname();  // it's just that easy
			}
		}
	}
	else
	{
		killer_weapon_name = pInflictor->GetClassname();
	}

	// strip the monster_* or weapon_* from the inflictor's classname
	if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		killer_weapon_name += 7;
	else if ( strncmp( killer_weapon_name, "monster_", 8 ) == 0 )
		killer_weapon_name += 8;
	else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		killer_weapon_name += 5;

	MESSAGE_BEGIN( MSG_ALL, gmsgDeathMsg );
		WRITE_BYTE( killer_index );						// the killer
		WRITE_BYTE( pVictim->entindex() );		// the victim
		WRITE_STRING( killer_weapon_name );		// what they were killed by (should this be a string?)
	MESSAGE_END();

	// replace the code names with the 'real' names
	if ( !strcmp( killer_weapon_name, "egon" ) )
		killer_weapon_name = gluon;
	else if ( !strcmp( killer_weapon_name, "gauss" ) )
		killer_weapon_name = tau;

	if ( pVictim == pKiller )  
	{
		// killed self

		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"%s\"\n",  
				pVictim->GetNetName(),
				UTIL_GetPlayerUserId( pVictim ),
				UTIL_GetPlayerAuthId( pVictim ),
				g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pVictim->edict() ), "model" ),
				killer_weapon_name );		
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" committed suicide with \"%s\"\n",  
				pVictim->GetNetName(),
				UTIL_GetPlayerUserId( pVictim ),
				UTIL_GetPlayerAuthId( pVictim ),
				UTIL_GetPlayerUserId( pVictim ),
				killer_weapon_name );		
		}
	}
	else if ( pKiller->GetFlags().Any( FL_CLIENT ) )
	{
		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" killed \"%s<%i><%s><%s>\" with \"%s\"\n",  
				pKiller->GetNetName(),
				UTIL_GetPlayerUserId( pKiller ),
				UTIL_GetPlayerAuthId( pKiller ),
				g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( ENT(pKiller) ), "model" ),
				pVictim->GetNetName(),
				UTIL_GetPlayerUserId( pVictim ),
				UTIL_GetPlayerAuthId( pVictim ),
				g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pVictim->edict() ), "model" ),
				killer_weapon_name );
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" killed \"%s<%i><%s><%i>\" with \"%s\"\n",  
				pKiller->GetNetName(),
				UTIL_GetPlayerUserId( pKiller ),
				UTIL_GetPlayerAuthId( pKiller ),
				UTIL_GetPlayerUserId( pKiller ),
				pVictim->GetNetName(),
				UTIL_GetPlayerUserId( pVictim ),
				UTIL_GetPlayerAuthId( pVictim ),
				UTIL_GetPlayerUserId( pVictim ),
				killer_weapon_name );
		}
	}
	else
	{ 
		// killed by the world

		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"%s\" (world)\n",
				pVictim->GetNetName(),
				UTIL_GetPlayerUserId( pVictim ),
				UTIL_GetPlayerAuthId( pVictim ),
				g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pVictim->edict() ), "model" ),
				killer_weapon_name );		
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" committed suicide with \"%s\" (world)\n",
				pVictim->GetNetName(),
				UTIL_GetPlayerUserId( pVictim ),
				UTIL_GetPlayerAuthId( pVictim ),
				UTIL_GetPlayerUserId( pVictim ),
				killer_weapon_name );		
		}
	}

	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE ( 9 );	// command length in bytes
		WRITE_BYTE ( DRC_CMD_EVENT );	// player killed
		WRITE_SHORT( pVictim->entindex() );	// index number of primary entity
		if ( pInflictor )
			WRITE_SHORT( pInflictor->entindex() );	// index number of secondary entity
		else
			WRITE_SHORT( pKiller->entindex() );	// index number of secondary entity
		WRITE_LONG( 7 | DRC_FLAG_DRAMATIC);   // eventflags (priority and flags)
	MESSAGE_END();

	// Print a standard message

	//TODO: add a cvar to control this? - Solokiller

	char szBuffer[ 512 ];

	if( pKiller->GetFlags().Any( FL_MONSTER ) )
	{
		// killed by a monster
		snprintf( szBuffer, sizeof( szBuffer ), "%s was killed by a monster.\n", pVictim->GetNetName() );
	}
	else if( pKiller == pVictim )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "%s commited suicide.\n", pVictim->GetNetName() );
	}
	else if( pKiller->GetFlags().Any( FL_CLIENT ) )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "%s : %s : %s\n", pKiller->GetNetName(), killer_weapon_name, pVictim->GetNetName() );
	}
	else if( pKiller->ClassnameIs( "worldspawn" ) )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "%s fell or drowned or something.\n", pVictim->GetNetName() );
	}
	else if( pKiller->GetSolidType() == SOLID_BSP )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "%s was mooshed.\n", pVictim->GetNetName() );
	}
	else
	{
		snprintf( szBuffer, sizeof( szBuffer ), "%s died mysteriously.\n", pVictim->GetNetName() );
	}

	UTIL_ClientPrintAll( HUD_PRINTNOTIFY, szBuffer );
}

//=========================================================
// PlayerGotWeapon - player has grabbed a weapon that was
// sitting in the world
//=========================================================
void CHalfLifeMultiplay :: PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerWeapon *pWeapon )
{
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHalfLifeMultiplay :: FlWeaponRespawnTime( CBasePlayerWeapon *pWeapon )
{
	if ( weaponstay.value > 0 )
	{
		// make sure it's only certain weapons
		if ( !(pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD) )
		{
			return gpGlobals->time + 0;		// weapon respawns almost instantly
		}
	}

	return gpGlobals->time + WEAPON_RESPAWN_TIME;
}

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHalfLifeMultiplay :: FlWeaponTryRespawn( CBasePlayerWeapon *pWeapon )
{
	if ( pWeapon && pWeapon->m_iId && (pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD) )
	{
		if ( NUMBER_OF_ENTITIES() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE) )
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime( pWeapon );
	}

	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeMultiplay :: VecWeaponRespawnSpot( CBasePlayerWeapon *pWeapon )
{
	return pWeapon->GetAbsOrigin();
}

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHalfLifeMultiplay :: WeaponShouldRespawn( CBasePlayerWeapon *pWeapon )
{
	if ( pWeapon->GetSpawnFlags().Any( SF_NORESPAWN ) )
	{
		return GR_WEAPON_RESPAWN_NO;
	}

	return GR_WEAPON_RESPAWN_YES;
}

//=========================================================
// CanHaveWeapon - returns false if the player is not allowed
// to pick up this weapon
//=========================================================
bool CHalfLifeMultiplay::CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerWeapon *pItem )
{
	if ( weaponstay.value > 0 )
	{
		if ( pItem->iFlags() & ITEM_FLAG_LIMITINWORLD )
			return CGameRules::CanHavePlayerItem( pPlayer, pItem );

		// check if the player already has this weapon
		for ( int i = 0 ; i < MAX_WEAPON_SLOTS; i++ )
		{
			CBasePlayerWeapon *it = pPlayer->m_rgpPlayerItems[i];

			while ( it != NULL )
			{
				if ( it->m_iId == pItem->m_iId )
				{
					return false;
				}

				it = it->m_pNext;
			}
		}
	}

	return CGameRules::CanHavePlayerItem( pPlayer, pItem );
}

//=========================================================
//=========================================================
bool CHalfLifeMultiplay::CanHaveItem( CBasePlayer *pPlayer, CItem *pItem )
{
	return true;
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay::PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem )
{
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::ItemShouldRespawn( CItem *pItem )
{
	if ( pItem->GetSpawnFlags().Any( SF_NORESPAWN ) )
	{
		return GR_ITEM_RESPAWN_NO;
	}

	return GR_ITEM_RESPAWN_YES;
}


//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHalfLifeMultiplay::FlItemRespawnTime( CItem *pItem )
{
	return gpGlobals->time + ITEM_RESPAWN_TIME;
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeMultiplay::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->GetAbsOrigin();
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay::PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount )
{
}

//=========================================================
//=========================================================
bool CHalfLifeMultiplay::IsAllowedToSpawn( CBaseEntity *pEntity )
{
//	if ( pEntity->AnyFlagsSet( FL_MONSTER ) )
//		return false;

	return true;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::AmmoShouldRespawn( CBasePlayerAmmo *pAmmo )
{
	if ( pAmmo->GetSpawnFlags().Any( SF_NORESPAWN ) )
	{
		return GR_AMMO_RESPAWN_NO;
	}

	return GR_AMMO_RESPAWN_YES;
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay::FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo )
{
	return gpGlobals->time + AMMO_RESPAWN_TIME;
}

//=========================================================
//=========================================================
Vector CHalfLifeMultiplay::VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo )
{
	return pAmmo->GetAbsOrigin();
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay::FlHealthChargerRechargeTime( void )
{
	return 60;
}


float CHalfLifeMultiplay::FlHEVChargerRechargeTime( void )
{
	return 30;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::DeadPlayerWeapons( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_GUN_ACTIVE;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::DeadPlayerAmmo( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_AMMO_ACTIVE;
}

CBaseEntity* CHalfLifeMultiplay::GetPlayerSpawnSpot( CBasePlayer* pPlayer )
{
	CBaseEntity* pSpawnSpot = CGameRules::GetPlayerSpawnSpot( pPlayer );

	if( auto pSpawnPoint = dynamic_cast<CBaseSpawnPoint*>( pSpawnSpot ) )
	{
		pSpawnPoint->PlayerSpawned( pPlayer );
	}

	return pSpawnSpot;
}


//=========================================================
//=========================================================
int CHalfLifeMultiplay::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	// half life deathmatch has only enemies
	return GR_NOTTEAMMATE;
}

bool CHalfLifeMultiplay :: PlayFootstepSounds( CBasePlayer *pl, float fvol )
{
	if ( g_footsteps && g_footsteps->value == 0 )
		return false;

	if ( pl->IsOnLadder() || pl->GetAbsVelocity().Length2D() > PLAYER_STEP_SOUND_SPEED )
		return true;  // only make step sounds in multiplayer if the player is moving fast enough

	return false;
}

bool CHalfLifeMultiplay::FAllowFlashlight() const
{ 
	return flashlight.value != 0; 
}

//=========================================================
//=========================================================
bool CHalfLifeMultiplay::FAllowMonsters() const
{
	return ( allowmonsters.value != 0 );
}

//=========================================================
//======== CHalfLifeMultiplay private functions ===========

void CHalfLifeMultiplay::GoToIntermission()
{
	if ( g_fGameOver )
		return;  // intermission has already been triggered, so ignore.

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();

	// bounds check
	int time = (int)CVAR_GET_FLOAT( "mp_chattime" );
	if ( time < 1 )
		CVAR_SET_STRING( "mp_chattime", "1" );
	else if ( time > MAX_INTERMISSION_TIME )
		CVAR_SET_STRING( "mp_chattime", UTIL_dtos( MAX_INTERMISSION_TIME ) );

	m_flIntermissionEndTime = gpGlobals->time + ( (int)mp_chattime.value );
	g_flIntermissionStartTime = gpGlobals->time;

	g_fGameOver = true;
	m_bEndIntermissionButtonHit = false;
}

/*
==============
ChangeLevel

Server is changing to a new level, check mapcycle.txt for map name and setup info
==============
*/
void CHalfLifeMultiplay :: ChangeLevel( void )
{
	char szNextMap[ cchMapNameMost ];
	char szFirstMapInList[ cchMapNameMost ];
	char szCommands[ 1500 ];
	char szRules[ 1500 ];
	int minplayers = 0, maxplayers = 0;
	strcpy( szFirstMapInList, "hldm1" );  // the absolute default level is hldm1

	int	curplayers;
	bool do_cycle = true;

	// find the map to change to
	char *mapcfile = (char*)CVAR_GET_STRING( "mapcyclefile" );
	ASSERT( mapcfile != NULL );

	szCommands[ 0 ] = '\0';
	szRules[ 0 ] = '\0';

	curplayers = UTIL_CountPlayers();

	// Has the map cycle filename changed?
	if ( stricmp( mapcfile, g_MapCycle.GetFileName() ) )
	{
		g_MapCycle.Clear();

		if ( !g_MapCycle.LoadMapCycleFile( mapcfile ) || ( !g_MapCycle.GetItems() ) )
		{
			ALERT( at_console, "Unable to load map cycle file %s\n", mapcfile );
			do_cycle = false;
		}
	}

	if ( do_cycle && g_MapCycle.GetItems() )
	{
		bool keeplooking = false;
		bool found = false;
		CMapCycle::Item_t *item;

		// Assume current map
		strcpy( szNextMap, STRING(gpGlobals->mapname) );
		strcpy( szFirstMapInList, STRING(gpGlobals->mapname) );

		// Traverse list
		for ( item = g_MapCycle.GetNextItem(); item->next != g_MapCycle.GetNextItem(); item = item->next )
		{
			keeplooking = false;

			ASSERT( item != NULL );

			if ( item->minplayers != 0 )
			{
				if ( curplayers >= item->minplayers )
				{
					found = true;
					minplayers = item->minplayers;
				}
				else
				{
					keeplooking = true;
				}
			}

			if ( item->maxplayers != 0 )
			{
				if ( curplayers <= item->maxplayers )
				{
					found = true;
					maxplayers = item->maxplayers;
				}
				else
				{
					keeplooking = true;
				}
			}

			if ( keeplooking )
				continue;

			found = true;
			break;
		}

		if ( !found )
		{
			item = g_MapCycle.GetNextItem();
		}			
		
		// Increment next item pointer
		g_MapCycle.SetNextItem( item->next );

		// Perform logic on current item
		strcpy( szNextMap, item->mapname );

		ExtractCommandString( item->rulebuffer, szCommands );
		strcpy( szRules, item->rulebuffer );
	}

	if ( !IS_MAP_VALID(szNextMap) )
	{
		strcpy( szNextMap, szFirstMapInList );
	}

	g_fGameOver = true;

	ALERT( at_console, "CHANGE LEVEL: %s\n", szNextMap );
	if ( minplayers || maxplayers )
	{
		ALERT( at_console, "PLAYER COUNT:  min %i max %i current %i\n", minplayers, maxplayers, curplayers );
	}
	if ( strlen( szRules ) > 0 )
	{
		ALERT( at_console, "RULES:  %s\n", szRules );
	}
	
	CHANGE_LEVEL( szNextMap, NULL );
	if ( strlen( szCommands ) > 0 )
	{
		SERVER_COMMAND( szCommands );
	}
}

void CHalfLifeMultiplay :: SendMOTDToClient( CBasePlayer* pPlayer )
{
	// read from the MOTD.txt file
	int length, char_count = 0;
	char *pFileList;
	char *aFileList = pFileList = (char*)LOAD_FILE_FOR_ME( (char *)CVAR_GET_STRING( "motdfile" ), &length );

	// send the server name
	MESSAGE_BEGIN( MSG_ONE, gmsgServerName, NULL, pPlayer );
		WRITE_STRING( CVAR_GET_STRING("hostname") );
	MESSAGE_END();

	// Send the message of the day
	// read it chunk-by-chunk,  and send it in parts

	while ( pFileList && *pFileList && char_count < MAX_MOTD_LENGTH )
	{
		char chunk[MAX_MOTD_CHUNK+1];
		
		if ( strlen( pFileList ) < MAX_MOTD_CHUNK )
		{
			strcpy( chunk, pFileList );
		}
		else
		{
			strncpy( chunk, pFileList, MAX_MOTD_CHUNK );
			chunk[MAX_MOTD_CHUNK] = 0;		// strncpy doesn't always append the null terminator
		}

		char_count += strlen( chunk );
		if ( char_count < MAX_MOTD_LENGTH )
			pFileList = aFileList + char_count; 
		else
			*pFileList = 0;

		MESSAGE_BEGIN( MSG_ONE, gmsgMOTD, NULL, pPlayer );
			WRITE_BYTE( *pFileList ? 0 : 1 );	// 0 means there is still more message to come
			WRITE_STRING( chunk );
		MESSAGE_END();
	}

	FREE_FILE( aFileList );
}
	

