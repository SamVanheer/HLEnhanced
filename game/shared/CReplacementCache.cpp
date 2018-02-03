#include <cassert>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#include "extdll.h"
#include "util.h"

#include "CReplacementMap.h"

#include "CReplacementCache.h"

#include "xml/CStrX.h"
#include "xml/CXMLManager.h"
#include "xml/XMLUtils.h"
#include "xml/CXStr.h"

CReplacementMap* CReplacementCache::GetMap( const char* const pszFileName ) const
{
	assert( pszFileName );

	char szAbsoluteName[ MAX_PATH ];

	if( g_pFileSystem->GetLocalPath( pszFileName, szAbsoluteName, sizeof( szAbsoluteName ) ) )
	{
		auto it = m_Cache.find( szAbsoluteName );

		if( it != m_Cache.end() )
			return it->second.get();
	}

	return nullptr;
}

CReplacementMap* CReplacementCache::AcquireMap( const char* const pszFileName )
{
	assert( pszFileName );

	char szAbsoluteName[ MAX_PATH ];

	if( !g_pFileSystem->GetLocalPath( pszFileName, szAbsoluteName, sizeof( szAbsoluteName ) ) )
	{
		Alert( at_error, "CReplacementCache::AcquireMap: Couldn't format absolute filename for \"%s\"!\n", pszFileName );
		return nullptr;
	}

	auto it = m_Cache.find( szAbsoluteName );

	if( it != m_Cache.end() )
		return it->second.get();

	auto map = LoadMap( pszFileName, szAbsoluteName );

	auto pMap = map.get();

	//Insert it in either case so non-existent files aren't looked up a bunch. - Solokiller
	auto result = m_Cache.emplace( szAbsoluteName, std::move( map ) );

	if( !result.second )
		return nullptr;

	return pMap;
}

std::unique_ptr<CReplacementMap> CReplacementCache::LoadMap( const char* const pszFileName, const char* const pszAbsFileName ) const
{
	assert( pszAbsFileName );

	auto document = xml::XMLManager().ParseFile( pszFileName );

	if( !document )
	{
		return nullptr;
	}

	auto pRootElement = document->getDocumentElement();

	if( !pRootElement || xercesc::XMLString::compareString( pRootElement->getNodeName(), xml::AsciiToXMLCh( "replacement_map" ).data() ) != 0 )
	{
		Alert( at_console, "CReplacementCache::LoadMap: File \"%s\": no replacement keyvalues found, ignoring\n", pszFileName );
		return nullptr;
	}

	auto pChildren = xml::GetElementsByTagName( *pRootElement, "keyvalue" );

	const auto count = pChildren->getLength();

	auto map = std::make_unique<CReplacementMap>( pszAbsFileName );

	std::string szKey, szValue;

	for( decltype( pChildren->getLength() ) index = 0; index < count; ++index )
	{
		auto pChild = pChildren->item( index );

		if( !pChild->hasAttributes() )
		{
			Alert( at_console, "CReplacementCache::LoadMap: File \"%s\": encountered keyvalue with no attributes, ignoring\n", pszFileName );
			continue;
		}

		auto& attrs = *pChild->getAttributes();

		if( !xml::GetKeyValue( attrs, szKey, szValue ) )
		{
			Alert( at_console, "CReplacementCache::LoadMap: File \"%s\": encountered keyvalue with one or more missing parameters, ignoring\n", pszFileName );
			continue;
		}

		if( szKey.empty() )
		{
			Alert( at_console, "CReplacementCache::LoadMap: File \"%s\": encountered keyvalue with empty key, ignoring\n", pszFileName );
			continue;
		}

		if( !map->AddReplacement( szKey.c_str(), szValue.c_str() ) )
		{
			Alert( at_warning, "CReplacementCache::LoadMap: File \"%s\": Duplicate original filename \"%s\", ignoring\n", pszFileName, szKey.c_str() );
		}
	}

	return map;
}