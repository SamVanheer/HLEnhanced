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

#ifndef GAME_SERVER_ENTITIES_WEAPONS_CSHOCKBEAM_H
#define GAME_SERVER_ENTITIES_WEAPONS_CSHOCKBEAM_H

class CShockBeam : public CGrenade
{
public:
	DECLARE_CLASS( CShockBeam, CGrenade );
	DECLARE_DATADESC();

	void Precache() override;

	void Spawn() override;

	EntityClassification_t GetClassification() override;

	void FlyThink();

	void ExplodeThink();

	void WaterExplodeThink();

	void BallTouch( CBaseEntity* pOther );

	static CShockBeam* CreateShockBeam( const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner );

private:
	void Explode();

private:
	CBeam* m_pBeam1;
	CBeam* m_pBeam2;

	CSprite* m_pSprite;

	int m_iBeams;
};

#endif // GAME_SERVER_ENTITIES_WEAPONS_CSHOCKBEAM_H

#endif // GAME_OPFOR
