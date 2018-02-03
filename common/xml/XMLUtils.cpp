#include <xercesc/util/XMLString.hpp>

#include "extdll.h"
#include "util.h"

#include "Logging.h"
#include "XMLUtils.h"

namespace xml
{
bool GetKeyValue( const xercesc::DOMNamedNodeMap& map, std::string& szKey, std::string& szValue )
{
	szKey.clear();
	szValue.clear();

	auto pKey = GetNamedItem( map, "key" );
	auto pValue = GetNamedItem( map, "value" );

	if( !pKey )
	{
		log->debug( "Couldn't find XML keyvalue key attribute\n" );
		return false;
	}

	if( !pValue )
	{
		log->debug( "Couldn't find XML keyvalue value attribute\n" );
		return false;
	}

	auto pszKey = pKey->getNodeValue();

	const auto uiKeyLength = xercesc::XMLString::stringLen( pszKey );

	szKey.resize( uiKeyLength );

	if( !xercesc::XMLString::transcode( pszKey, &szKey[ 0 ], uiKeyLength ) )
	{
		log->debug( "Couldn't transcode XML keyvalue key attribute\n" );
		return false;
	}

	auto pszValue = pValue->getNodeValue();

	const auto uiValueLength = xercesc::XMLString::stringLen( pszValue );

	szValue.resize( uiValueLength );

	if( !xercesc::XMLString::transcode( pszValue, &szValue[ 0 ], uiValueLength ) )
	{
		log->debug( "Couldn't transcode XML keyvalue value attribute\n" );
		szKey.clear();
		return false;
	}

	return true;
}
}
