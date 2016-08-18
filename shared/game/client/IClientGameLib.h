#ifndef SHARED_GAME_CLIENT_ICLIENTGAMELIB_H
#define SHARED_GAME_CLIENT_ICLIENTGAMELIB_H

#include "interface.h"

class CSteamID;

/**
*	This interface is exposed by the client to allow the engine to set up newer interfaces and invoke callbacks at appropriate moments.
*	Based on Source's IBaseClientDLL interface.
*/
class IClientGameLib : public IBaseInterface
{
public:

	/**
	*	Called once on library load to connect the server with other libraries.
	*	Called after cldll_func_t::pInitFunc for backwards compatibility.
	*	@param pFactories List of factories.
	*	@param uiNumFactories Number of factories.
	*	@return true if the library initialized successfully, false if there was an error.
	*/
	virtual bool LibInit( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) = 0;

	/**
	*	Called once when the library is unloaded.
	*	Called before cldll_func_t::pShutdown for backwards compatibility.
	*/
	virtual void LibShutdown() = 0;

	/**
	*	Called once after everything's been loaded and initialized by the engine.
	*/
	virtual void PostInit() = 0;

	/**
	*	Called when this client has connected to a server and has received the server info packet.
	*	IClientEngine::GetServerSteamID should be used to retrieve the server's Steam ID if necessary.
	*	@param pszIP Server IP.
	*	@param pszHostName The name of the server as shown in the server browser.
	*	@param pszTags List of tags, delimited by a comma(TODO?).
	*	@return true if the connection should be maintained, false if the client should disconnect.
	*	@see IClientEngine::GetServerSteamID
	*/
	virtual bool ConnectedToServer( const char* const pszIP, const char* const pszHostName, const char* const pszTags ) = 0;

	/**
	*	Called when this client has disconnected from a server.
	*	IClientEngine::GetServerSteamID should be used to retrieve the server's Steam ID if necessary.
	*	@param pszIP Server IP.
	*	@see IClientEngine::GetServerSteamID
	*/
	virtual void DisconnectedFromServer( const char* const pszIP ) = 0;

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
	*	Use IClientEngine::GetEntityDataString to get the entity data string.
	*	@param pszMapName Name of the map that has been started. This is the name of the file without the path or extension.
	*	@param pszPrevMapName If there was a previous map, name of the previous map. Otherwise, null. This is the name of the file without the path or extension.
	*	@param bLoadGame Whether this is a saved game being loaded or not.
	*	@see IClientEngine::GetEntityDataString
	*/
	virtual void LevelInitPreEntity(
		const char* const pszMapName, const char* const pszPrevMapName,
		const bool bLoadGame
	) = 0;

	/**
	*	Called after all entities have been created. These are the entities that initially make up the game, during the client's connection stage.
	*	That includes static entities (@see enginefuncs_t::pfnMakeStatic).
	*/
	virtual void LevelInitPostEntity() = 0;

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
	*	Called at the start of a game frame. Called before cldll_func_t::pHudFrame.
	*/
	virtual void StartFrame() = 0;

	/**
	*	Called every frame after StartFrame. Called after cldll_func_t::pHudFrame.
	*/
	virtual void GameFrame() = 0;

	/**
	*	Called at the end of a game frame. Counterpart to StartFrame.
	*	@see StartFrame
	*/
	virtual void EndFrame() = 0;
};

#define ICLIENTGAMELIB_NAME "IClientGameLibV001"

#endif //SHARED_GAME_CLIENT_ICLIENTGAMELIB_H
