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
#ifndef GAME_SERVER_ENTITIES_CBLOODSPLAT_H
#define GAME_SERVER_ENTITIES_CBLOODSPLAT_H

class CBloodSplat : public CBaseEntity
{
public:
	DECLARE_CLASS( CBloodSplat, CBaseEntity );
	DECLARE_DATADESC();

	void	CreateSplat( CBaseEntity* pOwner );
	void	Spray();
};

#endif //GAME_SERVER_ENTITIES_CBLOODSPLAT_H
