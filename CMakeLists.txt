###################################################
#                                                 #
#                                                 #
#	Half-Life Enhanced CMake build file           #
#                                                 #
#                                                 #
###################################################
cmake_minimum_required( VERSION 3.6 )

# function to collect all the sources from sub-directories
# into a single list
function(add_sources)
  get_property(is_defined GLOBAL PROPERTY SRCS_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY SRCS_LIST
      BRIEF_DOCS "List of source files"
      FULL_DOCS "List of source files to be compiled in one library")
  endif()
  # make absolute paths
  set(SRCS)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND SRCS "${s}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY SRCS_LIST "${SRCS}")
endfunction(add_sources)

# preprocess sources
function( preprocess_sources )
	set(PREP_SRCS PARENT_SCOPE)
	get_property(SRCS GLOBAL PROPERTY SRCS_LIST)
	foreach(s IN LISTS SRCS)
	  file(RELATIVE_PATH rs "${CMAKE_CURRENT_SOURCE_DIR}" "${s}")
	  string(REGEX REPLACE "r$" "" o "${CMAKE_CURRENT_BINARY_DIR}/${rs}")
	  add_custom_command(
		OUTPUT "${o}"
		COMMAND ${CMAKE_COMMAND} -E copy "${s}" "${o}"
		DEPENDS "${s}"
		COMMENT "Creating ${o}"
		VERBATIM
		)
	  list(APPEND PREP_SRCS "${s}")
	endforeach()
	set(PREP_SRCS ${PREP_SRCS} PARENT_SCOPE)
endfunction( preprocess_sources )

function( create_source_groups _src_root_path )
	get_property(SRCS GLOBAL PROPERTY SRCS_LIST)
	foreach(_source IN ITEMS ${SRCS})
		get_filename_component(_source_path "${_source}" PATH)
		file(RELATIVE_PATH _source_path_rel "${_src_root_path}" "${_source_path}")
		string(REPLACE "/" "\\" _group_path "${_source_path_rel}")
		source_group("${_group_path}" FILES "${_source}")
	endforeach()
endfunction( create_source_groups )

#Function to clear the sources list.
function( clear_sources )
	set_property( GLOBAL PROPERTY SRCS_LIST "" )
	set( PREP_SRCS PARENT_SCOPE )
endfunction( clear_sources )

# Function to add include files.
function( add_includes )
  get_property(is_defined GLOBAL PROPERTY INCLUDES_LIST DEFINED)
  if(NOT is_defined)
    define_property(GLOBAL PROPERTY INCLUDES_LIST
      BRIEF_DOCS "List of include files"
      FULL_DOCS "List of include files to be compiled in one library")
  endif()
  # make absolute paths
  set(INCLUDES)
  foreach(s IN LISTS ARGN)
    if(NOT IS_ABSOLUTE "${s}")
      get_filename_component(s "${s}" ABSOLUTE)
    endif()
    list(APPEND INCLUDES "${s}")
  endforeach()
  # append to global list
  set_property(GLOBAL APPEND PROPERTY INCLUDES_LIST "${INCLUDES}")
endfunction( add_includes )

# Function to install includes.
function( install_includes _include_root_path )
	get_property(INCLUDES GLOBAL PROPERTY INCLUDES_LIST)
	
	foreach(_include IN ITEMS ${INCLUDES})
		get_filename_component(_include_path "${_include}" PATH)
		file(RELATIVE_PATH _include_path_rel "${_include_root_path}" "${_include_path}")
		string(REPLACE "/" "\\" _group_path "${_include_path_rel}")
		install( FILES "${_include}" DESTINATION "include/${_group_path}" )
	endforeach()
	
	set_property( GLOBAL PROPERTY INCLUDES_LIST "" )
endfunction( install_includes )

macro(configure_msvc_runtime)
  if(MSVC)
    # Default to statically-linked runtime.
    if("${MSVC_RUNTIME}" STREQUAL "")
      set(MSVC_RUNTIME "static")
    endif()
    # Set compiler options.
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
    if(${MSVC_RUNTIME} STREQUAL "static")
      message(STATUS
        "MSVC -> forcing use of statically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS
        "MSVC -> forcing use of dynamically-linked runtime."
      )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    endif()
  endif()
endmacro()

project( HLEnhanced )

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

#Get the steam/steamapps/common directory
set( STEAMCOMMON "$ENV{SteamCommon}" CACHE PATH "Steam common directory path" )

if( "${STEAMCOMMON}" STREQUAL "" )

	MESSAGE( FATAL_ERROR "Unable to get Steam common directory path" )

endif()

#Base dir for game outputs
set( GAME_BASE_PATH "${STEAMCOMMON}/Half-Life/hlenhanced" )

MESSAGE( STATUS "Base Game Path: ${GAME_BASE_PATH}" )

set( USE_ANGELSCRIPT "0" CACHE BOOL "Whether to include Angelscript" )
set( USE_AS_SQL "0" CACHE BOOL "Whether to include Angelscript SQL APIs" )
set( USE_OPFOR "0" CACHE BOOL "Whether to include Opposing Force related stuff" )

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

#Shared preprocessor definitions
set( SHARED_DEFS 
	_CRT_SECURE_NO_WARNINGS
)

#Define LINUX so code can check for it
if( UNIX )
	set( SHARED_DEFS
		"${SHARED_DEFS}"
		LINUX
	)
	
	set( LINUX_32BIT_FLAG "-m32" )
else()
	set( LINUX_32BIT_FLAG "" )
endif()

#C++14 support
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y" )

#Shared directories

add_subdirectory( common )
add_subdirectory( game/shared )
add_subdirectory( pm_shared )
add_subdirectory( public )

#Save the shares sources into SHARED_SRCS
get_property(SHARED_SRCS GLOBAL PROPERTY SRCS_LIST)

clear_sources()

#Find shared dependencies
if( USE_ANGELSCRIPT )
	find_library( ANGELSCRIPTLIB Angelscript PATHS external/Angelscript/lib/ )
	find_library( ANGELSCRIPTUTILS AngelscriptUtils PATHS external/Angelscript/lib/ )
	
	if( NOT WIN32 )

		#This mucking about is needed because CMake can't figure out how to handle libraries not named lib<name>.a
		set( ANGELSCRIPTLIB "-l:\"${ANGELSCRIPTLIB}\"" )
		set( ANGELSCRIPTUTILS "-l:\"${ANGELSCRIPTUTILS}\"" )

	endif()
else()
	set( ANGELSCRIPTLIB )
	set( ANGELSCRIPTUTILS )
endif()

set( SHARED_DEPENDENCIES
	"${ANGELSCRIPTUTILS}"
	"${ANGELSCRIPTLIB}"
)

#
#Client library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add in the client directories
add_subdirectory( game/client )
add_subdirectory( vgui_utils )
add_subdirectory( shared/game/client )
add_subdirectory( shared/engine/client )

preprocess_sources()

#Cache the old include directories list
#TODO: this doesn't work. The include dirs as they are at the end of processing are used for all exes/libs defined in this file
get_property( INCLUDE_DIRS DIRECTORY PROPERTY INCLUDE_DIRECTORIES )

include_directories(
	common
	public
	public/math
	engine
	shared/engine/client
	game_shared
	game/shared
	game/shared/entities/player
	utils/vgui/include
	vgui_utils
	pm_shared
	game/client
	game/client/ui/hud
	game/client/ui/vgui
	game/server
	shared/game/client
	external
	source_sdk/public/tier1
	source_sdk/public
	source_sdk/common
)

add_library( client SHARED ${PREP_SRCS} )

#Default to false
#TODO: change to cached var
if( NOT DEFINED USE_VGUI2 )
	set( USE_VGUI2 0 )
endif()

target_compile_definitions( client PRIVATE
	"${SHARED_DEFS}"
	CLIENT_DLL
	CLIENT_WEAPONS
	HL_DLL
	"USE_VGUI2=${USE_VGUI2}"
	USE_ANGELSCRIPT=${USE_ANGELSCRIPT_DEFINE}
	USE_OPFOR=${USE_OPFOR_DEFINE}
)

#Find client dependencies
if( UNIX )
	set( VGUI1_NAME "vgui.so" )
else()
	set( VGUI1_NAME "vgui.lib" )
endif()

find_library( VGUI1 ${VGUI1_NAME} PATHS utils/vgui/lib/win32_vc6/ "${STEAMCOMMON}/Half-Life/" )

if( UNIX )
	set( SDL2_NAME "libSDL2-2.0.so.0" )
else()
	set( SDL2_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}SDL2${CMAKE_STATIC_LIBRARY_SUFFIX}" )
endif()

find_library( SDL2 ${SDL2_NAME} PATHS lib/public/ "${STEAMCOMMON}/Half-Life/" )

if( NOT WIN32 )
	#This mucking about is needed because CMake can't figure out how to handle libraries not named lib<name>.so
	set( VGUI1 "-l:\"${VGUI1}\"" )

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
	set( CLIENT_PLAT_LIBS "" )
endif()

#Link with client dependencies
target_link_libraries( client 
	"${SHARED_DEPENDENCIES}"
	"${VGUI1}"
	"${CLIENT_PLAT_LIBS}"
	"${SDL2}"
	"${VGUI2_LIBS}"
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( client PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${GAME_BASE_PATH}/cl_dlls"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${GAME_BASE_PATH}/cl_dlls"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GAME_BASE_PATH}/cl_dlls"
)

if( WIN32 AND MSVC )
	#Set Windows subsystem
	set( CLIENT_LINK_FLAGS "/SUBSYSTEM:WINDOWS" )
else()
	set( CLIENT_LINK_FLAGS "" )
endif()

set_target_properties( client PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" LINK_FLAGS "${CLIENT_LINK_FLAGS} ${LINUX_32BIT_FLAG}" )

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

#Reset include dirs
set_property( DIRECTORY PROPERTY INCLUDE_DIRECTORIES "${INCLUDE_DIRS}" )

#
#Server library
#

#Add in the shared sources
add_sources( "${SHARED_SRCS}" )

#Add in the server directories
add_subdirectory( game/server )
add_subdirectory( shared/game/server )
add_subdirectory( shared/engine/server )

preprocess_sources()

if( USE_AS_SQL )
	include_directories(
		"external/CTPL/include"
		"external/SQLite/include"
		"external/MariaDB/include"
		"external/ASSQL/include"
	)
endif()

if( USE_ANGELSCRIPT )
include_directories(
		"external/Angelscript/include"
	)
endif()

include_directories(
	common
	public
	public/math
	engine
	game_shared
	game/shared
	game/shared/entities/player
	shared/game/server
	shared/engine/server
	pm_shared
	game/server
	external
)

add_library( hl SHARED ${PREP_SRCS} )

#So many of these... Do we need all of them?
target_compile_definitions( hl PRIVATE
	"${SHARED_DEFS}"
	SERVER_DLL
	QUIVER
	VOXEL
	QUAKE2
	VALVE_DLL
	CLIENT_WEAPONS
	USE_ANGELSCRIPT=${USE_ANGELSCRIPT_DEFINE}
	USE_AS_SQL=${USE_AS_SQL_DEFINE}
	USE_OPFOR=${USE_OPFOR_DEFINE}
)

#Find server dependencies
if( USE_AS_SQL )
	find_library( ASSQL AngelscriptSQL PATHS external/ASSQL/lib/ )

	if( WIN32 )
		find_library( SQLITE3 "sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX}" PATHS external/SQLite/lib )
		find_library( MARIADB "mariadbclient${CMAKE_STATIC_LIBRARY_SUFFIX}" PATHS external/MariaDB/lib )
		set( PTHREAD "" )
	else()
		find_library( SQLITE3 "${CMAKE_SHARED_LIBRARY_PREFIX}sqlite3${CMAKE_SHARED_LIBRARY_SUFFIX}" PATHS "${GAME_BASE_PATH}" )
		find_library( MARIADB "${CMAKE_SHARED_LIBRARY_PREFIX}mariadb.so.2" PATHS "${GAME_BASE_PATH}" )
		set( PTHREAD "pthread" )
		
		#This mucking about is needed because CMake can't figure out how to handle libraries not named lib<name>.a
		set( ASSQL "-l:\"${ASSQL}\"" )
	endif()
else()
	set( ASSQL )
	set( SQLITE3 )
	set( MARIABDB )
	set( PTHREAD )
endif()

#Link with server dependencies
target_link_libraries( hl 
	"${SQLITE3}"
	"${MARIADB}"
	"${ASSQL}"
	"${SHARED_DEPENDENCIES}"
	"${DELAYIMP}"
)

#CMake places libraries in /Debug or /Release on Windows, so explicitly set the paths for both.
#On Linux, it uses LIBRARY_OUTPUT_DIRECTORY
set_target_properties( hl PROPERTIES
	LIBRARY_OUTPUT_DIRECTORY "${GAME_BASE_PATH}/dlls"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${GAME_BASE_PATH}/dlls"
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${GAME_BASE_PATH}/dlls"
)

if( WIN32 AND MSVC )
	if( USE_AS_SQL )
		set( SQL_DLL_DELAYLOAD "/DELAYLOAD:sqlite3.dll /DELAYLOAD:libmariadb.dll" )
	else()
		set( SQL_DLL_DELAYLOAD )
	endif()

	#Set Windows subsystem
	#Add the module definition file. Needed for GiveFnptrsToDll function export. Requires the project dir to be near the file.
	#Add delay loaded libraries.
	set( SERVER_LINK_FLAGS "/SUBSYSTEM:WINDOWS /DEF:../hl.def ${SQL_DLL_DELAYLOAD}" )
else()
	set( SERVER_LINK_FLAGS "" )
endif()

#SQL libraries are delay loaded to keep them in the game directory.
set_target_properties( hl 
	PROPERTIES COMPILE_FLAGS "${LINUX_32BIT_FLAG}" 
	LINK_FLAGS "${SERVER_LINK_FLAGS} ${LINUX_32BIT_FLAG}" )

#No lib prefix
SET_TARGET_PROPERTIES( hl PROPERTIES PREFIX "" )

#Create filters
create_source_groups( "${CMAKE_SOURCE_DIR}" )

clear_sources()

if( USE_AS_SQL )
	if( WIN32 )
		#Copy the sqlite3 dll so the exe can get to it
		add_custom_command( TARGET hl POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/external/SQLite/lib/sqlite3.dll"
			"$<TARGET_FILE_DIR:hl>/.."
		)

		#Copy the mariadb dll so the exe can get to it
		add_custom_command( TARGET hl POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/external/MariaDB/lib/libmariadb.dll"
			"$<TARGET_FILE_DIR:hl>/.."
		)
		else()
		#Copy the sqlite3 library so the exe can get to it
		add_custom_command( TARGET hl PRE_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/external/SQLite/lib/libsqlite3.so"
			"$<TARGET_FILE_DIR:hl>/.."
		)

		#Copy the mariadb library so the exe can get to it
		add_custom_command( TARGET hl PRE_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_CURRENT_SOURCE_DIR}/external/MariaDB/lib/libmariadb.so.2"
			"$<TARGET_FILE_DIR:hl>/.."
		)
	endif()
endif()

#
#End server library
#

#TODO: add utility exes here

#project( HLEnhanced_Utils )