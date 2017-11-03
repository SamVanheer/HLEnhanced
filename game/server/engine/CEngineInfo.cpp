#include "extdll.h"
#include "util.h"

#include "interface.h"

#include "CEngineInfo.h"
#include "CServerEngineHacks.h"

#include "engine/server/Server.h"

EXPOSE_SINGLE_INTERFACE( CEngineInfo, IEngineInfo, ENGINEINFO_INTERFACE_VERSION );

uintptr_t CEngineInfo::QueryEngineInfo( const char* pszName )
{
	ASSERT( pszName );

	if( !EngineHacks().IsInitialized() )
	{
		UTIL_ServerPrintf( "Server engine hacks not initialized yet, cannot query engine info\n" );
		return static_cast<uintptr_t>( 0 );
	}

	if( pszName && *pszName )
	{
		if( !strcmp( "engine.num_edicts", pszName ) )
		{
			return static_cast<uintptr_t>( EngineHacks().GetServerData().num_edicts );
		}
		else if( !strcmp( "engine.max_edicts", pszName ) )
		{
			return static_cast<uintptr_t>( EngineHacks().GetServerData().max_edicts );
		}
	}

	return static_cast<uintptr_t>( 0 );
}
