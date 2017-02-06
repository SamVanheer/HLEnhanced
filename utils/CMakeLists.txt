#
#	Utility programs
#

project( HLEnhanced_Utils )

include( ${CMAKE_SOURCE_DIR}/cmake/WinXPSupport.cmake )

if( MSVC )
	#Warning level 4 for extra warnings.
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4" )
endif()

set( SHARED_UTIL_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/common
)

#Common code shared between all tools.
set( SHARED_UTIL_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmdlib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmdlib.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/mathlib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/mathlib.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/scriplib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/scriplib.h
)

set( BSPLIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/bsplib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/bsplib.h
)

set( THREADS_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/threads.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/threads.h
)

set( WADLIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/wadlib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/wadlib.h
)

set( POLYLIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/polylib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/polylib.h
)

set( LBMLIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/lbmlib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/lbmlib.h
)

set( TRILIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/trilib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/common/trilib.h
)

#Directory to use when generating filters for Visual Studio.
set( ROOT_FILTER_DIR ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory( bspinfo )
add_subdirectory( light )
add_subdirectory( makefont )
add_subdirectory( makels )
add_subdirectory( mdlviewer )
add_subdirectory( mkmovie )
add_subdirectory( procinfo )
add_subdirectory( qbsp2 )
add_subdirectory( qcsg )
add_subdirectory( qlumpy )
add_subdirectory( qrad )
add_subdirectory( serverctrl )
add_subdirectory( smdlexp )
add_subdirectory( sprgen )
add_subdirectory( studiomdl )
add_subdirectory( visx2 )
add_subdirectory( xwad )
