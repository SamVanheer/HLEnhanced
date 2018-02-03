#include <algorithm>
#include <cstdio>

#include "extdll.h"
#include "util.h"

#include "DefaultClassifications.h"

#include "EntityClasses.h"

//TODO: move to utility header - Solokiller
//Taken from cppreference.com documentation for std::lower_bound - Solokiller
template<class ForwardIt, class T, class Compare = std::less<>>
ForwardIt binary_find( ForwardIt first, ForwardIt last, const T& value, Compare comp = {} )
{
	// Note: BOTH type T and the type after ForwardIt is dereferenced 
	// must be implicitly convertible to BOTH Type1 and Type2, used in Compare. 
	// This is stricter than lower_bound requirement (see above)

	first = std::lower_bound( first, last, value, comp );
	return first != last && !comp( value, *first ) ? first : last;
}

namespace
{
static CEntityClassificationsManager g_EntityClassifications;
}

CEntityClassificationsManager& EntityClassifications()
{
	return g_EntityClassifications;
}

#ifdef SERVER_DLL
static void EntityClassifications_WriteToFile_ServerCommand()
{
	if( CMD_ARGC() != 2 )
	{
		Alert( at_console, "Usage: entityclassifications_writetofile <filename including extension>\n" );
		return;
	}

	char szGameDir[ MAX_PATH ];

	if( !UTIL_GetGameDir( szGameDir, ARRAYSIZE( szGameDir ) ) )
	{
		Alert( at_console, "Couldn't get game directory\n" );
		return;
	}

	char szPath[ MAX_PATH ];

	if( !PrintfSuccess( snprintf( szPath, ARRAYSIZE( szPath ), "%s/%s", szGameDir, CMD_ARGV( 1 ) ), ARRAYSIZE( szPath ) ) )
	{
		Alert( at_console, "Couldn't format file path\n" );
		return;
	}

	EntityClassifications().WriteToFile( szPath );
}
#endif

bool CEntityClassificationData::HasRelationshipToClassification( EntityClassification_t targetClassId ) const
{
	return binary_find( m_Relationships.begin(), m_Relationships.end(), targetClassId ) != m_Relationships.end();
}

bool CEntityClassificationData::GetRelationshipToClassification( EntityClassification_t targetClassId, Relationship& outRelationship )
{
	auto it = binary_find( m_Relationships.begin(), m_Relationships.end(), targetClassId );

	if( it == m_Relationships.end() )
	{
		outRelationship = R_NO;
		return false;
	}

	outRelationship = it->m_Relationship;

	return true;
}

void CEntityClassificationData::AddRelationship( EntityClassification_t targetClassId, Relationship relationship )
{
	auto it = binary_find( m_Relationships.begin(), m_Relationships.end(), targetClassId );

	if( it == m_Relationships.end() )
	{
		it = std::upper_bound(
			m_Relationships.begin(),
			m_Relationships.end(),
			targetClassId );

		m_Relationships.insert(
			it,
			CClassificationRelationship( targetClassId, relationship ) );
	}
	else
	{
		//Already existed; update.
		Alert( at_aiconsole, "CEntityClassificationData::AddRelationship: Updating relationship between \"%s\" and \"%u\" from \"%u\" to \"%u\"\n", 
			   m_szName.c_str(), targetClassId, it->m_Relationship, relationship );
		it->m_Relationship = relationship;
	}
}

void CEntityClassificationData::RemoveRelationship( EntityClassification_t targetClassId )
{
	auto it = binary_find( m_Relationships.begin(), m_Relationships.end(), targetClassId );

	if( it != m_Relationships.end() )
	{
		m_Relationships.erase( it );
	}
}

const std::string CEntityClassificationsManager::EMPTY_STRING;

void CEntityClassificationsManager::Initialize()
{
#ifdef SERVER_DLL
	g_engfuncs.pfnAddServerCommand( "entityclassifications_writetofile", &EntityClassifications_WriteToFile_ServerCommand );
#endif
}

void CEntityClassificationsManager::Reset()
{
	m_ClassMap.clear();
	m_ClassList.clear();

	m_NoneId = AddClassification( classify::NONE );
}

bool CEntityClassificationsManager::IsClassIdValid( EntityClassification_t classId ) const
{
	return classId != INVALID_ENTITY_CLASSIFICATION && classId <= m_ClassList.size() && m_ClassList[ IdToIndex( classId ) ];
}

EntityClassification_t CEntityClassificationsManager::AddClassification( std::string&& szName, Relationship defaultSourceRelationship )
{
	return AddClassification( std::move( szName ), defaultSourceRelationship, R_NO, false );
}

EntityClassification_t CEntityClassificationsManager::AddClassification( std::string&& szName, Relationship defaultSourceRelationship, Relationship defaultTargetRelationship )
{
	return AddClassification( std::move( szName ), defaultSourceRelationship, defaultTargetRelationship, true );
}

EntityClassification_t CEntityClassificationsManager::AddClassification( std::string&& szName, 
																		 Relationship defaultSourceRelationship, Relationship defaultTargetRelationship, 
																		 bool bHasDefaultTargetRelationship )
{
	ASSERT( !szName.empty() );

	if( szName.empty() )
	{
		Alert( at_console, "CEntityClassificationsManager::AddClassification: Cannot add classification with empty name\n" );

		return INVALID_ENTITY_CLASSIFICATION;
	}

	{
		auto it = m_ClassMap.find( szName );

		if( it != m_ClassMap.end() )
		{
			auto& classification = m_ClassList[ it->second.first ];

			if( !it->second.second )
			{
				Alert( at_aiconsole, "CEntityClassificationsManager::AddClassification: Updating classification \"%s\"\n", szName.c_str() );

				classification->m_DefaultSourceRelationship = defaultSourceRelationship;
				classification->m_DefaultTargetRelationship = defaultTargetRelationship;
				classification->m_bHasDefaultTargetRelationship = bHasDefaultTargetRelationship;
			}
			else
			{
				Alert( at_console, "CEntityClassificationsManager::AddClassification: Cannot add classification \"%s\"; an alias with that name already exists\n", szName.c_str() );
			}

			return classification->m_ClassId;
		}
	}

	if( m_ClassList.size() >= MAX_ENTITY_CLASSIFICATIONS )
	{
		Alert( at_console, "CEntityClassificationsManager::AddClassification: Couldn't add \"%s\", maximum classifications %u reached\n", szName.c_str(), MAX_ENTITY_CLASSIFICATIONS );
		return GetNoneId();
	}

	const auto classId = m_ClassList.size() + FIRST_ID_OFFSET;

	m_ClassList.emplace_back( std::make_unique<CEntityClassificationData>( classId, std::move( szName ), defaultSourceRelationship, defaultTargetRelationship, bHasDefaultTargetRelationship ) );

	auto& data = m_ClassList.back();

	m_ClassMap.emplace( data->m_szName, std::make_pair( m_ClassList.size() - 1, false ) );

	return classId;
}

bool CEntityClassificationsManager::RemoveClassification( EntityClassification_t classId )
{
	if( !IsClassIdValid( classId ) )
	{
		Alert( at_error, "CEntityClassificationsManager::RemoveClassification: Class Id (\"%u\") is invalid\n", classId );
		return false;
	}

	const auto index = IdToIndex( classId );

	//Erase all entries, both classifications and aliases.
	for( auto it = m_ClassMap.begin(); it != m_ClassMap.end(); )
	{
		if( it->second.first == index )
		{
			it = m_ClassMap.erase( it );
		}
		else
			++it;
	}

	m_ClassList[ index ].reset();

	//Remove it from all classifications as well.
	for( auto& classification : m_ClassList )
	{
		if( classification )
			classification->RemoveRelationship( classId );
	}

	//TODO: reclaim freed Ids? - Solokiller
	return true;
}

bool CEntityClassificationsManager::RemoveClassification( const std::string& szName, bool bRemoveAliases )
{
	auto it = m_ClassMap.find( szName );

	if( it == m_ClassMap.end() )
		return false;

	if( it->second.second && !bRemoveAliases )
		return false;

	return RemoveClassification( IndexToId( it->second.first ) );
}

EntityClassification_t CEntityClassificationsManager::GetClassificationId( const std::string& szName ) const
{
	//The empty string is equivalent to the invalid classification.
	if( szName == EMPTY_STRING )
		return INVALID_ENTITY_CLASSIFICATION;

	auto it = m_ClassMap.find( szName );

	if( it == m_ClassMap.end() )
		return GetNoneId();

	return m_ClassList[ it->second.first ]->m_ClassId;
}

EntityClassification_t CEntityClassificationsManager::AddAlias( std::string&& szName, std::string&& szTarget )
{
	EntityClassification_t targetId = INVALID_ENTITY_CLASSIFICATION;

	//First get the Id for the target, inserting the classification if it doesn't exist yet.
	{
		auto it = m_ClassMap.find( szTarget );

		if( it != m_ClassMap.end() )
		{
			targetId = IndexToId( it->second.first );
		}
		else
		{
			targetId = AddClassification( std::move( szTarget ) );
		}
	}

	if( targetId == INVALID_ENTITY_CLASSIFICATION )
	{
		//The name is only moved from if it was inserted, so this can't be an empty string.
		Alert( at_error, "CEntityClassificationsManager::AddAlias: Couldn't add or find classification \"%s\"\n",
			   szTarget.c_str() );
		return targetId;
	}

	//Now check if the alias exists or not. If not, add it, otherwise, change target.
	auto it = m_ClassMap.find( szName );

	bool bInsert = false;

	if( it == m_ClassMap.end() )
	{
		bInsert = true;
	}
	else
	{
		if( it->second.second )
		{
			it->second.first = IdToIndex( targetId );
		}
		else
		{
			//A classification with that name exists; remove classification and add as alias.
			RemoveClassification( szName, false );
			bInsert = true;
		}
	}

	if( bInsert )
	{
		m_ClassMap.emplace( std::move( szName ), std::make_pair( IdToIndex( targetId ), true ) );
	}

	return targetId;
}

bool CEntityClassificationsManager::RemoveAlias( const std::string& szName )
{
	auto it = m_ClassMap.find( szName );

	if( it == m_ClassMap.end() )
		return false;

	//Only remove aliases.
	if( !it->second.second )
	{
		return false;
	}

	m_ClassMap.erase( it );

	return true;
}

void CEntityClassificationsManager::AddRelationship( EntityClassification_t sourceClassId, EntityClassification_t targetClassId, Relationship relationship, bool bBidirectional )
{
	if( !IsClassIdValid( sourceClassId ) || !IsClassIdValid( targetClassId ) )
	{
		Alert( at_error, "CEntityClassificationsManager::AddRelationship: One or both class Ids (\"%u\" and \"%u\") are invalid\n", sourceClassId, targetClassId );
		return;
	}

	auto& from = m_ClassList[ IdToIndex( sourceClassId ) ];
	from->AddRelationship( targetClassId, relationship );

	if( bBidirectional )
	{
		auto& to = m_ClassList[ IdToIndex( targetClassId ) ];
		to->AddRelationship( sourceClassId, relationship );
	}
}

void CEntityClassificationsManager::AddRelationship( const std::string& sourceClassName, const std::string& targetClassName, Relationship relationship, bool bBidirectional )
{
	auto sourceId = GetClassificationId( sourceClassName );
	auto targetId = GetClassificationId( targetClassName );

	if( !IsClassIdValid( sourceId ) || !IsClassIdValid( targetId ) )
	{
		Alert( at_error, "CEntityClassificationsManager::AddRelationship: One or both classifications (\"%s\" and \"%s\") are nonexistent\n", sourceClassName.c_str(), targetClassName.c_str() );
		return;
	}

	AddRelationship( sourceId, targetId, relationship, bBidirectional );
}

void CEntityClassificationsManager::RemoveRelationship( EntityClassification_t sourceClassId, EntityClassification_t targetClassId, bool bBidirectional )
{
	if( !IsClassIdValid( sourceClassId ) || !IsClassIdValid( targetClassId ) )
	{
		Alert( at_error, "CEntityClassificationsManager::RemoveRelationship: One or both class Ids (\"%u\" and \"%u\") are invalid\n", sourceClassId, targetClassId );
		return;
	}

	auto& from = m_ClassList[ IdToIndex( sourceClassId ) ];
	from->RemoveRelationship( targetClassId );

	if( bBidirectional )
	{
		auto& to = m_ClassList[ IdToIndex( targetClassId ) ];
		to->RemoveRelationship( sourceClassId );
	}
}

void CEntityClassificationsManager::RemoveRelationship( const std::string& sourceClassName, const std::string& targetClassName, bool bBidirectional )
{
	auto sourceId = GetClassificationId( sourceClassName );
	auto targetId = GetClassificationId( targetClassName );

	if( !IsClassIdValid( sourceId ) || !IsClassIdValid( targetId ) )
	{
		Alert( at_error, "CEntityClassificationsManager::RemoveRelationship: One or both classifications (\"%s\" and \"%s\") are nonexistent\n", sourceClassName.c_str(), targetClassName.c_str() );
		return;
	}

	RemoveRelationship( sourceId, targetId, bBidirectional );
}

Relationship CEntityClassificationsManager::GetRelationshipBetween( EntityClassification_t sourceClassId, EntityClassification_t targetClassId, bool bBidirectional ) const
{
	if( !IsClassIdValid( sourceClassId ) || !IsClassIdValid( targetClassId ) )
	{
		Alert( at_error, "CEntityClassificationsManager::GetRelationshipBetween: One or both class Ids (\"%u\" and \"%u\") are invalid\n", sourceClassId, targetClassId );
		return R_NO;
	}

	Relationship result = R_NO;

	auto& from = m_ClassList[ IdToIndex( sourceClassId ) ];
	auto& to = m_ClassList[ IdToIndex( targetClassId ) ];

	//If there exists a relationship from source to target, return relationship.
	if( from->GetRelationshipToClassification( targetClassId, result ) )
	{
		return result;
	}

	//No relationship from source to target, should we check for target to source?
	if( bBidirectional )
	{
		//There is a relationship, return value.
		if( to->GetRelationshipToClassification( sourceClassId, result ) )
		{
			return result;
		}
	}

	if( to->m_bHasDefaultTargetRelationship )
	{
		return to->m_DefaultTargetRelationship;
	}

	//No explicit relationship, return default.
	return from->m_DefaultSourceRelationship;
}

Relationship CEntityClassificationsManager::GetRelationshipBetween( const std::string& sourceClassName, const std::string& targetClassName, bool bBidirectional ) const
{
	auto sourceId = GetClassificationId( sourceClassName );
	auto targetId = GetClassificationId( targetClassName );

	if( !IsClassIdValid( sourceId ) || !IsClassIdValid( targetId ) )
	{
		Alert( at_error, "CEntityClassificationsManager::GetRelationshipBetween: One or both classifications (\"%s\" and \"%s\") are nonexistent\n", 
			   sourceClassName.c_str(), targetClassName.c_str() );
		return R_NO;
	}

	return GetRelationshipBetween( sourceId, targetId, bBidirectional );
}

const std::string& CEntityClassificationsManager::GetClassificationName( EntityClassification_t classification ) const
{
	if( !IsClassIdValid( classification ) )
		return EMPTY_STRING;

	return m_ClassList[ IdToIndex( classification ) ]->m_szName;
}

void CEntityClassificationsManager::WriteToFile( const char* pszFilename ) const
{
	ASSERT( pszFilename );

	FILE* pFile = fopen( pszFilename, "w" );

	if( !pFile )
	{
		Alert( at_error, "CEntityClassificationsManager::WriteToFile: Couldn't open file \"%s\" for writing!\n", pszFilename );
		return;
	}

	size_t uiClassCount = 0;
	//Calculate the longest classification name so we can align everything.
	size_t uiLongest = 0;

	for( const auto& classification : m_ClassList )
	{
		if( !classification )
			continue;

		++uiClassCount;

		const auto uiLength = classification->m_szName.length();

		if( uiLength > uiLongest )
			uiLongest = uiLength;
	}

	if( LONGEST_RELATIONSHIP_PRETTY_STRING > uiLongest )
		uiLongest = LONGEST_RELATIONSHIP_PRETTY_STRING;

	fprintf( pFile, "%u classifications\n", static_cast<unsigned int>( uiClassCount ) );
	//Offset to match the first value in the matrix.
	fprintf( pFile, "%-*s ", static_cast<int>( uiLongest ), "" );

	//Write the headers.
	for( const auto& classification : m_ClassList )
	{
		if( !classification )
			continue;

		//Align the headers so the pretty strings fit inside them.
		fprintf( pFile, "%-*s ", static_cast<int>( max( classification->m_szName.length(), LONGEST_RELATIONSHIP_PRETTY_STRING ) ), classification->m_szName.c_str() );
	}

	fprintf( pFile, "\n" );

	//Write the matrix of classification relationships.
	for( const auto& classification : m_ClassList )
	{
		if( !classification )
			continue;

		fprintf( pFile, "%-*s ", static_cast<int>( uiLongest ), classification->m_szName.c_str() );

		for( const auto& class2 : m_ClassList )
		{
			if( !class2 )
				continue;

			Relationship relationship;
			
			if( !classification->GetRelationshipToClassification( class2->m_ClassId, relationship ) )
			{
				if( class2->m_bHasDefaultTargetRelationship )
				{
					relationship = classification->m_DefaultTargetRelationship;
				}
				else
				{
					relationship = classification->m_DefaultSourceRelationship;
				}
			}

			//Align with header.
			fprintf( pFile, "%-*s ", static_cast<int>( max( class2->m_szName.length(), LONGEST_RELATIONSHIP_PRETTY_STRING ) ), RelationshipToPrettyString( relationship ) );
		}

		fprintf( pFile, "\n" );
	}

	size_t uiNumAliases = 0;

	for( const auto& classification : m_ClassMap )
	{
		if( classification.second.second )
			++uiNumAliases;
	}

	fprintf( pFile, "\n%u aliases\n", static_cast<unsigned int>( uiNumAliases ) );

	for( const auto& classification : m_ClassMap )
	{
		if( classification.second.second )
		{
			fprintf( pFile, "%s->%s\n", classification.first.c_str(), m_ClassList[ classification.second.first ]->m_szName.c_str() );
		}
	}

	fclose( pFile );

	Alert( at_console, "Written entity classifications to file \"%s\"\n", pszFilename );
}

size_t CEntityClassificationsManager::IdToIndex( EntityClassification_t classId )
{
	return classId - FIRST_ID_OFFSET;
}

EntityClassification_t CEntityClassificationsManager::IndexToId( size_t index )
{
	return index + FIRST_ID_OFFSET;
}
