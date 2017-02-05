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
	gldraw.cpp
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
	${EXTERNAL_DIR}/SDL2/include
)

target_compile_definitions( ${UTIL_NAME} PRIVATE
	${SHARED_DEFS}
	DOUBLEVEC_T
)

target_link_libraries( ${UTIL_NAME} PRIVATE
	${SDL2}
)

if( MSVC )
	target_link_libraries( ${UTIL_NAME} PRIVATE
		opengl32.lib
		glu32.lib
	)
endif()

if( MSVC )
	set( SDL2_LIBNAME ${EXTERNAL_DIR}/SDL2/lib/SDL2.dll )
else()
	set( SDL2_LIBNAME ${EXTERNAL_DIR}/SDL2/lib/libSDL2-2.0.0.${CMAKE_SHARED_LIBRARY_SUFFIX} )
endif()

if( MSVC )
	foreach( CONFIG in LISTS ${CMAKE_CONFIGURATION_TYPES} )
		file( COPY ${SDL2_LIBNAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG} )
	endforeach()
else()
	file( COPY ${SDL2_LIBNAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
endif()
