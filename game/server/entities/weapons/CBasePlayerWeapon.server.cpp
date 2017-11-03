#include "extdll.h"
#include "util.h"
#include "gamerules/GameRules.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "Weapons.h"

#include "entities/weapons/CBasePlayerWeapon.h"

extern bool gEvilImpulse101;

bool CBasePlayerWeapon::AddToPlayer( CBasePlayer *pPlayer )
{
	m_pPlayer = pPlayer;

	pPlayer->GetWeapons().AddFlags( 1 << m_iId );

	return AddWeapon();
}

// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
bool CBasePlayerWeapon::AddDuplicate( CBasePlayerWeapon *pOriginal )
{
	if( m_iDefaultAmmo )
	{
		return ExtractAmmo( pOriginal );
	}
	else
	{
		// a dead player dropped this.
		return ExtractClipAmmo( pOriginal );
	}
}

void CBasePlayerWeapon::DestroyItem( void )
{
	if( m_pPlayer )
	{
		// if attached to a player, remove. 
		m_pPlayer->RemovePlayerItem( this );
	}

	Kill();
}

void CBasePlayerWeapon::DefaultTouch( CBaseEntity *pOther )
{
	// if it's not a player, ignore
	if( !pOther->IsPlayer() )
		return;

	CBasePlayer *pPlayer = ( CBasePlayer * ) pOther;

	// can I have this?
	if( !g_pGameRules->CanHavePlayerItem( pPlayer, this ) )
	{
		if( gEvilImpulse101 )
		{
			UTIL_Remove( this );
		}
		return;
	}

	if( pPlayer->AddPlayerItem( this ) )
	{
		AttachToPlayer( pPlayer );
		EMIT_SOUND( pPlayer, CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM );
	}

	SUB_UseTargets( pOther, USE_TOGGLE, 0 ); // UNDONE: when should this happen?
}

//=========================================================
// FallThink - Items that have just spawned run this think
// to catch them when they hit the ground. Once we're sure
// that the object is grounded, we change its solid type
// to trigger and set it in a large box that helps the
// player get it.
//=========================================================
void CBasePlayerWeapon::FallThink( void )
{
	SetNextThink( gpGlobals->time + 0.1 );

	if( GetFlags().Any( FL_ONGROUND ) )
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if( !FNullEnt( GetOwner() ) )
		{
			int pitch = 95 + RANDOM_LONG( 0, 29 );
			EMIT_SOUND_DYN( this, CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch );
		}

		// lie flat
		Vector vecAngles = GetAbsAngles();
		vecAngles.x = 0;
		vecAngles.z = 0;
		SetAbsAngles( vecAngles );

		Materialize();
	}
}

//=========================================================
// Materialize - make a CBasePlayerWeapon visible and tangible
//=========================================================
void CBasePlayerWeapon::Materialize( void )
{
	if( GetEffects().Any( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN( this, CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150 );
		GetEffects().ClearFlags( EF_NODRAW );
		GetEffects() |= EF_MUZZLEFLASH;
	}

	SetSolidType( SOLID_TRIGGER );

	SetAbsOrigin( GetAbsOrigin() );// link into world.
	SetTouch( &CBasePlayerWeapon::DefaultTouch );
	SetThink( NULL );

}

//=========================================================
// AttemptToMaterialize - the item is trying to rematerialize,
// should it do so now or wait longer?
//=========================================================
void CBasePlayerWeapon::AttemptToMaterialize( void )
{
	float time = g_pGameRules->FlWeaponTryRespawn( this );

	if( time == 0 )
	{
		Materialize();
		return;
	}

	SetNextThink( gpGlobals->time + time );
}

//=========================================================
// Respawn- this item is already in the world, but it is
// invisible and intangible. Make it visible and tangible.
//=========================================================
CBaseEntity* CBasePlayerWeapon::Respawn( void )
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity *pNewWeapon = CBaseEntity::Create( ( char * ) GetClassname(), g_pGameRules->VecWeaponRespawnSpot( this ), GetAbsAngles(), pev->owner );

	if( pNewWeapon )
	{
		pNewWeapon->GetEffects() |= EF_NODRAW;// invisible for now
		pNewWeapon->SetTouch( NULL );// no touch
		pNewWeapon->SetThink( &CBasePlayerWeapon::AttemptToMaterialize );

		UTIL_DropToFloor( this );

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->SetNextThink( g_pGameRules->FlWeaponRespawnTime( this ) );
	}
	else
	{
		ALERT( at_console, "Respawn failed to create %s!\n", GetClassname() );
	}

	return pNewWeapon;
}

//=========================================================
// Sets up movetype, size, solidtype for a new weapon. 
//=========================================================
void CBasePlayerWeapon::FallInit( void )
{
	SetMoveType( MOVETYPE_TOSS );
	SetSolidType( SOLID_BBOX );

	SetAbsOrigin( GetAbsOrigin() );
	SetSize( Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );//pointsize until it lands on the ground.

	SetTouch( &CBasePlayerWeapon::DefaultTouch );
	SetThink( &CBasePlayerWeapon::FallThink );

	SetNextThink( gpGlobals->time + 0.1 );
}

//=========================================================
// CheckRespawn - a player is taking this weapon, should 
// it respawn?
//=========================================================
void CBasePlayerWeapon::CheckRespawn( void )
{
	switch( g_pGameRules->WeaponShouldRespawn( this ) )
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}

//=========================================================
// called by the new item with the existing item as parameter
//
// if we call ExtractAmmo(), it's because the player is picking up this type of weapon for 
// the first time. If it is spawned by the world, m_iDefaultAmmo will have a default ammo amount in it.
// if  this is a weapon dropped by a dying player, has 0 m_iDefaultAmmo, which means only the ammo in 
// the weapon clip comes along. 
//=========================================================
bool CBasePlayerWeapon::ExtractAmmo( CBasePlayerWeapon *pWeapon )
{
	bool bReturn = false;

	if( pszAmmo1() != NULL )
	{
		// blindly call with m_iDefaultAmmo. It's either going to be a value or zero. If it is zero,
		// we only get the ammo in the weapon's clip, which is what we want. 
		bReturn = pWeapon->AddPrimaryAmmo( m_iDefaultAmmo, ( char * ) pszAmmo1(), iMaxClip() );
		m_iDefaultAmmo = 0;
	}

	if( pszAmmo2() != NULL )
	{
		bReturn = pWeapon->AddSecondaryAmmo( 0, pszAmmo2() );
	}

	return bReturn;
}

//=========================================================
// called by the new item's class with the existing item as parameter
//=========================================================
bool CBasePlayerWeapon::ExtractClipAmmo( CBasePlayerWeapon *pWeapon )
{
	int			iAmmo;

	if( m_iClip == WEAPON_NOCLIP )
	{
		iAmmo = 0;// guns with no clips always come empty if they are second-hand
	}
	else
	{
		iAmmo = m_iClip;
	}

	//Used to return the return value directly, but this was never 0. It's supposed to return true if ammo is allowed to be added (even if no ammo was actually added). - Solokiller
	return pWeapon->m_pPlayer->GiveAmmo( iAmmo, ( char * ) pszAmmo1() ) != -1; // , &m_iPrimaryAmmoType
}

bool CBasePlayerWeapon::AddPrimaryAmmo( int iCount, const char *szName, int iMaxClip )
{
	int iIdAmmo;

	if( iMaxClip < 1 )
	{
		m_iClip = -1;
		iIdAmmo = m_pPlayer->GiveAmmo( iCount, szName );
	}
	else if( m_iClip == 0 )
	{
		int i;
		i = min( m_iClip + iCount, iMaxClip ) - m_iClip;
		m_iClip += i;
		iIdAmmo = m_pPlayer->GiveAmmo( iCount - i, szName );
	}
	else
	{
		iIdAmmo = m_pPlayer->GiveAmmo( iCount, szName );
	}

	// m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = iMaxCarry; // hack for testing

	if( iIdAmmo > 0 )
	{
		if( m_pPlayer->HasPlayerItem( this ) )
		{
			// play the "got ammo" sound only if we gave some ammo to a player that already had this gun.
			// if the player is just getting this gun for the first time, DefaultTouch will play the "picked up gun" sound for us.
			EMIT_SOUND( this, CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
		}
	}

	return iIdAmmo > 0;
}


bool CBasePlayerWeapon::AddSecondaryAmmo( int iCount, const char *szName )
{
	int iIdAmmo;

	iIdAmmo = m_pPlayer->GiveAmmo( iCount, szName );

	//m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = iMax; // hack for testing

	if( iIdAmmo > 0 )
	{
		EMIT_SOUND( this, CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
	}
	return iIdAmmo > 0;
}

void CBasePlayerWeapon::SendWeaponAnim( int iAnim, int body )
{
	const bool bSkipLocal = IsPredicted();

	m_pPlayer->SetWeaponAnim( iAnim );

#if defined( CLIENT_WEAPONS )
	if( bSkipLocal && ENGINE_CANSKIP( m_pPlayer->edict() ) )
		return;
#endif

	MESSAGE_BEGIN( MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer );
	WRITE_BYTE( iAnim );						// sequence number
	WRITE_BYTE( GetBody() );					// weaponmodel bodygroup.
	MESSAGE_END();
}

void CBasePlayerWeapon::Drop( void )
{
	SetTouch( NULL );
	SetThink( &CBasePlayerWeapon::SUB_Remove );
	SetNextThink( gpGlobals->time + .1 );
}

void CBasePlayerWeapon::Kill( void )
{
	SetTouch( NULL );
	SetThink( &CBasePlayerWeapon::SUB_Remove );
	SetNextThink( gpGlobals->time + .1 );
}

void CBasePlayerWeapon::AttachToPlayer( CBasePlayer *pPlayer )
{
	SetMoveType( MOVETYPE_FOLLOW );
	SetSolidType( SOLID_NOT );
	SetAimEntity( pPlayer );
	GetEffects() = EF_NODRAW; // ??
	SetModelIndex( 0 );// server won't send down to clients if modelindex == 0
	SetModelName( iStringNull );
	SetOwner( pPlayer );
	SetNextThink( gpGlobals->time + .1 );
	SetTouch( NULL );
}

//=========================================================
// IsUseable - this function determines whether or not a 
// weapon is useable by the player in its current state. 
// (does it have ammo loaded? do I have any ammo for the 
// weapon?, etc)
//=========================================================
bool CBasePlayerWeapon::IsUseable()
{
	if( m_iClip <= 0 )
	{
		if( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] <= 0 && ( GetWeaponInfo()->GetPrimaryAmmo() && GetWeaponInfo()->GetPrimaryAmmo()->GetMaxCarry() != -1 ) )
		{
			// clip is empty (or nonexistant) and the player has no more ammo of this type. 
			return false;
		}
	}

	return true;
}

bool CBasePlayerWeapon::DefaultDeploy( const char* const pszViewModel, const char* const pszWeaponModel, int iAnim, const char* const pszAnimExt, int body )
{
	if( !CanDeploy() )
		return false;

	//TODO: need to alloc these for custom ents - Solokiller
	m_pPlayer->SetViewModelName( pszViewModel );
	m_pPlayer->SetWeaponModelName( pszWeaponModel );
	m_pPlayer->SetWeaponAnimType( pszAnimExt );
	SendWeaponAnim( iAnim, body );

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	m_flLastFireTime = 0.0;

	return true;
}

bool CBasePlayerWeapon::UpdateClientData( CBasePlayer *pPlayer )
{
	bool bSend = false;
	WpnOnTargetState state = WpnOnTargetState::NOT_ACTIVE_WEAPON;
	if( pPlayer->m_pActiveItem == this )
	{
		if( pPlayer->m_fOnTarget )
			state = WpnOnTargetState::ACTIVE_IS_ONTARGET;
		else
			state = WpnOnTargetState::ACTIVE_WEAPON;
	}

	// Forcing send of all data!
	if( !pPlayer->m_fWeapon )
	{
		bSend = true;
	}

	// This is the current or last weapon, so the state will need to be updated
	if( this == pPlayer->m_pActiveItem ||
		this == pPlayer->m_pClientActiveItem )
	{
		if( pPlayer->m_pActiveItem != pPlayer->m_pClientActiveItem )
		{
			bSend = true;
		}
	}

	// If the ammo, state, or fov has changed, update the weapon
	if( m_iClip != m_iClientClip ||
		state != m_iClientWeaponState ||
		pPlayer->m_iFOV != pPlayer->m_iClientFOV )
	{
		bSend = true;
	}

	if( bSend )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pPlayer );
		WRITE_BYTE( static_cast<int>( state ) );
		WRITE_BYTE( m_iId );
		WRITE_BYTE( m_iClip );
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = true;
	}

	if( m_pNext )
		m_pNext->UpdateClientData( pPlayer );

	return true;
}

//=========================================================
// RetireWeapon - no more ammo for this gun, put it away.
//=========================================================
void CBasePlayerWeapon::RetireWeapon( void )
{
	// first, no viewmodel at all.
	m_pPlayer->ClearViewModelName();
	m_pPlayer->ClearWeaponModelName();

	g_pGameRules->GetNextBestWeapon( m_pPlayer, this );
}

void CBasePlayerWeapon::Holster()
{
	m_fInReload = false; // cancel any reload in progress.
	m_pPlayer->ClearViewModelName();
	m_pPlayer->ClearWeaponModelName();
}

void CBasePlayerWeapon::PrintState( void )
{
	ALERT( at_console, "primary:  %f\n", m_flNextPrimaryAttack );
	ALERT( at_console, "idle   :  %f\n", m_flTimeWeaponIdle );

	//	ALERT( at_console, "nextrl :  %f\n", m_flNextReload );
	//	ALERT( at_console, "nextpum:  %f\n", m_flPumpTime );

	//	ALERT( at_console, "m_frt  :  %f\n", m_fReloadTime );
	ALERT( at_console, "m_finre:  %s\n", m_fInReload ? "true" : "false" );
	//	ALERT( at_console, "m_finsr:  %i\n", m_fInSpecialReload );

	ALERT( at_console, "m_iclip:  %i\n", m_iClip );
}

//=========================================================================
// GetNextAttackDelay - An accurate way of calcualting the next attack time.
//=========================================================================
float CBasePlayerWeapon::GetNextAttackDelay( float delay )
{
	if( m_flLastFireTime == 0 || m_flNextPrimaryAttack == -1 )
	{
		// At this point, we are assuming that the client has stopped firing
		// and we are going to reset our book keeping variables.
		m_flLastFireTime = gpGlobals->time;
		m_flPrevPrimaryAttack = delay;
	}
	// calculate the time between this shot and the previous
	float flTimeBetweenFires = gpGlobals->time - m_flLastFireTime;
	float flCreep = 0.0f;
	if( flTimeBetweenFires > 0 )
		flCreep = flTimeBetweenFires - m_flPrevPrimaryAttack; // postive or negative

															  // save the last fire time
	m_flLastFireTime = gpGlobals->time;

	float flNextAttack = UTIL_WeaponTimeBase() + delay - flCreep;
	// we need to remember what the m_flNextPrimaryAttack time is set to for each shot, 
	// store it as m_flPrevPrimaryAttack.
	m_flPrevPrimaryAttack = flNextAttack - UTIL_WeaponTimeBase();
	// 	char szMsg[256];
	// 	V_sprintf_safe( szMsg, "next attack time: %0.4f\n", gpGlobals->time + flNextAttack );
	// 	OutputDebugString( szMsg );
	return flNextAttack;
}