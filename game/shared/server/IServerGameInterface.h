#ifndef GAME_SHARED_SERVER_ISERVERGAMEINTERFACE_H
#define GAME_SHARED_SERVER_ISERVERGAMEINTERFACE_H

#include "interface.h"

/**
*	@brief Main interface used to communicate with the server library from the engine
*	Handles library initialization, shutdown
*	Not actually used (no engine support), but it makes future upgrades easier
*	@author Solokiller
*/
class IServerGameInterface : public IBaseInterface
{
public:

	/**
	*	@brief Initializes the server library
	*	@param pFactories List of factories to get interfaces from
	*	@param uiNumFactories Number of factories in pFactories
	*	@return Whether initialization succeeded
	*/
	virtual bool LibInit( CreateInterfaceFn* pFactories, size_t uiNumFactories ) = 0;

	/**
	*	@brief Shuts down the server library
	*	Called even if LibInit returned false
	*/
	virtual void LibShutdown() = 0;
};

#define SERVERGAMEINTERFACE_INTERFACE_VERSION "ServerGameInterfaceV001"

#endif //GAME_SHARED_SERVER_ISERVERGAMEINTERFACE_H
