#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "CServerConfig.h"

#include "xml/CStrX.h"
#include "xml/CXMLManager.h"
#include "xml/XMLUtils.h"
#include "xml/CXStr.h"

//TODO: need a separate cvar to control logging - Solokiller

bool CServerConfig::Parse( const char* pszFilename, const char* pszPathID, bool bOptional )
{
	ASSERT( pszFilename );

	Alert( at_aiconsole, "Parsing server config \"%s\"\n", pszFilename );

	m_CVars.Purge();
	m_EntityClasses = CEntityClasses{};

	Q_strncpy( m_szFilename, pszFilename, sizeof( m_szFilename ) );

	auto document = xml::XMLManager().ParseFile( pszFilename, pszPathID );

	if( !document )
	{
		if( !bOptional )
			Alert( at_console, "CServerConfig::Parse: Couldn't open server configuration file \"%s\"\n", pszFilename );
		return false;
	}

	auto pRoot = xml::FindRootNode( *document, "config" );

	if( !pRoot )
	{
		Alert( at_console, "CServerConfig::Parse: File \"%s\": no configuration found, ignoring\n", pszFilename );
		return true;
	}

	auto pChildren = pRoot->getChildNodes();

	const auto count = pChildren->getLength();

	const auto szCVarNodeName = xml::AsciiToXMLCh( "cvar" );
	const auto szEntClassesNodeName = xml::AsciiToXMLCh( "entity_classifications" );

	bool bFoundEntClasses = false;

	for( decltype( pChildren->getLength() ) index = 0; index < count; ++index )
	{
		auto pSetting = pChildren->item( index );

		//Ignore non-element nodes silently
		if( pSetting->getNodeType() != xercesc::DOMNode::ELEMENT_NODE )
			continue;

		if( xercesc::XMLString::compareString( pSetting->getNodeName(), szCVarNodeName.data() ) == 0 )
		{
			//Process and store cvar
			if( !pSetting->hasAttributes() )
			{
				Alert( at_console, "CServerConfig::Parse: File \"%s\": encountered cvar with no attributes, ignoring\n", pszFilename );
				continue;
			}

			auto& attrs = *pSetting->getAttributes();

			auto pName = xml::GetNamedItem( attrs, "name" );

			if( !pName )
			{
				Alert( at_console, "CServerConfig::Parse: File \"%s\": encountered cvar with one or more missing parameters, ignoring\n", pszFilename );
				continue;
			}

			const xml::CStrX szName{ pName->getNodeValue() };

			if( !( *szName.LocalForm() ) )
			{
				Alert( at_console, "CServerConfig::Parse: File \"%s\": encountered cvar with one or more missing parameters, ignoring\n", pszFilename );
				continue;
			}

			CCVarData data;

			data.m_szName = szName.LocalForm();

			if( auto pValue = xml::GetNamedItem( attrs, "value" ) )
			{
				data.m_szValue = xml::CStrX{ pValue->getNodeValue() }.LocalForm();
			}

			//TODO: need to backport C++11 support for move operators - Solokiller
			m_CVars.AddToTail( std::move( data ) );
		}
		else if( xercesc::XMLString::compareString( pSetting->getNodeName(), szEntClassesNodeName.data() ) == 0 )
		{
			if( bFoundEntClasses )
			{
				Alert( at_console, "CServerConfig::Parse: File \"%s\": encountered multiple entity classifications sections, ignoring\n", pszFilename );
				continue;
			}

			bFoundEntClasses = true;

			ParseEntityClassifications( pszFilename, *pSetting );
		}
	}

	return true;
}

void CServerConfig::ParseEntityClassifications( const char* pszFilename, const xercesc::DOMNode& entClassData )
{
	//Process and store entity classifications
	if( entClassData.hasAttributes() )
	{
		auto& attrs = *entClassData.getAttributes();

		auto pReset = xml::GetNamedItem( attrs, "reset" );

		if( pReset && pReset->hasAttributes() )
		{
			auto& resetAttrs = *pReset->getAttributes();

			if( auto pValue = xml::GetNamedItem( resetAttrs, "value" ) )
			{
				const XMLCh sz1[] = { xercesc::chDigit_1, xercesc::chNull }; //"1"
				m_EntityClasses.m_bReset = xercesc::XMLString::compareString( pValue->getNodeValue(), sz1 ) == 0;
			}
		}
	}

	auto pChildren = entClassData.getChildNodes();

	const auto count = pChildren->getLength();

	const auto szClassNodeName = xml::AsciiToXMLCh( "class" );
	const auto szAliasNodeName = xml::AsciiToXMLCh( "alias" );
	const auto szRelationshipNodeName = xml::AsciiToXMLCh( "relationship" );

	for( decltype( pChildren->getLength() ) index = 0; index < count; ++index )
	{
		auto pSetting = pChildren->item( index );

		//Ignore non-element nodes silently
		if( pSetting->getNodeType() != xercesc::DOMNode::ELEMENT_NODE )
			continue;

		if( xercesc::XMLString::compareString( pSetting->getNodeName(), szClassNodeName.data() ) == 0 )
		{
			//Process and store class data
			if( !pSetting->hasAttributes() )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered entity class with no attributes, ignoring\n", pszFilename );
				continue;
			}

			auto& attrs = *pSetting->getAttributes();

			auto pName = xml::GetNamedItem( attrs, "name" );

			if( !pName )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered entity class with one or more missing parameters, ignoring\n", pszFilename );
				continue;
			}

			const xml::CStrX szName{ pName->getNodeValue() };

			if( !( *szName.LocalForm() ) )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered entity class with one or more missing parameters, ignoring\n", pszFilename );
				continue;
			}

			CEntityClass entClass;

			entClass.m_szName = szName.LocalForm();

			if( auto pValue = xml::GetNamedItem( attrs, "defaultSourceRelationship" ) )
			{
				entClass.m_DefaultSourceRelationship = RelationshipFromString( xml::CStrX{ pValue->getNodeValue() }.LocalForm() );
			}

			if( auto pValue = xml::GetNamedItem( attrs, "defaultTargetRelationship" ) )
			{
				entClass.m_DefaultTargetRelationship = RelationshipFromString( xml::CStrX{ pValue->getNodeValue() }.LocalForm() );
			}

			m_EntityClasses.m_Classes.AddToTail( std::move( entClass ) );
		}
		else if( xercesc::XMLString::compareString( pSetting->getNodeName(), szAliasNodeName.data() ) == 0 )
		{
			//Process and store alias data
			if( !pSetting->hasAttributes() )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered alias with no attributes, ignoring\n", pszFilename );
				continue;
			}

			auto& attrs = *pSetting->getAttributes();

			auto pSource = xml::GetNamedItem( attrs, "source" );
			auto pTarget = xml::GetNamedItem( attrs, "target" );

			if( !pSource || !pTarget )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered alias with one or more missing parameters, ignoring\n", pszFilename );
				continue;
			}

			const xml::CStrX szSource{ pSource->getNodeValue() };
			const xml::CStrX szTarget{ pTarget->getNodeValue() };

			if( !( *szSource.LocalForm() ) || !( *szTarget.LocalForm() ) )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered alias with one or more missing parameters, ignoring\n", pszFilename );
				continue;
			}

			m_EntityClasses.m_Aliases.AddToTail( { szSource.LocalForm(), szTarget.LocalForm() } );
		}
		else if( xercesc::XMLString::compareString( pSetting->getNodeName(), szRelationshipNodeName.data() ) == 0 )
		{
			//Process and store relationship data
			if( !pSetting->hasAttributes() )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered relationship with no attributes, ignoring\n", pszFilename );
				continue;
			}

			auto& attrs = *pSetting->getAttributes();

			auto pSource = xml::GetNamedItem( attrs, "source" );
			auto pTarget = xml::GetNamedItem( attrs, "target" );
			auto pRelationship = xml::GetNamedItem( attrs, "relationship" );

			if( !pSource || !pTarget || !pRelationship )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered relationship with one or more missing parameters, ignoring\n", pszFilename );
				continue;
			}

			const xml::CStrX szSource{ pSource->getNodeValue() };
			const xml::CStrX szTarget{ pTarget->getNodeValue() };
			const xml::CStrX szRelationship{ pRelationship->getNodeValue() };

			if( !( *szSource.LocalForm() ) || !( *szTarget.LocalForm() ) || !( *szRelationship.LocalForm() ) )
			{
				Alert( at_console, "CServerConfig::ParseEntityClassifications: File \"%s\": encountered relationship with one or more missing parameters, ignoring\n", pszFilename );
				continue;
			}

			CEntityRelationship relationship;

			relationship.m_szSource = szSource.LocalForm();
			relationship.m_szTarget = szTarget.LocalForm();
			relationship.m_Relationship = RelationshipFromString( szRelationship.LocalForm() );

			if( auto pValue = xml::GetNamedItem( attrs, "biDirectional" ) )
			{
				const XMLCh sz1[] = { xercesc::chDigit_1, xercesc::chNull }; //"1"
				relationship.m_bBiDirectional = xercesc::XMLString::compareString( pValue->getNodeValue(), sz1 ) == 0;
			}

			m_EntityClasses.m_Relationships.AddToTail( std::move( relationship ) );
		}
	}
}

void CServerConfig::ProcessCVars()
{
	//Process all cvars.
	for( decltype( m_CVars.Count() ) index = 0; index < m_CVars.Count(); ++index )
	{
		const auto& cvar = m_CVars[ index ];
		//Set the cvar.
		//TODO: filter cvars so map configs can't set important cvars like rcon nilly willy - Solokiller
		CVAR_SET_STRING( cvar.m_szName.c_str(), cvar.m_szValue.c_str() );
	}
}

void CServerConfig::ProcessEntityClassifications()
{
	if( m_EntityClasses.m_bReset )
	{
		EntityClassifications().Reset();
	}

	//Process classes first.
	for( decltype( m_EntityClasses.m_Classes.Count() ) index = 0; index < m_EntityClasses.m_Classes.Count(); ++index )
	{
		const auto& entClass = m_EntityClasses.m_Classes[ index ];

		EntityClassifications().AddClassification( entClass.m_szName.c_str(), entClass.m_DefaultSourceRelationship, entClass.m_DefaultTargetRelationship );
	}

	//Now process aliases.
	for( decltype( m_EntityClasses.m_Aliases.Count() ) index = 0; index < m_EntityClasses.m_Aliases.Count(); ++index )
	{
		const auto& alias = m_EntityClasses.m_Aliases[ index ];

		EntityClassifications().AddAlias( alias.m_szSource.c_str(), alias.m_szTarget.c_str() );
	}

	//Now process relationships.
	for( decltype( m_EntityClasses.m_Relationships.Count() ) index = 0; index < m_EntityClasses.m_Relationships.Count(); ++index )
	{
		const auto& relationship = m_EntityClasses.m_Relationships[ index ];

		EntityClassifications().AddRelationship(
			relationship.m_szSource.c_str(), relationship.m_szTarget.c_str(),
			relationship.m_Relationship,
			relationship.m_bBiDirectional
		);
	}
}
