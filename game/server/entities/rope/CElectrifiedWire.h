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

#ifndef GAME_SERVER_ENTITIES_ROPE_CELECTRIFIEDWIRE_H
#define GAME_SERVER_ENTITIES_ROPE_CELECTRIFIEDWIRE_H

#include "CRope.h"

/**
*	An electrified wire.
*	Can be toggled on and off. Starts on.
*/
class CElectrifiedWire : public CRope
{
public:
	DECLARE_CLASS( CElectrifiedWire, CRope );
	DECLARE_DATADESC();

	CElectrifiedWire();
	
	void KeyValue( KeyValueData* pkvd ) override;

	void Precache() override;

	void Spawn() override;

	void Think() override;

	void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue ) override;

	/**
	*	@return Whether the wire is active.
	*/
	bool IsActive() const { return m_bIsActive; }

	/**
	*	@param iFrequency Frequency.
	*	@return Whether the spark effect should be performed.
	*/
	bool ShouldDoEffect( const int iFrequency );

	/**
	*	Do spark effects.
	*/
	void DoSpark( const size_t uiSegment, const bool bExertForce );

	/**
	*	Do lightning effects.
	*/
	void DoLightning();

private:
	bool m_bIsActive;

	int m_iTipSparkFrequency;
	int m_iBodySparkFrequency;
	int m_iLightningFrequency;

	int m_iXJoltForce;
	int m_iYJoltForce;
	int m_iZJoltForce;

	size_t m_uiNumUninsulatedSegments;
	size_t m_uiUninsulatedSegments[ MAX_SEGMENTS ];

	int m_iLightningSprite;

	float m_flLastSparkTime;
};

#endif // GAME_SERVER_ENTITIES_ROPE_CELECTRIFIEDWIRE_H

#endif // GAME_OPFOR
