#ifndef ENGINE_SERVER_IENGINEINFO_H
#define ENGINE_SERVER_IENGINEINFO_H

#include <cstdint>

#include "interface.h"

/**
*	@brief Provides a way to request information from the engine
*	@author Solokiller
*/
class IEngineInfo : public IBaseInterface
{
public:

	/**
	*	@brief Request engine info by name
	*	@param pszName Name of the info to retrieve
	*	@return	Value representing requested info
	*			Depending on info, this may need to be cast to a different type
	*			If info does not exist, returns 0
	*/
	virtual uintptr_t QueryEngineInfo( const char* pszName ) = 0;
};

#define ENGINEINFO_INTERFACE_VERSION "EngineInfoV001"

#endif //ENGINE_SERVER_IENGINEINFO_H
