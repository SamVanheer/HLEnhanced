/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#ifndef GAME_CLIENT_UI_HUD_CHUDHEALTH_H
#define GAME_CLIENT_UI_HUD_CHUDHEALTH_H

#include "shared/hud/CHudElement.h"
#include "hud.h"

#define DMG_IMAGE_LIFE		2	// seconds that image is up

#define DMG_IMAGE_POISON	0
#define DMG_IMAGE_ACID		1
#define DMG_IMAGE_COLD		2
#define DMG_IMAGE_DROWN		3
#define DMG_IMAGE_BURN		4
#define DMG_IMAGE_NERVE		5
#define DMG_IMAGE_RAD		6
#define DMG_IMAGE_SHOCK		7
//tf defines
#define DMG_IMAGE_CALTROP	8
#define DMG_IMAGE_TRANQ		9
#define DMG_IMAGE_CONCUSS	10
#define DMG_IMAGE_HALLUC	11
#define NUM_DMG_TYPES		12
// instant damage

#include "Damage.h"

struct DAMAGE_IMAGE
{
	float fExpire;
	float fBaseline;
	int	x, y;
};
	
//
//-----------------------------------------------------
//
class CHudHealth: public CBaseHudElement<CHLHud>
{
public:
	DECLARE_CLASS( CHudHealth, CBaseHudElement<CHLHud> );

	CHudHealth( const char* const pszName, CHLHud& hud );

	void Init() override;
	void VidInit() override;
	bool Draw( float fTime ) override;
	void Reset()  override;
	void MsgFunc_Health(const char *pszName,  int iSize, void *pbuf);
	void MsgFunc_Damage(const char *pszName,  int iSize, void *pbuf);
	int m_iHealth;
	int m_HUD_dmg_bio;
	int m_HUD_cross;
	float m_fAttackFront, m_fAttackRear, m_fAttackLeft, m_fAttackRight;
	void GetPainColor( int &r, int &g, int &b );
	float m_fFade;

private:
	HSPRITE m_hSprite;
	//HSPRITE m_hDamage;

	int m_iDmgHeight, m_iDmgWidth;
	
	DAMAGE_IMAGE m_dmg[NUM_DMG_TYPES];
	int	m_bitsDamage;
	bool DrawPain(float fTime);
	bool DrawDamage(float fTime);
	void CalcDamageDirection(Vector vecFrom);
	void UpdateTiles(float fTime, long bits);
};	

#endif //GAME_CLIENT_UI_HUD_CHUDHEALTH_H
