/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef GAME_SERVER_ENTITIES_PLATS_CSPRITETRAIN_H
#define GAME_SERVER_ENTITIES_PLATS_CSPRITETRAIN_H

#include "entities/plats/CBasePlatTrain.h"

/**
*	@brief Train that uses a sprite or studio model
*/
class CSpriteTrain : public CBasePlatTrain
{
public:
	DECLARE_CLASS( CSpriteTrain, CBasePlatTrain );
	DECLARE_DATADESC();

	void Precache() override;
	void Spawn() override;
	void Activate() override;

	void OverrideReset() override;

	void Blocked( CBaseEntity* pOther ) override;
	void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue ) override;
	void Think() override;

	void Wait();
	void Next();

	void LinearMove( Vector vecDest, float flSpeed );

	void Animate( float frames );

private:
	//TODO: should be EHANDLE - Solokiller
	entvars_t* m_pevCurrentTarget;

	//TODO: never used, remove - Solokiller
	int m_sounds;
	bool m_activated;

	float m_maxFrame;
	float m_lastTime;

	bool m_waiting;
	bool m_nexting;

	float m_nextTime;
	float m_waitTime;

	bool m_stopSprite;
};

#endif //GAME_SERVER_ENTITIES_PLATS_CSPRITETRAIN_H
