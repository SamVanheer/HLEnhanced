#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "Weapons.h"
#include "CBasePlayer.h"
#include "config/CServerConfig.h"

#include "CMap.h"

BEGIN_DATADESC_NOBASE( CMap )
	DEFINE_FIELD( m_flPrevFrameTime, FIELD_TIME ),
	DEFINE_FIELD( m_bUseCustomHudColors, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLastHudColorChangeTime, FIELD_TIME ),
	DEFINE_FIELD( m_HudColors.m_PrimaryColor, FIELD_INTEGER ),
	DEFINE_FIELD( m_HudColors.m_EmptyItemColor, FIELD_INTEGER ),
	DEFINE_FIELD( m_HudColors.m_AmmoBarColor, FIELD_INTEGER ),
END_DATADESC()

CMap* CMap::m_pInstance = nullptr;

CMap* CMap::CreateInstance()
{
	ASSERT( !m_pInstance );

	m_pInstance = new CMap();

	m_pInstance->Create();

	return m_pInstance;
}

CMap* CMap::GetInstance()
{
	ASSERT( m_pInstance );

	return m_pInstance;
}

void CMap::DestroyInstance()
{
	ASSERT( m_pInstance );

	delete m_pInstance;

	m_pInstance = nullptr;
}

bool CMap::Exists()
{
	return m_pInstance != nullptr;
}

CMap* CMap::CreateIfNeeded()
{
	if( !Exists() )
		CreateInstance();

	return GetInstance();
}

CMap::CMap()
{
	EntityClassifications().Reset();
	InitializeEntityClassifications();

	LoadMapConfig();
}

CMap::~CMap()
{
}

void CMap::LoadMapConfig()
{
	m_MapConfig = std::make_unique<CServerConfig>();
	char szConfigName[ MAX_PATH ];

	V_sprintf_safe( szConfigName, "maps/%s.xml", STRING( gpGlobals->mapname ) );

	if( !m_MapConfig->Parse( szConfigName, nullptr, true ) )
	{
		m_MapConfig.reset();
	}
}

bool CMap::Save( CSave& save )
{
	auto pDataMap = GetDataMap();

	return save.WriteFields( "CMap", this, *pDataMap, pDataMap->pTypeDesc, pDataMap->uiNumDescriptors );
}

bool CMap::Restore( CRestore& restore )
{
	auto pDataMap = GetDataMap();

	return restore.ReadFields( "CMap", this, *pDataMap, pDataMap->pTypeDesc, pDataMap->uiNumDescriptors );
}

void CMap::WorldInit()
{
	if( m_MapConfig )
	{
		//TODO: invoke Angelscript pre classification parse - Solokiller
		m_MapConfig->ProcessEntityClassifications();
		//TODO: invoke Angelscript post classification parse - Solokiller
	}
}

void CMap::WorldActivated()
{
	if( m_MapConfig )
		m_MapConfig->ProcessCVars();
}

void CMap::Create()
{
	//Ammo types have to be registered ahead of time in all cases.
	RegisterAmmoTypes();

	ResetHudColors();
}

void CMap::Think()
{
	CBasePlayer* pPlayer;

	for( int iPlayer = 1; iPlayer <= gpGlobals->maxClients; ++iPlayer )
	{
		pPlayer = UTIL_PlayerByIndex( iPlayer );

		if( !pPlayer || !pPlayer->IsConnected() )
			continue;

		SendHudColors( pPlayer );
	}

	m_flPrevFrameTime = gpGlobals->time;
}

void CMap::SendHudColors( CBasePlayer* pPlayer, const bool bForce )
{
	if( !pPlayer->UsesCustomHudColors() )
	{
		//Not using per-player settings and last global change time was after last think, send changes.
		//Players may have reset their color; signalled by UsesCustomHudColors returning false and GetLastHudColorChangeTime being >= m_flPrevFrameTime.
		if( bForce || ( m_flLastHudColorChangeTime >= m_flPrevFrameTime ) || ( pPlayer->GetLastHudColorChangeTime() >= m_flPrevFrameTime ) )
			SendHudColors( pPlayer, m_HudColors );
	}
	else
	{
		//Using per-player settings and last change time was after last think, send changes.
		if( bForce || (pPlayer->GetLastHudColorChangeTime() >= m_flPrevFrameTime ) )
			SendHudColors( pPlayer, pPlayer->GetHudColors() );
	}
}

void CMap::SendHudColors( CBasePlayer* pPlayer, const CHudColors& colors )
{
	MESSAGE_BEGIN( MSG_ONE, gmsgHudColors, nullptr, pPlayer );
		WRITE_BYTE( colors.m_PrimaryColor.r() );
		WRITE_BYTE( colors.m_PrimaryColor.g() );
		WRITE_BYTE( colors.m_PrimaryColor.b() );

		WRITE_BYTE( colors.m_EmptyItemColor.r() );
		WRITE_BYTE( colors.m_EmptyItemColor.g() );
		WRITE_BYTE( colors.m_EmptyItemColor.b() );

		WRITE_BYTE( colors.m_AmmoBarColor.r() );
		WRITE_BYTE( colors.m_AmmoBarColor.g() );
		WRITE_BYTE( colors.m_AmmoBarColor.b() );
	MESSAGE_END();
}

void CMap::ResetHudColors()
{
	m_bUseCustomHudColors = false;

	m_flLastHudColorChangeTime = gpGlobals->time;

	m_HudColors.Reset();
}

void CMap::LoadGlobalModelReplacement( const char* const pszFileName )
{
	ASSERT( pszFileName );

	if( ( m_pGlobalModelReplacement = m_ModelReplacement.AcquireMap( pszFileName ) ) != nullptr )
	{
		Alert( at_console, "Global Model Replacement file \"%s\" loaded\n", pszFileName );
	}
}

void CMap::InitializeEntityClassifications()
{
	//Initialize classifications with defaults.
	const auto machineId = EntityClassifications().AddClassification( classify::MACHINE );
	const auto playerId = EntityClassifications().AddClassification( classify::PLAYER );
	const auto humanPassiveId = EntityClassifications().AddClassification( classify::HUMAN_PASSIVE );
	const auto humanMilitaryId = EntityClassifications().AddClassification( classify::HUMAN_MILITARY );
	const auto alienMilitaryId = EntityClassifications().AddClassification( classify::ALIEN_MILITARY );
	const auto alienPassiveId = EntityClassifications().AddClassification( classify::ALIEN_PASSIVE );
	const auto alienMonsterId = EntityClassifications().AddClassification( classify::ALIEN_MONSTER );
	const auto alienPreyId = EntityClassifications().AddClassification( classify::ALIEN_PREY );
	const auto alienPredatorId = EntityClassifications().AddClassification( classify::ALIEN_PREDATOR );
	const auto insectId = EntityClassifications().AddClassification( classify::INSECT, R_FR );
	const auto playerAllyId = EntityClassifications().AddClassification( classify::PLAYER_ALLY );
	const auto plrBioWpnId = EntityClassifications().AddClassification( classify::PLAYER_BIOWEAPON );
	const auto alienBioWpnId = EntityClassifications().AddClassification( classify::ALIEN_BIOWEAPON );
	/*const auto ignoreId = */EntityClassifications().AddClassification( classify::IGNORE, R_NO, R_NO );

	EntityClassifications().AddRelationship( machineId, playerId, R_DL, true );
	EntityClassifications().AddRelationship( machineId, humanPassiveId, R_DL );
	EntityClassifications().AddRelationship( machineId, alienMilitaryId, R_DL );
	EntityClassifications().AddRelationship( machineId, alienPassiveId, R_DL );
	EntityClassifications().AddRelationship( machineId, alienMonsterId, R_DL );
	EntityClassifications().AddRelationship( machineId, alienPreyId, R_DL );
	EntityClassifications().AddRelationship( machineId, alienPredatorId, R_DL );
	EntityClassifications().AddRelationship( machineId, playerAllyId, R_DL );
	EntityClassifications().AddRelationship( machineId, plrBioWpnId, R_DL );
	EntityClassifications().AddRelationship( machineId, alienBioWpnId, R_DL );

	EntityClassifications().AddRelationship( playerId, humanMilitaryId, R_DL );
	EntityClassifications().AddRelationship( playerId, alienMilitaryId, R_DL );
	EntityClassifications().AddRelationship( playerId, alienPassiveId, R_DL );
	EntityClassifications().AddRelationship( playerId, alienMonsterId, R_DL );
	EntityClassifications().AddRelationship( playerId, alienPreyId, R_DL );
	EntityClassifications().AddRelationship( playerId, alienPredatorId, R_DL );
	EntityClassifications().AddRelationship( playerId, plrBioWpnId, R_DL );
	EntityClassifications().AddRelationship( playerId, alienBioWpnId, R_DL );

	EntityClassifications().AddRelationship( humanPassiveId, playerId, R_AL );
	EntityClassifications().AddRelationship( humanPassiveId, humanPassiveId, R_AL );
	EntityClassifications().AddRelationship( humanPassiveId, humanMilitaryId, R_HT );
	EntityClassifications().AddRelationship( humanPassiveId, alienMilitaryId, R_FR );
	EntityClassifications().AddRelationship( humanPassiveId, alienMonsterId, R_HT );
	EntityClassifications().AddRelationship( humanPassiveId, alienPreyId, R_DL );
	EntityClassifications().AddRelationship( humanPassiveId, alienPredatorId, R_FR );
	EntityClassifications().AddRelationship( humanPassiveId, playerAllyId, R_AL );

	EntityClassifications().AddRelationship( humanMilitaryId, playerId, R_HT );
	EntityClassifications().AddRelationship( humanMilitaryId, humanPassiveId, R_DL );
	EntityClassifications().AddRelationship( humanMilitaryId, alienMilitaryId, R_HT );
	EntityClassifications().AddRelationship( humanMilitaryId, alienPassiveId, R_DL );
	EntityClassifications().AddRelationship( humanMilitaryId, alienMonsterId, R_DL );
	EntityClassifications().AddRelationship( humanMilitaryId, alienPreyId, R_DL );
	EntityClassifications().AddRelationship( humanMilitaryId, alienPredatorId, R_DL );
	EntityClassifications().AddRelationship( humanMilitaryId, playerAllyId, R_HT );

	EntityClassifications().AddRelationship( alienMilitaryId, machineId, R_DL );
	EntityClassifications().AddRelationship( alienMilitaryId, playerId, R_HT );
	EntityClassifications().AddRelationship( alienMilitaryId, humanPassiveId, R_DL );
	EntityClassifications().AddRelationship( alienMilitaryId, humanMilitaryId, R_HT );
	EntityClassifications().AddRelationship( alienMilitaryId, playerAllyId, R_DL );

	//No special relationships for Alien Passive

	EntityClassifications().AddRelationship( alienMonsterId, machineId, R_DL );
	EntityClassifications().AddRelationship( alienMonsterId, playerId, R_DL );
	EntityClassifications().AddRelationship( alienMonsterId, humanPassiveId, R_DL );
	EntityClassifications().AddRelationship( alienMonsterId, humanMilitaryId, R_DL );
	EntityClassifications().AddRelationship( alienMonsterId, playerAllyId, R_DL );

	EntityClassifications().AddRelationship( alienPreyId, playerId, R_DL );
	EntityClassifications().AddRelationship( alienPreyId, humanPassiveId, R_DL );
	EntityClassifications().AddRelationship( alienPreyId, humanMilitaryId, R_DL );
	EntityClassifications().AddRelationship( alienPreyId, alienPredatorId, R_FR );
	EntityClassifications().AddRelationship( alienPreyId, playerAllyId, R_DL );

	EntityClassifications().AddRelationship( alienPredatorId, playerId, R_DL );
	EntityClassifications().AddRelationship( alienPredatorId, humanPassiveId, R_DL );
	EntityClassifications().AddRelationship( alienPredatorId, humanMilitaryId, R_DL );
	EntityClassifications().AddRelationship( alienPredatorId, alienPreyId, R_HT );
	EntityClassifications().AddRelationship( alienPredatorId, alienPredatorId, R_DL );
	EntityClassifications().AddRelationship( alienPredatorId, playerAllyId, R_DL );

	EntityClassifications().AddRelationship( insectId, alienMilitaryId, R_NO );
	EntityClassifications().AddRelationship( insectId, insectId, R_NO );
	EntityClassifications().AddRelationship( insectId, plrBioWpnId, R_NO );
	EntityClassifications().AddRelationship( insectId, alienBioWpnId, R_NO );

	EntityClassifications().AddRelationship( playerAllyId, machineId, R_DL );
	EntityClassifications().AddRelationship( playerAllyId, playerId, R_AL );
	EntityClassifications().AddRelationship( playerAllyId, humanPassiveId, R_AL );
	EntityClassifications().AddRelationship( playerAllyId, humanMilitaryId, R_DL );
	EntityClassifications().AddRelationship( playerAllyId, alienMilitaryId, R_DL );
	EntityClassifications().AddRelationship( playerAllyId, alienPassiveId, R_DL );
	EntityClassifications().AddRelationship( playerAllyId, alienMonsterId, R_DL );
	EntityClassifications().AddRelationship( playerAllyId, alienPreyId, R_DL );
	EntityClassifications().AddRelationship( playerAllyId, alienPredatorId, R_DL );

	EntityClassifications().AddRelationship( plrBioWpnId, playerId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, humanPassiveId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, humanMilitaryId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, alienMilitaryId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, alienPassiveId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, alienMonsterId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, alienPreyId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, alienPredatorId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, playerAllyId, R_DL );
	EntityClassifications().AddRelationship( plrBioWpnId, alienBioWpnId, R_DL );

	EntityClassifications().AddRelationship( alienBioWpnId, playerId, R_DL );
	EntityClassifications().AddRelationship( alienBioWpnId, humanPassiveId, R_DL );
	EntityClassifications().AddRelationship( alienBioWpnId, humanMilitaryId, R_DL );
	EntityClassifications().AddRelationship( alienBioWpnId, alienMilitaryId, R_AL );
	EntityClassifications().AddRelationship( alienBioWpnId, alienMonsterId, R_DL );
	EntityClassifications().AddRelationship( alienBioWpnId, alienPreyId, R_DL );
	EntityClassifications().AddRelationship( alienBioWpnId, playerAllyId, R_DL );
	EntityClassifications().AddRelationship( alienBioWpnId, plrBioWpnId, R_DL );
}
