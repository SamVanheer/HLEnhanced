#ifdef CLIENT_DLL
#include "hud.h"
#include "cl_util.h"
#else
#include "extdll.h"
#include "util.h"
#endif

#include "FileSystemHelpers.h"

namespace
{
CSysModule* g_pFileSystemModule = nullptr;
}

IFileSystem* g_pFileSystem = nullptr;

bool FileSystem_LoadModule()
{
	// Determine which filesystem to use.
#if defined( _WIN32 )
	const char* pszFsModule = "filesystem_stdio.dll";
#elif defined( OSX )
	const char* pszFsModule = "filesystem_stdio.dylib";
#elif defined( LINUX )
	const char* pszFsModule = "filesystem_stdio.so";
#else
#error
#endif

	char szFSDir[ MAX_PATH ] = {};

	//TODO: on the client IClientVGUI can probably take care of this, so this can probably be removed at some point - Solokiller
#ifdef CLIENT_DLL
	if( gEngfuncs.COM_ExpandFilename( pszFsModule, szFSDir, sizeof( szFSDir ) ) == false )
	{
		return false;
	}
#else
	//Just use the filename for the server. No COM_ExpandFilename here.
	V_strncpy( szFSDir, pszFsModule, ARRAYSIZE( szFSDir ) );
#endif

	// Get filesystem interface.
	g_pFileSystemModule = Sys_LoadModule( szFSDir );

	assert( g_pFileSystemModule );

	if( !g_pFileSystemModule )
	{
		return false;
	}

	return true;
}

void FileSystem_FreeModule()
{
	if( g_pFileSystemModule )
	{
		Sys_UnloadModule( g_pFileSystemModule );
		g_pFileSystemModule = nullptr;
	}
}

CSysModule* FileSystem_GetModule()
{
	return g_pFileSystemModule;
}
