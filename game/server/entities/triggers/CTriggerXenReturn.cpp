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
#include "CBasePlayer.h"
#include "CBaseTrigger.h"

#include "CTriggerXenReturn.h"

LINK_ENTITY_TO_CLASS( info_displacer_earth_target, CPointEntity );

BEGIN_DATADESC( CTriggerXenReturn )
	DEFINE_TOUCHFUNC( ReturnTouch ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_xen_return, CTriggerXenReturn );

void CTriggerXenReturn::Spawn()
{
	InitTrigger();

	SetTouch( &CTriggerXenReturn::ReturnTouch );
	SetUse( nullptr );
}

void CTriggerXenReturn::ReturnTouch( CBaseEntity* pOther )
{
	if( !pOther->IsPlayer() )
		return;

	CBasePlayer* pPlayer = static_cast<CBasePlayer*>( pOther );

	CBaseEntity* pDestination = nullptr;

	float flDist = 8192;

	CBaseEntity* pTarget = nullptr;

	//Find the earth target nearest to the player's original location.

	while( ( pDestination = UTIL_FindEntityByClassname( pDestination, "info_displacer_earth_target" ) ) != nullptr )
	{
		const float flThisDist = ( pPlayer->GetDisplacerReturn() - pDestination->GetAbsOrigin() ).Length();

		if(flDist > flThisDist )
		{
			pTarget = pDestination;

			flDist = flThisDist;
		}
	}

	if( !pTarget )
		return;

	if( pTarget && pTarget != CWorld::GetInstance() )
	{
		pPlayer->GetFlags().ClearFlags( FL_SKIPLOCALHOST );

		Vector vecDest = pTarget->GetAbsOrigin();

		vecDest.z -= pPlayer->GetRelMin().z;
		vecDest.z += 1;

		pPlayer->SetAbsOrigin( vecDest );

		pPlayer->SetAbsAngles( pTarget->GetAbsAngles() );
		pPlayer->SetViewAngle( pTarget->GetAbsAngles() );
		pPlayer->SetFixAngleMode( FIXANGLE_SET );

		pPlayer->SetBaseVelocity( g_vecZero );
		pPlayer->SetAbsVelocity( g_vecZero );

		pPlayer->SetGravity( 1.0 );

		//TODO: this might not always be correct if the destination has a different room type. - Solokiller
		pPlayer->m_flSndRoomtype = pPlayer->GetDisplacerSndRoomtype();

		EMIT_SOUND( 
			pPlayer, 
			CHAN_WEAPON, "weapons/displacer_self.wav", 
			UTIL_RandomFloat( 0.8, 0.9 ), ATTN_NORM );
	}
}

#endif // GAME_OPFOR
