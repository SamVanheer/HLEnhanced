#include <memory>
#include <string>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

#include <Angelscript/CASModule.h>

#include <Angelscript/ScriptAPI/CASScheduler.h>
#include <Angelscript/wrapper/ASCallable.h>

#include "extdll.h"
#include "util.h"
#include "Server.h"

#include "CFile.h"

#include "CHLASServerManager.h"

#include "CASPluginData.h"

#include "CASPluginModuleBuilder.h"

#include "CASPluginManager.h"

#include "xml/CStrX.h"
#include "xml/CXMLManager.h"
#include "xml/XMLUtils.h"
#include "xml/CXStr.h"

//TODO: need a separate cvar to control logging - Solokiller

//TODO: should be a client command. - Solokiller
static void ServerCommand_ListPlugins()
{
	for( size_t uiIndex = 0; uiIndex < g_ASManager.GetPluginManager().GetPluginCount(); ++uiIndex )
	{
		auto pModule = g_ASManager.GetPluginManager().GetPluginByIndex( uiIndex );

		auto pData = CASModule_GetPluginData( pModule );

		Alert( at_console, "Plugin \"%s\": lifetime: %s\n", pModule->GetModuleName(), PluginLifetimeToString( pData->GetLifetime() ) );

		if( pData->GetFlags().test( PluginFlag::UNLOAD ) )
		{
			Alert( at_console, "Flagged to unload\n" );
		}

		if( pData->GetFlags().test( PluginFlag::LOAD ) )
		{
			Alert( at_console, "Flagged to reload\n" );
		}
	}
}

static void MarkPluginForUnload( CASModule* pModule, const bool bReload )
{
	auto pData = CASModule_GetPluginData( pModule );

	if( pData->GetLifetime() >= PluginLifetime::SERVER )
	{
		Alert( at_console, "This plugin has %s lifetime and cannot be unloaded or reloaded\n", PluginLifetimeToString( pData->GetLifetime() ) );
	}
	else
	{
		pData->GetFlags().set( PluginFlag::UNLOAD );

		if( bReload )
			pData->GetFlags().set( PluginFlag::LOAD );

		Alert( at_console, "The plugin \"%s\" will be %s at a later time\n", pModule->GetModuleName(), bReload ? "reloaded" : "unloaded" );
	}
}

static void MarkPluginForUnload( const char* const pszName, const bool bReload )
{
	auto pModule = g_ASManager.GetPluginManager().FindPluginByName( pszName );

	if( pModule )
	{
		MarkPluginForUnload( pModule, bReload );
	}
	else
	{
		if( bReload )
		{
			//Queue it up for delay loading.
			g_ASManager.GetPluginManager().AddDelayLoadPlugin( pszName );

			Alert( at_console, "Queued plugin \"%s\" for delay loading\n", pszName );
		}
		else
		{
			Alert( at_console, "Couldn't find a plugin named \"%s\"\n", pszName );
		}
	}
}

static void ServerCommand_ReloadPlugin()
{
	if( CMD_ARGC() < 2 )
	{
		Alert( at_console, "as_reloadplugin usage: as_reloadplugin <plugin name>\n" );
		return;
	}

	const char* const pszName = CMD_ARGV( 1 );

	MarkPluginForUnload( pszName, true );
}

static void ServerCommand_UnloadPlugin()
{
	if( CMD_ARGC() < 2 )
	{
		Alert( at_console, "as_unloadplugin usage: as_unloadplugin <plugin name>\n" );
		return;
	}

	const char* const pszName = CMD_ARGV( 1 );

	MarkPluginForUnload( pszName, false );
}

static void MarkAllPluginsForUnload( const bool bReload )
{
	for( size_t uiIndex = 0; uiIndex < g_ASManager.GetPluginManager().GetPluginCount(); ++uiIndex )
	{
		MarkPluginForUnload( g_ASManager.GetPluginManager().GetPluginByIndex( uiIndex ), bReload );
	}
}

static void ServerCommand_ReloadAllPlugins()
{
	MarkAllPluginsForUnload( true );

	g_ASManager.GetPluginManager().MarkLoadNewPlugins();
}

static void ServerCommand_UnloadAllPlugins()
{
	MarkAllPluginsForUnload( false );
}

static void ServerCommand_ProcessPluginEvents()
{
	g_ASManager.GetPluginManager().SetNextPluginFlagCheckTime( gpGlobals->time );
}

//Every 5 seconds keeps plugin checks from being too expensive.
const float CASPluginManager::PLUGIN_FLAG_INTERVAL = 5;

CASPluginManager::CASPluginManager( CHLASServerManager& ASManager )
	: m_ASManager( ASManager )
{
}

const char* CASPluginManager::GetPluginListFile() const
{
	return as_plugin_list_file.string;
}

bool CASPluginManager::Initialize()
{
	g_engfuncs.pfnAddServerCommand( "as_listplugins", &::ServerCommand_ListPlugins );
	g_engfuncs.pfnAddServerCommand( "as_reloadplugin", &::ServerCommand_ReloadPlugin );
	g_engfuncs.pfnAddServerCommand( "as_unloadplugin", &::ServerCommand_UnloadPlugin );
	g_engfuncs.pfnAddServerCommand( "as_reloadallplugins", &::ServerCommand_ReloadAllPlugins );
	g_engfuncs.pfnAddServerCommand( "as_unloadallplugins", &::ServerCommand_UnloadAllPlugins );
	g_engfuncs.pfnAddServerCommand( "as_processpluginevents", &::ServerCommand_ProcessPluginEvents );

	return true;
}

void CASPluginManager::Shutdown()
{
	for( auto pPlugin : m_PluginList )
	{
		UnloadPlugin( pPlugin );
	}

	m_PluginList.clear();
}

void CASPluginManager::WorldCreated()
{
	//Adjust all timers.
	for( auto pPlugin : m_PluginList )
	{
		pPlugin->GetScheduler()->AdjustTime( m_flPrevThinkTime - 1 );
	}

	//Reset to starting time. TODO: define this constant. - Solokiller
	m_flPrevThinkTime = 1;

	//Check if any plugins need to be unloaded or reloaded.
	LoadList_t pluginsToLoad( std::move( m_DelayLoadList ) );

	//Server lifetime plugins don't get to do unload or reload.
	ProcessPluginLoadFlags( PluginLifetime::MAP, pluginsToLoad );

	//Load plugin file if needed.
	if( m_bShouldLoadPluginFile )
	{
		m_bShouldLoadPluginFile = false;

		LoadPluginsFile( GetPluginListFile(), nullptr );
	}
	else if( !pluginsToLoad.empty() )
	{
		//Load plugins that were unloaded.
		//Only do this if the plugin file wasn't just loaded, since that takes care of it as well.
		LoadPluginsFile( GetPluginListFile(), &pluginsToLoad );
	}

	m_flNextPluginFlagCheck = gpGlobals->time + PLUGIN_FLAG_INTERVAL;
}

void CASPluginManager::Think()
{
	if( m_flNextPluginFlagCheck <= gpGlobals->time )
	{
		//Check if any plugins need to be unloaded or reloaded.
		LoadList_t pluginsToLoad;

		ProcessPluginLoadFlags( PluginLifetime::HOTRELOADABLE, pluginsToLoad );

		if( !pluginsToLoad.empty() )
		{
			LoadPluginsFile( GetPluginListFile(), &pluginsToLoad );
		}

		m_flNextPluginFlagCheck = gpGlobals->time + PLUGIN_FLAG_INTERVAL;
	}

	for( auto pPlugin : m_PluginList )
	{
		pPlugin->GetScheduler()->Think( gpGlobals->time );
	}

	m_flPrevThinkTime = gpGlobals->time;
}

CASModule* CASPluginManager::GetPluginByIndex( const size_t uiIndex )
{
	ASSERT( uiIndex < m_PluginList.size() );

	return m_PluginList[ uiIndex ];
}

CASModule* CASPluginManager::FindPluginByName( const char* const pszName )
{
	ASSERT( pszName );
	ASSERT( *pszName );

	for( auto pPlugin : m_PluginList )
	{
		if( stricmp( pPlugin->GetModuleName(), pszName ) == 0 )
		{
			return pPlugin;
		}
	}

	return nullptr;
}

void CASPluginManager::AddDelayLoadPlugin( const char* const pszName )
{
	m_DelayLoadList.emplace_back( pszName );
}

void CASPluginManager::MarkLoadNewPlugins()
{
	m_bShouldLoadPluginFile = true;
}

void CASPluginManager::LoadPluginsFile( const char* const pszPluginFile, LoadList_t* pPluginsToLoad )
{
	ASSERT( pszPluginFile );
	ASSERT( *pszPluginFile );

	if( pPluginsToLoad && pPluginsToLoad->empty() )
	{
		return;
	}

	auto document = xml::XMLManager().ParseFile( pszPluginFile, "GAMECONFIG" );

	if( !document )
	{
		Alert( at_console, "CASPluginManager::LoadPluginsFile: Couldn't open plugin file \"%s\"\n", pszPluginFile );
		return;
	}

	xercesc::DOMNode* pRoot = xml::FindRootNode( *document, "plugins_config" );

	if( !pRoot )
	{
		Alert( at_console, "CASPluginManager::LoadPluginsFile: File \"%s\": no configuration found, ignoring\n", pszPluginFile );
		return;
	}

	auto pChildren = pRoot->getChildNodes();

	const auto count = pChildren->getLength();

	const auto szPluginNodeName = xml::AsciiToXMLCh( "plugin" );

	for( decltype( pChildren->getLength() ) index = 0; index < count; ++index )
	{
		auto pPlugin = pChildren->item( index );

		//Ignore non-element nodes silently
		if( pPlugin->getNodeType() != xercesc::DOMNode::ELEMENT_NODE )
			continue;

		//For now, warn if unknown nodes are encountered - Solokiller
		if( xercesc::XMLString::compareString( pPlugin->getNodeName(), szPluginNodeName.data() ) != 0 )
		{
			Alert( at_console, "CASPluginManager::LoadPluginsFile: File \"%s\": encountered unknown node \"%s\", ignoring\n",
				   pszPluginFile, xml::CStrX( pPlugin->getNodeName() ).LocalForm() );
			continue;
		}

		if( !pPlugin->hasAttributes() )
		{
			Alert( at_console, "CASPluginManager::LoadPluginsFile: File \"%s\": encountered plugin with no attributes, ignoring\n", pszPluginFile );
			continue;
		}

		auto& attrs = *pPlugin->getAttributes();

		auto pName = xml::GetNamedItem( attrs, "name" );
		auto pScript = xml::GetNamedItem( attrs, "script" );

		if( !pName || !pScript )
		{
			Alert( at_console, "CASPluginManager::LoadPluginsFile: File \"%s\": encountered plugin with one or more missing parameters, ignoring\n", pszPluginFile );
			continue;
		}

		const xml::CStrX szName{ pName->getNodeValue() };
		const xml::CStrX szScript{ pScript->getNodeValue() };

		if( !( *szName.LocalForm() ) || !( *szScript.LocalForm() ) )
		{
			Alert( at_console, "CASPluginManager::LoadPluginsFile: File \"%s\": encountered plugin with one or more missing parameters, ignoring\n", pszPluginFile );
			continue;
		}

		//If we're only loading a subset, skip any plugins that are not listed.
		if( pPluginsToLoad )
		{
			auto it = std::find_if( pPluginsToLoad->begin(), pPluginsToLoad->end(), 
				[ & ]( const auto& szPlugin )
				{
					return stricmp( szPlugin.c_str(), szName.LocalForm() ) == 0;
				}
			);

			if( it == pPluginsToLoad->end() )
				continue;

			pPluginsToLoad->erase( it );
		}

		//Defaults to hot reloadable if not specified.
		PluginLifetime lifetime = PluginLifetime::HOTRELOADABLE;

		if( auto pLifetime = xml::GetNamedItem( attrs, "lifetime" ) )
		{
			const xml::CStrX szLifetime{ pLifetime->getNodeValue() };

			if( *szLifetime.LocalForm() )
				lifetime = PluginLifetimeFromString( szLifetime.LocalForm() );
			else
				Alert( at_console, "CASPluginManager::LoadPluginsFile: File \"%s\": encountered plugin with empty lifetime attribute, ignoring\n", pszPluginFile );
		}

		LoadPlugin( szName.LocalForm(), szScript.LocalForm(), lifetime );
	}

	if( pPluginsToLoad && !pPluginsToLoad->empty() )
	{
		//Missed some plugins.
		Alert( at_console, "CASPluginManager::LoadPluginsFile: Couldn't reload %u plugins:\n", pPluginsToLoad->size() );

		for( const auto& szPlugin : *pPluginsToLoad )
		{
			Alert( at_console, "%s\n", szPlugin.c_str() );
		}

		Alert( at_console, "\n" );
	}
}

bool CASPluginManager::LoadPlugin( const char* const pszName, const char* const pszScript, const PluginLifetime lifetime )
{
	if( FindPluginByName( pszName ) )
	{
		Alert( at_console, "Attempted to load plugin \"%s\" multiple times. Ensure that there is only one plugin by that name\n", pszName );
		return false;
	}

	Alert( at_console, "Loading plugin \"%s\"\n", pszName );

	CASPluginModuleBuilder builder( pszScript );

	auto pluginData = std::make_unique<CASPluginData>( lifetime );

	auto pModule = m_ASManager.GetASManager().GetModuleManager().BuildModule( "Plugin", pszName, builder, pluginData.get() );

	if( pModule )
	{
		m_PluginList.emplace_back( pModule );

		pluginData.release();

		if( auto pFunction = pModule->GetModule()->GetFunctionByDecl( "void PluginInit()" ) )
		{
			as::Call( pFunction );
		}
	}

	return pModule != nullptr;
}

void CASPluginManager::UnloadPlugin( CASModule* pPlugin )
{
	Alert( at_console, "Unloading plugin \"%s\"\n", pPlugin->GetModuleName() );

	//Tell the plugin it's about to be unloaded.
	if( auto pFunction = pPlugin->GetModule()->GetFunctionByDecl( "void PluginUnload()" ) )
	{
		as::Call( pFunction );
	}

	ASSERT( CASModule_GetPluginData( pPlugin ) );

	//Plugin data is now removed by the module's destructor.

	m_ASManager.GetASManager().GetModuleManager().RemoveModule( pPlugin );
}

void CASPluginManager::ProcessPluginLoadFlags( const PluginLifetime lifetime, LoadList_t& pluginsToLoad )
{
	for( auto it = m_PluginList.begin(); it != m_PluginList.end(); )
	{
		auto pData = CASModule_GetPluginData( *it );

		if( pData->GetLifetime() <= lifetime &&
			pData->GetFlags().test( PluginFlag::UNLOAD ) )
		{
			if( pData->GetFlags().test( PluginFlag::LOAD ) )
			{
				pluginsToLoad.emplace_back( ( *it )->GetModuleName() );
			}

			UnloadPlugin( *it );

			it = m_PluginList.erase( it );
		}
		else
			++it;
	}
}
