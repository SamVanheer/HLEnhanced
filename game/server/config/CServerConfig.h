#ifndef GAME_SERVER_CONFIG_CSERVERCONFIG_H
#define GAME_SERVER_CONFIG_CSERVERCONFIG_H

#include <memory>
#include <string>

#include <xercesc/util/XercesDefs.hpp>

#include "UtlVector.h"

#include "EntityClasses.h"

XERCES_CPP_NAMESPACE_BEGIN

class DOMNode;

XERCES_CPP_NAMESPACE_END

/**
*	Parses a server config file and stores off persistent data.
*/
class CServerConfig
{
private:
	//TODO: should have general use structures for these things - Solokiller
	struct CCVarData
	{
		std::string m_szName;
		std::string m_szValue;
	};

	struct CEntityClass
	{
		std::string m_szName;
		Relationship m_DefaultSourceRelationship = R_NO;
		Relationship m_DefaultTargetRelationship = R_NO;
	};

	struct CEntityClassAlias
	{
		std::string m_szSource;
		std::string m_szTarget;
	};

	struct CEntityRelationship
	{
		std::string m_szSource;
		std::string m_szTarget;
		Relationship m_Relationship;
		bool m_bBiDirectional = false;
	};

	struct CEntityClasses
	{
		bool m_bReset = false;

		CUtlVector<CEntityClass> m_Classes;
		CUtlVector<CEntityClassAlias> m_Aliases;
		CUtlVector<CEntityRelationship> m_Relationships;
	};

public:
	CServerConfig() = default;
	~CServerConfig() = default;

	/**
	*	Parses the given config file.
	*	@param pszFilename Name of the file, starting in Half-Life/
	*	@param pszPathID If not null, ID of the path to search in. If null, all paths are searched
	*	@return Whether the file was successfully parsed
	*/
	bool Parse( const char* pszFilename, const char* pszPathID = nullptr, bool bOptional = false );

	const char* GetFilename() const { return m_szFilename; }

	/**
	*	Processes all CVars specified by this file.
	*/
	void ProcessCVars();

	/**
	*	Processes the entity classifications specified by this file.
	*/
	void ProcessEntityClassifications();

private:
	void ParseEntityClassifications( const char* pszFilename, const xercesc::DOMNode& entClassData );

private:
	char m_szFilename[ MAX_PATH ] = {};

	CUtlVector<CCVarData> m_CVars;
	CEntityClasses m_EntityClasses;

private:
	CServerConfig( const CServerConfig& ) = delete;
	CServerConfig& operator=( const CServerConfig& ) = delete;
};

#endif //GAME_SERVER_CONFIG_CSERVERCONFIG_H
