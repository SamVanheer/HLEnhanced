#
#	Show BSP Info to the user
#

set( UTIL_NAME bspinfo )

add_sources(
	${SHARED_UTIL_SRCS}
	${BSPLIB_SRCS}
	bspinfo.c
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
