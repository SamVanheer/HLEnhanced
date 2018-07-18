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

#ifndef GAME_SHARED_ENTITIES_WEAPONS_CSNIPERRIFLE_H
#define GAME_SHARED_ENTITIES_WEAPONS_CSNIPERRIFLE_H

enum SniperRifleAnim
{
	SNIPERRIFLE_DRAW = 0,
	SNIPERRIFLE_SLOWIDLE,
	SNIPERRIFLE_FIRE,
	SNIPERRIFLE_FIRELASTROUND,
	SNIPERRIFLE_RELOAD1,
	SNIPERRIFLE_RELOAD2,
	SNIPERRIFLE_RELOAD3,
	SNIPERRIFLE_SLOWIDLE2,
	SNIPERRIFLE_HOLSTER
};

/**
*	Opposing force sniper rifle.
*	TODO: use base class for snipers?
*	- Solokiller
*/
class CSniperRifle : public CBasePlayerWeapon
{
public:
	DECLARE_CLASS( CSniperRifle, CBasePlayerWeapon );
	DECLARE_DATADESC();

	CSniperRifle();

	void Precache() override;
	void Spawn() override;

	bool AddToPlayer( CBasePlayer* pPlayer ) override;

	bool Deploy() override;

	void Holster() override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

	void Reload() override;

	void ToggleZoom();

private:
	unsigned short m_usSniper;

	bool m_bInZoom;
	bool m_bReloading;
	float m_flReloadStart;
};

#endif // GAME_SHARED_ENTITIES_WEAPONS_CSNIPERRIFLE_H
#endif // GAME_OPFOR
