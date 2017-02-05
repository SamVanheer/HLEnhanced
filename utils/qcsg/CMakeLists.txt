#
#	Construct the geometry for a map
#

set( UTIL_NAME qcsg )

add_sources(
	${SHARED_UTIL_SRCS}
	${BSPLIB_SRCS}
	${THREADS_SRCS}
	${POLYLIB_SRCS}
	brush.c
	csg.h
	gldraw.c
	hullfile.c
	map.c
	qcsg.c
	textures.c
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
	DOUBLEVEC_T
)

if( MSVC )
	target_link_libraries( ${UTIL_NAME} PRIVATE
		opengl32.lib
		glu32.lib
		glaux.lib
	)
endif()
