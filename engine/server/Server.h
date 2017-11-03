#ifndef ENGINE_SERVER_SERVER_H
#define ENGINE_SERVER_SERVER_H

#include <cstddef>

#include "extdll.h"
#include "util.h"
#include "com_model.h"
#include "EngineDefs.h"
#include "entity_state.h"
#include "sizebuf.h"

/**
*	Reverse engineered engine data structures.
*/

struct consistency_t
{
	const char* filename;
	int issound;
	int orig_index;
	int value;
	int check_type;
	Vector mins;
	Vector maxs;
};

struct event_t
{
	short index;
	byte padding[ 2 ];

	const char* filename;
	int filesize;
	const char* pszScript;
};

enum server_state_t
{
	ss_loading,
	ss_active
};

struct extra_baselines_t
{
	int number;
	int classname[ 64 ];
	entity_state_t baseline[ 64 ];
};

/**
*	@brief Reverse engineered from the engine. This is the server data structure that contains a bunch of state. - Solokiller
*/
struct server_t
{
	qboolean active;
	qboolean paused;
	qboolean loadgame;

	double time;
	double oldtime;

	int lastcheck; // number of last checked client
	double lastchecktime;

	char name[ MAX_QPATH ];
	char oldname[ MAX_QPATH ];
	char startspot[ MAX_QPATH ];
	char modelname[ MAX_QPATH ];

	/**
	*	@brief This is worldspawn. Contains the entity data string.
	*/
	model_t* worldmodel;

	CRC32_t worldmapCRC;

	byte clientdllmd5[ 16 ];

	/**
	*	@brief Resource download list. Could be used to trim the files needed for download?
	*/
	resource_t resourcelist[ MAX_RESOURCES ];
	int num_resources;

	/**
	*	@brief List of files that must be consistent between client and server (i.e. identical CRC)
	*/
	consistency_t consistency_list[ MAX_CONSISTENCY ];
	int num_consistency;

	/**
	*	@brief List of models that have been precached.
	*/
	const char* model_precache[ MAX_MODELS ];

	/**
	*	@brief List of models that have been loaded
	*	Model 0 is unused (modelindex 0 is used for entities without models)
	*	Model 1 is the world (BSP file data)
	*/
	model_t* models[ MAX_MODELS ];

	/**
	*	@brief Resource flags
	*	@see ResFlag
	*/
	byte model_precache_flags[ MAX_MODELS ];

	/**
	*	@brief List of precached events
	*/
	event_t event_precache[ MAX_EVENTS ];

	/**
	*	@brief List of precached sounds
	*/
	const char* sound_precache[ MAX_SOUNDS ];

	/**
	*	@brief Hash map mapping filename to index into sound_precache
	*	TODO: reverse engineer hashing algorithm - Solokiller
	*/
	short sound_precache_hashedlookup[ ( MAX_SOUNDS * 2 ) - 1 ];

	byte padding[ 2 ];

	/**
	*	@brief Whether the sound precache hash table has been built yet
	*	Until it's built, you must use the list to find sounds
	*/
	qboolean sound_precache_hashedlookup_built;

	/**
	*	@brief Files to precache in addition to precached sounds, models, etc
	*/
	const char* generic_precache[ MAX_GENERIC ];

	/**
	*	@brief Stores names of files to precache, loaded from mapname.res file
	*/
	char generic_precache_names[ MAX_GENERIC ][ MAX_QPATH ];

	/**
	*	@brief Number of .res entries loaded and precached
	*/
	int num_generic_names;

	const char* lightstyles[ MAX_LIGHTSTYLES ];

	/**
	*	@brief The current number of allocated edicts
	*/
	int num_edicts;

	/**
	*	@brief The maximum number of edicts that can be allocated
	*	This will not match MAX_EDICTS, nor the "edicts" liblist.gam/num_edicts command line parameter, as additional edicts are allocated for each player
	*/
	int max_edicts;

	/**
	*	@brief Array of edicts, max_edicts large
	*/
	edict_t* edicts;

	entity_state_t* baselines;
	extra_baselines_t* instance_baselines;

	server_state_t state;

	sizebuf_t datagram;
	byte datagram_buf[ MAX_DATAGRAM ];

	sizebuf_t reliable_datagram;
	byte reliable_datagram_buf[ MAX_DATAGRAM ];

	sizebuf_t multicast;
	byte multicast_buf[ MAX_MULTICAST ];

	sizebuf_t spectator;
	byte spectator_buf[ MAX_MULTICAST ];

	sizebuf_t signon;
	byte signon_data[ MAX_SIGNON ];
};

/**
*	@brief This gets the server data structure from the engine. It's a complete hack, and only works with the current GoldSource build, but that's unlikely to change.
*	Doesn't work with Xash.
*	- Solokiller
*	@return server_t instance.
*/
inline server_t* SV_GetServer()
{
	ASSERT( gpGlobals );

	//Map not loaded yet, can't retrieve data
	if( FStringNull( gpGlobals->startspot ) )
		return nullptr;

	byte* pStartSpot = ( byte* ) STRING( gpGlobals->startspot );

	return ( server_t* ) ( pStartSpot - offsetof( server_t, startspot ) );
}

#endif //ENGINE_SERVER_SERVER_H