#ifndef SHARED_ENGINE_CLIENT_ICLIENTENGINE_H
#define SHARED_ENGINE_CLIENT_ICLIENTENGINE_H

#include "interface.h"

class Vector;
class CSteamID;

/**
*	General purpose client engine functions.
*	Based on Source's IVEngineClient interface.
*/
class IClientEngine : public IBaseInterface
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
	*	Plays a sound for the given entity at the given location.
	*	@param ent Entity index.
	*	@param vecOrigin Origin.
	*	@param channel Channel to play on. @see SoundChannel
	*	@param iSoundIndex Index of the sound to play.
	*	@param volume Volume. [ 0, 1 ].
	*	@param attenuation.
	*	@param fFlags Sound flags.
	*	@param pitch Pitch. [ 0, 255 ].
	*/
	virtual void PlaySoundByIndex( int ent, const Vector& vecOrigin, int channel, const int iSoundIndex, float volume, float attenuation, int fFlags, int pitch ) = 0;

	/**
	*	Gets the Steam ID of this client.
	*	@return The client's Steam ID.
	*/
	virtual const CSteamID* GetClientSteamID() = 0;

	/**
	*	@return If the client is currently connected to a server, returns the Steam ID of that server. Otherwise, returns null.
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
};

#define ICLIENTENGINE_NAME "IClientEngineV001"

#endif //SHARED_ENGINE_CLIENT_ICLIENTENGINE_H
