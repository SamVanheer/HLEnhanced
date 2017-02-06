#
#	Generate lighting info for a BSP
#

set( UTIL_NAME light )

add_sources(
	${SHARED_UTIL_SRCS}
	${BSPLIB_SRCS}
	${THREADS_SRCS}
	light.cpp
	light.h
	ltface.cpp
	trace.cpp
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
