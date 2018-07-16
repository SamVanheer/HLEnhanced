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
#ifndef GAME_SHARED_ENTITIES_WEAPONS_CDISPLACER_H
#define GAME_SHARED_ENTITIES_WEAPONS_CDISPLACER_H

enum DisplacerAnim
{
	DISPLACER_IDLE1 = 0,
	DISPLACER_IDLE2,
	DISPLACER_SPINUP,
	DISPLACER_SPIN,
	DISPLACER_FIRE,
	DISPLACER_DRAW,
	DISPLACER_HOLSTER1
};

class CDisplacer : public CBasePlayerWeapon
{
public:
	static const size_t NUM_BEAMS = 4;

	enum class Mode
	{
		STARTED = 0,
		SPINNING_UP,
		SPINNING,
		FIRED
	};

public:
	DECLARE_CLASS( CDisplacer, CBasePlayerWeapon );
	DECLARE_DATADESC();

	CDisplacer();

	void Precache() override;

	void Spawn() override;

	bool AddToPlayer( CBasePlayer* pPlayer ) override;

	bool Deploy() override;

	void Holster() override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

	void Reload() override;

	void SpinupThink();

	void AltSpinupThink();

	void FireThink();

	void AltFireThink();

private:
	int m_iSpriteTexture;

	float m_flStartTime;
	float m_flSoundDelay;

	Mode m_Mode;

	int m_iImplodeCounter;
	int m_iSoundState;

	unsigned short m_usFireDisplacer;
};

#endif // GAME_SHARED_ENTITIES_WEAPONS_CDISPLACER_H

#endif // GAME_OPFOR
