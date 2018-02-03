#include <cstdarg>
#include <memory>
#include <string>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#include <angelscript.h>

#include <Angelscript/ScriptAPI/ASCDateTime.h>
#include <Angelscript/ScriptAPI/ASCTime.h>

#include <Angelscript/ScriptAPI/SQL/CASSQLThreadPool.h>

#include <Angelscript/ScriptAPI/SQL/SQLite/ASSQLite.h>
#include <Angelscript/ScriptAPI/SQL/SQLite/CASSQLiteConnection.h>

#include <Angelscript/ScriptAPI/SQL/MySQL/ASMySQL.h>
#include <Angelscript/ScriptAPI/SQL/MySQL/CASMySQLConnection.h>

#include "extdll.h"
#include "util.h"
#include "Server.h"

#include "xml/CStrX.h"
#include "xml/CXMLManager.h"
#include "xml/XMLUtils.h"
#include "xml/CXStr.h"

#include "ASHLSQL.h"

#define SQLITE_BASE_DIR "scripts/databases"
#define SQLITE_EXT ".sqlite"

#define MYSQL_DEFAULT_CONN_BLOCK "default_mysql_connection"

static void SQLLogFunc( const char* const pszFormat, ... )
{
	char szBuffer[ 4096 ];

	va_list list;

	va_start( list, pszFormat );
	const int iResult = vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );
	va_end( list );

	if( iResult >= 0 || static_cast<size_t>( iResult ) < sizeof( szBuffer ) )
	{
		Alert( at_console, "SQL: %s", szBuffer );
	}
	else
	{
		ASSERT( false );
		Alert( at_console, "SQL: Log formatting failed\n" );
	}
}

static size_t CalculateThreadCount()
{
	const unsigned int uiThreads = std::thread::hardware_concurrency();

	//Only use 1 thread for now. - Solokiller
	if( uiThreads >= 1 )
		return 1;

	return 0;
}

CASSQLThreadPool* g_pSQLThreadPool = nullptr;

static CASSQLiteConnection* HLCreateSQLiteConnection( const std::string& szDatabase )
{
	if( szDatabase.empty() )
	{
		Alert( at_error, "SQL::CreateSQLiteConnection: Empty database name!\n" );
		return nullptr;
	}

	g_pFileSystem->CreateDirHierarchy( SQLITE_BASE_DIR, nullptr );

	char szGameDir[ MAX_PATH ];
	char szFilename[ MAX_PATH ];

	if( !UTIL_GetGameDir( szGameDir, sizeof( szGameDir ) ) )
	{
		Alert( at_error, "SQL::CreateSQLiteConnection: Failed to get game directory!\n" );
		return nullptr;
	}

	const int iResult = snprintf( szFilename, sizeof( szFilename ), "%s/%s/%s%s", szGameDir, SQLITE_BASE_DIR, szDatabase.c_str(), SQLITE_EXT );
	
	if( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szFilename ) )
	{
		Alert( at_error, "SQL::CreateSQLiteConnection: Failed to format database \"%s\" filename!\n", szDatabase.c_str() );
		return nullptr;
	}

	//TODO: filter access to databases here so only authorized access works - Solokiller

	return new CASSQLiteConnection( *g_pSQLThreadPool, szFilename );
}

static unsigned int ParseMySQLPort( std::string& szHostName )
{
	//Based on AMX's SQLX interface; allow scripts to specify a port using host:port format. - Solokiller
	size_t uiPortSep = szHostName.find( ':' );

	//TODO: define default in config - Solokiller
	unsigned int uiPort = 3306;

	if( uiPortSep != std::string::npos )
	{
		uiPort = strtoul( &szHostName[ uiPortSep + 1 ], nullptr, 10 );

		//Trim the port part from the string.
		szHostName.resize( uiPortSep );
	}

	return uiPort;
}

static CASMySQLConnection* HLCreateMySQLConnection( const std::string& szHost, const std::string& szUser, const std::string& szPassword, const std::string& szDatabase = "" )
{
	std::string szHostName = szHost;

	const unsigned int uiPort = ParseMySQLPort( szHostName );

	return new CASMySQLConnection( *g_pSQLThreadPool, szHostName.c_str(), szUser.c_str(), szPassword.c_str(), szDatabase.c_str(), uiPort, "", 0 );
}

/**
*	Creates a MySQL connection using default connection settings defined in the server's MySQL config.
*/
static CASMySQLConnection* HLCreateMySQLConnectionWithDefaults( const std::string& szDatabase = "" )
{
	//TODO: parse config file ahead of time, add option to reparse as needed, support multiple named connections - Solokiller

	if( !( *as_mysql_config.string ) )
	{
		Alert( at_logged, "SQL::CreateMySQLConnectionWithDefaults: No config file specified; cannot create connection\n" );
		return nullptr;
	}

	//Parse the config file.
	//Only load from the mod directory to prevent malicious servers from downloading files and overriding them.
	auto document = xml::XMLManager().ParseFile( as_mysql_config.string, "GAMECONFIG" );

	if( !document )
	{
		Alert( at_error, "SQL::CreateMySQLConnectionWithDefaults: Couldn't open config file \"%s\"!\n", as_mysql_config.string );
		return nullptr;
	}

	auto pRoot = xml::FindRootNode( *document, "mysql_config" );

	if( !pRoot )
	{
		Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": no configuration found, ignoring\n", as_mysql_config.string );
		return nullptr;
	}

	auto pConnections = xml::GetElementsByTagName( *document, "connection" );

	if( !pConnections )
	{
		Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": no connections found, ignoring\n", as_mysql_config.string );
		return nullptr;
	}

	//Find the default connection
	xercesc::DOMNode* pDefault = nullptr;

	const auto count = pConnections->getLength();

	const auto szDefaultName = xml::AsciiToXMLCh( "default" );

	for( decltype( pConnections->getLength() ) index = 0; index < count; ++index )
	{
		auto pConn = pConnections->item( index );

		if( pConn->getNodeType() != xercesc::DOMNode::ELEMENT_NODE )
		{
			Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": Non-element connection node found, ignoring\n", as_mysql_config.string );
			continue;
		}

		if( !pConn->hasAttributes() )
		{
			Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": encountered connection with no attributes, ignoring\n", as_mysql_config.string );
			continue;
		}

		auto& attrs = *pConn->getAttributes();

		auto pName = xml::GetNamedItem( attrs, "name" );

		if( !pName )
		{
			Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": encountered connection with one or more missing parameters, ignoring\n", as_mysql_config.string );
			continue;
		}

		if( xercesc::XMLString::compareString( pName->getNodeValue(), szDefaultName.data() ) == 0 )
		{
			pDefault = pConn;
			break;
		}
	}

	if( !pDefault )
	{
		Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": No default connection found\n", as_mysql_config.string );
		return nullptr;
	}

	//We know it has attributes since we checked it earlier
	auto& attrs = *pDefault->getAttributes();

	const auto pHost = xml::GetNamedItem( attrs, "host" );
	const auto pUser = xml::GetNamedItem( attrs, "user" );
	const auto pPass = xml::GetNamedItem( attrs, "pass" );

	if( !pHost || !pUser || !pPass )
	{
		Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": Connection is missing one or more parameters, cannot create connection\n", as_mysql_config.string );
		return nullptr;
	}

	const xml::CStrX szHost{ pHost->getNodeValue() };
	const xml::CStrX szUser{ pUser->getNodeValue() };
	const xml::CStrX szPass{ pPass->getNodeValue() };

	if( !( *szHost.LocalForm() ) )
	{
		Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": Connection has empty host setting, cannot create connection\n", as_mysql_config.string );
		return nullptr;
	}

	if( !( *szUser.LocalForm() ) )
	{
		Alert( at_console, "SQL::CreateMySQLConnectionWithDefaults: File \"%s\": Connection has empty username setting, cannot create connection\n", as_mysql_config.string );
		return nullptr;
	}

	std::string szHostName = szHost.LocalForm();

	const unsigned int uiPort = ParseMySQLPort( szHostName );

	return new CASMySQLConnection( *g_pSQLThreadPool, szHostName.c_str(), szUser.LocalForm(), szPass.LocalForm(), szDatabase.c_str(), uiPort, "", 0 );
}

void RegisterScriptHLSQL( asIScriptEngine& engine )
{
	g_pSQLThreadPool = new CASSQLThreadPool( CalculateThreadCount(), &::SQLLogFunc );

	//Call an SQLite function to load the library. - Solokiller
	Alert( at_console, "SQLite library version: %s\n", sqlite3_libversion() );

	//Call a MySQL function to load the library. - Solokiller
	Alert( at_console, "MariaDB library version: %s\n", mysql_get_client_info() );

	RegisterScriptCTime( engine );
	RegisterScriptCDateTime( engine );

	RegisterScriptSQLCommon( engine );
	RegisterScriptSQLite( engine );
	RegisterScriptMySQL( engine );

	const std::string szOldNS = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace( "SQL" );

	engine.RegisterGlobalFunction(
		"SQLiteConnection@ CreateSQLiteConnection(const string& in szDatabase)",
		asFUNCTION( HLCreateSQLiteConnection ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"MySQLConnection@ CreateMySQLConnection(const string& in szHost, const string& in szUser, const string& in szPassword, const string& in szDatabase = \"\")",
		asFUNCTION( HLCreateMySQLConnection ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"MySQLConnection@ CreateMySQLConnectionWithDefaults(const string& in szDatabase = \"\")",
		asFUNCTION( HLCreateMySQLConnectionWithDefaults ), asCALL_CDECL );

	engine.SetDefaultNamespace( szOldNS.c_str() );
}