#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"

#include "Decals.h"

#include "CSprayCan.h"

LINK_ENTITY_TO_CLASS( spray_can, CSprayCan );

void CSprayCan::CreateSprayCan( CBaseEntity* pOwner )
{
	SetAbsOrigin( pOwner->GetAbsOrigin() + Vector( 0, 0, 32 ) );
	SetAbsAngles( pOwner->GetViewAngle() );
	SetOwner( pOwner );
	SetFrame( 0 );

	SetNextThink( gpGlobals->time + 0.1 );
	EMIT_SOUND( this, CHAN_VOICE, "player/sprayer.wav", 1, ATTN_NORM );
}

void CSprayCan::Think()
{
	TraceResult	tr;
	int nFrames;
	CBasePlayer* pPlayer = ( CBasePlayer* ) GetOwner();

	if( pPlayer )
		nFrames = pPlayer->GetCustomDecalFrames();
	else
		nFrames = -1;

	const int playernum = pPlayer ? pPlayer->entindex() : 0;

	// ALERT(at_console, "Spray by player %i, %i of %i\n", playernum, (int)(GetFrame() + 1), nFrames);

	UTIL_MakeVectors( GetAbsAngles() );
	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, &tr );

	// No customization present.
	if( nFrames == -1 )
	{
		UTIL_DecalTrace( &tr, DECAL_LAMBDA6 );
		UTIL_Remove( this );
	}
	else
	{
		UTIL_PlayerDecalTrace( &tr, playernum, GetFrame(), true );
		// Just painted last custom frame.
		const auto flLastFrame = GetFrame();
		SetFrame( flLastFrame + 1 );
		if( flLastFrame >= ( nFrames - 1 ) )
			UTIL_Remove( this );
	}

	SetNextThink( gpGlobals->time + 0.1 );
}
