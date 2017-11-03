#ifndef GAME_SERVER_ENGINE_CSERVERENGINEHACKS_H
#define GAME_SERVER_ENGINE_CSERVERENGINEHACKS_H

struct server_t;

/**
*	@brief Server side engine hacks, used to implement functionality that is otherwise unavailable
*	All code here is a hack and not expected to work after updates, or with other engines
*/
class CServerEngineHacks final
{
public:
	CServerEngineHacks() = default;
	~CServerEngineHacks() = default;

	bool Initialize();

	void Shutdown();

	bool IsInitialized() const { return m_bInitialized; }

	/**
	*	@brief Gets the engine server data structure
	*/
	server_t& GetServerData() { return *m_pServerData; }

private:
	bool m_bInitialized = false;
	server_t* m_pServerData = nullptr;

private:
	CServerEngineHacks( const CServerEngineHacks& ) = delete;
	CServerEngineHacks& operator=( const CServerEngineHacks& ) = delete;
};

CServerEngineHacks& EngineHacks();

#endif //GAME_SERVER_ENGINE_CSERVERENGINEHACKS_H
