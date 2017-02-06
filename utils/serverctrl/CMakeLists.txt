#
#	Server Control GUI
#

set( UTIL_NAME serverctrl )

#TODO: need to implement precompiled headers - Solokiller
add_sources(
	${SHARED_UTIL_SRCS}
	${BSPLIB_SRCS}
	resource.h
	ServerCtrl.cpp
	ServerCtrl.h
	ServerCtrlDlg.cpp
	ServerCtrlDlg.h
	StdAfx.cpp
	StdAfx.h
)

if( MSVC )
	add_sources(
		ServerCtrl.rc
	)
endif()

preprocess_sources()

add_executable( ${UTIL_NAME} ${PREP_SRCS} )

create_source_groups( "${ROOT_FILTER_DIR}" )

clear_sources()

target_include_directories( ${UTIL_NAME} PRIVATE
	${SHARED_UTIL_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions( ${UTIL_NAME} PRIVATE
	${SHARED_DEFS}
)

if( MSVC )
	#We need to correct the dependency order because otherwise operator delete gets defined multiple times:
	#See http://stackoverflow.com/questions/1146338/error-lnk2005-new-and-delete-already-defined-in-libcmtd-libnew-obj
	set( CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"Nafxcwd,Nafxcw,Libcmtd,Libcmt\" )

	target_link_libraries( ${UTIL_NAME} PRIVATE
		debug Nafxcwd.lib
		debug Libcmtd.lib
		optimized Nafxcw.lib
		optimized Libcmt.lib
	)

	#Set to Windows so it uses WinMain, required by MFC.
	set_target_properties( ${UTIL_NAME} PROPERTIES
		LINK_FLAGS "/SUBSYSTEM:WINDOWS"
	)
endif()