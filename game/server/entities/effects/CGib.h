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
#ifndef GAME_SERVER_ENTITIES_EFFECTS_CGIB_H
#define GAME_SERVER_ENTITIES_EFFECTS_CGIB_H

//
// A gib is a chunk of a body, or a piece of wood/metal/rocks/etc.
//
class CGib : public CBaseEntity
{
public:
	DECLARE_CLASS( CGib, CBaseEntity );
	DECLARE_DATADESC();

	static CGib* GibCreate( const char* szGibModel );

	void CreateGib( const char *szGibModel );
	void BounceGibTouch( CBaseEntity *pOther );
	void StickyGibTouch( CBaseEntity *pOther );
	void WaitTillLand( void );
	void		LimitVelocity( void );

	virtual int	ObjectCaps() const override { return ( CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION ) | FCAP_DONT_SAVE; }
	static	void SpawnHeadGib( CBaseEntity* pVictim );
	static	void SpawnRandomGibs( CBaseEntity* pVictim, int cGibs, int human );
	static  void SpawnStickyGibs( CBaseEntity* pVictim, Vector vecOrigin, int cGibs );

	int		m_bloodColor;
	int		m_cBloodDecals;
	int		m_material;
	float	m_lifeTime;
};

#endif //GAME_SERVER_ENTITIES_EFFECTS_CGIB_H
