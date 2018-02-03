#ifndef COMMON_XML_XMLUTILS_H
#define COMMON_XML_XMLUTILS_H

#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMNode;

XERCES_CPP_NAMESPACE_END

namespace xml
{
/**
*	@brief Deleter that calls release() on Xerces instances
*/
template<typename T>
struct XMLDeleter
{
public:

	void operator()( T* pPtr )
	{
		pPtr->release();
	}
};

/**
*	@brief Unique pointer specialized for Xerces types that require calling of release() to destroy instances
*/
template<typename T>
using unique_xml_ptr = std::unique_ptr<T, XMLDeleter<T>>;

template<size_t SIZE, size_t INDEX>
struct InternalAsciiToXMLCh
{
	static void Convert( const char( &szString )[ SIZE ], std::array<XMLCh, SIZE>& result )
	{
		result[ INDEX ] = static_cast<XMLCh>( szString[ INDEX ] );

		InternalAsciiToXMLCh<SIZE, INDEX - 1>::Convert( szString, result );
	}
};

template<size_t SIZE>
struct InternalAsciiToXMLCh<SIZE, 0>
{
	static void Convert( const char( &szString )[ SIZE ], std::array<XMLCh, SIZE>& result )
	{
		result[ 0 ] = static_cast<XMLCh>( szString[ 0 ] );
	}
};

/**
*	@brief Converts an Ascii string literal to its XMLCh equivalent
*	Eliminates the need to check for errors during conversion
*	Only supports a limited character set, use with caution
*	@see xercesc/util/XMLUniDefs.hpp for the supported characters
*/
template<size_t SIZE>
std::array<XMLCh, SIZE> AsciiToXMLCh( const char( &szString )[ SIZE ] )
{
	std::array<XMLCh, SIZE> result;

	InternalAsciiToXMLCh<SIZE, SIZE - 1>::Convert( szString, result );

	return result;
}

/**
*	@brief Default maximum for stack buffers used to transcode strings
*/
const size_t DEFAULT_MAX_TRANSCODE = 2048;

/**
*	@brief Gets a list of elements by tag name
*	Only supports a limited subset of characters in names
*	@param doc Document to get item from
*	@param szName Name of tags to find
*	@return If successful this is the list, otherwise null
*/
template<size_t SIZE>
inline xercesc::DOMNodeList* GetElementsByTagName( const xercesc::DOMDocument& doc, const char ( & szName )[ SIZE ] )
{
	auto xmlName = AsciiToXMLCh( szName );

	return doc.getElementsByTagName( xmlName.data() );
}

/**
*	@copydoc GetElementsByTagName( const xercesc::DOMDocument& doc, const char ( & szName )[ SIZE ] )
*/
template<size_t SIZE>
inline xercesc::DOMNodeList* GetElementsByTagName( const xercesc::DOMElement& element, const char( &szName )[ SIZE ] )
{
	auto xmlName = AsciiToXMLCh( szName );

	return element.getElementsByTagName( xmlName.data() );
}

/**
*	@brief Find a root level node in a given node
*	Only supports a limited subset of characters in names
*	@param node Node to get item from
*	@param szName Name of tags to find
*	@return If found this is the node, otherwise null
*/
template<size_t SIZE>
inline xercesc::DOMNode* FindRootNode( const xercesc::DOMNode& node, const char ( & szName )[ SIZE ] )
{
	auto xmlName = AsciiToXMLCh( szName );

	auto pChildren = node.getChildNodes();

	const auto count = pChildren->getLength();

	for( decltype( pChildren->getLength() ) index = 0; index < count; ++index )
	{
		auto pChild = pChildren->item( index );

		if( xercesc::XMLString::compareString( pChild->getNodeName(), xmlName.data() ) == 0 )
		{
			return pChild;
		}
	}

	return nullptr;
}

/**
*	@brief Gets a named item from a DOMNamedNodeMap
*	Only supports a limited subset of characters in names
*	@param map Map to get item from
*	@param szName Name of item to retrieve
*	@return If found this is the node, otherwise null
*/
template<size_t SIZE>
inline xercesc::DOMNode* GetNamedItem( const xercesc::DOMNamedNodeMap& map, const char ( & szName )[ SIZE ] )
{
	auto xmlName = AsciiToXMLCh( szName );

	return map.getNamedItem( xmlName.data() );

	return nullptr;
}

/**
*	@brief Gets the key and value attributes from a given map
*/
bool GetKeyValue( const xercesc::DOMNamedNodeMap& map, std::string& szKey, std::string& szValue );
}

#endif //COMMON_XML_XMLUTILS_H
