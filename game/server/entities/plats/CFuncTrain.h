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
#ifndef GAME_SERVER_ENTITIES_PLATS_CFUNCTRAIN_H
#define GAME_SERVER_ENTITIES_PLATS_CFUNCTRAIN_H

#include "CBasePlatTrain.h"

// Trains
enum
{
	SF_TRAIN_WAIT_RETRIGGER	= 1,

	/**
	*	Train is initially moving.
	*/
	SF_TRAIN_START_ON		= 4,

	/**
	*	Train is not solid -- used to make water trains.
	*/
	SF_TRAIN_PASSABLE		= 8,
};

/*QUAKED func_train (0 .5 .8) ?
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
sounds
1) ratchet metal
*/
class CFuncTrain : public CBasePlatTrain
{
public:
	DECLARE_CLASS( CFuncTrain, CBasePlatTrain );
	DECLARE_DATADESC();

	void Spawn( void ) override;
	void Precache( void ) override;
	void Activate( void ) override;
	void OverrideReset( void ) override;

	void Blocked( CBaseEntity *pOther ) override;
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) override;
	void KeyValue( KeyValueData *pkvd ) override;


	void Wait( void );
	void Next( void );

	entvars_t	*m_pevCurrentTarget;
	//TODO: never used, remove - Solokiller
	int			m_sounds;
	bool		m_activated;
};

#endif //GAME_SERVER_ENTITIES_PLATS_CFUNCTRAIN_H