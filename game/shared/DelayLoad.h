#ifndef GAME_SHARED_DELAYLOAD_H
#define GAME_SHARED_DELAYLOAD_H

/**
*	@file
*
*	Helpers and shared code for delay loading on Windows
*/

#ifdef WIN32
//See post VS 2015 update 3 delayimp.h for the reason why this has to be defined. - Solokiller
#define DELAYIMP_INSECURE_WRITABLE_HOOKS
#include <delayimp.h>

/**
*	@brief Loads a delay loaded game library, i.e. a library located in <code>Half-Life/moddir/</code>
*	@param pszDLL Name of the DLL to load. szDll member of PDelayLoadInfo structure
*	@param pszLocalPath Optional. Local path, starting in <code>moddir</code> to prepend to DLL name. A slash is appended automatically
*/
HMODULE DelayLoad_LoadGameLib( const char* pszDLL, const char* pszLocalPath = nullptr );

/**
*	@brief Implemented by each game library, handles DelayLoad callback
*	@return null if not applicable
*/
HMODULE DelayLoad_LoadDelayLoadLib( unsigned dliNotify, PDelayLoadInfo pdli );
#endif

#endif //GAME_SHARED_DELAYLOAD_H
