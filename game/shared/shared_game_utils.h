#ifndef GAME_SHARED_SHARED_GAME_UTILS_H
#define GAME_SHARED_SHARED_GAME_UTILS_H

#include "Color.h"

#include "FileSystem.h"
#include "FileSystemHelpers.h"

#include "CHashStringPool.h"

//This really needs to be done better. - Solokiller
#ifdef CLIENT_DLL
#include "cl_dll.h"
#endif

#include "strtools.h"

/**
*	dll agnostic game utility functionality - Solokiller
*/

// Makes these more explicit, and easier to find
#define FILE_GLOBAL static
#define DLL_GLOBAL

// Until we figure out why "const" gives the compiler problems, we'll just have to use
// this bogus "empty" define to mark things as constant.
#define CONSTANT

#define BUBBLES_MODEL "sprites/bubble.spr"

/**
*	Number of static buffers used by functions that return pointers to static string buffers.
*/
const size_t NUM_STATIC_BUFFERS = 4;

extern DLL_GLOBAL const Vector g_vecZero;

extern cvar_t* g_pDeveloper;

class CBaseEntity;

/**
*	Gets the CBaseEntity instance out of the edict, if it's a valid instance.
*	This used to return void*. This led to unsafe casts that could cause invalid conversions (and has, in some cases). Use static_cast or dynamic_cast. - Solokiller
*/
CBaseEntity* GET_PRIVATE( edict_t* pent );

extern CHashStringPool g_StringPool;

int UTIL_SharedRandomLong( unsigned int seed, int low, int high );
float UTIL_SharedRandomFloat( unsigned int seed, float low, float high );

/**
*	Generates a random long number in the range [ lLow, lHigh ].
*	@param lLow Lower bound.
*	@param lHigh Higher bound.
*	@return Random number, or lLow if lHigh is smaller than or equal to lLow.
*/
inline int32 UTIL_RandomLong( int32 lLow, int32 lHigh )
{
	return
#ifdef CLIENT_DLL
		gEngfuncs.pfnRandomLong
#else
		g_engfuncs.pfnRandomLong
#endif
		( lLow, lHigh );
}

/**
*	Generates a random float number in the range [ flLow, flHigh ].
*	@param flLow Lower bound.
*	@param flHigh Higher bound.
*	@return Random number.
*/
inline float UTIL_RandomFloat( float flLow, float flHigh )
{
	return
#ifdef CLIENT_DLL
		gEngfuncs.pfnRandomFloat
#else
		g_engfuncs.pfnRandomFloat
#endif
		( flLow, flHigh );
}

/**
*	Gets the game directory.
*	@param pszBuffer Buffer that will contain the game directory.
*	@param uiBufferSize Size of the buffer, in bytes.
*	@return Whether the directory was successfully copied into the buffer.
*/
bool UTIL_GetGameDir( char* pszBuffer, const size_t uiBufferSize );

/**
*	Gets circular gaussian spread.
*	@param[ out ] x X value.
*	@param[ out ] y Y value.
*/
inline void UTIL_GetCircularGaussianSpread( float& x, float& y )
{
	float z;
	do {
		x = UTIL_RandomFloat( -0.5, 0.5 ) + UTIL_RandomFloat( -0.5, 0.5 );
		y = UTIL_RandomFloat( -0.5, 0.5 ) + UTIL_RandomFloat( -0.5, 0.5 );
		z = x*x + y*y;
	}
	while( z > 1 );
}

/**
*	Gets shared circular gaussian spread.
*	Very specific to weapons code.
*	@param shared_rand Shared random seed.
*	@param iShot Shot.
*	@param[ out ] x X value.
*	@param[ out ] y Y value.
*/
inline void UTIL_GetSharedCircularGaussianSpread( const int shared_rand, const unsigned int iShot, float& x, float& y )
{
	x = UTIL_SharedRandomFloat( shared_rand + iShot, -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 1 + iShot ), -0.5, 0.5 );
	y = UTIL_SharedRandomFloat( shared_rand + ( 2 + iShot ), -0.5, 0.5 ) + UTIL_SharedRandomFloat( shared_rand + ( 3 + iShot ), -0.5, 0.5 );
}

void UTIL_StringToVector( Vector& vecOut, const char *pString );

void UTIL_StringToIntArray( int *pVector, int count, const char *pString );

/**
*	Parses a color out of a string. Format is RGBA.
*	@param color Color instance to store the result in.
*	@param uiCount Number of components to parse. Should never be more than 4.
*	@param pszString String to parse.
*/
void UTIL_StringToColor( Color& color, const size_t uiCount, const char* const pszString );

/**
*	Utility function to format strings without creating a buffer to store the result in.
*	@param pszFormat Format string.
*	@param ... Arguments.
*	@return Pointer to the string. Up to NUM_STATIC_BUFFERS strings returned sequentially from this can be valid at the same time.
*	@see NUM_STATIC_BUFFERS.
*/
char* UTIL_VarArgs( const char* pszFormat, ... );

/**
*	For handy use with ClientPrint params. This returns the string representation of the given integer.
*	@param iValue Value.
*	@return Pointer to the string. Up to NUM_STATIC_BUFFERS strings returned sequentially from this can be valid at the same time.
*	@see NUM_STATIC_BUFFERS.
*/
char* UTIL_dtos( const int iValue );

void UTIL_StripToken( const char *pKey, char *pDest );// for redundant keynames

char *safe_strcpy( char *dst, const char *src, const size_t len_dst );

int safe_sprintf( char *dst, const size_t len_dst, const char *format, ... );

//
// How did I ever live without ASSERT?
//
#ifdef	DEBUG
void DBG_AssertFunction( const bool fExpr, const char* szExpr, const char* szFile, int szLine, const char* szMessage );
#define ASSERT(f)		DBG_AssertFunction( !!( f ), #f, __FILE__, __LINE__, NULL )
#define ASSERTSZ(f, sz)	DBG_AssertFunction( !!( f ), #f, __FILE__, __LINE__, sz )
#else	// !DEBUG
#define ASSERT(f)
#define ASSERTSZ(f, sz)
#endif	// !DEBUG

#define MAX_COM_TOKEN 1500

extern char com_token[ MAX_COM_TOKEN ];

/**
*	Parses the given string and returns a pointer to the first non-whitespace character.
*	Returns null if there are no non-whitespace characters left.
*/
const char* COM_SkipWhitespace( const char* pszData );

/**
*	Parses the given string and skips past any single line comments ("//") that there may be.
*	@param pszData String to parse.
*	@param bWasComment Whether this line was a comment or not.
*	@return Pointer to the next line, or null if there was no more data.
*/
const char* COM_SkipComments( const char* pszData, bool& bWasComment );

/**
*	Parses the given string and stores the result in pszBuffer.
*	@param pszData Data to parse.
*	@param[ out ] pszBuffer Destination buffer.
*	@param uiBufferSize Size of the destination buffer, in characters.
*	@return Pointer to the next token, or null if there are no more tokens.
*/
const char* COM_Parse( const char* pszData, char* pszBuffer, const size_t uiBufferSize );

/**
*	Parses the given string and stores the result in com_token.
*	@param pszData Data to parse.
*	@return Pointer to the next token, or null if there are no more tokens.
*/
const char* COM_Parse( const char* pszData );

/**
*	@param pszBuffer Buffer to check.
*	@return true if additional data is waiting to be processed on this line.
*/
bool COM_TokenWaiting( const char* const pszBuffer );

char *memfgets( byte *pMemFile, int fileSize, int &filePos, char *pBuffer, int bufferSize );

/**
*	Cross-dll get cvar pointer function.
*	@param pszName Name of the cvar.
*	@return CVar, or null if the cvar doesn't exist.
*/
cvar_t* CVarGetPointer( const char* const pszName );

/**
*	Cross-dll alert function.
*	@param aType Alert type.
*	@param pszFormat Format string.
*	@param ... Arguments.
*/
void Alert( ALERT_TYPE aType, const char* const pszFormat, ... );

/**
*	Print to console.
*	@param pszFormat Format string.
*	@param ... Arguments.
*/
void Con_Printf( const char* const pszFormat, ... );

/**
*	Print to console if debug is enabled.
*	@param pszFormat Format string.
*	@param ... Arguments.
*/
void Con_DPrintf( const char* const pszFormat, ... );

/**
*	Gets the contents for the given position.
*	@param vec Position.
*	@return Contents.
*	@see Contents
*/
Contents UTIL_PointContents( const Vector& vec );

/**
*	Search for water transition along a vertical line.
*	@param vecPosition Position.
*	@param minz Minimum height level.
*	@param maxz Maximum height level.
*	@return Water level.
*/
float UTIL_WaterLevel( const Vector& vecPosition, float minz, float maxz );

/**
*	Creates bubbles that float upwards inside of a box.
*	@param mins Box mins.
*	@param maxs Box maxs.
*	@param count Number of bubbles to create.
*	@param flSpeed Speed of the bubbles.
*/
void UTIL_Bubbles( const Vector& mins, const Vector& maxs, int count, const float flSpeed = 8 );

/**
*	Creates a trail of bubbles that float upwards.
*	@param from Starting position.
*	@param to End position.
*	@param count Number of bubbles to create.
*	@param flSpeed Speed of the bubbles.
*/
void UTIL_BubbleTrail( const Vector& from, const Vector& to, int count, const float flSpeed = 8 );

/**
*	Converts a directional vector to angles.
*	@param vec Directional vector.
*	@return Angles.
*/
Vector UTIL_VecToAngles( const Vector& vec );

/**
*	Gets a velocity vector for the given amount of damage.
*/
Vector VecVelocityForDamage( const float flDamage );

// Testing strings for nullity
#define iStringNull ( ( string_t ) 0 )
inline bool FStringNull( string_t iString ) { return iString == iStringNull; }

inline bool FStrEq( const char* sz1, const char* sz2 )
{
	return ( strcmp( sz1, sz2 ) == 0 );
}

void UTIL_MakeVectors( const Vector& vecAngles );

inline void UTIL_MakeVectorsPrivate( const Vector& vecAngles, Vector* p_vForward, Vector* p_vRight, Vector* p_vUp )
{
#ifdef CLIENT_DLL
	AngleVectors( vecAngles, p_vForward, p_vRight, p_vUp );
#else
	g_engfuncs.pfnAngleVectors( vecAngles, 
								reinterpret_cast<float*>( p_vForward ), 
								reinterpret_cast<float*>( p_vRight ), 
								reinterpret_cast<float*>( p_vUp ) );
#endif
}

/**
*	Like MakeVectors, but assumes pitch isn't inverted.
*/
inline void UTIL_MakeAimVectors( const Vector& vecAngles )
{
	Vector rgflVec = vecAngles;
	rgflVec[ 0 ] = -rgflVec[ 0 ];
	UTIL_MakeVectors( rgflVec );
}

void UTIL_MakeInvVectors( const Vector& vec, globalvars_t* pgv );

void UTIL_Sparks( const Vector& position );

void UTIL_Ricochet( const Vector& position, float scale );

float UTIL_WeaponTimeBase();

/**
*	Used only by specific code to run code on destruction of the entity.
*	If you're not already using it, you probably shouldn't use it.
*/
void UTIL_DestructEntity( CBaseEntity* pEntity );

void UTIL_Remove( CBaseEntity* pEntity );
void UTIL_RemoveNow( CBaseEntity* pEntity );

/**
*	Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
*/
bool COM_FileBase( const char *in, char *out, size_t uiSizeInCharacters );

template<size_t SIZE>
bool COM_FileBase( const char *in, char ( & out )[ SIZE ] )
{
	return COM_FileBase( in, out, SIZE );
}

/**
*	@return Whether the current game is the given game name. This is the name of the game directory
*/
bool UTIL_IsGame( const char* game );

/**
*	Can cheats (e.g. impulse 101) be used?
*/
bool UTIL_CheatsAllowed();

#endif //GAME_SHARED_SHARED_GAME_UTILS_H