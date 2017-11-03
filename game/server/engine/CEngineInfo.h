#ifndef GAME_SERVER_ENGINE_CENGINEINFO_H
#define GAME_SERVER_ENGINE_CENGINEINFO_H

#include "engine/server/IEngineInfo.h"

/**
*	@brief Game code implementation of the IEngineInfo interface
*	@author Solokiller
*/
class CEngineInfo : public IEngineInfo
{
public:
	uintptr_t QueryEngineInfo( const char* pszName ) override;
};

#endif //GAME_SERVER_ENGINE_CENGINEINFO_H
