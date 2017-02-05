###################################################
#                                                 #
#                                                 #
#	Half-Life Enhanced CMake build file           #
#                                                 #
#                                                 #
###################################################
cmake_minimum_required( VERSION 3.6 )

include( cmake/InputFilesList.cmake )
include( cmake/MSVCRuntime.cmake )

project( HLEnhanced )

include( cmake/WinXPSupport.cmake )

#Needs static CRT to avoid alloc issues.
set( MSVC_RUNTIME "static" )

configure_msvc_runtime()

set(variables
  CMAKE_C_FLAGS_DEBUG
  CMAKE_C_FLAGS_MINSIZEREL
  CMAKE_C_FLAGS_RELEASE
  CMAKE_C_FLAGS_RELWITHDEBINFO
  CMAKE_CXX_FLAGS_DEBUG
  CMAKE_CXX_FLAGS_MINSIZEREL
  CMAKE_CXX_FLAGS_RELEASE
  CMAKE_CXX_FLAGS_RELWITHDEBINFO
)
message(STATUS "Initial build flags:")
foreach(variable ${variables})
  message(STATUS "  '${variable}': ${${variable}}")
endforeach()
message(STATUS "")

#C++14 support
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )

#Get the steam/steamapps/common directory
set( STEAMCOMMON "$ENV{SteamCommon}" CACHE PATH "Steam common directory path" )

if( "${STEAMCOMMON}" STREQUAL "" )

	MESSAGE( FATAL_ERROR "Unable to get Steam common directory path" )

endif()

#Base dir for game outputs
set( GAME_BIN_PATH "${STEAMCOMMON}/Half-Life" )
set( GAME_BASE_PATH "${GAME_BIN_PATH}/hlenhanced" )

MESSAGE( STATUS "Base Game Path: ${GAME_BASE_PATH}" )

set( EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external )

set( USE_ANGELSCRIPT "0" CACHE BOOL "Whether to include Angelscript" )
set( USE_AS_SQL "0" CACHE BOOL "Whether to include Angelscript SQL APIs" )
set( USE_OPFOR "0" CACHE BOOL "Whether to include Opposing Force related stuff" )

#Some libraries that we use don't come with .a files (import libraries) for Cygwin compilation (Unix Makefiles on Windows).
#This isn't really supported, and since we only use Makefiles on Windows for the compile_commands.json file right now this isn't really an issue.
#Should we choose to support Cygwin we will need the import libraries though.
set( CLANG_DB_BUILD OFF CACHE BOOL "Whether this is a Clang database build. If enabled, some settings are disabled in order to function on Windows" )

if( NOT USE_ANGELSCRIPT AND USE_AS_SQL )
	MESSAGE( STATUS "Forcing USE_AS_SQL to off due to no Angelscript usage" )
	set( USE_AS_SQL 0 )
endif()

if( USE_ANGELSCRIPT )
	set( USE_ANGELSCRIPT_DEFINE 1 )
else()
	set( USE_ANGELSCRIPT_DEFINE 0 )
endif()

if( USE_AS_SQL )
	set( USE_AS_SQL_DEFINE 1 )
else()
	set( USE_AS_SQL_DEFINE 0 )
endif()

if( USE_OPFOR )
	set( USE_OPFOR_DEFINE 1 )
else()
	set( USE_OPFOR_DEFINE 0 )
endif()

if( CLANG_DB_BUILD )
	MESSAGE( STATUS "Exporting Compile Commands" )
	set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
endif()

#Shared preprocessor definitions
set( SHARED_DEFS 
)

if( MSVC )
	set( SHARED_DEFS
		${SHARED_DEFS}
		_CRT_SECURE_NO_WARNINGS
		_SCL_SECURE_NO_WARNINGS
	)
	
	set( LINUX_32BIT_FLAG "" )
else()
	#All non-MSVC targets need this. That includes Cygwin and Mingw.
	set( SHARED_DEFS
		${SHARED_DEFS}
		POSIX
	)

	if( APPLE )
		#Define OSX so code can check for it.
		set( SHARED_DEFS
			${SHARED_DEFS}
			OSX
		)
	endif()

	if( UNIX )
		#Define LINUX so code can check for it.
		#Define _LINUX for Source SDK dependencies.
		set( SHARED_DEFS
			${SHARED_DEFS}
			LINUX
			_LINUX
		)
	endif()

	set( LINUX_32BIT_FLAG "-m32" )
endif()

#Shared definitions for the client & server libaries.
set( SHARED_GAME_DEFS
	CLIENT_WEAPONS
	USE_ANGELSCRIPT=${USE_ANGELSCRIPT_DEFINE}
	USE_OPFOR=${USE_OPFOR_DEFINE}
)

#Shared linker flags
set( SHARED_GAME_LINKER_FLAGS
)

if( MSVC )
	#Set Windows subsystem
	set( SHARED_GAME_LINKER_FLAGS
		${SHARED_GAME_LINKER_FLAGS} "/SUBSYSTEM:WINDOWS "
)
elseif( UNIX )
	#From the Github 2013 Makefile, match the settings for Unix environments.
	# force 387 for FP math so the precision between win32 and linux and osx match
	# Trigger an error if any code tries to use an implicit return type
	# Default visibility is hidden unless explicitly altered with __attribute__( visibility() )
	set( SHARED_GAME_LINKER_FLAGS
		${SHARED_GAME_LINKER_FLAGS} "-mfpmath=387 -Werror=return-type -fvisibility=hidden "
	)

	if( APPLE )
		set( SHARED_GAME_LINKER_FLAGS
			${SHARED_GAME_LINKER_FLAGS} "-momit-leaf-frame-pointer -mtune=core2 "
		)
	else()
		#Linux, Cygwin, etc.
	endif()
endif()

#Shared directories

set( SHARED_EXTERNAL_INCLUDE_PATHS
)

if( USE_ANGELSCRIPT )
	set( SHARED_EXTERNAL_INCLUDE_PATHS
		${SHARED_EXTERNAL_INCLUDE_PATHS}
		${EXTERNAL_DIR}/Angelscript/include
	)
endif()

set( SHARED_INCLUDE_PATHS
	common
	public
	public/math
	engine
	shared/engine
	game_shared
	game/shared
	game/shared/entities/player
	pm_shared
	game/server
	${CMAKE_SOURCE_DIR}
)

#keyvalues and utility are temporary until the code between these repositories can be shared. - Solokiller
add_subdirectory( common )
add_subdirectory( engine )
add_subdirectory( game/shared )
add_subdirectory( keyvalues )
add_subdirectory( pm_shared )
add_subdirectory( public )
add_subdirectory( shared/engine )
add_subdirectory( utility )

#Save the shared sources into SHARED_SRCS
get_property(SHARED_SRCS GLOBAL PROPERTY SRCS_LIST)

clear_sources()

#Find shared dependencies
if( USE_ANGELSCRIPT )
	find_library( ANGELSCRIPTLIB Angelscript PATHS external/Angelscript/lib/ )
	find_library( ANGELSCRIPTUTILS AngelscriptUtils PATHS external/Angelscript/lib/ )
	find_library( ANGELSCRIPTLIBD Angelscriptd PATHS external/Angelscript/lib/ )
	find_library( ANGELSCRIPTUTILSD AngelscriptUtilsd PATHS external/Angelscript/lib/ )
else()
	set( ANGELSCRIPTLIB )
	set( ANGELSCRIPTUTILS )
	set( ANGELSCRIPTLIBD )
	set( ANGELSCRIPTUTILSD )
endif()

set( SHARED_DEPENDENCIES
)

set( THREADS_PREFER_PTHREAD_FLAG ON )
find_package( Threads REQUIRED )

if( UNIX AND NOT APPLE )
	set( UNIX_FS_LIB stdc++fs )
else()
	set( UNIX_FS_LIB )
endif()

#
#Client library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add in the client directories
add_subdirectory( game/client )
add_subdirectory( vgui_utils )
add_subdirectory( shared/engine/client )

preprocess_sources()

add_library( client SHARED ${PREP_SRCS} )

target_include_directories( client PRIVATE
	shared/engine/client
	utils/vgui/include
	vgui_utils
	game/client
	game/client/ui/hud
	game/client/ui/vgui
	${SHARED_INCLUDE_PATHS}
	source_sdk/public/tier1
	source_sdk/public
	source_sdk/common
	external/SDL2/include
	${SHARED_EXTERNAL_INCLUDE_PATHS}
)

#Default to false
#TODO: change to cached var
if( NOT DEFINED USE_VGUI2 )
	set( USE_VGUI2 0 )
endif()

target_compile_definitions( client PRIVATE
	${SHARED_DEFS}
	${SHARED_GAME_DEFS}
	CLIENT_DLL
	USE_VGUI2=${USE_VGUI2}
)

#Find client dependencies
if( UNIX )
	if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
		set( VGUI1_NAME "vgui.dylib" )
	else()
		set( VGUI1_NAME "vgui.so" )
	endif()
else()
	set( VGUI1_NAME "vgui.lib" )
endif()

#VGUI1 has issues linking with the client due to its non-standard filename. To work around this, we can instead import it and manually specify the location.
#That sets the correct linker path and filename.
add_library( vgui SHARED IMPORTED )
set_property( TARGET vgui PROPERTY IMPORTED_LOCATION ${GAME_BIN_PATH}/${VGUI1_NAME} )
set_property( TARGET vgui PROPERTY IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/utils/vgui/lib/win32_vc6/vgui.lib )

if( NOT CLANG_DB_BUILD )
	if( UNIX )
		if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
			set( SDL2_NAME "libSDL2-2.0.0.dylib" )
		else()
			set( SDL2_NAME "libSDL2-2.0.so.0" )
		endif()
	else()
		set( SDL2_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${CMAKE_STATIC_LIBRARY_SUFFIX}" )
	endif()

	find_library( SDL2 ${SDL2_NAME} PATHS lib/public/ external/SDL2/lib "${STEAMCOMMON}/Half-Life/" )
else()
	set( SDL2 )
endif()

if( "${USE_VGUI2}" STREQUAL "1" )
	find_library( VGUI2 vgui_controls PATHS source_sdk/lib/public/ )
	find_library( TIER0 ${CMAKE_SHARED_LIBRARY_PREFIX}tier0${CMAKE_STATIC_LIBRARY_SUFFIX} PATHS lib/public/ )
	find_library( TIER1 tier1 PATHS source_sdk/lib/public/ )

	if( WIN32 )
		find_library( VSTDLIB vstdlib PATHS lib/public/ )
	else()
		set( VSTDLIB "" )
	endif()
	
	set( VGUI2_LIBS
		VGUI2
		TIER0
		TIER1
		VSTDLIB
	)
else()
	set( VGUI2_LIBS "" )
endif()

#Set the list of platform specific libraries
if( WIN32 )
	set( CLIENT_PLAT_LIBS "wsock32" )
else()
	set( CLIENT_PLAT_LIBS "dl" )
endif()

#Link with client dependencies
target_link_libraries( client 
	"${SHARED_DEPENDENCIES}"
	optimized "${ANGELSCRIPTUTILS}"
	optimized "${ANGELSCRIPTLIB}"
	debug "${ANGELSCRIPTUTILSD}"
	debug "${ANGELSCRIPTLIBD}"
	vgui
	"${CLIENT_PLAT_LIBS}"
	"${SDL2}"
	"${VGUI2_LIBS}"
	Threads::Threads
	${UNIX_FS_LIB}
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( client PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${GAME_BASE_PATH}/cl_dlls"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${GAME_BASE_PATH}/cl_dlls"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GAME_BASE_PATH}/cl_dlls"
)

set( CLIENT_LINK_FLAGS )

if( NOT MSVC AND NOT APPLE )
	#Generate a map file that provides information about the linking stage.
	set( CLIENT_LINK_FLAGS
		${CLIENT_LINK_FLAGS} "-Wl,-Map,client_map.txt "
	)
endif()

set_target_properties( client PROPERTIES
	COMPILE_FLAGS "${LINUX_32BIT_FLAG}"
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${CLIENT_LINK_FLAGS} ${LINUX_32BIT_FLAG}"
)

#No lib prefix
SET_TARGET_PROPERTIES( client PROPERTIES PREFIX "" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

#Set the startup project to the client
#TODO: does this even work?
set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT client )

clear_sources()

#
#End client library
#

#
#Server library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add in the server directories
add_subdirectory( game/server )

preprocess_sources()

add_library( hl SHARED ${PREP_SRCS} )

target_include_directories( hl PRIVATE
	${SHARED_INCLUDE_PATHS}
	${SHARED_EXTERNAL_INCLUDE_PATHS}
)

if( USE_AS_SQL )
	target_include_directories( hl PRIVATE
		${EXTERNAL_DIR}/CTPL/include
		${EXTERNAL_DIR}/SQLite/include
		${EXTERNAL_DIR}/MariaDB/include
		${EXTERNAL_DIR}/ASSQL/include
	)
endif()

target_compile_definitions( hl PRIVATE
	${SHARED_DEFS}
	${SHARED_GAME_DEFS}
	SERVER_DLL
	USE_AS_SQL=${USE_AS_SQL_DEFINE}
)

#Copy the libraries first so they can be found by the code below.
set( LIBS_TO_COPY )

if( USE_AS_SQL AND NOT CLANG_DB_BUILD )
	if( WIN32 )
		#Copy the sqlite3 dll so the exe can get to it
		set( LIBS_TO_COPY ${LIBS_TO_COPY} "${EXTERNAL_DIR}/SQLite/lib/sqlite3.dll" )
	else()
		#Copy the sqlite3 library so the exe can get to it
		#Copy the mariadb library so the exe can get to it		
		set( LIBS_TO_COPY ${LIBS_TO_COPY} "${EXTERNAL_DIR}/SQLite/lib/libsqlite3.so" "${EXTERNAL_DIR}/MariaDB/lib/libmariadb.so.2" )
	endif()
endif()

foreach( lib IN LISTS LIBS_TO_COPY )
	MESSAGE( STATUS "Copying library \"${lib}\"" )
	file( COPY ${lib} DESTINATION ${GAME_BASE_PATH} )
endforeach()

#Find server dependencies
if( USE_AS_SQL AND NOT CLANG_DB_BUILD )
	find_library( ASSQL AngelscriptSQL PATHS external/ASSQL/lib/ )
	find_library( ASSQLD AngelscriptSQLd PATHS external/ASSQL/lib/ )

	if( WIN32 )
		find_library( SQLITE3 "sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX}" PATHS external/SQLite/lib )
		find_library( MARIADB "mariadbclient${CMAKE_STATIC_LIBRARY_SUFFIX}" PATHS external/MariaDB/lib )
		find_library( MARIADBD "mariadbclientd${CMAKE_STATIC_LIBRARY_SUFFIX}" PATHS external/MariaDB/lib )
		set( PTHREAD "" )
	else()
		#Never use the default install for these (could try to use 64 bit versions, which don't work for 32 bit builds).
		find_library( SQLITE3 "${CMAKE_SHARED_LIBRARY_PREFIX}sqlite3${CMAKE_SHARED_LIBRARY_SUFFIX}" PATHS "${GAME_BASE_PATH}" NO_DEFAULT_PATH )
		find_library( MARIADB "${CMAKE_SHARED_LIBRARY_PREFIX}mariadb.so.2" PATHS "${GAME_BASE_PATH}" NO_DEFAULT_PATH )
		set( MARIADBD ${MARIADB} )
		set( PTHREAD "pthread" )
	endif()
else()
	set( ASSQL )
	set( ASSQLD )
	set( SQLITE3 )
	set( MARIADB )
	set( MARIADBD )
	set( PTHREAD )
endif()

#Set the list of platform specific libraries
if( WIN32 )
	set( SERVER_PLAT_LIBS "" )
else()
	set( SERVER_PLAT_LIBS "dl" )
endif()

#Link with server dependencies
target_link_libraries( hl 
	"${SHARED_DEPENDENCIES}"
	optimized "${ASSQL}"
	optimized "${ANGELSCRIPTUTILS}"
	optimized "${ANGELSCRIPTLIB}"
	debug "${ASSQLD}"
	debug "${ANGELSCRIPTUTILSD}"
	debug "${ANGELSCRIPTLIBD}"
	"${SQLITE3}"
	optimized "${MARIADB}"
	debug "${MARIADBD}"
	"${DELAYIMP}"
	Threads::Threads
	${SERVER_PLAT_LIBS}
	${UNIX_FS_LIB}
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( hl PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${GAME_BASE_PATH}/dlls"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${GAME_BASE_PATH}/dlls"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GAME_BASE_PATH}/dlls"
)

set( SERVER_LINK_FLAGS )

if( MSVC )
	if( USE_AS_SQL )
		set( SQL_DLL_DELAYLOAD "/DELAYLOAD:sqlite3.dll" )
	else()
		set( SQL_DLL_DELAYLOAD )
	endif()

	#Add the module definition file. Needed for GiveFnptrsToDll function export. Requires the project dir to be near the file.
	#Add delay loaded libraries.
	set( SERVER_LINK_FLAGS "/DEF:../hl.def ${SQL_DLL_DELAYLOAD} " )
else()
	if( NOT APPLE )
		#Generate a map file that provides information about the linking stage.
		set( SERVER_LINK_FLAGS
			${SERVER_LINK_FLAGS} "-Wl,-Map,hl_map.txt "
		)
	endif()
endif()

#SQL libraries are delay loaded to keep them in the game directory.
set_target_properties( hl PROPERTIES
	COMPILE_FLAGS "${LINUX_32BIT_FLAG}" 
	LINK_FLAGS "${SHARED_GAME_LINKER_FLAGS} ${SERVER_LINK_FLAGS} ${LINUX_32BIT_FLAG}"
)

#No lib prefix
SET_TARGET_PROPERTIES( hl PROPERTIES PREFIX "" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()

#
#End server library
#

add_subdirectory( utils )
