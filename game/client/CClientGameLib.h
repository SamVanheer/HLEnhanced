#ifndef GAME_CLIENT_CCLIENTGAMELIB_H
#define GAME_CLIENT_CCLIENTGAMELIB_H

#include "IClientGameLib.h"

class IClientEngine;

class CClientGameLib : public IClientGameLib
{
public:

	bool LibInit( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	void LibShutdown() override;

	void PostInit() override;

	bool ConnectedToServer( const char* const pszIP, const char* const pszHostName, const char* const pszTags ) override;

	void DisconnectedFromServer( const char* const pszIP ) override;

	void GameInit() override;

	void GameShutdown() override;

	void LevelInitPreEntity(
		const char* const pszMapName, const char* const pszPrevMapName,
		const bool bLoadGame
	) override;

	void LevelInitPostEntity() override;

	void LevelShutdown() override;

	void Tick() override;

	void StartFrame() override;

	void GameFrame() override;

	void EndFrame() override;

	void OnCvarChanged( cvar_t* pCvar, const char* pszOldValue, float flOldValue ) override;
};

extern IClientEngine* g_pEngine;

extern CClientGameLib g_ClientLib;

#endif //GAME_CLIENT_CCLIENTGAMELIB_H
