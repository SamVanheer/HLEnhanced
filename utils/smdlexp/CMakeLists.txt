#
#	3DS Max 4.2 SMD exporter
#

set( UTIL_NAME smdlexp )

add_sources(
	smdlexp.cpp
	smedefs.h
	smexprc.h
)

if( MSVC )
	add_sources(
		smdlexp.rc
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
