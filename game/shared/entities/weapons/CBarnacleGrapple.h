/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#ifdef GAME_OPFOR

#ifndef GAME_SHARED_ENTITIES_WEAPONS_CBARNACLEGRAPPLE_H
#define GAME_SHARED_ENTITIES_WEAPONS_CBARNACLEGRAPPLE_H

class CBarnacleGrappleTip;

enum BarnacleGrappleAnim
{
	BGRAPPLE_BREATHE = 0,
	BGRAPPLE_LONGIDLE,
	BGRAPPLE_SHORTIDLE,
	BGRAPPLE_COUGH,
	BGRAPPLE_DOWN,
	BGRAPPLE_UP,
	BGRAPPLE_FIRE,
	BGRAPPLE_FIREWAITING,
	BGRAPPLE_FIREREACHED,
	BGRAPPLE_FIRETRAVEL,
	BGRAPPLE_FIRERELEASE
};

class CBarnacleGrapple : public CBasePlayerWeapon
{
private:
	enum class FireState
	{
		OFF		= 0,
		CHARGE	= 1
	};

public:
	DECLARE_CLASS( CBarnacleGrapple, CBasePlayerWeapon );
	DECLARE_DATADESC();

	CBarnacleGrapple();

	void Precache() override;

	void Spawn() override;

	bool AddToPlayer( CBasePlayer* pPlayer ) override;

	bool Deploy() override;

	void Holster() override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

private:
	void Fire( const Vector& vecOrigin, const Vector& vecDir );
	void EndAttack();

	void CreateEffect();
	void UpdateEffect();
	void DestroyEffect();

private:
	CBarnacleGrappleTip* m_pTip;

	CBeam* m_pBeam;

	float m_flShootTime;
	float m_flDamageTime;

	FireState m_FireState;

	bool m_bGrappling;

	bool m_bMissed;

	bool m_bMomentaryStuck;
};

#endif // GAME_SHARED_ENTITIES_WEAPONS_CBARNACLEGRAPPLE_H

#endif // GAME_OPFOR
