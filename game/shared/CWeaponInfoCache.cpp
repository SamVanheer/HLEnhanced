#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "Weapons.h"

#include "CWeaponInfo.h"

#ifdef CLIENT_DLL
#include "hud.h"
#include "CWeaponHUDInfo.h"
#endif

#include "CWeaponInfoCache.h"

#include "xml/CStrX.h"
#include "xml/CXMLManager.h"
#include "xml/XMLUtils.h"
#include "xml/CXStr.h"

CWeaponInfoCache g_WeaponInfoCache;

const char* const CWeaponInfoCache::WEAPON_INFO_DIR = "weapon_info";

CWeaponInfoCache::CWeaponInfoCache()
{
	//Set dummy HUD info so missing files don't cause crashes. - Solokiller
#ifdef CLIENT_DLL
	m_DefaultInfo.SetHUDInfo( new CWeaponHUDInfo() );
#endif
}

CWeaponInfoCache::~CWeaponInfoCache()
{
}

const CWeaponInfo* CWeaponInfoCache::FindWeaponInfo( const char* const pszWeaponName ) const
{
	ASSERT( pszWeaponName );

	if( !pszWeaponName )
		return nullptr;

	auto it = m_InfoMap.find( pszWeaponName );

	if( it != m_InfoMap.end() )
		return m_InfoList[ it->second ].get();

	return nullptr;
}

const CWeaponInfo* CWeaponInfoCache::LoadWeaponInfo( const int iID, const char* const pszWeaponName, const char* const pszSubDir )
{
	ASSERT( pszWeaponName );
	ASSERT( *pszWeaponName );

	if( auto pInfo = FindWeaponInfo( pszWeaponName ) )
		return pInfo;

	std::unique_ptr<CWeaponInfo> info = std::make_unique<CWeaponInfo>();

	if( LoadWeaponInfoFromFile( pszWeaponName, pszSubDir, *info ) )
	{
		CWeaponInfo* pInfo = info.get();

		m_InfoList.emplace_back( std::move( info ) );

		auto result = m_InfoMap.insert( std::make_pair( pInfo->GetWeaponName(), m_InfoList.size() - 1 ) );

		if( result.second )
		{
			pInfo->SetID( iID );

#ifdef CLIENT_DLL
			//Load HUD info right away.
			CWeaponHUDInfo* pHUDInfo = new CWeaponHUDInfo();

			if( !pHUDInfo->LoadFromFile( pszWeaponName ) )
			{
				Alert( at_warning, "CWeaponInfoCache::LoadWeaponInfo: Couldn't load weapon \"%s\" HUD info\n", pszWeaponName );
			}

			pInfo->SetHUDInfo( pHUDInfo );
#endif

			return pInfo;
		}

		Alert( at_error, "CWeaponInfoCache::LoadWeaponInfo: Failed to insert weapon info \"%s\" into cache!\n", pszWeaponName );

		m_InfoList.erase( m_InfoList.end() - 1 );
	}

	return &m_DefaultInfo;
}

void CWeaponInfoCache::ClearInfos()
{
	m_InfoMap.clear();
	m_InfoList.clear();
}

void CWeaponInfoCache::EnumInfos( EnumInfoCallback pCallback, void* pUserData ) const
{
	ASSERT( pCallback );

	if( !pCallback )
		return;

	for( const auto& info : m_InfoList )
	{
		if( !pCallback( *info, pUserData ) )
			break;
	}
}

size_t CWeaponInfoCache::GenerateHash() const
{
	size_t uiHash = 0;

	//TODO: not the best hash, but it'll do. - Solokiller
	for( const auto& info : m_InfoList )
	{
		uiHash += StringHash( info->GetWeaponName() );
	}

	return uiHash;
}

bool CWeaponInfoCache::LoadWeaponInfoFromFile( const char* const pszWeaponName, const char* const pszSubDir, CWeaponInfo& info )
{
	char szPath[ MAX_PATH ] = {};

	int iResult;
	
	if( pszSubDir )
	{
		iResult = snprintf( szPath, sizeof( szPath ), "%s/%s/%s.xml", WEAPON_INFO_DIR, pszSubDir, pszWeaponName );
	}
	else
	{
		iResult = snprintf( szPath, sizeof( szPath ), "%s/%s.xml", WEAPON_INFO_DIR, pszWeaponName );
	}

	if( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szPath ) )
	{
		//Subdir can be null, that's expected. - Solokiller
		Alert( at_error, "CWeaponInfoCache::LoadWeaponInfoFromFile: Failed to format file path for \"%s\" (subdir: \"%s\")!\n", pszWeaponName, pszSubDir );
		return false;
	}

	auto document = xml::XMLManager().ParseFile( szPath );

	if( !document )
	{
		return false;
	}

	info.SetWeaponName( pszWeaponName );

	auto pRoot = document->getDocumentElement();

	if( !pRoot || xercesc::XMLString::compareString( pRoot->getNodeName(), xml::AsciiToXMLCh( "weapon" ).data() ) != 0 )
	{
		Alert( at_console, "CWeaponInfoCache::LoadWeaponInfoFromFile: File \"%s\": No weapon data found, ignoring\n", szPath );
		return false;
	}

	auto pKeyvalues = xml::GetElementsByTagName( *pRoot, "keyvalue" );

	if( pKeyvalues )
	{
		const auto count = pKeyvalues->getLength();

		std::string szKey, szValue;

		for( decltype( pKeyvalues->getLength() ) index = 0; index < count; ++index )
		{
			const auto pKeyvalue = pKeyvalues->item( index );

			if( !pKeyvalue->hasAttributes() )
			{
				Alert( at_aiconsole, "CWeaponInfoCache::LoadWeaponInfoFromFile: Keyvalue with no attributes, ignoring\n" );
				continue;
			}

			auto& attrs = *pKeyvalue->getAttributes();

			if( !xml::GetKeyValue( attrs, szKey, szValue ) )
			{
				Alert( at_console, "CWeaponInfoCache::LoadWeaponInfoFromFile: File \"%s\": encountered keyvalue with one or more missing parameters, ignoring\n", szPath );
				continue;
			}

			if( !info.KeyValue( szKey.c_str(), szValue.c_str() ) )
			{
				Alert( at_aiconsole, "CWeaponInfoCache::LoadWeaponInfoFromFile: Unhandled keyvalue \"%s\" \"%s\"\n", szKey.c_str(), szValue.c_str() );
			}
		}
	}

	return true;
}
