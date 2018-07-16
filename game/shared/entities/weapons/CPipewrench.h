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

#ifndef GAME_SHARED_ENTITIES_WEAPONS_CPIPEWRENCH_H
#define GAME_SHARED_ENTITIES_WEAPONS_CPIPEWRENCH_H

enum pipewrench_e
{
	PIPEWRENCH_IDLE1 = 0,
	PIPEWRENCH_IDLE2,
	PIPEWRENCH_IDLE3,
	PIPEWRENCH_DRAW,
	PIPEWRENCH_HOLSTER,
	PIPEWRENCH_ATTACK1HIT,
	PIPEWRENCH_ATTACK1MISS,
	PIPEWRENCH_ATTACK2HIT,
	PIPEWRENCH_ATTACK2MISS,
	PIPEWRENCH_ATTACK3HIT,
	PIPEWRENCH_ATTACK3MISS,
	PIPEWRENCH_BIG_SWING_START,
	PIPEWRENCH_BIG_SWING_HIT,
	PIPEWRENCH_BIG_SWING_MISS,
	PIPEWRENCH_BIG_SWING_IDLE
};

class CPipewrench : public CBasePlayerWeapon
{
public:
	DECLARE_CLASS( CPipewrench, CBasePlayerWeapon );
#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	CPipewrench();

	void Spawn( void ) override;
	void Precache( void ) override;
	void SwingAgain( void );
	void Smack( void );

	void PrimaryAttack( void ) override;
	void SecondaryAttack( void ) override;
	bool Swing( const bool bFirst );
	void BigSwing( void );
	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;

	void GetWeaponData( weapon_data_t& data ) override;

	void SetWeaponData( const weapon_data_t& data ) override;

	float m_flBigSwingStart;
	int m_iSwingMode;
	int m_iSwing;
	TraceResult m_trHit;

private:
	unsigned short m_usPipewrench;
};

#endif // GAME_SHARED_ENTITIES_WEAPONS_CPIPEWRENCH_H

#endif // GAME_OPFOR
