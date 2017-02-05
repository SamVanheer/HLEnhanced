#
#	Generates the BSP nodes for a map
#

set( UTIL_NAME qbsp2 )

add_sources(
	${SHARED_UTIL_SRCS}
	${BSPLIB_SRCS}
	${THREADS_SRCS}
	bsp5.h
	cull.c
	gldraw.c
	merge.c
	nodraw.c
	outside.c
	portals.c
	qbsp.c
	solidbsp.c
	surfaces.c
	tjunc.c
	writebsp.c
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
