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

#ifdef GAME_OPFOR

#ifndef GAME_SERVER_ENTITIES_TRIGGERS_CTRIGGERXENRETURN_H
#define GAME_SERVER_ENTITIES_TRIGGERS_CTRIGGERXENRETURN_H

class CTriggerXenReturn : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerXenReturn, CBaseTrigger );
	DECLARE_DATADESC();

	void Spawn() override;

	void ReturnTouch( CBaseEntity* pOther );
};

#endif // GAME_SERVER_ENTITIES_TRIGGERS_CTRIGGERXENRETURN_H

#endif // GAME_OPFOR
