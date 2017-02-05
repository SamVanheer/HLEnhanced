#
#	Utility programs
#

project( HLEnhanced_Utils )

include( ${CMAKE_SOURCE_DIR}/cmake/WinXPSupport.cmake )

set( SHARED_UTIL_INCLUDES
	${CMAKE_CURRENT_SOURCE_DIR}/common
)

#Common code shared between all tools.
set( SHARED_UTIL_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmdlib.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/cmdlib.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/mathlib.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/mathlib.h
	${CMAKE_CURRENT_SOURCE_DIR}/common/scriplib.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/scriplib.h
)

set( BSPLIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/bsplib.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/bsplib.h
)

set( THREADS_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/threads.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/threads.h
)

set( WADLIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/wadlib.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/wadlib.h
)

set( POLYLIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/polylib.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/polylib.h
)

set( LBMLIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/lbmlib.c
	${CMAKE_CURRENT_SOURCE_DIR}/common/lbmlib.h
)

set( TRILIB_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/common/trilib.c
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
