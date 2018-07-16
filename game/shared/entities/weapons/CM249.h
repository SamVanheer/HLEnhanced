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

#ifndef GAME_SHARED_ENTITIES_WEAPONS_CM249_H
#define GAME_SHARED_ENTITIES_WEAPONS_CM249_H

enum M249Anim
{
	M249_SLOWIDLE = 0,
	M249_IDLE2,
	M249_RELOAD_START,
	M249_RELOAD_END,
	M249_HOLSTER,
	M249_DRAW,
	M249_SHOOT1,
	M249_SHOOT2,
	M249_SHOOT3
};

class CM249 : public CBasePlayerWeapon
{
public:
	DECLARE_CLASS( CM249, CBasePlayerWeapon );
	DECLARE_DATADESC();

	CM249();

	void Precache() override;

	void Spawn() override;

	bool AddDuplicate( CBasePlayerWeapon* pOriginal ) override;

	bool AddToPlayer( CBasePlayer* pPlayer ) override;

	bool Deploy() override;

	void Holster() override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void Reload() override;

	void GetWeaponData( weapon_data_t& data ) override;

	void SetWeaponData( const weapon_data_t& data ) override;

	void DecrementTimers( float flTime ) override;

private:
	static int RecalculateBody( int iClip );

private:
	int m_iShell;
	int m_iLink;
	int m_iSmoke;
	int m_iFire;

	float m_flReloadEnd;

	//Used to alternate between ejecting shells and links. - Solokiller
	bool m_bAlternatingEject;

	bool m_bReloading;

	float m_flNextAnimTime;

	unsigned short m_usFireM249;
};

#endif // GAME_SHARED_ENTITIES_WEAPONS_CM249_H

#endif // GAME_OPFOR
