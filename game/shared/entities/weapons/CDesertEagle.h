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

#ifndef GAME_SHARED_ENTITIES_WEAPONS_CDESERTEAGLE_H
#define GAME_SHARED_ENTITIES_WEAPONS_CDESERTEAGLE_H

class CDesertEagleLaser;

enum DesertEagleAnim
{
	DEAGLE_IDLE1 = 0,
	DEAGLE_IDLE2,
	DEAGLE_IDLE3,
	DEAGLE_IDLE4,
	DEAGLE_IDLE5,
	DEAGLE_SHOOT,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD_NOSHOT,
	DEAGLE_RELOAD,
	DEAGLE_DRAW,
	DEAGLE_HOLSTER
};

class CDesertEagle : public CBasePlayerWeapon
{
public:
	DECLARE_CLASS( CDesertEagle, CBasePlayerWeapon );

	CDesertEagle();

	void Precache() override;

	void Spawn() override;

	bool AddToPlayer( CBasePlayer* pPlayer ) override;

	bool Deploy() override;

	void Holster() override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

	void Reload() override;

	void GetWeaponData( weapon_data_t& data ) override;

	void SetWeaponData( const weapon_data_t& data ) override;

private:
	void UpdateLaser();

private:
	int m_iShell;
	unsigned short m_usFireEagle;

	bool m_bSpotVisible;
	bool m_bLaserActive;
	CDesertEagleLaser* m_pLaser;
};

#endif // GAME_SHARED_ENTITIES_WEAPONS_CDESERTEAGLE_H

#endif // GAME_OPFOR
