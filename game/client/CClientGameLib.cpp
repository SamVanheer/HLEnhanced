#include "hud.h"
#include "cl_util.h"

#include "IClientEngine.h"

#include "CClientGameLib.h"

IClientEngine* g_pEngine = nullptr;

CClientGameLib g_ClientLib;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CClientGameLib, IClientGameLib, ICLIENTGAMELIB_NAME, g_ClientLib );

bool CClientGameLib::LibInit( const CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	for( size_t uiIndex = 0; uiIndex < uiNumFactories; ++uiIndex )
	{
		auto pFactory = pFactories[ uiIndex ];

		if( !g_pEngine )
			g_pEngine = static_cast<IClientEngine*>( pFactory( ICLIENTENGINE_NAME, nullptr ) );
	}

	if( !g_pEngine )
	{
		Alert( at_error, "CClientGameLib::LibInit: Couldn't load interface \"%s\"\n", ICLIENTENGINE_NAME );
		return false;
	}

	return true;
}

void CClientGameLib::LibShutdown()
{
}

void CClientGameLib::PostInit()
{
}

bool CClientGameLib::ConnectedToServer( const char* const pszIP, const char* const pszHostName, const char* const pszTags )
{
	return true;
}

void CClientGameLib::DisconnectedFromServer( const char* const pszIP )
{
}

void CClientGameLib::GameInit()
{
}

void CClientGameLib::GameShutdown()
{
}

void CClientGameLib::LevelInitPreEntity(
	const char* const pszMapName, const char* const pszPrevMapName,
	const bool bLoadGame
)
{
}

void CClientGameLib::LevelInitPostEntity()
{
}

void CClientGameLib::LevelShutdown()
{
}

void CClientGameLib::Tick()
{
}

void CClientGameLib::StartFrame()
{
}

void CClientGameLib::GameFrame()
{
}

void CClientGameLib::EndFrame()
{
}
