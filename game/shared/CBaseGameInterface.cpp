#include <cassert>

#ifdef CLIENT_DLL
#include "hud.h"
#include "cl_util.h"
#else
#include "extdll.h"
#include "util.h"
#endif

#include "interface.h"
#include "FileSystem.h"
#include "pm_shared.h"

#include "CBaseGameInterface.h"

bool CBaseGameInterface::InitializeCommon( CreateInterfaceFn* pFactories, size_t uiNumFactories )
{
	g_pFileSystem = reinterpret_cast<IFileSystem*>( InitializeInterface( FILESYSTEM_INTERFACE_VERSION, pFactories, uiNumFactories ) );

	assert( g_pFileSystem );

	if( !g_pFileSystem )
	{
		return false;
	}

	g_pDeveloper = CVarGetPointer( "developer" );

	return g_pDeveloper != nullptr;
}

bool CBaseGameInterface::PostInit()
{
	PM_PostInit();

	return true;
}

void CBaseGameInterface::ShutdownCommon()
{
	g_pDeveloper = nullptr;

	if( g_pFileSystem )
	{
		g_pFileSystem = nullptr;
	}
}
