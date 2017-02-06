#ifndef COMMON_PLATFORM_H
#define COMMON_PLATFORM_H

/*
*	Platform specific code compatibility.
*/

// Misc C-runtime library headers
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef WIN32

// Prevent tons of unused windows definitions
#define WIN32_LEAN_AND_MEAN
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX
#include "winsani_in.h"
#include "windows.h"
#include "winsani_out.h"

#include <malloc.h> // _alloca
#include <sys/stat.h>

#define stricmp _stricmp
#define strupr _strupr
#define strdup _strdup
#define stricmp _stricmp
#define strnicmp _strnicmp
#define alloca _alloca
#define unlink _unlink
#define stat _stat

//Older versions of Visual Studio (< VS2015) didn't define these. - Solokiller
#if _MSC_VER < 1900
#define snprintf _snprintf
#define vsnprintf _vsnprintf
//Provided by stdlib in VS2015 - Solokiller
//TODO: needed on Linux?
#define fabs(x) ((x) > 0 ? (x) : 0 - (x))
#endif

#define MakeDirectory( pszDirectory ) CreateDirectoryA( pszDirectory, nullptr )

//Define these macros once only. The SDK is such a mess. - Solokiller
#define DLLEXPORT __declspec( dllexport )
#define DLLIMPORT __declspec( dllimport )

#define FILESYSTEM_PATH_SEPARATOR "\\"
#define FILESYSTEM_OTHER_PATH_SEPARATOR "/"
#define FILESYSTEM_PATH_SEPARATOR_CHAR '\\'
#define FILESYSTEM_OTHER_PATH_SEPARATOR_CHAR '/'

//This isn't needed 99.99% of the time. Provide an escape hatch nonetheless. - Solokiller
#ifndef HL_USE_VOID_DEF
#undef VOID
#endif

/**
*	@defgroup InheritanceDecls
*	Declarations used to specify a class has a specific inheritance model.
*	Used to forward declare classes and allow the correct pointer to member pointer size to be specified.
*	If this is not done, and the default model isn't correct for a pointer, it'll cause stack corruption issues at runtime.
*	@{
*/

#define SINGLE_INHERITANCE __single_inheritance
#define MULTIPLE_INHERITANCE __multiple_inheritance
#define VIRTUAL_INHERITANCE __virtual_inheritance

/** @} */

#else

//mkdir
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <alloca.h>

#define MAX_PATH PATH_MAX

#define stricmp strcasecmp
#define strnicmp strncasecmp

#define MakeDirectory( pszDirectory ) mkdir( pszDirectory, 0777 )

#define DLLEXPORT __attribute__( ( visibility( "default" ) ) )
#define DLLIMPORT

#define FILESYSTEM_PATH_SEPARATOR "/"
#define FILESYSTEM_OTHER_PATH_SEPARATOR "\\"
#define FILESYSTEM_PATH_SEPARATOR_CHAR '/'
#define FILESYSTEM_OTHER_PATH_SEPARATOR_CHAR '\\'

#define __cdecl

//TODO: GLM has ivec2, use that. - Solokiller
typedef struct tagPOINT
{
	int x;
	int y;
} POINT;

//TODO: Should handle windows too - Solokiller
#if defined(OSX)
#define SO_ARCH_SUFFIX ".dylib"
#else
#if defined ( __x86_64__ )
#define SO_ARCH_SUFFIX "_amd64.so"
#else
#define SO_ARCH_SUFFIX ".so"
#endif
#endif

//Not used on Linux arches.
#define SINGLE_INHERITANCE
#define MULTIPLE_INHERITANCE
#define VIRTUAL_INHERITANCE

#endif

#undef ARRAYSIZE

/*
*	C++14 array sizeof. Detects pointers, works in static_assert. - Solokiller
*/
template<typename T, size_t SIZE>
inline constexpr size_t _ArraySizeof( const T ( & )[ SIZE ] )
{
	return SIZE;
}

#define ARRAYSIZE( p )	_ArraySizeof( p )

/**
*	offsetof that doesn't trigger compiler warnings in GCC. - Solokiller
*	Based on Angelscript's asOFFSET.
*/
#define OFFSETOF( type, member ) ( ( size_t ) ( &reinterpret_cast<type*>( 100000 )->member ) - 100000 )

#ifndef DISABLE_CXX11_ATTRIBUTES
#define NORETURN [[noreturn]]
#else
#define NORETURN
#endif

#endif //COMMON_PLATFORM_H
