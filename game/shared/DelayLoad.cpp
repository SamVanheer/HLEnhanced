#include "extdll.h"
#include "util.h"

#include "DelayLoad.h"

#ifdef WIN32
HMODULE DelayLoad_LoadGameLib( const char* pszDLL, const char* pszLocalPath )
{
	ASSERT( pszDLL );

	char szGameDir[ MAX_PATH ];
	char szPath[ MAX_PATH ];

	if( !UTIL_GetGameDir( szGameDir, sizeof( szGameDir ) ) )
		return nullptr;

	int iResult;
	
	if( pszLocalPath )
		iResult = snprintf( szPath, ARRAYSIZE( szPath ), "%s/%s/%s", szGameDir, pszLocalPath, pszDLL );
	else
		iResult = snprintf( szPath, ARRAYSIZE( szPath ), "%s/%s", szGameDir, pszDLL );

	if( iResult < 0 || static_cast<size_t>( iResult ) >= ARRAYSIZE( szPath ) )
		return nullptr;

	return LoadLibraryA( szPath );
}

/*
*	@brief Handles loading of shared delay loaded libraries
*/
HMODULE DelayLoad_HandleSharedLibs( unsigned dliNotify, PDelayLoadInfo pdli )
{
	return nullptr;
}

FARPROC WINAPI DelayHook(
	unsigned        dliNotify,
	PDelayLoadInfo  pdli
)
{
	if( dliNotify == dliNotePreLoadLibrary )
	{
		return reinterpret_cast<FARPROC>( DelayLoad_LoadDelayLoadLib( dliNotify, pdli ) );
	}

	return nullptr;
}

ExternC PfnDliHook __pfnDliNotifyHook2 = DelayHook;

ExternC PfnDliHook   __pfnDliFailureHook2 = nullptr;
#endif
