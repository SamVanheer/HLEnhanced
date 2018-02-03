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

#include "logging/CLogSystem.h"
#include "xml/CXMLManager.h"

#include "CBaseGameInterface.h"

CSysModule* g_pFileSystemModule = nullptr;
IFileSystem* g_pFileSystem = nullptr;
namespace
{
CBaseGameInterface* g_pInstance = nullptr;
}

CBaseGameInterface* CBaseGameInterface::GetInstance()
{
	ASSERT( g_pInstance );

	return g_pInstance;
}

CBaseGameInterface::CBaseGameInterface()
{
	ASSERT( !g_pInstance );
	g_pInstance = this;
}

CBaseGameInterface::~CBaseGameInterface()
{
	ASSERT( g_pInstance == this );
	g_pInstance = nullptr;
}

bool CBaseGameInterface::InitializeCommon()
{
	if( !UTIL_GetGameDir( m_szGameDirectory, sizeof( m_szGameDirectory ) ) )
	{
		return false;
	}

	if( !InitFileSystem() )
		return false;

	g_pDeveloper = CVarGetPointer( "developer" );

	if( nullptr == g_pDeveloper )
	{
		return false;
	}

	if( !logging::LogSystem().Initialize() )
	{
		return false;
	}

	if( !xml::XMLManager().Initialize() )
	{
		return false;
	}

	return true;
}

void CBaseGameInterface::ShutdownCommon()
{
	xml::XMLManager().Shutdown();

	logging::LogSystem().Shutdown();

	g_pDeveloper = nullptr;

	ShutdownFileSystem();

	memset( m_szGameDirectory, 0, sizeof( m_szGameDirectory ) );
}

bool CBaseGameInterface::InitFileSystem()
{
	// Determine which filesystem to use.
#if defined ( _WIN32 )
	const char* szFsModule = "filesystem_stdio.dll";
#elif defined(OSX)
	const char* szFsModule = "filesystem_stdio.dylib";
#elif defined(LINUX)
	const char* szFsModule = "filesystem_stdio.so";
#else
#error
#endif


	char szFSDir[ MAX_PATH ];
	szFSDir[ 0 ] = 0;
#ifdef CLIENT_DLL
	if( gEngfuncs.COM_ExpandFilename( szFsModule, szFSDir, sizeof( szFSDir ) ) == false )
	{
		return false;
	}
#else
	//Just use the filename for the server. No COM_ExpandFilename here.
	strcpy( szFSDir, szFsModule );
#endif

	// Get filesystem interface.
	g_pFileSystemModule = Sys_LoadModule( szFSDir );
	assert( g_pFileSystemModule );
	if( !g_pFileSystemModule )
	{
		return false;
	}

	CreateInterfaceFn fileSystemFactory = Sys_GetFactory( g_pFileSystemModule );
	if( !fileSystemFactory )
	{
		return false;
	}

	g_pFileSystem = ( IFileSystem* ) fileSystemFactory( FILESYSTEM_INTERFACE_VERSION, nullptr );
	assert( g_pFileSystem );
	if( !g_pFileSystem )
	{
		return false;
	}

	return true;
}

void CBaseGameInterface::ShutdownFileSystem()
{
	if( g_pFileSystemModule )
	{
		g_pFileSystem = nullptr;
		Sys_UnloadModule( g_pFileSystemModule );
		g_pFileSystemModule = nullptr;
	}
}