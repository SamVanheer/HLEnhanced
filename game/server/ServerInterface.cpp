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
#include "gamerules/GameRules.h"
#include "cbase.h"
#include "Server.h"
#include "client.h"
#include "pm_shared.h"
#include "materials/Materials.h"

#include "r_studioint.h"

#include "ServerEngineOverride.h"

#include "CStudioBlending.h"

#include "CMap.h"

#include "engine/saverestore/CSaveRestoreBuffer.h"
#include "engine/saverestore/CSave.h"
#include "engine/saverestore/CRestore.h"

#include "ServerInterface.h"

/**
*	Holds engine functionality callbacks. Can be overridden by the server to implement additional functionality on top of the engine.
*/
enginefuncs_t g_engfuncs;

/**
*	Holds engine functionality callbacks. Identical to g_engfuncs, except this will always point to the original functions.
*/
enginefuncs_t g_hlenginefuncs;

/**
*	Holds global variables shared between the engine and the server.
*/
globalvars_t* gpGlobals;

DLL_FUNCTIONS gFunctionTable =
{
	GameDLLInit,				//pfnGameInit
	DispatchSpawn,				//pfnSpawn
	DispatchThink,				//pfnThink
	DispatchUse,				//pfnUse
	DispatchTouch,				//pfnTouch
	DispatchBlocked,			//pfnBlocked
	DispatchKeyValue,			//pfnKeyValue
	DispatchSave,				//pfnSave
	DispatchRestore,			//pfnRestore
	DispatchObjectCollisionBox,	//pfnAbsBox

	SaveWriteFields,			//pfnSaveWriteFields
	SaveReadFields,				//pfnSaveReadFields

	SaveGlobalState,			//pfnSaveGlobalState
	RestoreGlobalState,			//pfnRestoreGlobalState
	ResetGlobalState,			//pfnResetGlobalState

	ClientConnect,				//pfnClientConnect
	ClientDisconnect,			//pfnClientDisconnect
	ClientKill,					//pfnClientKill
	ClientPutInServer,			//pfnClientPutInServer
	ClientCommand,				//pfnClientCommand
	ClientUserInfoChanged,		//pfnClientUserInfoChanged
	ServerActivate,				//pfnServerActivate
	ServerDeactivate,			//pfnServerDeactivate

	PlayerPreThink,				//pfnPlayerPreThink
	PlayerPostThink,			//pfnPlayerPostThink

	StartFrame,					//pfnStartFrame
	ParmsNewLevel,				//pfnParmsNewLevel
	ParmsChangeLevel,			//pfnParmsChangeLevel

	GetGameDescription,         //pfnGetGameDescription    Returns string describing current .dll game.
	PlayerCustomization,        //pfnPlayerCustomization   Notifies .dll of new customization for player.

	SpectatorConnect,			//pfnSpectatorConnect      Called when spectator joins server
	SpectatorDisconnect,        //pfnSpectatorDisconnect   Called when spectator leaves the server
	SpectatorThink,				//pfnSpectatorThink        Called when spectator sends a command packet (usercmd_t)

	Sys_Error,					//pfnSys_Error				Called when engine has encountered an error

	PM_Move,					//pfnPM_Move
	PM_Init,					//pfnPM_Init				Server version of player movement initialization
	PM_FindTextureType,			//pfnPM_FindTextureType

	SetupVisibility,			//pfnSetupVisibility        Set up PVS and PAS for networking for this client
	UpdateClientData,			//pfnUpdateClientData       Set up data sent only to specific client
	AddToFullPack,				//pfnAddToFullPack
	CreateBaseline,				//pfnCreateBaseline			Tweak entity baseline for network encoding, allows setup of player baselines, too.
	RegisterEncoders,			//pfnRegisterEncoders		Callbacks for network encoding
	GetWeaponData,				//pfnGetWeaponData
	CmdStart,					//pfnCmdStart
	CmdEnd,						//pfnCmdEnd
	ConnectionlessPacket,		//pfnConnectionlessPacket
	GetHullBounds,				//pfnGetHullBounds
	CreateInstancedBaselines,   //pfnCreateInstancedBaselines
	InconsistentFile,			//pfnInconsistentFile
	AllowLagCompensation,		//pfnAllowLagCompensation
};

NEW_DLL_FUNCTIONS gNewDLLFunctions =
{
	OnFreeEntPrivateData,
	GameDLLShutdown,
	ShouldCollide,
	CvarValue,
	CvarValue2
};

extern "C"
{
void GIVEFNPTRS_DLLEXPORT GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine, globalvars_t* pGlobals )
{
	memcpy( &g_engfuncs, pengfuncsFromEngine, sizeof( enginefuncs_t ) );
	memcpy( &g_hlenginefuncs, pengfuncsFromEngine, sizeof( enginefuncs_t ) );
	gpGlobals = pGlobals;

	engine::InitOverrides();
}

int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
	if( !pFunctionTable || interfaceVersion != INTERFACE_VERSION )
	{
		return false;
	}

	memcpy( pFunctionTable, &gFunctionTable, sizeof( DLL_FUNCTIONS ) );
	return true;
}

int GetEntityAPI2( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion )
{
	if( !pFunctionTable || *interfaceVersion != INTERFACE_VERSION )
	{
		// Tell engine what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return false;
	}

	memcpy( pFunctionTable, &gFunctionTable, sizeof( DLL_FUNCTIONS ) );
	return true;
}

int GetNewDLLFunctions( NEW_DLL_FUNCTIONS* pFunctionTable, int* pInterfaceVersion )
{
	if( !pFunctionTable || *pInterfaceVersion != NEW_DLL_FUNCTIONS_VERSION )
	{
		*pInterfaceVersion = NEW_DLL_FUNCTIONS_VERSION;
		return false;
	}

	memcpy( pFunctionTable, &gNewDLLFunctions, sizeof( gNewDLLFunctions ) );
	return true;
}

#if 0
int Server_GetBlendingInterface( int version, sv_blending_interface_t** ppInterface, server_studio_api_t* pStudio, Matrix3x4* pRotationMatrix, Matrix3x4* pBoneTransform )
{
	return g_StudioBlending.Initialize( version, ppInterface, pStudio, pRotationMatrix, pBoneTransform );
}
#endif
}

int DispatchSpawn( edict_t *pent )
{
	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pent );

	if( pEntity )
	{
		// Initialize these or entities who don't link to the world won't have anything in here
		pEntity->SetAbsMin( pEntity->GetAbsOrigin() - Vector( 1, 1, 1 ) );
		pEntity->SetAbsMax( pEntity->GetAbsOrigin() + Vector( 1, 1, 1 ) );

		pEntity->Spawn();

		// Try to get the pointer again, in case the spawn function deleted the entity.
		// UNDONE: Spawn() should really return a code to ask that the entity be deleted, but
		// that would touch too much code for me to do that right now.
		pEntity = ( CBaseEntity * ) GET_PRIVATE( pent );

		if( pEntity )
		{
			if( g_pGameRules && !g_pGameRules->IsAllowedToSpawn( pEntity ) )
				return -1;	// return that this entity should be deleted
			if( pEntity->GetFlags().Any(FL_KILLME ) )
				return -1;
		}


		// Handle global stuff here
		if( pEntity && pEntity->HasGlobalName() )
		{
			const globalentity_t *pGlobal = gGlobalState.EntityFromTable( MAKE_STRING( pEntity->GetGlobalName() ) );
			if( pGlobal )
			{
				// Already dead? delete
				if( pGlobal->state == GLOBAL_DEAD )
					return -1;
				else if( !FStrEq( STRING( gpGlobals->mapname ), pGlobal->levelName ) )
					pEntity->MakeDormant();	// Hasn't been moved to this level yet, wait but stay alive
											// In this level & not dead, continue on as normal
			}
			else
			{
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd( MAKE_STRING( pEntity->GetGlobalName() ), gpGlobals->mapname, GLOBAL_ON );
				//				ALERT( at_console, "Added global entity %s (%s)\n", pEntity->GetClassname(), pEntity->GetGlobalName() );
			}
		}

	}

	return 0;
}

void DispatchThink( edict_t *pent )
{
	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pent );
	if( pEntity )
	{
		if( pEntity->GetFlags().Any( FL_DORMANT ) )
			ALERT( at_error, "Dormant entity %s is thinking!!\n", pEntity->GetClassname() );

		pEntity->Think();
	}
}

void DispatchUse( edict_t *pentUsed, edict_t *pentOther )
{
	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pentUsed );
	CBaseEntity *pOther = ( CBaseEntity * ) GET_PRIVATE( pentOther );

	if( pEntity && !pEntity->GetFlags().Any( FL_KILLME ) )
		pEntity->Use( pOther, pOther, USE_TOGGLE, 0 );
}

// HACKHACK -- this is a hack to keep the node graph entity from "touching" things (like triggers)
// while it builds the graph
bool gTouchDisabled = false;
void DispatchTouch( edict_t *pentTouched, edict_t *pentOther )
{
	if( gTouchDisabled )
		return;

	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pentTouched );
	CBaseEntity *pOther = ( CBaseEntity * ) GET_PRIVATE( pentOther );

	if( pEntity && pOther && !( ( pEntity->GetFlags() | pOther->GetFlags() ) & FL_KILLME ) )
		pEntity->Touch( pOther );
}

void DispatchBlocked( edict_t *pentBlocked, edict_t *pentOther )
{
	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pentBlocked );
	CBaseEntity *pOther = ( CBaseEntity * ) GET_PRIVATE( pentOther );

	if( pEntity )
		pEntity->Blocked( pOther );
}

void DispatchKeyValue( edict_t *pentKeyvalue, KeyValueData *pkvd )
{
	if( !pkvd || !pentKeyvalue )
		return;

	EntvarsKeyvalue( VARS( pentKeyvalue ), pkvd );

	// If the key was an entity variable, or there's no class set yet, don't look for the object, it may
	// not exist yet.
	if( pkvd->fHandled || pkvd->szClassName == NULL )
		return;

	// Get the actualy entity object
	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pentKeyvalue );

	if( !pEntity )
		return;

	//See if the keyvalue is in the datadesc as a key.
	if( auto pDesc = UTIL_FindTypeDescInDataMap( *pEntity->GetDataMap(), pkvd->szKeyName, true ) )
	{
		if( pDesc->flags & TypeDescFlag::KEY )
		{
			if( UTIL_SetTypeDescValue( pEntity, *pDesc, pkvd->szValue ) )
			{
				pkvd->fHandled = true;
				return;
			}
		}
	}

	pEntity->KeyValue( pkvd );
}

void DispatchSave( edict_t *pent, SAVERESTOREDATA *pSaveData )
{
	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pent );

	if( pEntity && pSaveData )
	{
		ENTITYTABLE *pTable = &pSaveData->pTable[ pSaveData->currentIndex ];

		if( pTable->pent != pent )
			ALERT( at_error, "ENTITY TABLE OR INDEX IS WRONG!!!!\n" );

		if( pEntity->ObjectCaps() & FCAP_DONT_SAVE )
			return;

		// These don't use ltime & nextthink as times really, but we'll fudge around it.
		if( pEntity->GetMoveType() == MOVETYPE_PUSH )
		{
			float delta = pEntity->GetNextThink() - pEntity->GetLastThink();
			pEntity->SetLastThink( gpGlobals->time );
			pEntity->SetNextThink( pEntity->GetLastThink() + delta );
		}

		pTable->location = pSaveData->size;		// Remember entity position for file I/O
		pTable->classname = MAKE_STRING( pEntity->GetClassname() );	// Remember entity class for respawn

		CSave saveHelper( pSaveData );
		pEntity->Save( saveHelper );

		pTable->size = pSaveData->size - pTable->location;	// Size of entity block is data size written to block
	}
}

// Find the matching global entity.  Spit out an error if the designer made entities of
// different classes with the same global name
CBaseEntity *FindGlobalEntity( string_t classname, string_t globalname )
{
	CBaseEntity* pReturn = UTIL_FindEntityByString( nullptr, "globalname", STRING( globalname ) );

	if( pReturn )
	{
		if( !pReturn->ClassnameIs( STRING( classname ) ) )
		{
			ALERT( at_console, "Global entity found %s, wrong class %s\n", STRING( globalname ), pReturn->GetClassname() );
			pReturn = nullptr;
		}
	}

	return pReturn;
}

int DispatchRestore( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity )
{
	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pent );

	if( pEntity && pSaveData )
	{
		entvars_t tmpVars;
		Vector oldOffset;

		CRestore restoreHelper( pSaveData );
		if( globalEntity )
		{
			CRestore tmpRestore( pSaveData );
			tmpRestore.PrecacheMode( 0 );
			tmpRestore.ReadEntVars( "ENTVARS", &tmpVars );

			// HACKHACK - reset the save pointers, we're going to restore for real this time
			pSaveData->size = pSaveData->pTable[ pSaveData->currentIndex ].location;
			pSaveData->pCurrentData = pSaveData->pBaseData + pSaveData->size;
			// -------------------


			const globalentity_t *pGlobal = gGlobalState.EntityFromTable( tmpVars.globalname );

			// Don't overlay any instance of the global that isn't the latest
			// pSaveData->szCurrentMapName is the level this entity is coming from
			// pGlobla->levelName is the last level the global entity was active in.
			// If they aren't the same, then this global update is out of date.
			if( !FStrEq( pSaveData->szCurrentMapName, pGlobal->levelName ) )
				return 0;

			// Compute the new global offset
			oldOffset = pSaveData->vecLandmarkOffset;
			CBaseEntity *pNewEntity = FindGlobalEntity( tmpVars.classname, tmpVars.globalname );
			if( pNewEntity )
			{
				//				ALERT( at_console, "Overlay %s with %s\n", pNewEntity->GetClassname(), STRING(tmpVars.classname) );
				// Tell the restore code we're overlaying a global entity from another level
				restoreHelper.SetGlobalMode( 1 );	// Don't overwrite global fields
				pSaveData->vecLandmarkOffset = ( pSaveData->vecLandmarkOffset - pNewEntity->GetRelMin() ) + tmpVars.mins;
				pEntity = pNewEntity;// we're going to restore this data OVER the old entity
				pent = ENT( pEntity->pev );
				// Update the global table to say that the global definition of this entity should come from this level
				gGlobalState.EntityUpdate( MAKE_STRING( pEntity->GetGlobalName() ), gpGlobals->mapname );
			}
			else
			{
				// This entity will be freed automatically by the engine.  If we don't do a restore on a matching entity (below)
				// or call EntityUpdate() to move it to this level, we haven't changed global state at all.
				return 0;
			}

		}

		if( pEntity->ObjectCaps() & FCAP_MUST_SPAWN )
		{
			pEntity->Restore( restoreHelper );
			pEntity->Spawn();
		}
		else
		{
			pEntity->Restore( restoreHelper );
			pEntity->Precache();
		}

		// Again, could be deleted, get the pointer again.
		pEntity = ( CBaseEntity * ) GET_PRIVATE( pent );

#if 0
		if( pEntity && pEntity->HasGlobalName() && globalEntity )
		{
			ALERT( at_console, "Global %s is %s\n", pEntity->GetGlobalName(), pEntity->GetModelName() );
		}
#endif

		// Is this an overriding global entity (coming over the transition), or one restoring in a level
		if( globalEntity )
		{
			//			ALERT( at_console, "After: %f %f %f %s\n", pEntity->GetAbsOrigin().x, pEntity->GetAbsOrigin().y, pEntity->GetAbsOrigin().z, pEntity->GetModelName() );
			pSaveData->vecLandmarkOffset = oldOffset;
			if( pEntity )
			{
				pEntity->SetAbsOrigin( pEntity->GetAbsOrigin() );
				pEntity->OverrideReset();
			}
		}
		else if( pEntity && pEntity->HasGlobalName() )
		{
			const globalentity_t *pGlobal = gGlobalState.EntityFromTable( MAKE_STRING( pEntity->GetGlobalName() ) );
			if( pGlobal )
			{
				// Already dead? delete
				if( pGlobal->state == GLOBAL_DEAD )
					return -1;
				else if( !FStrEq( STRING( gpGlobals->mapname ), pGlobal->levelName ) )
				{
					pEntity->MakeDormant();	// Hasn't been moved to this level yet, wait but stay alive
				}
				// In this level & not dead, continue on as normal
			}
			else
			{
				ALERT( at_error, "Global Entity %s (%s) not in table!!!\n", pEntity->GetGlobalName(), pEntity->GetClassname() );
				// Spawned entities default to 'On'
				gGlobalState.EntityAdd( MAKE_STRING( pEntity->GetGlobalName() ), gpGlobals->mapname, GLOBAL_ON );
			}
		}
	}
	return 0;
}

void DispatchObjectCollisionBox( edict_t *pent )
{
	CBaseEntity *pEntity = ( CBaseEntity * ) GET_PRIVATE( pent );
	if( pEntity )
	{
		pEntity->SetObjectCollisionBox();
	}
	else
		SetObjectCollisionBox( &pent->v );
}

void SaveWriteFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, engine::TYPEDESCRIPTION *pFields, int fieldCount )
{
	engine::CSave saveHelper( pSaveData );
	saveHelper.WriteFields( pname, pBaseData, pFields, fieldCount );
}

void SaveReadFields( SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, engine::TYPEDESCRIPTION *pFields, int fieldCount )
{
	engine::CRestore restoreHelper( pSaveData );
	restoreHelper.ReadFields( pname, pBaseData, pFields, fieldCount );
}

void OnFreeEntPrivateData( edict_t* pEdict )
{
	if( pEdict && pEdict->pvPrivateData )
	{
		CBaseEntity* pEntity = GET_PRIVATE( pEdict );

		UTIL_DestructEntity( pEntity );
	}
}

int ShouldCollide( edict_t *pentTouched, edict_t *pentOther )
{
	return 1;
}

void CvarValue( const edict_t *pEnt, const char *value )
{
}

void CvarValue2( const edict_t *pEnt, int requestID, const char *cvarName, const char *value )
{
}

#ifdef WIN32
#include "DelayLoad.h"

//See DelayHook in DelayLoad.cpp
HMODULE DelayLoad_LoadDelayLoadLib( unsigned dliNotify, PDelayLoadInfo pdli )
{
	if( strcmp( pdli->szDll, "sqlite3.dll" ) == 0 )
	{
		return DelayLoad_LoadGameLib( pdli->szDll );
	}

	return nullptr;
}
#endif
