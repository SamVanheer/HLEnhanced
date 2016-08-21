/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#ifndef GAME_SERVER_ENTITIES_WEAPONS_CDEAGLELASER_H
#define GAME_SERVER_ENTITIES_WEAPONS_CDEAGLELASER_H

#if USE_OPFOR
#include "entities/CLaserSpot.h"

/**
*	Identical to CLaserSpot, different class to avoid RPG laser confusion logic. - Solokiller
*/
class CDEagleLaser : public CLaserSpot
{
public:
	DECLARE_CLASS( CDEagleLaser, CLaserSpot );

	static CDEagleLaser* CreateSpot();
};
#endif //USE_OPFOR

#endif //GAME_SERVER_ENTITIES_WEAPONS_CDEAGLELASER_H