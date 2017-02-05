#
#	This file checks if thread-safe initialization needs to be disabled.
#	If targeting Windows XP (v*_xp toolset) and using an MSVC version of Visual Studio 2015 or newer, it is disabled.
#	This file must be included AFTER starting the project, else the required variables will not have been defined yet.
#

#Avoid doing this more than once.
if( NOT MSVC OR CMAKE_CXX_FLAGS MATCHES ".*/Zc:threadSafeInit-.*" )
	return()
endif()

#Verify that we have the data we need.
if( NOT MSVC_VERSION OR NOT CMAKE_VS_PLATFORM_TOOLSET )
	MESSAGE( FATAL_ERROR "Include WinXPSupport.cmake after starting a project" )
endif()

if( NOT "${MSVC_VERSION}" LESS 1900 AND CMAKE_VS_PLATFORM_TOOLSET MATCHES ".*_xp$" )
	MESSAGE( STATUS "Disabling Thread-safe initialization for Windows XP support" )
	#Disable thread-safe init so Windows XP users don't get crashes.
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zc:threadSafeInit-" )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:threadSafeInit-" )
endif()
