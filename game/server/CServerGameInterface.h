#ifndef GAME_SERVER_CSERVERGAMEINTERFACE_H
#define GAME_SERVER_CSERVERGAMEINTERFACE_H

#include <memory>

#include "interface.h"
#include "CBaseGameInterface.h"
#include "server/IServerGameInterface.h"

class CServerConfig;
class IEngineInfo;

/**
*	The server's representation of itself.
*/
class CServerGameInterface : public CBaseGameInterface, public IServerGameInterface
{
public:
	CServerGameInterface() = default;
	~CServerGameInterface() = default;

	bool LibInit( CreateInterfaceFn* pFactories, size_t uiNumFactories ) override;

	void LibShutdown() override;

	/**
	*	Called whenever an entity is created. Used to detect when a new map has started. - Solokiller
	*/
	void EntityCreated( entvars_t* pev );

	/**
	*	@return Whether the server is active.
	*/
	bool IsActive() const { return m_bActive; }

	bool ClientConnect( edict_t* pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ CCONNECT_REJECT_REASON_SIZE ] );

	void ClientDisconnect( edict_t* pEdict );

	void ClientKill( edict_t* pEdict );

	void ClientPutInServer( edict_t* pEntity );

	void ClientCommand( edict_t* pEntity );

	void ClientUserInfoChanged( edict_t* pEntity, char* infobuffer );

	void WorldInit();

	void Activate( edict_t* pEdictList, const int edictCount, const int clientMax );

	void Deactivate();

	void StartFrame();

	void ParmsNewLevel();

	void ParmsChangeLevel();

	void ClientPrecache();

	const char* GetGameDescription() const;

	void Sys_Error( const char* error_string );

	void PlayerCustomization( edict_t* pEntity, customization_t* pCust );

	void SpectatorConnect( edict_t* pEntity );

	void SpectatorDisconnect( edict_t* pEntity );

	void SpectatorThink( edict_t* pEntity );

private:
	bool m_bMapStartedLoading = false;
	bool m_bActive = false;

	std::unique_ptr<CServerConfig> m_ServerConfig;

private:
	CServerGameInterface( const CServerGameInterface& ) = delete;
	CServerGameInterface& operator=( const CServerGameInterface& ) = delete;
};

extern CServerGameInterface g_Server;

IEngineInfo& EngineInfo();

#endif //GAME_SERVER_CSERVERGAMEINTERFACE_H