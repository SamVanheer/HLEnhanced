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
#ifndef GAME_SERVER_ENTITIES_CSPRAYCAN_H
#define GAME_SERVER_ENTITIES_CSPRAYCAN_H

//==============================================
// !!!UNDONE:ultra temporary SprayCan entity to apply
// decal frame at a time. For PreAlpha CD
//==============================================
class CSprayCan : public CBaseEntity
{
public:
	DECLARE_CLASS( CSprayCan, CBaseEntity );

	void	CreateSprayCan( CBaseEntity* pOwner );
	void	Think() override;

	virtual int	ObjectCaps() const override { return FCAP_DONT_SAVE; }
};

#endif //GAME_SERVER_ENTITIES_CSPRAYCAN_H
