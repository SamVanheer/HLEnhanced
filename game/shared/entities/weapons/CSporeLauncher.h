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

#ifndef GAME_SHARED_ENTITIES_WEAPONS_CSPORELAUNCHER_H
#define GAME_SHARED_ENTITIES_WEAPONS_CSPORELAUNCHER_H

enum SporeLauncherAnim
{
	SPLAUNCHER_IDLE = 0,
	SPLAUNCHER_FIDGET,
	SPLAUNCHER_RELOAD_REACH,
	SPLAUNCHER_RELOAD,
	SPLAUNCHER_AIM,
	SPLAUNCHER_FIRE,
	SPLAUNCHER_HOLSTER1,
	SPLAUNCHER_DRAW1,
	SPLAUNCHER_IDLE2
};

class CSporeLauncher : public CBasePlayerWeapon
{
private:
	enum class ReloadState
	{
		NOT_RELOADING = 0,
		DO_RELOAD_EFFECTS,
		RELOAD_ONE
	};

public:
	DECLARE_CLASS( CSporeLauncher, CBasePlayerWeapon );
	DECLARE_DATADESC();

	CSporeLauncher();

	void Precache() override;

	void Spawn() override;

	bool AddToPlayer( CBasePlayer* pPlayer ) override;

	bool Deploy() override;

	void Holster() override;

	bool ShouldWeaponIdle() override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

	void Reload() override;

	void GetWeaponData( weapon_data_t& data ) override;

	void SetWeaponData( const weapon_data_t& data ) override;

private:
	unsigned short m_usFireSpore;

	ReloadState m_ReloadState;

	float m_flNextReload;
};

#endif // GAME_SHARED_ENTITIES_WEAPONS_CSPORELAUNCHER_H

#endif // GAME_OPFOR
