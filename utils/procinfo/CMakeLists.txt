#
#	Provides utility code to retrieve processor info
#

set( UTIL_NAME procinfo )

add_sources(
	procinfo.cpp
	procinfo.h
)

preprocess_sources()

add_library( ${UTIL_NAME} STATIC ${PREP_SRCS} )

create_source_groups( "${ROOT_FILTER_DIR}" )

clear_sources()

target_include_directories( ${UTIL_NAME} PRIVATE
	${SHARED_UTIL_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions( ${UTIL_NAME} PRIVATE
	${SHARED_DEFS}
)
