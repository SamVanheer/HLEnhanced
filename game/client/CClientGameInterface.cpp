#include "hud.h"
#include "cl_util.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "Weapons.h"

#include "CWeaponInfoCache.h"

#include "hl/hl_weapons.h"
#include "com_weapons.h"
#include "com_model.h"

#include "ammohistory.h"

#include "BSPIO.h"

#if USE_ANGELSCRIPT
#include "Angelscript/CHLASClientManager.h"
#endif

#include "hl/CClientPrediction.h"

#include "CClientGameInterface.h"

CClientGameInterface g_Client;

/**
*	Parses in the map's data and gets the map script from it. - Solokiller
*/
const char* ParseMapDataCallback( const char* pszBuffer, bool& bError )
{
	char szKey[ MAX_COM_TOKEN ];
	char token[ MAX_COM_TOKEN ];

	char szMapScript[ MAX_PATH ] = {};

	bool bIsWorldspawn = false;

	bool bEnd = false;

	while( true )
	{
		pszBuffer = bsp::ParseKeyValue( pszBuffer, szKey, sizeof( szKey ), token, sizeof( token ), bError, bEnd );

		if( bEnd )
			break;

		if( bError )
			return pszBuffer;

		if( strcmp( szKey, "mapscript" ) == 0 )
		{
			strncpy( szMapScript, token, sizeof( szMapScript ) );
			szMapScript[ sizeof( szMapScript ) - 1 ] = '\0';
		}
		else if( strcmp( szKey, "classname" ) == 0 && strcmp( token, "worldspawn" ) == 0 )
		{
			bIsWorldspawn = true;
		}
	}

	if( bIsWorldspawn && *szMapScript )
	{
#if USE_ANGELSCRIPT
		g_ASManager.WorldCreated( szMapScript );
#endif
	}

	return pszBuffer;
}

bool CClientGameInterface::LibInit( CreateInterfaceFn* pFactories, size_t uiNumFactories )
{
	if( !InitializeCommon( pFactories, uiNumFactories ) )
		return false;

	//Init ASAP so functions like AlertMessage get set up.
	CL_SetupServerSupport();

#if USE_ANGELSCRIPT
	if( !g_ASManager.Initialize() )
		return false;
#endif

	if( !PostInit() )
		return false;

	return true;
}

void CClientGameInterface::LibShutdown()
{
#if USE_ANGELSCRIPT
	g_ASManager.Shutdown();
#endif

	Hud().GameShutdown();

	ShutdownCommon();
}

bool CClientGameInterface::ConnectionEstablished()
{
	m_bNewMapStarted = true;
	m_bParseMapData = true;

	return true;
}

void CClientGameInterface::Frame( double flTime )
{
	if( m_bParseMapData )
	{
		cl_entity_t* pWorldModel = gEngfuncs.GetEntityByIndex( 0 );

		if( pWorldModel && pWorldModel->model )
		{
			m_bParseMapData = false;
			MapInit( pWorldModel );
		}
	}
}

void CClientGameInterface::CAM_Think()
{
	//This is the earliest unconditional client library callback that gets called after a connection to a server has been established.
	//It's the first chance we have to get the map name. - Solokiller
	CheckNewMapStarted();
}

void CClientGameInterface::NewMapStarted( const char* const pszMapName, const char* const pszLevelName )
{
	//Set the map name so scripts can access it the same way they can on the server. - Solokiller
	gpGlobals->mapname = MAKE_STRING( g_StringPool.Allocate( pszMapName ) );

	g_Prediction.NewMapStarted();
}

void CClientGameInterface::MapInit( cl_entity_t* pWorldModel )
{
	RegisterAmmoTypes();
	g_Prediction.MapInit();
	PrecacheWeapons();

	//Parse in map data now, since the map has been downloaded. - Solokiller
	bsp::ProcessEnts( pWorldModel->model->entities, ParseMapDataCallback );

	//TODO: call map script MapInit here - Solokiller

	g_Prediction.SetupWeapons();

	const size_t uiClientAmmoHash = g_AmmoTypes.GenerateHash();
	const size_t uiClientWeaponHash = g_WeaponInfoCache.GenerateHash();

	gEngfuncs.pfnServerCmd( UTIL_VarArgs( "WpnInfo %u %u %u %u\n", g_AmmoTypes.GetAmmoTypesCount(), uiClientAmmoHash, g_WeaponInfoCache.GetWeaponCount(), uiClientWeaponHash ) );
}

void CClientGameInterface::CheckNewMapStarted()
{
	if( m_bNewMapStarted )
	{
		m_bNewMapStarted = false;

		memset( m_szMapName, 0, sizeof( m_szMapName ) );

		if( UTIL_GetMapName( m_szMapName, sizeof( m_szMapName ) ) )
		{
			NewMapStarted( m_szMapName, gEngfuncs.pfnGetLevelName() );
		}
		else
		{
			//Failed to get the map name, so quit now.
			//This should never happen, but if it does, we're kinda screwed.
			gEngfuncs.pfnClientCmd( "disconnect\n" );
			gEngfuncs.Con_Printf( "CClientGameInterface::CheckNewMapStarted: Couldn't get map name from level name!\n" );
		}
	}
}