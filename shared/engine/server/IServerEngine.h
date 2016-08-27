#ifndef SHARED_ENGINE_SERVER_ISERVERENGINE_H
#define SHARED_ENGINE_SERVER_ISERVERENGINE_H

#include "interface.h"

class CSteamID;

typedef struct cvar_s cvar_t;

/**
*	General purpose server engine functions.
*	Based on Source's IVEngineServer interface.
*/
class IServerEngine : public IBaseInterface
{
public:

	/**
	*	@return The number of entities that currently exist.
	*/
	virtual int GetNumEntities() const = 0;

	/**
	*	@return The highest entity index that is currently in use.
	*/
	virtual int GetHighestEntIndexInUse() const = 0;

	/**
	*	Gets the entity data string for the current map, or null if no map has been loaded.
	*/
	virtual const char* GetEntityDataString() const = 0;

	/**
	*	@return Whether the game is paused.
	*/
	virtual bool IsPaused() const = 0;

	/**
	*	Checks whether the given model has been precached.
	*/
	virtual bool IsModelPrecached( const char* const pszModelName ) const = 0;

	/**
	*	Checks whether the given sound has been precached.
	*/
	virtual bool IsSoundPrecached( const char* const pszSoundName ) const = 0;

	/**
	*	Checks whether the given generic file has been precached.
	*/
	virtual bool IsGenericPrecached( const char* const pszFileName ) const = 0;

	/**
	*	Gets the given client's IP address as a string.
	*	Returns null if the client is invalid.
	*/
	virtual const char* GetClientIP( const edict_t* pClientEdict ) = 0;

	/**
	*	@return Whether the given client has been fully authenticated with Steam.
	*/
	virtual bool IsClientFullyAuthenticated( edict_t* pClientEdict ) = 0;

	/**
	*	Gets the Steam ID of the given player.
	*	@param pClientEdict Edict of the client.
	*	@return If the client has fully authenticated, the client's Steam ID. Otherwise, null.
	*/
	virtual const CSteamID* GetClientSteamID( edict_t* pClientEdict ) = 0;

	/**
	*	Gets the Steam ID of the given player.
	*	@param iClientEntIndex Entity index of the client.
	*	@return If the client has fully authenticated, the client's Steam ID. Otherwise, null.
	*/
	virtual const CSteamID* GetClientSteamIDByPlayerIndex( const int iClientEntIndex ) = 0;

	/**
	*	Gets the Steam ID of this server.
	*	@return The server's Steam ID.
	*/
	virtual const CSteamID* GetServerSteamID() = 0;

	/**
	*	Converts a Steam3 Steam ID into a Steam2 auth string.
	*	@param steamID Steam ID to convert.
	*	@param[ out ] pszDestBuffer Destination buffer to store the string in.
	*	@param uiBufferSize Size of the buffer, in bytes.
	*	@return If the conversion succeeded and the buffer was large enough, returns a pointer to the buffer. Otherwise, returns null.
	*/
	virtual const char* Steam3IDToAuthID( const CSteamID& steamID, char* pszDestBuffer, const size_t uiBufferSize ) = 0;

	/**
	*	@return The first cvar of the engine's list.
	*/
	virtual cvar_t* GetFirstCvarPtr() = 0;
};

#define ISERVERENGINE_NAME "IServerEngineV001"

#endif //SHARED_ENGINE_SERVER_ISERVERENGINE_H
