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

#ifndef GAME_SERVER_ENTITIES_NPCS_CPENGUINGRENADE_H
#define GAME_SERVER_ENTITIES_NPCS_CPENGUINGRENADE_H

enum PenguinGrenadeAnim
{
	PENGUINGRENADE_IDLE = 0,
	PENGUINGRENADE_FIDGET,
	PENGUINGRENADE_JUMP,
	PENGUINGRENADE_RUN
};

#define PENGUIN_DETONATE_DELAY 15.0

class CPenguinGrenade : public CGrenade
{
public:
	DECLARE_CLASS( CPenguinGrenade, CGrenade );
	DECLARE_DATADESC();

	void Precache() override;

	void Spawn() override;

	EntityClassification_t GetClassification() override;

	int BloodColor() const override { return BLOOD_COLOR_YELLOW; }

	void Killed( const CTakeDamageInfo& info, GibAction gibAction ) override;

	void GibMonster() override;

	void SuperBounceTouch( CBaseEntity *pOther );

	void HuntThink();

	void Smoke();

private:
	static float m_flNextBounceSoundTime;

	float m_flDie;
	Vector m_vecTarget;
	float m_flNextHunt;
	float m_flNextHit;
	Vector m_posPrev;
	EHANDLE m_hOwner;
	EntityClassification_t  m_iMyClass = EntityClassifications().GetNoneId();
};

#endif // GAME_SERVER_ENTITIES_NPCS_CPENGUINGRENADE_H

#endif // GAME_OPFOR
