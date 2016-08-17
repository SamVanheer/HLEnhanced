#include "extdll.h"
#include "util.h"

#include "IServerEngine.h"

#include "CServerGameLib.h"

IServerEngine* g_pEngine = nullptr;

CServerGameLib g_ServerLib;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CServerGameLib, IServerGameLib, ISERVERGAMELIB_NAME, g_ServerLib );

bool CServerGameLib::LibInit( const CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	for( size_t uiIndex = 0; uiIndex < uiNumFactories; ++uiIndex )
	{
		auto pFactory = pFactories[ uiIndex ];

		if( !g_pEngine )
			g_pEngine = static_cast<IServerEngine*>( pFactory( ISERVERENGINE_NAME, nullptr ) );
	}

	if( !g_pEngine )
	{
		Alert( at_error, "CServerGameLib::LibInit: Couldn't load interface \"%s\"\n", ISERVERENGINE_NAME );
		return false;
	}

	return true;
}

void CServerGameLib::LibShutdown()
{
}

void CServerGameLib::PostInit()
{
}

void CServerGameLib::GameInit()
{
}

void CServerGameLib::GameShutdown()
{
}

bool CServerGameLib::LevelInit(
	const char* const pszMapName, const char* const pszPrevMapName,
	const char* const pszEntityData, const char* const pszLandmarkName,
	const bool bLoadGame, bool& bHandledEntParsing
)
{
	return true;
}

void CServerGameLib::LevelShutdown()
{
}

void CServerGameLib::Tick()
{
}

void CServerGameLib::StartFrame()
{
}

bool CServerGameLib::GameFrame()
{
	return false;
}

void CServerGameLib::EndFrame()
{
}

void CServerGameLib::PreSave()
{
}

void CServerGameLib::PostSave()
{
}

void CServerGameLib::PreRestore()
{
}

void CServerGameLib::PostRestore()
{
}

void CServerGameLib::OnClientFullyAuthenticated( edict_t* pClientEdict )
{
}
