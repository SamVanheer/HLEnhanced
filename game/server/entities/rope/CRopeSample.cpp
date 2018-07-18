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

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "CRopeSample.h"

BEGIN_DATADESC( CRopeSample )
	DEFINE_FIELD( m_Data.mPosition, FIELD_VECTOR ),
	DEFINE_FIELD( m_Data.mVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_Data.mForce, FIELD_VECTOR ),
	DEFINE_FIELD( m_Data.mExternalForce, FIELD_VECTOR ),
	DEFINE_FIELD( m_Data.mApplyExternalForce, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_Data.mMassReciprocal, FIELD_FLOAT ),
	DEFINE_FIELD( m_pMasterRope, FIELD_CLASSPTR ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( rope_sample, CRopeSample );

void CRopeSample::Spawn()
{
	GetEffects() |= EF_NODRAW;
}

CRopeSample* CRopeSample::CreateSample()
{
	auto pSample = static_cast<CRopeSample*>( UTIL_CreateNamedEntity( "rope_sample" ) );

	pSample->Spawn();

	return pSample;
}

#endif // GAME_OPFOR
