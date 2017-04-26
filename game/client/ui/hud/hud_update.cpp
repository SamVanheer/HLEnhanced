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
//
//  hud_update.cpp
//

#include <math.h>
#include "hud.h"
#include "cl_util.h"
#include <stdlib.h>
#include <memory.h>

void CL_ResetButtonBits( int bits );

extern float v_idlescale;
float in_fov;

bool CHLHud::PreThinkUpdateClient( client_data_t* cdata )
{
	auto bChanged = BaseClass::PreThinkUpdateClient( cdata );

	in_fov = cdata->fov;

	return bChanged;
}

bool CHLHud::PostThinkUpdateClient( client_data_t* cdata )
{
	auto bChanged = BaseClass::PostThinkUpdateClient( cdata );

	v_idlescale = m_iConcussionEffect;

	// return 1 if in anything in the client_data struct has been changed, 0 otherwise
	return bChanged;
}