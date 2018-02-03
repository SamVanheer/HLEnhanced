
#ifndef TEAMFORTRESSVIEWPORT_H
#define TEAMFORTRESSVIEWPORT_H

#include<VGUI_Panel.h>
#include<VGUI_Frame.h>
#include<VGUI_TextPanel.h>
#include<VGUI_Label.h>
#include<VGUI_Button.h>
#include<VGUI_ActionSignal.h>
#include<VGUI_InputSignal.h>
#include<VGUI_Scheme.h>
#include<VGUI_Image.h>
#include<VGUI_FileInputStream.h>
#include<VGUI_BitmapTGA.h>
#include<VGUI_DesktopIcon.h>
#include<VGUI_App.h>
#include<VGUI_MiniApp.h>
#include<VGUI_LineBorder.h>
#include<VGUI_String.h>
#include<VGUI_ScrollPanel.h>
#include<VGUI_ScrollBar.h>
#include<VGUI_Slider.h>

#include "CImageLabel.h"

#include "vgui_defaultinputsignal.h"
// custom scheme handling
#include "vgui_SchemeManager.h"

#include "CHudHealth.h"
#include "CHudSpectator.h"

#include "vgui_CommandButton.h"
#include "vgui_CommandLabel.h"
#include "vgui_MenuPanel.h"

#include "vgui_Defs.h"

using namespace vgui;

class Cursor;
class ScorePanel;
class SpectatorPanel;
class CCommandMenu;
class CommandLabel;
class CommandButton;
class BuildButton;
class ClassButton;
class CMenuPanel;
class DragNDropPanel;
class CTransparentPanel;
class CClassMenuPanel;
class CTeamMenuPanel;
class TeamFortressViewport;

char* GetVGUITGAName(const char *pszName);
BitmapTGA *LoadTGAForRes(const char* pImageName);
extern TeamFortressViewport *gViewPort;

//==============================================================================
// VIEWPORT PIECES
//============================================================

class ColorButton : public CommandButton
{
private:

	vgui::Color *ArmedColor;
	vgui::Color *UnArmedColor;

	vgui::Color *ArmedBorderColor;
	vgui::Color *UnArmedBorderColor;

public:
	ColorButton( const char* text,int x,int y,int wide,int tall, bool bNoHighlight, bool bFlat ) : 
	  CommandButton( text, x, y, wide, tall, bNoHighlight, bFlat  ) 
	  {
		  ArmedColor = NULL;
		  UnArmedColor = NULL;
		  ArmedBorderColor = NULL;
		  UnArmedBorderColor = NULL;
	  }
	

	virtual void paintBackground()
	{
		int r, g, b, a;
		vgui::Color bgcolor;

		if ( isArmed() )
		{
			// Highlight background
		/*	getBgColor(bgcolor);
			bgcolor.getColor(r, g, b, a);
			drawSetColor( r,g,b,a );
			drawFilledRect(0,0,_size[0],_size[1]);*/

			if ( ArmedBorderColor )
			{
				ArmedBorderColor->getColor( r, g, b, a);
				drawSetColor( r, g, b, a );
				drawOutlinedRect(0,0,_size[0],_size[1]);
			}
		}
		else
		{
			if ( UnArmedBorderColor )
			{
				UnArmedBorderColor->getColor( r, g, b, a);
				drawSetColor( r, g, b, a );
				drawOutlinedRect(0,0,_size[0],_size[1]);
			}
		}
	}
	void paint()
	{
		int r, g, b, a;
		if ( isArmed() )
		{
			if (ArmedColor)
			{
				ArmedColor->getColor(r, g, b, a);
				setFgColor(r, g, b, a);
			}
			else
				setFgColor( Scheme::sc_secondary2 );
		}
		else
		{
			if (UnArmedColor)
			{
				UnArmedColor->getColor(r, g, b, a);
				setFgColor(r, g, b, a);
			}
			else
				setFgColor( Scheme::sc_primary1 );
		}
		
		Button::paint();
	}
	
	void setArmedColor ( int r, int g, int b, int a )
	{
		ArmedColor = new vgui::Color( r, g, b, a );
	}

	void setUnArmedColor ( int r, int g, int b, int a )
	{
		UnArmedColor = new vgui::Color( r, g, b, a );
	}

	void setArmedBorderColor ( int r, int g, int b, int a )
	{
		ArmedBorderColor = new vgui::Color( r, g, b, a );
	}

	void setUnArmedBorderColor ( int r, int g, int b, int a )
	{
		UnArmedBorderColor = new vgui::Color( r, g, b, a );
	}
};

class SpectButton : public CommandButton
{
private:
	
public:
	SpectButton( int iPlayerClass, const char* text,int x,int y,int wide,int tall ) : 
	  CommandButton( text, x, y, wide, tall, false)
	  {
		  	Init();
			
			setButtonText( text );
	  }
		
	virtual void paintBackground()
	{
		if ( isArmed())
		{
			drawSetColor( 143,143, 54, 125 ); 
			drawFilledRect( 5, 0,_size[0] - 5,_size[1]);
		}
	}

	virtual void paint()
	{
	
		if ( isArmed() )
		{
			setFgColor( 194, 202, 54, 0 );
		}
		else
		{
			setFgColor( 143, 143, 54, 15 );
		}

		Button::paint();
	}
};

//==============================================================================
// Command menu root button (drop down box style)

class DropDownButton : public ColorButton
{
private:
	CImageLabel *m_pOpenButton;

public:

		DropDownButton( const char* text,int x,int y,int wide,int tall, bool bNoHighlight, bool bFlat ) : 
						ColorButton( text, x, y, wide, tall, bNoHighlight, bFlat  ) 
	  {
			// Put a > to show it's a submenu
			m_pOpenButton = new CImageLabel( "arrowup", XRES( CMENU_SIZE_X-2 ) , YRES( BUTTON_SIZE_Y-2 ) );
			m_pOpenButton->setParent(this);
	
			int textwide, texttall;
			getSize( textwide, texttall);
		
			// Reposition
			m_pOpenButton->setPos( textwide-(m_pOpenButton->getImageWide()+6), -2 /*(tall - m_pOpenButton->getImageTall()*2) / 2*/ );
			m_pOpenButton->setVisible(true);

	  }

	virtual void   setVisible(bool state)
	{
		m_pOpenButton->setVisible(state);
		ColorButton::setVisible(state);
	}
	

};

//==============================================================================
// Button with image instead of text

class CImageButton : public ColorButton
{
private:
	CImageLabel *m_pOpenButton;

public:

		CImageButton( const char* text,int x,int y,int wide,int tall, bool bNoHighlight, bool bFlat ) : 
						ColorButton( " ", x, y, wide, tall, bNoHighlight, bFlat  ) 
	  {
			m_pOpenButton = new CImageLabel( text,1,1,wide-2 , tall-2 );
			m_pOpenButton->setParent(this);
	
			// Reposition
		//	m_pOpenButton->setPos( x+1,y+1 );
		//	m_pOpenButton->setSize(wide-2,tall-2);

			m_pOpenButton->setVisible(true);
	  }

	virtual void   setVisible(bool state)
	{
		m_pOpenButton->setVisible(state);
		ColorButton::setVisible(state);
	}
	

};

class CMenuHandler_SpectateFollow : public vgui::ActionSignal
{
protected:
	char	m_szplayer[ MAX_COMMAND_SIZE ];
public:
	CMenuHandler_SpectateFollow( const char *player );

	virtual void actionPerformed( vgui::Panel* panel );
};

//==============================================================================
class TeamFortressViewport : public Panel
{
private:
	//vgui::Cursor* _cursorNone;
	//vgui::Cursor* _cursorArrow;

	int			 m_iInitialized;

	CCommandMenu *m_pCommandMenus[ MAX_MENUS ];
	CCommandMenu *m_pCurrentCommandMenu;
	float		 m_flMenuOpenTime;
	float		 m_flScoreBoardLastUpdated;
	float		 m_flSpectatorPanelLastUpdated;
	int			 m_iNumMenus;
	int			 m_iCurrentTeamNumber;
	int			 m_iCurrentPlayerClass;
	int			 m_iUser1;
	int			 m_iUser2;
	int			 m_iUser3;

	// VGUI Menus
	void		 CreateTeamMenu( void );
	CMenuPanel*	 ShowTeamMenu( void );
	void		 CreateClassMenu( void );
	CMenuPanel*	 ShowClassMenu( void );
	void		 CreateSpectatorMenu( void );
	
	// Scheme handler
	CSchemeManager m_SchemeManager;

	// MOTD
	int		m_iGotAllMOTD;
	char	m_szMOTD[ MAX_MOTD_LENGTH ];

	//  Command Menu Team buttons
	CommandButton *m_pTeamButtons[6];
	CommandButton *m_pDisguiseButtons[5];
	//BuildButton   *m_pBuildButtons[3];
	//BuildButton   *m_pBuildActiveButtons[3];

	bool		m_bAllowSpectators;

	// Data for specific sections of the Command Menu
	int			m_iValidClasses[5];
	bool		m_bIsFeigning;
	DetpackState	m_DetpackState = DetpackState::CANNOT_DEPLOY;
	int			m_iNumberOfTeams;
	int			m_iBuildState;
	int			m_iRandomPC;
	char		m_sTeamNames[5][MAX_TEAMNAME_SIZE];

	// Localisation strings
	char		m_sDetpackStrings[3][MAX_BUTTON_SIZE];

	char		m_sMapName[64];

	// helper function to update the player menu entries
	void UpdatePlayerMenu(int menuIndex);

public:
	TeamFortressViewport(int x,int y,int wide,int tall);
	void Initialize( void );

	int		CreateCommandMenu( const char* const pszMenuFile, int direction, int yOffset, bool flatDesign, float flButtonSizeX, float flButtonSizeY, int xOffset );
	void	CreateScoreBoard( void );
	CommandButton * CreateCustomButton( char *pButtonText, char * pButtonName, int  iYOffset );
	CCommandMenu *	CreateDisguiseSubmenu( CommandButton *pButton, CCommandMenu *pParentMenu, const char *commandText, int iYOffset, int iXOffset = 0 );

	void UpdateCursorState( void );
	void UpdateCommandMenu(int menuIndex);
	void UpdateOnPlayerInfo( void );
	void UpdateHighlights( void );
	void UpdateSpectatorPanel( void );

	int	 KeyInput( int down, int keynum, const char *pszCurrentBinding );
	void InputPlayerSpecial( void );
	void GetAllPlayersInfo( void );
	void DeathMsg( int killer, int victim );

	void ShowCommandMenu(int menuIndex);
	void InputSignalHideCommandMenu( void );
	void HideCommandMenu( void );
	void SetCurrentCommandMenu( CCommandMenu *pNewMenu );
	void SetCurrentMenu( CMenuPanel *pMenu );

	void ShowScoreBoard( void );
	void HideScoreBoard( void );
	bool IsScoreBoardVisible( void );

	bool AllowedToPrintText() const;

	void ShowVGUIMenu( int iMenu );
	void HideVGUIMenu( void );
	void HideTopMenu( void );

	CMenuPanel* CreateTextWindow( int iTextToShow );

	CCommandMenu *CreateSubMenu( CommandButton *pButton, CCommandMenu *pParentMenu, int iYOffset, int iXOffset = 0 );

	// Data Handlers
	int GetValidClasses(int iTeam) { return m_iValidClasses[iTeam]; }
	int GetNumberOfTeams() { return m_iNumberOfTeams; }
	bool GetIsFeigning() const { return m_bIsFeigning; }
	DetpackState GetDetpackState() const { return m_DetpackState; }
	int GetBuildState() { return m_iBuildState; }
	int IsRandomPC() { return m_iRandomPC; }
	char *GetTeamName( int iTeam ) { return m_sTeamNames[iTeam]; }
	bool GetAllowSpectators() const { return m_bAllowSpectators; }

	// Message Handlers
	void MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf );
	void MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf );
	void MsgFunc_Feign(const char *pszName, int iSize, void *pbuf );
	void MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf );
	void MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf );
	void MsgFunc_MOTD( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_BuildSt( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_RandomPC( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_ServerName( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_TeamInfo( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_Spectator( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_AllowSpec( const char *pszName, int iSize, void *pbuf );
	void MsgFunc_SpecFade( const char *pszName, int iSize, void *pbuf );	
	void MsgFunc_ResetFade( const char *pszName, int iSize, void *pbuf );	

	void ResetFade();

	// Input
	bool SlotInput( int iSlot );

	virtual void paintBackground();

	CSchemeManager *GetSchemeManager( void ) { return &m_SchemeManager; }
	ScorePanel *GetScoreBoard( void ) { return m_pScoreBoard; }

	void *operator new( size_t stAllocateBlock );

public:
	// VGUI Menus
	CMenuPanel		*m_pCurrentMenu;
	CTeamMenuPanel	*m_pTeamMenu;
	int						m_StandardMenu;	// indexs in m_pCommandMenus
	int						m_SpectatorOptionsMenu;
	int						m_SpectatorCameraMenu;
	int						m_PlayerMenu; // a list of current player
	CClassMenuPanel	*m_pClassMenu;
	ScorePanel		*m_pScoreBoard;
	SpectatorPanel *		m_pSpectatorPanel;
	char			m_szServerName[ MAX_SERVERNAME_LENGTH ];
};

//============================================================
// Command Menu Button Handlers

class CMenuHandler_StringCommand : public ActionSignal
{
protected:
	char	m_pszCommand[MAX_COMMAND_SIZE];
	int		m_iCloseVGUIMenu;
public:
	CMenuHandler_StringCommand( const char* const pszCommand )
	{
		strncpy( m_pszCommand, pszCommand, MAX_COMMAND_SIZE);
		m_pszCommand[MAX_COMMAND_SIZE-1] = '\0';
		m_iCloseVGUIMenu = false;
	}

	CMenuHandler_StringCommand( const char* const pszCommand, int iClose )
	{
		strncpy( m_pszCommand, pszCommand, MAX_COMMAND_SIZE);
		m_pszCommand[MAX_COMMAND_SIZE-1] = '\0';
		m_iCloseVGUIMenu = true;
	}

	virtual void actionPerformed(Panel* panel)
	{
		gEngfuncs.pfnClientCmd(m_pszCommand);

		if (m_iCloseVGUIMenu)
			gViewPort->HideTopMenu();
		else
			gViewPort->HideCommandMenu();
	}
};

// This works the same as CMenuHandler_StringCommand, except it watches the string command 
// for specific commands, and modifies client vars based upon them.
class CMenuHandler_StringCommandWatch : public CMenuHandler_StringCommand
{
private:
public:
	CMenuHandler_StringCommandWatch( const char* const pszCommand ) : CMenuHandler_StringCommand( pszCommand )
	{
	}

	CMenuHandler_StringCommandWatch( const char* const pszCommand, int iClose ) : CMenuHandler_StringCommand( pszCommand, iClose )
	{
	}

	virtual void actionPerformed(Panel* panel)
	{
		CMenuHandler_StringCommand::actionPerformed( panel );

		// Try to guess the player's new team (it'll be corrected if it's wrong)
		if ( !strcmp( m_pszCommand, "jointeam 1" ) )
			g_iTeamNumber = 1;
		else if ( !strcmp( m_pszCommand, "jointeam 2" ) )
			g_iTeamNumber = 2;
		else if ( !strcmp( m_pszCommand, "jointeam 3" ) )
			g_iTeamNumber = 3;
		else if ( !strcmp( m_pszCommand, "jointeam 4" ) )
			g_iTeamNumber = 4;
	}
};

// Used instead of CMenuHandler_StringCommand for Class Selection buttons.
// Checks the state of hud_classautokill and kills the player if set
class CMenuHandler_StringCommandClassSelect : public CMenuHandler_StringCommand
{
private:
public:
	CMenuHandler_StringCommandClassSelect( char *pszCommand ) : CMenuHandler_StringCommand( pszCommand )
	{
	}

	CMenuHandler_StringCommandClassSelect( char *pszCommand, int iClose ) : CMenuHandler_StringCommand( pszCommand, iClose )
	{
	}

	virtual void actionPerformed(Panel* panel);
};

class CMenuHandler_PopupSubMenuInput : public CDefaultInputSignal
{
private:
	CCommandMenu *m_pSubMenu;
	Button		 *m_pButton;
public:
	CMenuHandler_PopupSubMenuInput( Button *pButton, CCommandMenu *pSubMenu )
	{
		m_pSubMenu = pSubMenu;
		m_pButton = pButton;
	}

	virtual void cursorMoved(int x,int y,Panel* panel)
	{
		//gViewPort->SetCurrentCommandMenu( m_pSubMenu );
	}

	virtual void cursorEntered(Panel* panel) 
	{
		gViewPort->SetCurrentCommandMenu( m_pSubMenu );

		if (m_pButton)
			m_pButton->setArmed(true);
	}
};

class CMenuHandler_LabelInput : public CDefaultInputSignal
{
private:
	ActionSignal *m_pActionSignal;
public:
	CMenuHandler_LabelInput( ActionSignal *pSignal )
	{
		m_pActionSignal = pSignal;
	}

	virtual void mousePressed(MouseCode code,Panel* panel)
	{
		m_pActionSignal->actionPerformed( panel );
	}
};



//================================================================
// Overidden Command Buttons for special visibilities
class ClassButton : public CommandButton
{
protected:
	int	m_iPlayerClass;

public:
	ClassButton( int iClass, const char* text,int x,int y,int wide,int tall, bool bNoHighlight ) : CommandButton( text,x,y,wide,tall, bNoHighlight)
	{
		m_iPlayerClass = iClass;
	}

	virtual int IsNotValid();
};

class TeamButton : public CommandButton
{
private:
	int	m_iTeamNumber;
public:
	TeamButton( int iTeam, const char* text,int x,int y,int wide,int tall ) : CommandButton( text,x,y,wide,tall)
	{
		m_iTeamNumber = iTeam;
	}

	virtual int IsNotValid()
	{
		int iTeams = gViewPort->GetNumberOfTeams();
		// Never valid if there's only 1 team
		if (iTeams == 1)
			return true;

		// Auto Team's always visible
		if (m_iTeamNumber == 5)
			return false;

		if (iTeams >= m_iTeamNumber && m_iTeamNumber != g_iTeamNumber)
			return false;

		return true;
	}
};

class FeignButton : public CommandButton
{
private:
	bool m_bFeignState;
public:
	FeignButton( const bool bState, const char* text,int x,int y,int wide,int tall ) : CommandButton( text,x,y,wide,tall)
	{
		m_bFeignState = bState;
	}

	virtual int IsNotValid()
	{
		// Only visible for spies
#ifdef _TFC
		if (g_iPlayerClass != PC_SPY)
			return true;
#endif

		if ( m_bFeignState == gViewPort->GetIsFeigning())
			return false;
		return true;
	}
};

class SpectateButton : public CommandButton
{
public:
	SpectateButton( const char* text,int x,int y,int wide,int tall, bool bNoHighlight ) : CommandButton( text,x,y,wide,tall, bNoHighlight)
	{
	}

	virtual int IsNotValid()
	{
		// Only visible if the server allows it
		if ( gViewPort->GetAllowSpectators() )
			return false;

		return true;
	}
};

class DisguiseButton : public CommandButton
{
private:
	int m_iValidTeamsBits;
public:
	DisguiseButton( int iValidTeamNumsBits, const char* text,int x,int y,int wide,int tall ) : CommandButton( text,x,y,wide,tall,false )
	{
		m_iValidTeamsBits = iValidTeamNumsBits;
	}

	virtual int IsNotValid()
	{
#ifdef _TFC
		// Only visible for spies
		if ( g_iPlayerClass != PC_SPY )
			return true;
#endif

		// if it's not tied to a specific team, then always show (for spies)
		if ( !m_iValidTeamsBits )
			return false;

		// if we're tied to a team make sure we can change to that team
		int iTmp = 1 << (gViewPort->GetNumberOfTeams() - 1);
		if ( m_iValidTeamsBits & iTmp )
			return false;
		return true;
	}
};

class DetpackButton : public CommandButton
{
private:
	DetpackState	m_DetpackState;
public:
	DetpackButton( DetpackState state, const char* text,int x,int y,int wide,int tall ) : CommandButton( text,x,y,wide,tall)
	{
		m_DetpackState = state;
	}

	virtual int IsNotValid()
	{
#ifdef _TFC
		// Only visible for demomen
		if (g_iPlayerClass != PC_DEMOMAN)
			return true;
#endif

		if ( m_DetpackState == gViewPort->GetDetpackState())
			return false;

		return true;
	}
};

class BuildButton : public CommandButton
{
private:
	int	m_iBuildState;
	int m_iBuildData;

public:
	enum Buildings
	{
		DISPENSER = 0,
		SENTRYGUN = 1,
		ENTRY_TELEPORTER = 2,
		EXIT_TELEPORTER = 3
	};

	BuildButton( int iState, int iData, const char* text,int x,int y,int wide,int tall ) : CommandButton( text,x,y,wide,tall)
	{
		m_iBuildState = iState;
		m_iBuildData = iData;
	}

	virtual int IsNotValid()
	{
#ifdef _TFC
		// Only visible for engineers
		if (g_iPlayerClass != PC_ENGINEER)
			return true;

		// If this isn't set, it's only active when they're not building
		if (m_iBuildState & BUILDSTATE_BUILDING)
		{
			// Make sure the player's building
			if ( !(gViewPort->GetBuildState() & BS_BUILDING) )
				return true;
		}
		else
		{
			// Make sure the player's not building
			if ( gViewPort->GetBuildState() & BS_BUILDING )
				return true;
		}

		if (m_iBuildState & BUILDSTATE_BASE)
		{
			// Only appear if we've got enough metal to build something, or something already built
			if ( gViewPort->GetBuildState() & (BS_HAS_SENTRYGUN | BS_HAS_DISPENSER | BS_CANB_SENTRYGUN | BS_CANB_DISPENSER | BS_HAS_ENTRY_TELEPORTER | BS_HAS_EXIT_TELEPORTER | BS_CANB_ENTRY_TELEPORTER | BS_CANB_EXIT_TELEPORTER) )
				return false;

			return true;
		}

		// Must have a building
		if (m_iBuildState & BUILDSTATE_HASBUILDING)
		{
			if ( m_iBuildData == BuildButton::DISPENSER && !(gViewPort->GetBuildState() & BS_HAS_DISPENSER) )
				return true;
			if ( m_iBuildData == BuildButton::SENTRYGUN && !(gViewPort->GetBuildState() & BS_HAS_SENTRYGUN) )
				return true;
			if ( m_iBuildData == BuildButton::ENTRY_TELEPORTER && !(gViewPort->GetBuildState() & BS_HAS_ENTRY_TELEPORTER) )
				return true;
			if ( m_iBuildData == BuildButton::EXIT_TELEPORTER && !(gViewPort->GetBuildState() & BS_HAS_EXIT_TELEPORTER) )
				return true;
		}

		// Can build something
		if (m_iBuildState & BUILDSTATE_CANBUILD)
		{
			// Make sure they've got the ammo and don't have one already
			if ( m_iBuildData == BuildButton::DISPENSER && (gViewPort->GetBuildState() & BS_CANB_DISPENSER) )
				return false;
			if ( m_iBuildData == BuildButton::SENTRYGUN && (gViewPort->GetBuildState() & BS_CANB_SENTRYGUN) )
				return false;
			if ( m_iBuildData == BuildButton::ENTRY_TELEPORTER && (gViewPort->GetBuildState() & BS_CANB_ENTRY_TELEPORTER) )
				return false;
			if ( m_iBuildData == BuildButton::EXIT_TELEPORTER && (gViewPort->GetBuildState() & BS_CANB_EXIT_TELEPORTER) )
				return false;

			return true;
		}
#endif
		return false;
	}
};

class MapButton : public CommandButton
{
private:
	char m_szMapName[ MAX_MAPNAME ];

public:
	MapButton( const char *pMapName, const char* text,int x,int y,int wide,int tall ) : CommandButton( text,x,y,wide,tall)
	{
		sprintf( m_szMapName, "maps/%s.bsp", pMapName );
	}

	virtual int IsNotValid()
	{
		const char *level = gEngfuncs.pfnGetLevelName();
		if (!level)
			return true;

		// Does it match the current map name?
		if ( strcmp(m_szMapName, level) )
			return true;

		return false;
	}
};

//-----------------------------------------------------------------------------
// Purpose: CommandButton which is only displayed if the player is on team X
//-----------------------------------------------------------------------------
class TeamOnlyCommandButton : public CommandButton
{
private:
	int m_iTeamNum;

public:
	TeamOnlyCommandButton( int iTeamNum, const char* text,int x,int y,int wide,int tall, bool flat ) : 
	  CommandButton( text, x, y, wide, tall, false, flat ), m_iTeamNum(iTeamNum) {}

	virtual int IsNotValid()
	{
		if ( g_iTeamNumber != m_iTeamNum )
			return true;

		return CommandButton::IsNotValid();
	}
};

//-----------------------------------------------------------------------------
// Purpose: CommandButton which is only displayed if the player is on team X
//-----------------------------------------------------------------------------
class ToggleCommandButton : public CommandButton, public CDefaultInputSignal
{
private:
	cvar_t *		m_cvar;
	CImageLabel *	pLabelOn;
	CImageLabel *	pLabelOff;
	

public:
	ToggleCommandButton( const char* cvarname, const char* text,int x,int y,int wide,int tall, bool flat ) : 
	  CommandButton( text, x, y, wide, tall, false, flat )
	 {
		m_cvar = gEngfuncs.pfnGetCvarPointer( cvarname );

			// Put a > to show it's a submenu
		pLabelOn = new CImageLabel( "checked", 0, 0 );
		pLabelOn->setParent(this);
		pLabelOn->addInputSignal(this);
				
		pLabelOff = new CImageLabel( "unchecked", 0, 0 );
		pLabelOff->setParent(this);
		pLabelOff->setEnabled(true);
		pLabelOff->addInputSignal(this);

		int textwide, texttall;
		getTextSize( textwide, texttall);
	
		// Reposition
		pLabelOn->setPos( textwide, (tall - pLabelOn->getTall()) / 2 );

		pLabelOff->setPos( textwide, (tall - pLabelOff->getTall()) / 2 );
		
		// Set text color to orange
		setFgColor(Scheme::sc_primary1);
	}

	virtual void cursorEntered(Panel* panel)
	{
		CommandButton::cursorEntered();
	}

	virtual void cursorExited(Panel* panel)
	{
		CommandButton::cursorExited();
	}

	virtual void mousePressed(MouseCode code,Panel* panel)
	{
		doClick();
	}

	virtual void paint( void )
	{
		if ( !m_cvar )
		{
			pLabelOff->setVisible(false);
			pLabelOn->setVisible(false);
		} 
		else if ( m_cvar->value )
		{
			pLabelOff->setVisible(false);
			pLabelOn->setVisible(true);
		}
		else
		{
			pLabelOff->setVisible(true);
			pLabelOn->setVisible(false);
		}

		CommandButton::paint();
	} 
};

class SpectToggleButton : public CommandButton, public CDefaultInputSignal
{
private:
	cvar_t *		m_cvar;
	CImageLabel *	pLabelOn; 

public:
	SpectToggleButton( const char* cvarname, const char* text,int x,int y,int wide,int tall, bool flat ) : 
	  CommandButton( text, x, y, wide, tall, false, flat )
	 {
		m_cvar = gEngfuncs.pfnGetCvarPointer( cvarname );

		// Put a > to show it's a submenu
		pLabelOn = new CImageLabel( "checked", 0, 0 );
		pLabelOn->setParent(this);
		pLabelOn->addInputSignal(this);
		

		int textwide, texttall;
		getTextSize( textwide, texttall);
	
		// Reposition
		pLabelOn->setPos( textwide, (tall - pLabelOn->getTall()) / 2 );
	  } 

	virtual void cursorEntered(Panel* panel)
	{
		CommandButton::cursorEntered();
	}

	virtual void cursorExited(Panel* panel)
	{
		CommandButton::cursorExited();
	}

	virtual void mousePressed(MouseCode code,Panel* panel)
	{
		doClick();
	}

	virtual void paintBackground()
	{
		if ( isArmed() )
		{
			drawSetColor( 143,143, 54, 125 ); 
			drawFilledRect( 5, 0,_size[0] - 5,_size[1]);
		}
	}
	
	virtual void paint( void )
	{
		if ( isArmed() )
		{ 
			setFgColor( 194, 202, 54, 0 );
		}
		else
		{
			setFgColor( 143, 143, 54, 15 );
		}

		if ( !m_cvar )
		{
			pLabelOn->setVisible(false);
		} 
		else if ( m_cvar->value )
		{
			pLabelOn->setVisible(true);
		}
		else
		{
			pLabelOn->setVisible(false);
		}

		Button::paint();
	} 
};

//================================================================
// Custom drawn scroll bars
class CTFScrollButton : public CommandButton
{
private:
	BitmapTGA	*m_pTGA;

public:
	CTFScrollButton(int iArrow, const char* text,int x,int y,int wide,int tall);

	virtual void paint( void );
	virtual void paintBackground( void );
};

// Custom drawn slider bar
class CTFSlider : public Slider
{
public:
	CTFSlider(int x,int y,int wide,int tall,bool vertical) : Slider(x,y,wide,tall,vertical)
	{
	};

	virtual void paintBackground( void );
};

// Custom drawn scrollpanel
class CTFScrollPanel : public ScrollPanel
{
public:
	CTFScrollPanel(int x,int y,int wide,int tall);
};

//================================================================
// Menu Panels that take key input
//============================================================
class CClassMenuPanel : public CMenuPanel
{
private:
	CTransparentPanel	*m_pClassInfoPanel[PC_LASTCLASS];
#ifdef _TFC
	Label				*m_pPlayers[PC_LASTCLASS];
	enum { STRLENMAX_PLAYERSONTEAM = 128 };
	char m_sPlayersOnTeamString[STRLENMAX_PLAYERSONTEAM];
#endif
	ClassButton			*m_pButtons[PC_LASTCLASS];
	CommandButton		*m_pCancelButton;
	ScrollPanel			*m_pScrollPanel;

	CImageLabel			*m_pClassImages[MAX_TEAMS][PC_LASTCLASS];

	int					m_iCurrentInfo;

public:
	CClassMenuPanel(int iTrans, int iRemoveMe, int x,int y,int wide,int tall);

	virtual bool SlotInput( int iSlot );
	virtual void Open( void );
	virtual void Update( void );
	virtual void SetActiveInfo( int iInput );
	virtual void Initialize( void );

	virtual void Reset( void )
	{
		CMenuPanel::Reset();
		m_iCurrentInfo = 0;
	}
};

#endif
