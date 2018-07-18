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

#ifndef GAME_SHARED_ENTITIES_WEAPONS_CPENGUIN_H
#define GAME_SHARED_ENTITIES_WEAPONS_CPENGUIN_H

enum PenguinNestAnim
{
	PENGUINNEST_WALK = 0,
	PENGUINNEST_IDLE
};

enum PenguinAnim
{
	PENGUIN_IDLE1 = 0,
	PENGUIN_FIDGETFIT,
	PENGUIN_FIDGETNIP,
	PENGUIN_DOWN,
	PENGUIN_UP,
	PENGUIN_THROW
};

class CPenguin : public CBasePlayerWeapon
{
public:
	DECLARE_CLASS( CPenguin, CBasePlayerWeapon );

	CPenguin();

	void Precache() override;

	void Spawn() override;

	bool Deploy() override;

	void Holster() override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

private:
	unsigned short m_usPenguinFire;

	bool m_bJustThrown;
};

#endif // GAME_SHARED_ENTITIES_WEAPONS_CPENGUIN_H

#endif // GAME_OPFOR
