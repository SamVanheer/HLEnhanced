#
#	Make a font
#

set( UTIL_NAME makefont )

add_sources(
	${SHARED_UTIL_SRCS}
	${WADLIB_SRCS}
	makefont.cpp
)

preprocess_sources()

add_executable( ${UTIL_NAME} ${PREP_SRCS} )

create_source_groups( "${ROOT_FILTER_DIR}" )

clear_sources()

target_include_directories( ${UTIL_NAME} PRIVATE
	${SHARED_UTIL_INCLUDES}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions( ${UTIL_NAME} PRIVATE
	${SHARED_DEFS}
)

if( MSVC )
	target_link_libraries( ${UTIL_NAME} PRIVATE
		winmm.lib
	)
endif()
