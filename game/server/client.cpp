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
// Robin, 4-22-98: Moved set_suicide_frame() here from player.cpp to allow us to 
//				   have one without a hardcoded player.mdl in tf_client.cpp

/*

===== client.cpp ========================================================

  client/server game specific stuff

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "Weapons.h"
#include "entities/CBaseSpectator.h"
#include "entities/CCorpse.h"
#include "entities/CSoundEnt.h"
#include "entities/weapons/CRpg.h"

#include "gamerules/GameRules.h"
#include "customentity.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "pm_shared.h"
#include "entity_state.h"

#include "Server.h"
#include "UTFUtils.h"

#include "CServerGameInterface.h"

#include "voice_gamemgr.h"

#include "client.h"

#if USE_ANGELSCRIPT
#include "Angelscript/ScriptAPI/ASEvents.h"
#include "Angelscript/ScriptAPI/CASSayArgs.h"
#endif

extern CVoiceGameMgr g_VoiceGameMgr;

//TODO: split this file into pieces - Solokiller

extern int g_teamplay;

qboolean ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ CCONNECT_REJECT_REASON_SIZE ] )
{	
	return g_Server.ClientConnect( pEntity, pszAddress, pszAddress, szRejectReason );
}

void ClientDisconnect( edict_t* pEdict )
{
	g_Server.ClientDisconnect( pEdict );
}

void ClientKill( edict_t* pEdict )
{
	g_Server.ClientKill( pEdict );
}

void ClientPutInServer( edict_t* pEntity )
{
	g_Server.ClientPutInServer( pEntity );
}

//// HOST_SAY
// String comes in as
// say blah blah blah
// or as
// blah blah blah
//
void Host_Say( CBasePlayer* pPlayer, const bool bTeamOnly )
{
	if( !pPlayer )
		return;

	CBasePlayer *client;
	int		j;
	char	*p;
	char	text[128];
	char    szTemp[256];
	const char *cpSay = "say";
	const char *cpSayTeam = "say_team";
	const char *pcmd = CMD_ARGV(0);

	// We can get a raw string now, without the "say " prepended
	if ( CMD_ARGC() == 0 )
		return;

	//Not yet.
	if ( pPlayer->m_flNextChatTime > gpGlobals->time )
		 return;

	if ( !stricmp( pcmd, cpSay) || !stricmp( pcmd, cpSayTeam ) )
	{
		if ( CMD_ARGC() >= 2 )
		{
			p = (char *)CMD_ARGS();
		}
		else
		{
			// say with a blank message, nothing to do
			return;
		}
	}
	else  // Raw text, need to prepend argv[0]
	{
		if ( CMD_ARGC() >= 2 )
		{
			sprintf( szTemp, "%s %s", ( char * )pcmd, (char *)CMD_ARGS() );
		}
		else
		{
			// Just a one word command, use the first word...sigh
			sprintf( szTemp, "%s", ( char * )pcmd );
		}
		p = szTemp;
	}

// remove quotes if present
	if( p && *p == '"' )
	{
		p++;
		p[strlen(p)-1] = 0;
	}

// make sure the text has content

	if ( !p || !p[0] || !Q_UnicodeValidate ( p ) )
		return;  // no character found, so say nothing

#if USE_ANGELSCRIPT
	{
		CASSayArgs* pArgs = new CASSayArgs( pPlayer, bTeamOnly ? SayType::SAY_TEAM : SayType::SAY, p );

		CallGlobalEvent( g_PlayerSayEvent, CallFlag::NONE, pArgs );

		const bool bShouldHide = pArgs->ShouldHide();

		pArgs->Release();

		if( bShouldHide )
			return;
	}
#endif

// turn on color set 2  (color on,  no sound)
	// turn on color set 2  (color on,  no sound)
	if ( pPlayer->IsObserver() && ( bTeamOnly ) )
		sprintf( text, "%c(SPEC) %s: ", 2, pPlayer->GetNetName() );
	else if ( bTeamOnly )
		sprintf( text, "%c(TEAM) %s: ", 2, pPlayer->GetNetName() );
	else
		sprintf( text, "%c%s: ", 2, pPlayer ->GetNetName() );

	j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
	if ( (int)strlen(p) > j )
		p[j] = 0;

	strcat( text, p );
	strcat( text, "\n" );


	pPlayer->m_flNextChatTime = gpGlobals->time + CHAT_INTERVAL;

	// loop through all players
	// Start with the first player.
	// This may return the world in single player if the client types something between levels or during spawn
	// so check it, or it will infinite loop

	client = NULL;
	while ( ((client = (CBasePlayer*)UTIL_FindEntityByClassname( client, "player" )) != NULL) && (!FNullEnt(client->edict())) ) 
	{
		if ( !client->pev )
			continue;
		
		if ( client == pPlayer )
			continue;

		if ( !(client->IsNetClient()) )	// Not a client ? (should never be true)
			continue;

		// can the receiver hear the sender? or has he muted him?
		if ( g_VoiceGameMgr.PlayerHasBlockedPlayer( client, pPlayer ) )
			continue;

		if ( !pPlayer->IsObserver() && bTeamOnly && g_pGameRules->PlayerRelationship( client, pPlayer ) != GR_TEAMMATE )
			continue;

		// Spectators can only talk to other specs
		if ( pPlayer->IsObserver() && bTeamOnly )
			if ( !client->IsObserver() )
				continue;

		MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, client );
			WRITE_BYTE( pPlayer ->entindex() );
			WRITE_STRING( text );
		MESSAGE_END();

	}

	// print to the sending client
	MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, pPlayer );
		WRITE_BYTE( pPlayer->entindex() );
		WRITE_STRING( text );
	MESSAGE_END();

	// echo to server console
	g_engfuncs.pfnServerPrint( text );

	const char* pszTemp;
	if ( bTeamOnly )
		pszTemp = "say_team";
	else
		pszTemp = "say";
	
	// team match?
	if ( g_teamplay )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" %s \"%s\"\n", 
			pPlayer->GetNetName(),
			UTIL_GetPlayerUserId( pPlayer ),
			UTIL_GetPlayerAuthId( pPlayer ),
			g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "model" ),
			pszTemp,
			p );
	}
	else
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%i>\" %s \"%s\"\n", 
			pPlayer->GetNetName(),
			UTIL_GetPlayerUserId( pPlayer ),
			UTIL_GetPlayerAuthId( pPlayer ),
			UTIL_GetPlayerUserId( pPlayer ),
			pszTemp,
			p );
	}
}

void ClientCommand( edict_t* pEntity )
{
	g_Server.ClientCommand( pEntity );
}

void ClientUserInfoChanged( edict_t* pEntity, char* infobuffer )
{
	g_Server.ClientUserInfoChanged( pEntity, infobuffer );
}

void ServerDeactivate()
{
	g_Server.Deactivate();
}

void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	g_Server.Activate( pEdictList, edictCount, clientMax );
}

void PlayerPreThink( edict_t *pEntity )
{
	if( auto pPlayer = ( CBasePlayer* ) GET_PRIVATE( pEntity ) )
		pPlayer->PreThink();
}

void PlayerPostThink( edict_t *pEntity )
{
	if( auto pPlayer = ( CBasePlayer* ) GET_PRIVATE( pEntity ) )
		pPlayer->PostThink();
}

void ParmsNewLevel()
{
	g_Server.ParmsNewLevel();
}

void ParmsChangeLevel()
{
	g_Server.ParmsChangeLevel();
}

void StartFrame()
{
	g_Server.StartFrame();
}

void ClientPrecache()
{
	g_Server.ClientPrecache();
}

const char* GetGameDescription()
{
	return g_Server.GetGameDescription();
}

void Sys_Error( const char *error_string )
{
	g_Server.Sys_Error( error_string );
}

void PlayerCustomization( edict_t* pEntity, customization_t* pCust )
{
	g_Server.PlayerCustomization( pEntity, pCust );
}

void SpectatorConnect( edict_t* pEntity )
{
	g_Server.SpectatorConnect( pEntity );
}

void SpectatorDisconnect( edict_t* pEntity )
{
	g_Server.SpectatorDisconnect( pEntity );
}

void SpectatorThink( edict_t* pEntity )
{
	g_Server.SpectatorThink( pEntity );
}

////////////////////////////////////////////////////////
// PAS and PVS routines for client messaging
//

/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
*/
void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas )
{
	Vector org;
	edict_t *pView = pClient;

	// Find the client's PVS
	if ( pViewEntity )
	{
		pView = pViewEntity;
	}

	if ( pClient->v.flags & FL_PROXY )
	{
		*pvs = NULL;	// the spectator proxy sees
		*pas = NULL;	// and hears everything
		return;
	}

	org = pView->v.origin + pView->v.view_ofs;
	if ( pView->v.flags & FL_DUCKING )
	{
		org = org + ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
	}

	*pvs = ENGINE_SET_PVS ( org );
	*pas = ENGINE_SET_PAS ( org );
}

/*
AddToFullPack

Return 1 if the entity state has been filled in for the ent and the entity will be propagated to the client, 0 otherwise

state is the server maintained copy of the state info that is transmitted to the client
a MOD could alter values copied into state to send the "host" a different look for a particular entity update, etc.
e and ent are the entity that is being added to the update, if 1 is returned
host is the player's edict of the player whom we are sending the update to
player is 1 if the ent/e is a player and 0 otherwise
pSet is either the PAS or PVS that we previous set up.  We can use it to ask the engine to filter the entity against the PAS or PVS.
we could also use the pas/ pvs that we set in SetupVisibility, if we wanted to.  Caching the value is valid in that case, but still only for the current frame
*/
int AddToFullPack( entity_state_t *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet )
{
	int					i;

	// don't send if flagged for NODRAW and it's not the host getting the message
	if ( ( ent->v.effects & EF_NODRAW ) &&
		 ( ent != host ) )
		return 0;

	// Ignore ents without valid / visible models
	if ( !ent->v.modelindex || !STRING( ent->v.model ) )
		return 0;

	// Don't send spectators to other players
	if ( ( ent->v.flags & FL_SPECTATOR ) && ( ent != host ) )
	{
		return 0;
	}

	// Ignore if not the host and not touching a PVS/PAS leaf
	// If pSet is NULL, then the test will always succeed and the entity will be added to the update
	if ( ent != host )
	{
		if ( !ENGINE_CHECK_VISIBILITY( ent, pSet ) )
		{
			return 0;
		}
	}


	// Don't send entity to local client if the client says it's predicting the entity itself.
	if ( ent->v.flags & FL_SKIPLOCALHOST )
	{
		if ( ( hostflags & HOSTFL_WEAPONPRED ) && ( ent->v.owner == host ) )
			return 0;
	}
	
	if ( host->v.groupinfo )
	{
		UTIL_SetGroupTrace( host->v.groupinfo, GROUP_OP_AND );

		// Should always be set, of course
		if ( ent->v.groupinfo )
		{
			if ( g_groupop == GROUP_OP_AND )
			{
				if ( !(ent->v.groupinfo & host->v.groupinfo ) )
					return 0;
			}
			else if ( g_groupop == GROUP_OP_NAND )
			{
				if ( ent->v.groupinfo & host->v.groupinfo )
					return 0;
			}
		}

		UTIL_UnsetGroupTrace();
	}

	memset( state, 0, sizeof( *state ) );

	// Assign index so we can track this entity from frame to frame and
	//  delta from it.
	state->number	  = e;
	state->entityType = ENTITY_NORMAL;
	
	// Flag custom entities.
	if ( ent->v.flags & FL_CUSTOMENTITY )
	{
		state->entityType = ENTITY_BEAM;
	}

	// 
	// Copy state data
	//

	// Round animtime to nearest millisecond
	state->animtime   = (int)(1000.0 * ent->v.animtime ) / 1000.0;

	memcpy( state->origin, ent->v.origin, 3 * sizeof( float ) );
	memcpy( state->angles, ent->v.angles, 3 * sizeof( float ) );
	memcpy( state->mins, ent->v.mins, 3 * sizeof( float ) );
	memcpy( state->maxs, ent->v.maxs, 3 * sizeof( float ) );

	memcpy( state->startpos, ent->v.startpos, 3 * sizeof( float ) );
	memcpy( state->endpos, ent->v.endpos, 3 * sizeof( float ) );

	state->impacttime = ent->v.impacttime;
	state->starttime = ent->v.starttime;

	state->modelindex = ent->v.modelindex;
		
	state->frame      = ent->v.frame;

	state->skin       = ent->v.skin;
	state->effects    = ent->v.effects;

	// This non-player entity is being moved by the game .dll and not the physics simulation system
	//  make sure that we interpolate it's position on the client if it moves
	if ( !player &&
		 ent->v.animtime &&
		 ent->v.velocity[ 0 ] == 0 && 
		 ent->v.velocity[ 1 ] == 0 && 
		 ent->v.velocity[ 2 ] == 0 )
	{
		state->eflags |= EFLAG_SLERP;
	}

	state->scale	  = ent->v.scale;
	state->solid	  = ent->v.solid;
	state->colormap   = ent->v.colormap;

	state->movetype   = ent->v.movetype;
	state->sequence   = ent->v.sequence;
	state->framerate  = ent->v.framerate;
	state->body       = ent->v.body;

	for (i = 0; i < 4; i++)
	{
		state->controller[i] = ent->v.controller[i];
	}

	for (i = 0; i < 2; i++)
	{
		state->blending[i]   = ent->v.blending[i];
	}

	state->rendermode    = ent->v.rendermode;
	state->renderamt     = ent->v.renderamt; 
	state->renderfx      = ent->v.renderfx;
	state->rendercolor.r = ent->v.rendercolor.x;
	state->rendercolor.g = ent->v.rendercolor.y;
	state->rendercolor.b = ent->v.rendercolor.z;

	state->aiment = 0;
	if ( ent->v.aiment )
	{
		state->aiment = ENTINDEX( ent->v.aiment );
	}

	state->owner = 0;
	if ( ent->v.owner )
	{
		int owner = ENTINDEX( ent->v.owner );
		
		// Only care if owned by a player
		if ( owner >= 1 && owner <= gpGlobals->maxClients )
		{
			state->owner = owner;	
		}
	}

	// HACK:  Somewhat...
	// Class is overridden for non-players to signify a breakable glass object ( sort of a class? )
	if ( !player )
	{
		state->playerclass  = ent->v.playerclass;
	}

	// Special stuff for players only
	if ( player )
	{
		memcpy( state->basevelocity, ent->v.basevelocity, 3 * sizeof( float ) );

		state->weaponmodel  = MODEL_INDEX( STRING( ent->v.weaponmodel ) );
		state->gaitsequence = ent->v.gaitsequence;
		state->spectator = ent->v.flags & FL_SPECTATOR;
		state->friction     = ent->v.friction;

		state->gravity      = ent->v.gravity;
//		state->team			= ent->v.team;
//		
		state->usehull      = ( ent->v.flags & FL_DUCKING ) ? 1 : 0;
		state->health		= ent->v.health;
	}

	return 1;
}

/*
===================
CreateBaseline

Creates baselines used for network encoding, especially for player data since players are not spawned until connect time.
===================
*/
void CreateBaseline( int player, int eindex, entity_state_t *baseline, edict_t* entity, int playermodelindex,
					 const Vector player_mins[ Hull::COUNT ], const Vector player_maxs[ Hull::COUNT ] )
{
	baseline->origin		= entity->v.origin;
	baseline->angles		= entity->v.angles;
	baseline->frame			= entity->v.frame;
	baseline->skin			= (short)entity->v.skin;

	// render information
	baseline->rendermode	= (byte)entity->v.rendermode;
	baseline->renderamt		= (byte)entity->v.renderamt;
	baseline->rendercolor.r	= (byte)entity->v.rendercolor.x;
	baseline->rendercolor.g	= (byte)entity->v.rendercolor.y;
	baseline->rendercolor.b	= (byte)entity->v.rendercolor.z;
	baseline->renderfx		= (byte)entity->v.renderfx;

	if ( player )
	{
		baseline->mins			= player_mins[ 0 ];
		baseline->maxs			= player_maxs[ 0 ];

		baseline->colormap		= eindex;
		baseline->modelindex	= playermodelindex;
		baseline->friction		= 1.0;
		baseline->movetype		= MOVETYPE_WALK;

		baseline->scale			= entity->v.scale;
		baseline->solid			= SOLID_SLIDEBOX;
		baseline->framerate		= 1.0;
		baseline->gravity		= 1.0;

	}
	else
	{
		baseline->mins			= entity->v.mins;
		baseline->maxs			= entity->v.maxs;

		baseline->colormap		= 0;
		baseline->modelindex	= entity->v.modelindex;//SV_ModelIndex(pr_strings + entity->v.model);
		baseline->movetype		= entity->v.movetype;

		baseline->scale			= entity->v.scale;
		baseline->solid			= entity->v.solid;
		baseline->framerate		= entity->v.framerate;
		baseline->gravity		= entity->v.gravity;
	}
}

struct entity_field_alias_t
{
	char name[32];
	int	 field;
};

#define FIELD_ORIGIN0			0
#define FIELD_ORIGIN1			1
#define FIELD_ORIGIN2			2
#define FIELD_ANGLES0			3
#define FIELD_ANGLES1			4
#define FIELD_ANGLES2			5

static entity_field_alias_t entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
};

void Entity_FieldInit( delta_t *pFields )
{
	entity_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN0 ].name );
	entity_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN1 ].name );
	entity_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN2 ].name );
	entity_field_alias[ FIELD_ANGLES0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES0 ].name );
	entity_field_alias[ FIELD_ANGLES1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES1 ].name );
	entity_field_alias[ FIELD_ANGLES2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES2 ].name );
}

/*
==================
Entity_Encode

Callback for sending entity_state_t info over network. 
FIXME:  Move to script
==================
*/
void Entity_Encode( delta_t *pFields, const unsigned char *from, const unsigned char *to )
{
	static bool initialized = false;

	if ( !initialized )
	{
		Entity_FieldInit( pFields );
		initialized = true;
	}

	entity_state_t* f = (entity_state_t *)from;
	entity_state_t* t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	const int localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->impacttime != 0 ) && ( t->starttime != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );

		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

static entity_field_alias_t player_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
};

void Player_FieldInit( delta_t *pFields )
{
	player_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN0 ].name );
	player_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN1 ].name );
	player_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN2 ].name );
}

/*
==================
Player_Encode

Callback for sending entity_state_t for players info over network. 
==================
*/
void Player_Encode( delta_t *pFields, const unsigned char *from, const unsigned char *to )
{
	static bool initialized = false;

	if ( !initialized )
	{
		Player_FieldInit( pFields );
		initialized = true;
	}

	entity_state_t* f = (entity_state_t *)from;
	entity_state_t* t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	const int localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

#define CUSTOMFIELD_ORIGIN0			0
#define CUSTOMFIELD_ORIGIN1			1
#define CUSTOMFIELD_ORIGIN2			2
#define CUSTOMFIELD_ANGLES0			3
#define CUSTOMFIELD_ANGLES1			4
#define CUSTOMFIELD_ANGLES2			5
#define CUSTOMFIELD_SKIN			6
#define CUSTOMFIELD_SEQUENCE		7
#define CUSTOMFIELD_ANIMTIME		8

entity_field_alias_t custom_entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
	{ "skin",				0 },
	{ "sequence",			0 },
	{ "animtime",			0 },
};

void Custom_Entity_FieldInit( delta_t *pFields )
{
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].name );
}

/*
==================
Custom_Encode

Callback for sending entity_state_t info ( for custom entities ) over network. 
FIXME:  Move to script
==================
*/
void Custom_Encode( delta_t *pFields, const unsigned char *from, const unsigned char *to )
{
	static bool initialized = false;

	if ( !initialized )
	{
		Custom_Entity_FieldInit( pFields );
		initialized = true;
	}

	entity_state_t* f = (entity_state_t *)from;
	entity_state_t* t = (entity_state_t *)to;

	const int beamType = t->rendermode & 0x0f;
		
	if ( beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field );
	}

	if ( beamType != BEAM_POINTS )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field );
	}

	if ( beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field );
	}

	// animtime is compared by rounding first
	// see if we really shouldn't actually send it
	if ( (int)f->animtime == (int)t->animtime )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field );
	}
}

/*
=================
RegisterEncoders

Allows game .dll to override network encoding of certain types of entities and tweak values, etc.
=================
*/
void RegisterEncoders()
{
	DELTA_ADDENCODER( "Entity_Encode", Entity_Encode );
	DELTA_ADDENCODER( "Custom_Encode", Custom_Encode );
	DELTA_ADDENCODER( "Player_Encode", Player_Encode );
}

int GetWeaponData( edict_t* pEntity, weapon_data_t* pInfo )
{
#if defined( CLIENT_WEAPONS )
	weapon_data_t *item;
	auto pPlayer = static_cast<CBasePlayer*>( CBasePlayer::Instance( pEntity ) );

	memset( pInfo, 0, MAX_WEAPONS * sizeof( weapon_data_t ) );

	if ( !pPlayer )
		return true;

	// go through all of the weapons and make a list of the ones to pack
	for ( int i = 0 ; i < MAX_WEAPON_SLOTS; i++ )
	{
		if ( pPlayer->m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerWeapon *pPlayerItem = pPlayer->m_rgpPlayerItems[ i ];

			while ( pPlayerItem )
			{
				if ( pPlayerItem->IsPredicted() )
				{
					if ( pPlayerItem->m_iId >= 0 && pPlayerItem->m_iId < MAX_WEAPONS )
					{
						item = &pInfo[ pPlayerItem->m_iId ];
					 	
						item->m_iId = pPlayerItem->m_iId;

						pPlayerItem->GetWeaponData( *item );
					}
				}
				pPlayerItem = pPlayerItem->m_pNext;
			}
		}
	}
#else
	memset( info, 0, MAX_WEAPONS * sizeof( weapon_data_t ) );
#endif
	return true;
}

/*
=================
UpdateClientData

Data sent to current client only
engine sets cd to 0 before calling.
=================
*/
void UpdateClientData( const edict_t* pClient, int sendweapons, clientdata_t* cd )
{
	if ( !pClient || !pClient->pvPrivateData )
		return;
	entvars_t*		pev	= const_cast<entvars_t*>( &pClient->v );
	CBasePlayer*	pl	= static_cast<CBasePlayer*>(CBasePlayer::Instance( pev ) );
	entvars_t*		pevOrg = nullptr;

	// if user is spectating different player in First person, override some vars
	if ( pl && pl->GetObserverMode() == OBS_IN_EYE )
	{
		if ( pl->m_hObserverTarget )
		{
			pevOrg = pev;
			pev = pl->m_hObserverTarget->pev;
			pl = static_cast<CBasePlayer*>(CBasePlayer::Instance( pev ) );
		}
	}

	cd->flags			= pl->GetFlags().Get();
	cd->health			= pl->GetHealth();

	cd->viewmodel		= MODEL_INDEX( pl->GetViewModelName() );

	cd->waterlevel		= pl->GetWaterLevel();
	cd->watertype		= pl->GetWaterType();
	cd->weapons			= pl->GetWeapons().Get();

	// Vectors
	cd->origin			= pl->GetAbsOrigin();
	cd->velocity		= pl->GetAbsVelocity();
	cd->view_ofs		= pl->GetViewOffset();
	cd->punchangle		= pl->GetPunchAngle();

	//This is only touched in pm_shared code, but it gets synced in the engine during SV_RunCmd - Solokiller
	cd->bInDuck			= pl->IsDucking();
	cd->flTimeStepSound = pl->GetStepSoundTime();
	cd->flDuckTime		= pl->GetDuckTime();
	cd->flSwimTime		= pl->GetSwimSoundTime();
	cd->waterjumptime	= pl->GetWaterJumpTime();

	strcpy( cd->physinfo, ENGINE_GETPHYSINFO( pClient ) );

	cd->maxspeed		= pl->GetMaxSpeed();
	cd->fov				= pl->GetFOV();
	cd->weaponanim		= pl->GetWeaponAnim();

	cd->pushmsec		= pev->pushmsec;

	//Spectator mode
	//TODO: this is kinda stupid, just always set pevOrg and use it for this - Solokiller
	if ( pevOrg != NULL )
	{
		// don't use spec vars from chased player
		cd->iuser1			= pevOrg->iuser1;
		cd->iuser2			= pevOrg->iuser2;
	}
	else
	{
		cd->iuser1			= pev->iuser1;
		cd->iuser2			= pev->iuser2;
	}

	

#if defined( CLIENT_WEAPONS )
	if ( sendweapons )
	{
		if ( pl )
		{
			cd->m_flNextAttack	= pl->m_flNextAttack;
			cd->fuser2			= pl->m_flNextAmmoBurn;
			cd->fuser3			= pl->m_flAmmoStartCharge;
			cd->vuser1.x		= pl->GetAmmoCount( "9mm" );
			cd->vuser1.y		= pl->GetAmmoCount( "357" );
			cd->vuser1.z		= pl->GetAmmoCount( "ARgrenades" );
			cd->ammo_nails		= pl->GetAmmoCount( "bolts" );
			cd->ammo_shells		= pl->GetAmmoCount( "buckshot" );
			cd->ammo_rockets	= pl->GetAmmoCount( "rockets" );
			cd->ammo_cells		= pl->GetAmmoCount( "uranium" );
			cd->vuser2.x		= pl->GetAmmoCount( "Hornets" );
#ifdef GAME_OPFOR
			cd->vuser3.x		= pl->GetAmmoCount( "762" );
			cd->vuser3.y		= pl->GetAmmoCount( "556" );
			cd->vuser3.z		= pl->GetAmmoCount( "shock_rounds" );
			cd->vuser4.x		= pl->GetAmmoCount( "spores" );
#endif	

			if ( pl->m_pActiveItem )
			{
				if ( pl->m_pActiveItem->IsPredicted() )
				{
					const CWeaponInfo* pInfo = pl->m_pActiveItem->GetWeaponInfo();

					cd->m_iId = pl->m_pActiveItem->m_iId;

					if( auto pPrimaryAmmo = pInfo->GetPrimaryAmmo() )
						cd->vuser4.y = pl->m_rgAmmo[ pPrimaryAmmo->GetID() ];

					if( auto pSecondaryAmmo = pInfo->GetSecondaryAmmo() )
						cd->vuser4.z = pl->m_rgAmmo[ pSecondaryAmmo->GetID() ];
					
					if ( pl->m_pActiveItem->m_iId == WEAPON_RPG )
					{
						if( !ENGINE_CANSKIP( pClient ) )
						{
							cd->vuser2.y = ( ( CRpg * )pl->m_pActiveItem)->m_bSpotActive;
						}
						cd->vuser2.z = ( ( CRpg * )pl->m_pActiveItem)->m_cActiveRockets;
					}
				}
			}
		}
	} 
#endif
}

/*
=================
CmdStart

We're about to run this usercmd for the specified player.  We can set up groupinfo and masking here, etc.
This is the time to examine the usercmd for anything extra.  This call happens even if think does not.
=================
*/
void CmdStart( const edict_t* player, const usercmd_t* cmd, unsigned int random_seed )
{
	auto pPlayer = static_cast<CBasePlayer*>( CBasePlayer::Instance( const_cast<edict_t*>( player ) ) );

	if( !pPlayer )
		return;

	if ( pPlayer->pev->groupinfo != 0 )
	{
		UTIL_SetGroupTrace( pPlayer->pev->groupinfo, GROUP_OP_AND );
	}

	pPlayer->random_seed = random_seed;
}

/*
=================
CmdEnd

Each cmdstart is exactly matched with a cmd end, clean up any group trace flags, etc. here
=================
*/
void CmdEnd ( const edict_t* player )
{
	auto pPlayer = static_cast<CBasePlayer*>( CBasePlayer::Instance( const_cast<edict_t*>( player ) ) );

	if( !pPlayer )
		return;

	if ( pPlayer->pev->groupinfo != 0 )
	{
		UTIL_UnsetGroupTrace();
	}
}

/*
================================
ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	ConnectionlessPacket( const netadr_t *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	/*int max_buffer_size = *response_buffer_size;*/

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

/*
================================
GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int GetHullBounds( int hullnumber, Vector& mins, Vector& maxs )
{
	bool bResult = false;

	//TODO: these hull sizes don't match the names for the hulls. Why? - Solokiller
	switch ( hullnumber )
	{
	case Hull::POINT:				// Normal player
		mins = VEC_HULL_MIN;
		maxs = VEC_HULL_MAX;
		bResult = true;
		break;
	case Hull::HUMAN:				// Crouched player
		mins = VEC_DUCK_HULL_MIN;
		maxs = VEC_DUCK_HULL_MAX;
		bResult = true;
		break;
	case Hull::LARGE:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		bResult = true;
		break;
	}

	return bResult;
}

/*
================================
CreateInstancedBaselines

Create pseudo-baselines for items that aren't placed in the map at spawn time, but which are likely
to be created during play ( e.g., grenades, ammo packs, projectiles, corpses, etc. )
================================
*/
void CreateInstancedBaselines()
{
	//int iret = 0;
	entity_state_t state;

	memset( &state, 0, sizeof( state ) );

	// Create any additional baselines here for things like grenades, etc.
	// iret = ENGINE_INSTANCE_BASELINE( MAKE_STRING( pc->GetClassname() ), &state );

	// Destroy objects.
	//UTIL_Remove( pc );
}

/*
================================
InconsistentFile

One of the ENGINE_FORCE_UNMODIFIED files failed the consistency check for the specified player
 Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
================================
*/
int	InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message )
{
	// Server doesn't care?
	if ( CVAR_GET_FLOAT( "mp_consistency" ) != 1 )
		return false;

	// Default behavior is to kick the player
	sprintf( disconnect_message, "Server is enforcing file consistency for %s\n", filename );

	// Kick now with specified disconnect message.
	return true;
}

/*
================================
AllowLagCompensation

 The game .dll should return 1 if lag compensation should be allowed ( could also just set
  the sv_unlag cvar.
 Most games right now should return 0, until client-side weapon prediction code is written
  and tested for them ( note you can predict weapons, but not do lag compensation, too, 
  if you want.
================================
*/
int AllowLagCompensation()
{
	return 1;
}
