#ifndef SHARED_GAME_SERVER_ISERVERGAMELIB_H
#define SHARED_GAME_SERVER_ISERVERGAMELIB_H

#include "interface.h"

/**
*	This interface is exposed by the server to allow the engine to set up newer interfaces and invoke callbacks at appropriate moments.
*	Based on Source's IServerGameDLL interface.
*/
class IServerGameLib : public IBaseInterface
{
public:

	/**
	*	Called once on library load to connect the server with other libraries.
	*	Called after GameDLLInit for backwards compatibility.
	*	@param pFactories List of factories.
	*	@param uiNumFactories Number of factories.
	*	@return true if the library initialized successfully, false if there was an error.
	*/
	virtual bool LibInit( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) = 0;

	/**
	*	Called once when the library is unloaded.
	*	Called before GameDLLShutdown for backwards compatibility.
	*/
	virtual void LibShutdown() = 0;

	/**
	*	Called once after everything's been loaded and initialized by the engine.
	*/
	virtual void PostInit() = 0;

	/**
	*	Called whenever a new game is started.
	*	E.g. new map started, restarted map. Does not get called for level transitions.
	*/
	virtual void GameInit() = 0;

	/**
	*	Called whenever a game has ended.
	*	E.g. in singleplayer: died, restarted map, loaded game, returned to main menu.
	*	In multiplayer: when disconnecting from a server (listen server).
	*	Not called for level transitions.
	*/
	virtual void GameShutdown() = 0;

	/**
	*	Called after GameInit whenever a new map has been started. This happens unconditionally for all map starts.
	*	@param pszMapName Name of the map that has been started. This is the name of the file without the path or extension.
	*	@param pszPrevMapName If there was a previous map, name of the previous map. Otherwise, null. This is the name of the file without the path or extension.
	*	@param pszEntityData Pointer to the entity data string for this map.
	*	@param pszLandmarkName Name of the landmark that was tied to the previous map's trigger_changelevel, or null if there was no landmark.
	*	@param bLoadGame Whether this is a saved game being loaded or not.
	*	@param[ out ] bHandledEntParsing	Default false. The server can set this to true to signal that it has handled the creation of entities itself. 
	*										If false, the engine handles entity data parsing.
	*	@return true if the server handled level init successfully, false if there was an error.
	*/
	virtual bool LevelInit(
		const char* const pszMapName, const char* const pszPrevMapName, 
		const char* const pszEntityData, const char* const pszLandmarkName, 
		const bool bLoadGame, bool& bHandledEntParsing
	) = 0;

	/**
	*	Called when the level has ended due to changelevel, restart, disconnect, etc.
	*	Called before GameShutdown.
	*/
	virtual void LevelShutdown() = 0;

	/**
	*	Called every frame even if no map is loaded or the game is paused.
	*/
	virtual void Tick() = 0;

	/**
	*	Called at the start of a game frame. Identical to DLL_FUNCTIONS::pfnStartFrame, called right after.
	*/
	virtual void StartFrame() = 0;

	/**
	*	Called every frame after StartFrame. If this returns true, the engine will not run physics & think/touch functions itself.
	*	Does not handle player physics (handled by usercmd inputs in the engine).
	*	@return true to handle game physics manually, false if the engine should handle it.
	*/
	virtual bool GameFrame() = 0;

	/**
	*	Called at the end of a game frame. Counterpart to StartFrame.
	*	@see StartFrame
	*/
	virtual void EndFrame() = 0;

	/**
	*	Called before a saved game is generated.
	*/
	virtual void PreSave() = 0;

	/**
	*	Called after a saved game has been generated.
	*/
	virtual void PostSave() = 0;

	/**
	*	Called before a saved game is loaded.
	*/
	virtual void PreRestore() = 0;

	/**
	*	Called after a saved game has been loaded.
	*/
	virtual void PostRestore() = 0;

	/**
	*	Called when a client has fully authenticated.
	*	This can occur at any point after the client has connected to the server in ClientConnect.
	*	Note that clients can already be authenticated in ClientConnect.
	*	@param pClientEdict Edict of the client.
	*	@see IServerEngine::IsClientFullyAuthenticated
	*	@see IServerEngine::GetClientSteamID
	*/
	virtual void OnClientFullyAuthenticated( edict_t* pClientEdict ) = 0;
};

/**
*	The name of this interface.
*/
#define ISERVERGAMELIB_NAME "IServerGameLibV001"

#endif //SHARED_GAME_SERVER_ISERVERGAMELIB_H
