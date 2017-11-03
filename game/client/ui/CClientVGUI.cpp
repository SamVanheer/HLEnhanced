#include <cassert>
#include <clocale>

#include "tier0/dbg.h"

#if USE_VGUI2
#include <vgui/VGUI2.h>
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>

#include <vgui/ISurface.h>

#include "IGameUIFuncs.h"
#include "IBaseUI.h"

#include "hud/CHudViewport.h"
#endif

#include "CClientGameInterface.h"

#include "KeyValuesCompat.h"

#include "CClientVGUI.h"

namespace
{
CClientVGUI g_ClientVGUI;

IGameUIFuncs* g_GameUIFuncs = nullptr;

IBaseUI* g_pBaseUI = nullptr;
}

CClientVGUI* clientVGUI()
{
	return &g_ClientVGUI;
}

IGameUIFuncs* gameUIFuncs()
{
	return g_GameUIFuncs;
}

IBaseUI* baseUI()
{
	return g_pBaseUI;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CClientVGUI, IClientVGUI, ICLIENTVGUI_NAME, g_ClientVGUI );

CClientVGUI::CClientVGUI()
{
}

void CClientVGUI::Initialize( CreateInterfaceFn* pFactories, int iNumFactories )
{
	/*
	*	Factories in the given array:
	*	engine
	*	vgui2
	*	filesystem
	*	chrome HTML
	*	GameUI
	*	client (this library)
	*/

	//4 factories to use.
	assert( static_cast<size_t>( iNumFactories ) >= NUM_FACTORIES - 1 );

	if( !g_Client.LibInit( pFactories, iNumFactories ) )
	{
		//Not much can be done here
		return;
	}

	m_FactoryList[ 0 ] = Sys_GetFactoryThis();

	for( size_t uiIndex = 0; uiIndex < NUM_FACTORIES - 1; ++uiIndex )
	{
		m_FactoryList[ uiIndex + 1 ] = pFactories[ uiIndex ];
	}

#if USE_VGUI2
	if( !vgui2::VGui_InitInterfacesList( "CLIENT", m_FactoryList, NUM_FACTORIES ) )
	{
		Msg( "Failed to initialize VGUI2\n" );
		return;
	}
#endif

	if( !KV_InitKeyValuesSystem( m_FactoryList, NUM_FACTORIES ) )
	{
		Msg( "Failed to initialize IKeyValues\n" );
		return;
	}

#if USE_VGUI2
	g_GameUIFuncs = ( IGameUIFuncs* ) pFactories[ 0 ]( IGAMEUIFUNCS_NAME, nullptr );
	g_pBaseUI = ( IBaseUI* ) pFactories[ 0 ]( IBASEUI_NAME, nullptr );

	//Constructor sets itself as the viewport.
	new CHudViewport();

	g_pViewport->Initialize( pFactories, iNumFactories );
#endif
}

void CClientVGUI::Start()
{
#if USE_VGUI2
	g_pViewport->Start();
#endif
}

void CClientVGUI::SetParent( vgui2::VPANEL parent )
{
	m_vRootPanel = parent;

#if USE_VGUI2
	g_pViewport->SetParent( parent );
#endif
}

int CClientVGUI::UseVGUI1()
{
#if USE_VGUI2
	return g_pViewport->UseVGUI1();
#else
	return true;
#endif
}

void CClientVGUI::HideScoreBoard()
{
#if USE_VGUI2
	g_pViewport->HideScoreBoard();
#endif
}

void CClientVGUI::HideAllVGUIMenu()
{
#if USE_VGUI2
	g_pViewport->HideAllVGUIMenu();
#endif
}

void CClientVGUI::ActivateClientUI()
{
#if USE_VGUI2
	g_pViewport->ActivateClientUI();
#endif
}

void CClientVGUI::HideClientUI()
{
#if USE_VGUI2
	g_pViewport->HideClientUI();
#endif
}

void CClientVGUI::Shutdown()
{
#if USE_VGUI2
	g_pViewport->Shutdown();
#endif
}
