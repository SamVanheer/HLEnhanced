#ifndef GAME_SERVER_CSERVERGAMELIB_H
#define GAME_SERVER_CSERVERGAMELIB_H

#include "IServerGameLib.h"

class IServerEngine;
class IEngineModel;

class CServerGameLib final : public IServerGameLib
{
public:
	CServerGameLib() = default;

	bool LibInit( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) override;

	void LibShutdown() override;

	void PostInit() override;

	void GameInit() override;

	void GameShutdown() override;

	bool LevelInit(
		const char* const pszMapName, const char* const pszPrevMapName,
		const char* const pszEntityData, const char* const pszLandmarkName,
		const bool bLoadGame, bool& bHandledEntParsing
	) override;

	void LevelShutdown() override;

	void Tick() override;

	void StartFrame() override;

	bool GameFrame() override;

	void EndFrame() override;

	void PreSave() override;

	void PostSave() override;

	void PreRestore() override;

	void PostRestore() override;

	void OnClientFullyAuthenticated( edict_t* pClientEdict ) override;

	void OnCvarChanged( cvar_t* pCvar, const char* pszOldValue, float flOldValue ) override;

private:
	CServerGameLib( const CServerGameLib& ) = delete;
	CServerGameLib& operator=( const CServerGameLib& ) = delete;
};

extern IServerEngine* g_pEngine;

extern IEngineModel* g_pEngineModel;

extern CServerGameLib g_ServerLib;

#endif //GAME_SERVER_CSERVERGAMELIB_H