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
#ifndef GAME_CLIENT_UI_HUD_CHUDBATTERY_H
#define GAME_CLIENT_UI_HUD_CHUDBATTERY_H

#include "shared/hud/CHudElement.h"
#include "hud.h"

class CHudBattery : public CBaseHudElement<CHLHud>
{
public:
	DECLARE_CLASS( CHudBattery, CBaseHudElement<CHLHud> );

	CHudBattery( const char* const pszName, CHLHud& hud );

	void Init()  override;
	void VidInit() override;
	bool Draw( float flTime ) override;
	void MsgFunc_Battery( const char *pszName, int iSize, void *pbuf );

private:
	HSPRITE m_hSprite1;
	HSPRITE m_hSprite2;
	const wrect_t *m_prc1;
	const wrect_t *m_prc2;
	int	  m_iBat;
#if defined( _TFC )
	int	  m_iBatMax;
#endif
	float m_fFade;
	int	  m_iHeight;		// width of the battery innards
};

#endif //GAME_CLIENT_UI_HUD_CHUDBATTERY_H
