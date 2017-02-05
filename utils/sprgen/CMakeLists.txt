#
#	Create sprite files
#

set( UTIL_NAME sprgen )

add_sources(
	${SHARED_UTIL_SRCS}
	${LBMLIB_SRCS}
	sprgen.c
	spritegn.h
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
