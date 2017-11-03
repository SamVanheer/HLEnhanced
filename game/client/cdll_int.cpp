/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#include "../public/interface.h"
//#include "vgui_schememanager.h"

#include "pm_shared.h"

#include <string.h>
#include "vgui_int.h"
#include "interface.h"

#include "Exports.h"

#include "renderer/tri.h"
#include "vgui_TeamFortressViewport.h"
#include "../public/interface.h"
#include "materials/Materials.h"

#include "CClientGameInterface.h"

#include "hl/CClientPrediction.h"

#include "effects/CEnvironment.h"

#include "MessageHandler.h"

#include "CHudSpectator.h"

cl_enginefunc_t gEngfuncs;
CHLHud gHUD;
TeamFortressViewport *gViewPort = NULL;


#include "particleman.h"
CSysModule *g_hParticleManModule = NULL;
IParticleMan *g_pParticleMan = NULL;

namespace
{
static CMessageHandlers g_MessageHandlers;
}

CMessageHandlers& MessageHandlers()
{
	return g_MessageHandlers;
}

void CL_LoadParticleMan();
void CL_UnloadParticleMan();

void InitInput();
void ShutdownInput();
void EV_HookEvents();
void IN_Commands();

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds( int hullnumber, Vector& mins, Vector& maxs )
{
	int iret = 0;

	//TODO: identical to GetHullBounds on the server side. Should ideally use the same code. - Solokiller
	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = Vector(-16, -16, -36);
		maxs = Vector(16, 16, 36);
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = Vector(-16, -16, -18 );
		maxs = Vector(16, 16, 18 );
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	DLLEXPORT HUD_ConnectionlessPacket( const netadr_t *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	/*int max_buffer_size = *response_buffer_size;*/

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void DLLEXPORT HUD_PlayerMoveInit( playermove_t *ppmove )
{
	PM_Init( ppmove );
}

char DLLEXPORT HUD_PlayerMoveTexture( const char* const pszName )
{
	return g_MaterialsList.FindTextureType( pszName );
}

void DLLEXPORT HUD_PlayerMove( playermove_t *ppmove, int server )
{
	PM_Move( ppmove, server );
}

int DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	gEngfuncs = *pEnginefuncs;

	if (iVersion != CLDLL_INTERFACE_VERSION)
		return false;

	memcpy(&gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t));

	EV_HookEvents();
	CL_LoadParticleMan();

	//TODO: use a dynamically allocated instance - Solokiller
	Hud().SetHud( &gHUD );

	// get tracker interface, if any
	return true;
}

void DLLEXPORT HUD_Shutdown( void )
{
	ShutdownInput();

#if defined( _TFC )
	ClearEventList();
#endif

	CL_UnloadParticleMan();

	g_Client.LibShutdown();

	Hud().SetHud( nullptr );
}


/*
==========================
	HUD_VidInit

Called when the game initializes
and whenever the vid_mode is changed
so the HUD can reinitialize itself.
==========================
*/

int DLLEXPORT HUD_VidInit( void )
{
	//Clear the string pool now.
	g_StringPool.Clear();

	Hud().VidInit();

	VGui_Startup();

	return g_Client.ConnectionEstablished();
}

/*
==========================
	HUD_Init

Called whenever the client connects
to a server.  Reinitializes all 
the hud variables.
==========================
*/

void DLLEXPORT HUD_Init( void )
{
	msghandler::Initialize();

	InitInput();
	Hud().Init();
	Scheme_Init();

	g_Prediction.Initialize();
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int DLLEXPORT HUD_Redraw( float time, int intermission )
{
	Hud().Redraw( time, intermission != 0 );

	return 1;
}


/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int DLLEXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime )
{
	IN_Commands();

	//Note: flTime isn't being passed in anymore because cl_enginefunc_t::GetClientTime returns the same value. - Solokiller
	return Hud().UpdateClientData( pcldata );
}

/*
==========================
	HUD_Reset

Called at start and end of demos to restore to "non"HUD state.
==========================
*/

void DLLEXPORT HUD_Reset( void )
{
	Hud().VidInit();
}

/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void DLLEXPORT HUD_Frame( double time )
{
	g_Client.Frame( time );

	GetClientVoiceMgr()->Frame(time);
}


/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);
}

/*
==========================
HUD_DirectorMessage

Called when a director event message was received
==========================
*/

void DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf )
{
	if( auto pSpectator = GETHUDCLASS( CHudSpectator ) )
		pSpectator->DirectorMessage( iSize, pbuf );
}

void CL_UnloadParticleMan( void )
{
	Sys_UnloadModule( g_hParticleManModule );

	g_pParticleMan = NULL;
	g_hParticleManModule = NULL;
}

void CL_LoadParticleMan( void )
{
	char szPDir[512];

	if ( !gEngfuncs.COM_ExpandFilename( PARTICLEMAN_DLLNAME, szPDir, sizeof( szPDir ) ) )
	{
		g_pParticleMan = NULL;
		g_hParticleManModule = NULL;
		return;
	}

	g_hParticleManModule = Sys_LoadModule( szPDir );
	CreateInterfaceFn particleManFactory = Sys_GetFactory( g_hParticleManModule );

	if ( particleManFactory == NULL )
	{
		g_pParticleMan = NULL;
		g_hParticleManModule = NULL;
		return;
	}

	g_pParticleMan = (IParticleMan *)particleManFactory( PARTICLEMAN_INTERFACE, NULL);

	if ( g_pParticleMan )
	{
		 g_pParticleMan->SetUp( &gEngfuncs );

		 // Add custom particle classes here BEFORE calling anything else or you will die.
		 g_pParticleMan->AddCustomParticleClassSize ( sizeof ( CBaseParticle ) );

		 g_Environment.RegisterParticleClasses();
	}
}

/**
*	This function is never called, but it has to exist in order for the engine to load stuff from the client. - Solokiller
*/
extern "C" DLLEXPORT void* ClientFactory()
{
	return nullptr;
}

/**
*	This function is used to load secure clients. It's actually obsolete, but the engine will reload the dll if it's not here. - Solokiller
*/
extern "C" void DLLEXPORT F(void *pv)
{
	cldll_func_t *pcldll_func = (cldll_func_t *)pv;

	cldll_func_t cldll_func = 
	{
	Initialize,
	HUD_Init,
	HUD_VidInit,
	HUD_Redraw,
	HUD_UpdateClientData,
	HUD_Reset,
	HUD_PlayerMove,
	HUD_PlayerMoveInit,
	HUD_PlayerMoveTexture,
	IN_ActivateMouse,
	IN_DeactivateMouse,
	IN_MouseEvent,
	IN_ClearStates,
	IN_Accumulate,
	CL_CreateMove,
	CL_IsThirdPerson,
	CL_CameraOffset,
	KB_Find,
	CAM_Think,
	V_CalcRefdef,
	HUD_AddEntity,
	HUD_CreateEntities,
	HUD_DrawNormalTriangles,
	HUD_DrawTransparentTriangles,
	HUD_StudioEvent,
	HUD_PostRunCmd,
	HUD_Shutdown,
	HUD_TxferLocalOverrides,
	HUD_ProcessPlayerState,
	HUD_TxferPredictionData,
	Demo_ReadBuffer,
	HUD_ConnectionlessPacket,
	HUD_GetHullBounds,
	HUD_Frame,
	HUD_Key_Event,
	HUD_TempEntUpdate,
	HUD_GetUserEntity,
	HUD_VoiceStatus,
	HUD_DirectorMessage,
	HUD_GetStudioModelInterface,
	HUD_ChatInputPosition,
	nullptr,
	ClientFactory,	//NOTE: Needed for the engine to query CreateInterface for interface instantiation. - Solokiller
	};

	*pcldll_func = cldll_func;
}

#include "cl_dll/IGameClientExports.h"

//-----------------------------------------------------------------------------
// Purpose: Exports functions that are used by the gameUI for UI dialogs
//-----------------------------------------------------------------------------
class CClientExports : public IGameClientExports
{
public:
	// returns the name of the server the user is connected to, if any
	virtual const char *GetServerHostName()
	{
		/*if (gViewPortInterface)
		{
			return gViewPortInterface->GetServerName();
		}*/
		return "";
	}

	// ingame voice manipulation
	virtual bool IsPlayerGameVoiceMuted(int playerIndex)
	{
		if (GetClientVoiceMgr())
			return GetClientVoiceMgr()->IsPlayerBlocked(playerIndex);
		return false;
	}

	virtual void MutePlayerGameVoice(int playerIndex)
	{
		if (GetClientVoiceMgr())
		{
			GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, true);
		}
	}

	virtual void UnmutePlayerGameVoice(int playerIndex)
	{
		if (GetClientVoiceMgr())
		{
			GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, false);
		}
	}
};

EXPOSE_SINGLE_INTERFACE(CClientExports, IGameClientExports, GAMECLIENTEXPORTS_INTERFACE_VERSION);