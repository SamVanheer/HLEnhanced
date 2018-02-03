//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#ifndef GAME_CLIENT_UI_VGUI_VGUI_COMMANDBUTTON_H
#define GAME_CLIENT_UI_VGUI_VGUI_COMMANDBUTTON_H

#include <VGUI_Button.h>

#include "vgui_SchemeManager.h"

#include "vgui_Defs.h"

class CCommandMenu;
class CommandLabel;

//============================================================
// Command Buttons
class CommandButton : public vgui::Button
{
public:
	//Was 255 in the SDK, but emits a truncation of constant value warning. -1 is the same value - Solokiller
	static const char NO_BOUND_KEY = static_cast<char>( -1 );

private:
	int		m_iPlayerClass;
	bool	m_bFlat;

	// Submenus under this button
	CCommandMenu *m_pSubMenu;
	CCommandMenu *m_pParentMenu;
	CommandLabel *m_pSubLabel;

	char m_sMainText[ MAX_BUTTON_SIZE ];
	char m_cBoundKey;

	//SchemeHandle_t m_hTextScheme;

	void RecalculateText( void );

public:
	bool	m_bNoHighlight;

public:
	CommandButton( const char* text, int x, int y, int wide, int tall, bool bNoHighlight, bool bFlat );
	// Constructors
	CommandButton( const char* text, int x, int y, int wide, int tall, bool bNoHighlight = false );
	CommandButton( int iPlayerClass, const char* text, int x, int y, int wide, int tall, bool bFlat );

	void Init( void );

	// Menu Handling
	void AddSubMenu( CCommandMenu *pNewMenu );
	void AddSubLabel( CommandLabel *pSubLabel )
	{
		m_pSubLabel = pSubLabel;
	}

	virtual int IsNotValid( void )
	{
		return false;
	}

	void UpdateSubMenus( int iAdjustment );
	int GetPlayerClass() { return m_iPlayerClass; };
	CCommandMenu *GetSubMenu() { return m_pSubMenu; };

	CCommandMenu *getParentMenu( void );
	void setParentMenu( CCommandMenu *pParentMenu );

	// Overloaded vgui functions
	virtual void paint();
	//Needed to rename this since Clang doesn't like the overload - Solokiller
	virtual void setButtonText( const char *text );
	virtual void paintBackground();

	void cursorEntered( void );
	void cursorExited( void );

	void setBoundKey( char boundKey );
	char getBoundKey( void );
};

#endif //GAME_CLIENT_UI_VGUI_VGUI_COMMANDBUTTON_H
