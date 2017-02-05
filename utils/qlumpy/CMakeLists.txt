#
#	Generates data lumps for use with the engine
#

set( UTIL_NAME qlumpy )

add_sources(
	${SHARED_UTIL_SRCS}
	${WADLIB_SRCS}
	${LBMLIB_SRCS}
	qlumpy.c
	qlumpy.h
	quakegrb.c
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
