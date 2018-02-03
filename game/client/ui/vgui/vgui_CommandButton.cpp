//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "hud.h"
#include "cl_util.h"

#include "vgui_CommandButton.h"
#include "vgui_CommandLabel.h"
#include "vgui_CommandMenu.h"
#include "vgui_InputSignalHandlers.h"

using namespace vgui;

//===========================================================
// All TFC Hud buttons are derived from this one.
CommandButton::CommandButton( const char* text, int x, int y, int wide, int tall, bool bNoHighlight ) : Button( "", x, y, wide, tall )
{
	m_iPlayerClass = 0;
	m_bNoHighlight = bNoHighlight;
	m_bFlat = false;
	Init();
	setText( text );
}

CommandButton::CommandButton( int iPlayerClass, const char* text, int x, int y, int wide, int tall, bool bFlat ) : Button( "", x, y, wide, tall )
{
	m_iPlayerClass = iPlayerClass;
	m_bNoHighlight = false;
	m_bFlat = bFlat;
	Init();
	setText( text );
}

CommandButton::CommandButton( const char *text, int x, int y, int wide, int tall, bool bNoHighlight, bool bFlat ) : Button( "", x, y, wide, tall )
{
	m_iPlayerClass = 0;
	m_bFlat = bFlat;
	m_bNoHighlight = bNoHighlight;
	Init();
	setText( text );
}

void CommandButton::Init( void )
{
	m_pSubMenu = NULL;
	m_pSubLabel = NULL;
	m_pParentMenu = NULL;

	// Set text color to orange
	setFgColor( Scheme::sc_primary1 );

	// left align
	setContentAlignment( vgui::Label::a_west );

	// Add the Highlight signal
	if( !m_bNoHighlight )
		addInputSignal( new CHandler_CommandButtonHighlight( this ) );

	// not bound to any button yet
	m_cBoundKey = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Prepends the button text with the current bound key
//			if no bound key, then a clear space ' ' instead
//-----------------------------------------------------------------------------
void CommandButton::RecalculateText( void )
{
	char szBuf[ 128 ];

	if( m_cBoundKey != 0 )
	{
		if( m_cBoundKey == NO_BOUND_KEY )
		{
			strcpy( szBuf, m_sMainText );
		}
		else
		{
			sprintf( szBuf, "  %c  %s", m_cBoundKey, m_sMainText );
		}
		szBuf[ MAX_BUTTON_SIZE - 1 ] = 0;
	}
	else
	{
		// just draw a space if no key bound
		sprintf( szBuf, "     %s", m_sMainText );
		szBuf[ MAX_BUTTON_SIZE - 1 ] = 0;
	}

	Button::setText( szBuf );
}

void CommandButton::setButtonText( const char *text )
{
	strncpy( m_sMainText, text, MAX_BUTTON_SIZE );
	m_sMainText[ MAX_BUTTON_SIZE - 1 ] = 0;

	RecalculateText();
}

void CommandButton::setBoundKey( char boundKey )
{
	m_cBoundKey = boundKey;
	RecalculateText();
}

char CommandButton::getBoundKey( void )
{
	return m_cBoundKey;
}

void CommandButton::AddSubMenu( CCommandMenu *pNewMenu )
{
	m_pSubMenu = pNewMenu;

	// Prevent this button from being pushed
	setMouseClickEnabled( MOUSE_LEFT, false );
}

void CommandButton::UpdateSubMenus( int iAdjustment )
{
	if( m_pSubMenu )
		m_pSubMenu->RecalculatePositions( iAdjustment );
}

void CommandButton::paint()
{
	// Make the sub label paint the same as the button
	if( m_pSubLabel )
	{
		if( isSelected() )
			m_pSubLabel->PushDown();
		else
			m_pSubLabel->PushUp();
	}

	// draw armed button text in white
	if( isArmed() )
	{
		setFgColor( Scheme::sc_secondary2 );
	}
	else
	{
		setFgColor( Scheme::sc_primary1 );
	}

	Button::paint();
}

void CommandButton::paintBackground()
{
	if( m_bFlat )
	{
		if( isArmed() )
		{
			// Orange Border
			drawSetColor( Scheme::sc_secondary1 );
			drawOutlinedRect( 0, 0, _size[ 0 ], _size[ 1 ] );
		}
	}
	else
	{
		if( isArmed() )
		{
			// Orange highlight background
			drawSetColor( Scheme::sc_primary2 );
			drawFilledRect( 0, 0, _size[ 0 ], _size[ 1 ] );
		}

		// Orange Border
		drawSetColor( Scheme::sc_secondary1 );
		drawOutlinedRect( 0, 0, _size[ 0 ], _size[ 1 ] );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Highlights the current button, and all it's parent menus
//-----------------------------------------------------------------------------
void CommandButton::cursorEntered( void )
{
	// unarm all the other buttons in this menu
	CCommandMenu *containingMenu = getParentMenu();
	if( containingMenu )
	{
		containingMenu->ClearButtonsOfArmedState();

		// make all our higher buttons armed
		CCommandMenu *pCParent = containingMenu->GetParentMenu();
		if( pCParent )
		{
			CommandButton *pParentButton = pCParent->FindButtonWithSubmenu( containingMenu );

			pParentButton->cursorEntered();
		}
	}

	// arm ourselves
	setArmed( true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CommandButton::cursorExited( void )
{
	// only clear ourselves if we have do not have a containing menu
	// only stay armed if we have a sub menu
	// the buttons only unarm themselves when another button is armed instead
	if( !getParentMenu() || !GetSubMenu() )
	{
		setArmed( false );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns the command menu that the button is part of, if any
// Output : CCommandMenu *
//-----------------------------------------------------------------------------
CCommandMenu *CommandButton::getParentMenu( void )
{
	return m_pParentMenu;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the menu that contains this button
// Input  : *pParentMenu - 
//-----------------------------------------------------------------------------
void CommandButton::setParentMenu( CCommandMenu *pParentMenu )
{
	m_pParentMenu = pParentMenu;
}