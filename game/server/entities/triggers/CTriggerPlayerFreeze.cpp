#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "gamerules/GameRules.h"

#include "CTriggerPlayerFreeze.h"

BEGIN_DATADESC( CTriggerPlayerFreeze )
	DEFINE_THINKFUNC( PlayerFreezeDelay ),
	DEFINE_FIELD( m_bUnFrozen, FIELD_BOOLEAN ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_playerfreeze, CTriggerPlayerFreeze );

void CTriggerPlayerFreeze::Spawn()
{
	//TODO: not really that useful if it removes itself - Solokiller
	if( g_pGameRules->IsDeathmatch() )
		REMOVE_ENTITY( edict() );
	else
		m_bUnFrozen = true;
}

void CTriggerPlayerFreeze::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue )
{
	//TODO: get rid of this member and use the player's own state to determine what to do - Solokiller
	m_bUnFrozen = !m_bUnFrozen;

	//TODO: This is pretty bad, but it's how OP4 and BS do it. Rework to ignore if player doesn't exist, add flags to control target - Solokiller
	auto pPlayer = UTIL_PlayerByIndex( 1 );

	pPlayer->EnableControl( m_bUnFrozen );
}

void CTriggerPlayerFreeze::PlayerFreezeDelay()
{
	auto pPlayer = UTIL_PlayerByIndex( 1 );

	if( pPlayer )
		pPlayer->EnableControl( m_bUnFrozen );

	SetThink( nullptr );
}

bool CTriggerPlayerFreeze::Restore( CRestore& restore )
{
	if( !BaseClass::Restore( restore ) )
		return false;

	//TODO: need to save player state in Save so it matches the singleplayer setting more closely. This can go out of sync if the player is modified by anything else - Solokiller
	if( !m_bUnFrozen )
	{
		SetThink( &ThisClass::PlayerFreezeDelay );
		SetNextThink( gpGlobals->time + 0.5f );
	}

	return true;
}
