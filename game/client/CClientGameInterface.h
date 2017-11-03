#ifndef GAME_CLIENT_CCLIENTGAMEINTERFACE_H
#define GAME_CLIENT_CCLIENTGAMEINTERFACE_H

#include "CBaseGameInterface.h"

struct cl_entity_t;

/**
*	The client's representation of itself.
*/
class CClientGameInterface : public CBaseGameInterface
{
public:
	CClientGameInterface() = default;
	~CClientGameInterface() = default;

	/**
	*	@return The map name.
	*/
	const char* GetMapName() const { return m_szMapName; }

	/**
	*	Initializes the client.
	*	@return true on success, false on failure.
	*/
	bool LibInit( CreateInterfaceFn* pFactories, size_t uiNumFactories );

	bool ConnectionEstablished();

	/**
	*	Shuts down the client.
	*	Should be called even if Initialize returned false.
	*/
	void LibShutdown();

	void Frame( double flTime );

	void CAM_Think();

private:
	/**
	*	A new map has been started. - Solokiller
	*	@param pszMapName Name of the map, without path or extension.
	*	@param pszLevelName Name of the map, with path and extension.
	*/
	void NewMapStarted( const char* const pszMapName, const char* const pszLevelName );

	/**
	*	Called when the map has been loaded.
	*/
	void MapInit( cl_entity_t* pWorldModel );

	/**
	*	Checks if a new map has been started. If so, calls HUD_NewMapStarted.
	*	- Solokiller
	*/
	void CheckNewMapStarted();

private:
	bool m_bNewMapStarted = false;
	bool m_bParseMapData = false;

	char m_szMapName[ MAX_PATH ] = {};

private:
	CClientGameInterface( const CClientGameInterface& ) = delete;
	CClientGameInterface& operator=( const CClientGameInterface& ) = delete;
};

extern CClientGameInterface g_Client;

#endif //GAME_CLIENT_CCLIENTGAMEINTERFACE_H