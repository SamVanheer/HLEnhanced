#
#	Make Studiomodel files
#

set( UTIL_NAME studiomdl )

add_sources(
	${SHARED_UTIL_SRCS}
	${LBMLIB_SRCS}
	${TRILIB_SRCS}
	bmpread.cpp
	studiomdl.cpp
	studiomdl.h
	tristrip.cpp
	write.cpp
)

preprocess_sources()

add_executable( ${UTIL_NAME} ${PREP_SRCS} )

create_source_groups( "${ROOT_FILTER_DIR}" )

clear_sources()

target_include_directories( ${UTIL_NAME} PRIVATE
	${SHARED_UTIL_INCLUDES}
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_SOURCE_DIR}/public
	${CMAKE_SOURCE_DIR}/game/server
	${CMAKE_SOURCE_DIR}/public/math
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions( ${UTIL_NAME} PRIVATE
	${SHARED_DEFS}
)
