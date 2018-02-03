#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "entities/CBreakable.h"
#include "entities/CSoundEnt.h"

#include "CGib.h"

#define GERMAN_GIB_COUNT		4
#define	HUMAN_GIB_COUNT			6
#define ALIEN_GIB_COUNT			4

extern DLL_GLOBAL Vector		g_vecAttackDir;

BEGIN_DATADESC( CGib )
	DEFINE_TOUCHFUNC( BounceGibTouch ),
	DEFINE_TOUCHFUNC( StickyGibTouch ),
	DEFINE_THINKFUNC( WaitTillLand ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( gib, CGib );

CGib* CGib::GibCreate( const char* szGibModel )
{
	auto pGib = static_cast<CGib*>( UTIL_CreateNamedEntity( "gib" ) );

	pGib->CreateGib( szGibModel );

	return pGib;
}

//
// Throw a chunk
//
void CGib::CreateGib( const char *szGibModel )
{
	SetMoveType( MOVETYPE_BOUNCE );
	SetFriction( 0.55 ); // deading the bounce a bit

						  // sometimes an entity inherits the edict from a former piece of glass,
						  // and will spawn using the same render FX or rendermode! bad!
	SetRenderAmount( 255 );
	SetRenderMode( kRenderNormal );
	SetRenderFX( kRenderFxNone );
	SetSolidType( SOLID_SLIDEBOX );/// hopefully this will fix the VELOCITY TOO LOW crap

	SetModel( szGibModel );
	SetSize( Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	SetNextThink( gpGlobals->time + 4 );
	m_lifeTime = 25;
	SetThink( &CGib::WaitTillLand );
	SetTouch( &CGib::BounceGibTouch );

	m_material = matNone;
	m_cBloodDecals = 5;// how many blood decals this gib can place (1 per bounce until none remain). 
}

//
// Gib bounces on the ground or wall, sponges some blood down, too!
//
void CGib::BounceGibTouch( CBaseEntity *pOther )
{
	Vector	vecSpot;
	TraceResult	tr;

	//if ( RANDOM_LONG(0,1) )
	//	return;// don't bleed everytime

	if( GetFlags().Any( FL_ONGROUND ) )
	{
		SetAbsVelocity( GetAbsVelocity() * 0.9 );
		SetAbsAngles( Vector( 0, GetAbsAngles().y, 0 ) );
		Vector vecAngles = GetAbsAngles();
		vecAngles.x = 0;
		vecAngles.z = 0;
		SetAbsAngles( vecAngles );
		Vector vecAVelocity = GetAngularVelocity();
		vecAVelocity.x = 0;
		vecAVelocity.z = 0;
		SetAngularVelocity( vecAVelocity );
	}
	else
	{
		if( g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED )
		{
			vecSpot = GetAbsOrigin() + Vector( 0, 0, 8 );//move up a bit, and trace down.
			UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -24 ), ignore_monsters, ENT( pev ), &tr );

			UTIL_BloodDecalTrace( &tr, m_bloodColor );

			m_cBloodDecals--;
		}

		if( m_material != matNone && RANDOM_LONG( 0, 2 ) == 0 )
		{
			float volume;
			float zvel = fabs( GetAbsVelocity().z );

			volume = 0.8 * min( 1.0, ( ( float ) zvel ) / 450.0 );

			CBreakable::MaterialSoundRandom( this, ( Materials ) m_material, volume );
		}
	}
}

//
// Sticky gib puts blood on the wall and stays put. 
//
void CGib::StickyGibTouch( CBaseEntity *pOther )
{
	Vector	vecSpot;
	TraceResult	tr;

	SetThink( &CGib::SUB_Remove );
	SetNextThink( gpGlobals->time + 10 );

	if( !pOther->ClassnameIs( "worldspawn" ) )
	{
		SetNextThink( gpGlobals->time );
		return;
	}

	UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 32, ignore_monsters, ENT( pev ), &tr );

	UTIL_BloodDecalTrace( &tr, m_bloodColor );

	SetAbsAngles( UTIL_VecToAngles( tr.vecPlaneNormal * -1 ) );
	SetAbsVelocity( g_vecZero );
	SetAngularVelocity( g_vecZero );
	SetMoveType( MOVETYPE_NONE );
}

//=========================================================
// WaitTillLand - in order to emit their meaty scent from
// the proper location, gibs should wait until they stop 
// bouncing to emit their scent. That's what this function
// does.
//=========================================================
void CGib::WaitTillLand( void )
{
	if( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}

	if( GetAbsVelocity() == g_vecZero )
	{
		SetThink( &CGib::SUB_StartFadeOut );
		SetNextThink( gpGlobals->time + m_lifeTime );

		// If you bleed, you stink!
		if( m_bloodColor != DONT_BLEED )
		{
			// ok, start stinkin!
			CSoundEnt::InsertSound( bits_SOUND_MEAT, GetAbsOrigin(), 384, 25 );
		}
	}
	else
	{
		// wait and check again in another half second.
		SetNextThink( gpGlobals->time + 0.5 );
	}
}

// HACKHACK -- The gib velocity equations don't work
void CGib::LimitVelocity( void )
{
	float length = GetAbsVelocity().Length();

	// ceiling at 1500.  The gib velocity equation is not bounded properly.  Rather than tune it
	// in 3 separate places again, I'll just limit it here.
	if( length > 1500.0 )
		SetAbsVelocity( GetAbsVelocity().Normalize() * 1500 );		// This should really be sv_maxvelocity * 0.75 or something
}

void CGib::SpawnHeadGib( CBaseEntity* pVictim )
{
	const char* pszModelName = nullptr;

	if( g_Language == LANGUAGE_GERMAN )
	{
		pszModelName = "models/germangibs.mdl";
	}
	else
	{
		pszModelName = "models/hgibs.mdl";
	}

	// throw one head
	auto pGib = CGib::GibCreate( pszModelName );
	pGib->SetBody( 0 );

	if( pVictim )
	{
		pGib->SetAbsOrigin( pVictim->GetAbsOrigin() + pVictim->GetViewOffset() );

		CBaseEntity* pPlayer = UTIL_FindClientInPVS( pGib );

		if( RANDOM_LONG( 0, 100 ) <= 5 && pPlayer )
		{
			// 5% chance head will be thrown at player's face.
			Vector vecVelocity = ( ( pPlayer->GetAbsOrigin() + pPlayer->GetViewOffset() ) - pGib->GetAbsOrigin() ).Normalize() * 300;
			vecVelocity.z += 100;
			pGib->SetAbsVelocity( vecVelocity );
		}
		else
		{
			pGib->SetAbsVelocity( Vector( RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( 200, 300 ) ) );
		}

		Vector vecAVelocity = pGib->GetAngularVelocity();
		vecAVelocity.x = RANDOM_FLOAT( 100, 200 );
		vecAVelocity.y = RANDOM_FLOAT( 100, 300 );
		pGib->SetAngularVelocity( vecAVelocity );

		// copy owner's blood color
		pGib->m_bloodColor = pVictim->BloodColor();

		if( pVictim->GetHealth() > -50 )
		{
			pGib->SetAbsVelocity( pGib->GetAbsVelocity() * 0.7 );
		}
		else if( pVictim->GetHealth() > -200 )
		{
			pGib->SetAbsVelocity( pGib->GetAbsVelocity() * 2 );
		}
		else
		{
			pGib->SetAbsVelocity( pGib->GetAbsVelocity() * 4 );
		}
	}
	pGib->LimitVelocity();
}

void CGib::SpawnRandomGibs( CBaseEntity* pVictim, int cGibs, int human )
{
	for( int cSplat = 0; cSplat < cGibs; cSplat++ )
	{
		const char* pszModelName = nullptr;
		int body = 0;

		if( g_Language == LANGUAGE_GERMAN )
		{
			pszModelName = "models/germangibs.mdl";
			body = RANDOM_LONG( 0, GERMAN_GIB_COUNT - 1 );
		}
		else
		{
			if( human )
			{
				// human pieces
				pszModelName = "models/hgibs.mdl";
				body = RANDOM_LONG( 1, HUMAN_GIB_COUNT - 1 );// start at one to avoid throwing random amounts of skulls (0th gib)
			}
			else
			{
				// aliens
				pszModelName = "models/agibs.mdl";
				body = RANDOM_LONG( 0, ALIEN_GIB_COUNT - 1 );
			}
		}

		auto pGib = CGib::GibCreate( pszModelName );
		pGib->SetBody( body );

		if( pVictim )
		{
			// spawn the gib somewhere in the monster's bounding volume
			pGib->SetAbsOrigin( Vector( 
				pVictim->GetAbsMin().x + pVictim->GetBounds().x * ( RANDOM_FLOAT( 0, 1 ) ),
				pVictim->GetAbsMin().y + pVictim->GetBounds().y * ( RANDOM_FLOAT( 0, 1 ) ),
				pVictim->GetAbsMin().z + pVictim->GetBounds().z * ( RANDOM_FLOAT( 0, 1 ) ) + 1 ) );	// absmin.z is in the floor because the engine subtracts 1 to enlarge the box

			// make the gib fly away from the attack vector
			Vector vecVelocity = g_vecAttackDir * -1;

			// mix in some noise
			vecVelocity.x += RANDOM_FLOAT( -0.25, 0.25 );
			vecVelocity.y += RANDOM_FLOAT( -0.25, 0.25 );
			vecVelocity.z += RANDOM_FLOAT( -0.25, 0.25 );

			vecVelocity = vecVelocity * RANDOM_FLOAT( 300, 400 );

			Vector vecAVelocity = pGib->GetAngularVelocity();
			vecAVelocity.x = RANDOM_FLOAT( 100, 200 );
			vecAVelocity.y = RANDOM_FLOAT( 100, 300 );
			pGib->SetAngularVelocity( vecAVelocity );

			// copy owner's blood color
			pGib->m_bloodColor = pVictim->BloodColor();

			if( pVictim->GetHealth() > -50 )
			{
				vecVelocity = vecVelocity * 0.7;
			}
			else if( pVictim->GetHealth() > -200 )
			{
				vecVelocity = vecVelocity * 2;
			}
			else
			{
				vecVelocity = vecVelocity * 4;
			}

			pGib->SetAbsVelocity( vecVelocity );

			pGib->SetSolidType( SOLID_BBOX );
			pGib->SetSize( Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
		}
		pGib->LimitVelocity();
	}
}

void CGib::SpawnStickyGibs( CBaseEntity* pVictim, Vector vecOrigin, int cGibs )
{
	//TODO: this code is similar to the code above - Solokiller
	int i;

	if( g_Language == LANGUAGE_GERMAN )
	{
		// no sticky gibs in germany right now!
		return;
	}

	for( i = 0; i < cGibs; i++ )
	{
		auto pGib = CGib::GibCreate( "models/stickygib.mdl" );
		pGib->SetBody( RANDOM_LONG( 0, 2 ) );

		if( pVictim )
		{
			pGib->SetAbsOrigin( Vector(
				vecOrigin.x + RANDOM_FLOAT( -3, 3 ),
				vecOrigin.y + RANDOM_FLOAT( -3, 3 ),
				vecOrigin.z + RANDOM_FLOAT( -3, 3 ) ) );

			/*
			pGib->SetAbsOrigin( Vector(
				pevVictim->absmin.x + pevVictim->size.x * (RANDOM_FLOAT ( 0 , 1 ) ),
				pevVictim->absmin.y + pevVictim->size.y * (RANDOM_FLOAT ( 0 , 1 ) ),
				pevVictim->absmin.z + pevVictim->size.z * (RANDOM_FLOAT ( 0 , 1 ) ) ) );
			*/

			// make the gib fly away from the attack vector
			Vector vecVelocity = g_vecAttackDir * -1;

			// mix in some noise
			vecVelocity.x += RANDOM_FLOAT( -0.15, 0.15 );
			vecVelocity.y += RANDOM_FLOAT( -0.15, 0.15 );
			vecVelocity.z += RANDOM_FLOAT( -0.15, 0.15 );

			vecVelocity = vecVelocity * 900;


			Vector vecAVelocity = pGib->GetAngularVelocity();
			vecAVelocity.x = RANDOM_FLOAT( 250, 400 );
			vecAVelocity.y = RANDOM_FLOAT( 250, 400 );
			pGib->SetAngularVelocity( vecAVelocity );

			// copy owner's blood color
			pGib->m_bloodColor = pVictim->BloodColor();

			if( pVictim->GetHealth() > -50 )
			{
				vecVelocity = vecVelocity * 0.7;
			}
			else if( pVictim->GetHealth() > -200 )
			{
				vecVelocity = vecVelocity * 2;
			}
			else
			{
				vecVelocity = vecVelocity * 4;
			}

			pGib->SetAbsVelocity( vecVelocity );

			pGib->SetMoveType( MOVETYPE_TOSS );
			pGib->SetSolidType( SOLID_BBOX );
			pGib->SetSize( Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );
			pGib->SetTouch( &CGib::StickyGibTouch );
			pGib->SetThink( NULL );
		}
		pGib->LimitVelocity();
	}
}
