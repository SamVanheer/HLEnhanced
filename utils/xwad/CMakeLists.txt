#
#	Make a wad file
#

set( UTIL_NAME xwad )

add_sources(
	${SHARED_UTIL_SRCS}
	${LBMLIB_SRCS}
	${WADLIB_SRCS}
	xwad.cpp
)

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
