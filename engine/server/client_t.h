#ifndef ENGINE_SERVER_CLIENT_T_H
#define ENGINE_SERVER_CLIENT_T_H

#include <cstdint>

#include "extdll.h"
#include "util.h"
#include "com_model.h"
#include "EngineDefs.h"
#include "FileSystem.h"
#include "progs.h"
#include "sizebuf.h"
#include "usercmd.h"


//Dummy version to make sure client_t is the right size.
struct netchan_t
{
	byte dummy[ 0x2414 ];
};

struct client_frame_t
{
};

/**
*	User ID type.
*/
enum class UserIdType : int
{
	WON		= 0,
	STEAM	= 1,
	VALVE	= 2,
	HLTV	= 3,
};

/**
*	A client's user ID.
*/
struct USERID_t
{
	UserIdType idtype;

	uint64_t m_SteamID;

	unsigned int clientip;
};

/**
*	Reverse engineered client_t structure.
*	Useful to check engine state manually.
*/
struct client_t
{
	qboolean active;
	qboolean spawned;
	qboolean fully_connected;
	qboolean connected;
	qboolean uploading;
	qboolean hasusrmsgs;
	qboolean has_force_unmodified;
	netchan_t netchan;

	int chokecount;
	int delta_sequence;
	qboolean fakeclient;
	qboolean proxy;

	usercmd_t lastcmd;

	double connecttime;
	double cmdtime;
	double ignorecmdtime;
	float latency;
	float packet_loss;
	double localtime;
	double nextping;
	double svtimebase;

	sizebuf_t datagram;
	byte datagram_buf[ MAX_DATAGRAM ];

	double connection_started;
	double next_messagetime;
	double next_messageinterval;

	qboolean send_message;
	qboolean skip_message;

	client_frame_t* frames;

	event_state_t events;
	edict_t* edict;
	edict_t* pViewEntity;

	int userid;
	USERID_t network_userid;

	char userinfo[ MAX_INFO_STRING ];
	qboolean sendinfo;
	float sendinfo_time;

	char hashedcdkey[ 64 ];
	char name[ 32 ];

	int topcolor;
	int bottomcolor;
	int entityId;

	resource_t resourcesonhand;
	resource_t resourcesneeded;
	FileHandle_t upload;
	qboolean uploaddoneregistering;
	customization_t customdata;
	int crcValue;

	int lw;
	int lc;

	char physinfo[ MAX_PHYSINFO_STRING ];

	qboolean m_bLoopback;
	uint32_t m_VoiceStreams[ 2 ];
	double m_lastvoicetime;
	int m_sendrescount;
};

/**
*	Gets a client_t from an edict. - Solokiller
*/
inline client_t* SV_ClientFromEdict( edict_t* pEdict )
{
	char* pszUserInfo = g_engfuncs.pfnGetInfoKeyBuffer( pEdict );

	if( !pszUserInfo )
		return nullptr;

	client_t* pClient = reinterpret_cast<client_t*>( reinterpret_cast<uint8_t*>( pszUserInfo ) - offsetof( client_t, userinfo ) );

	return pClient;
}

#endif //ENGINE_SERVER_CLIENT_T_H