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

#ifndef GAME_SERVER_ENTITIES_AMMO_CSPOREAMMO_H
#define GAME_SERVER_ENTITIES_AMMO_CSPOREAMMO_H

enum SporeAmmoAnim
{
	SPOREAMMO_IDLE = 0,
	SPOREAMMO_SPAWNUP,
	SPOREAMMO_SNATCHUP,
	SPOREAMMO_SPAWNDN,
	SPOREAMMO_SNATCHDN,
	SPOREAMMO_IDLE1,
	SPOREAMMO_IDLE2
};

enum SporeAmmoBody
{
	SPOREAMMOBODY_EMPTY = 0,
	SPOREAMMOBODY_FULL
};

class CSporeAmmo : public CBasePlayerAmmo
{
public:
	DECLARE_CLASS( CSporeAmmo, CBasePlayerAmmo );
	DECLARE_DATADESC();

	void Precache() override;

	void Spawn() override;

	void OnTakeDamage( const CTakeDamageInfo& info ) override;

	bool AddAmmo( CBaseEntity* pOther ) override;

	void Idling();

	void SporeTouch( CBaseEntity* pOther );
};

#endif // GAME_SERVER_ENTITIES_AMMO_CSPOREAMMO_H

#endif // GAME_OPFOR
