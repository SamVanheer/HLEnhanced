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
