#ifndef GAME_SHARED_FILESYSTEMHELPERS_H
#define GAME_SHARED_FILESYSTEMHELPERS_H

#include "interface.h"

class IFileSystem;

/**
*	@brief Loads the filesystem module
*	@return true on success, false on failure
*/
bool FileSystem_LoadModule();

/**
*	@brief Frees the filesystem module
*/
void FileSystem_FreeModule();

/**
*	@brief Gets the filesystem module
*/
CSysModule* FileSystem_GetModule();

/**
*	Global filesystem instance.
*/
extern IFileSystem* g_pFileSystem;

#endif //GAME_SHARED_FILESYSTEMHELPERS_H
