#include "extdll.h"
#include "util.h"

#include "engine/server/Server.h"

#include "CServerEngineHacks.h"

namespace
{
CServerEngineHacks g_EngineHacks;
}

CServerEngineHacks& EngineHacks()
{
	return g_EngineHacks;
}

bool CServerEngineHacks::Initialize()
{
	if( m_bInitialized )
		return true;

	//Even if it fails to initialize this should be set, let caller handle failure and exit
	m_bInitialized = true;

	m_pServerData = SV_GetServer();

	if( !m_pServerData )
	{
		//Can happen when this gets called too early
		UTIL_ServerPrintf( "Couldn't acquire server_t instance from the engine\n" );
		return false;
	}

	return true;
}

void CServerEngineHacks::Shutdown()
{
	if( !m_bInitialized )
		return;

	m_pServerData = nullptr;

	m_bInitialized = false;
}
