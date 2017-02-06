#
#	Generates the BSP nodes for a map
#

set( UTIL_NAME qbsp2 )

add_sources(
	${SHARED_UTIL_SRCS}
	${BSPLIB_SRCS}
	${THREADS_SRCS}
	bsp5.h
	cull.cpp
	merge.cpp
	outside.cpp
	portals.cpp
	qbsp.cpp
	solidbsp.cpp
	surfaces.cpp
	tjunc.cpp
	writebsp.cpp
)

set( ENABLE_3D_DRAWING ON CACHE BOOL "Whether to enable 3D drawing of the map" )

if( ENABLE_3D_DRAWING )
	add_sources(
		gldraw.cpp
	)
else()
	add_sources(
		nodraw.cpp
	)
endif()

preprocess_sources()

add_executable( ${UTIL_NAME} ${PREP_SRCS} )

create_source_groups( "${ROOT_FILTER_DIR}" )

clear_sources()

target_include_directories( ${UTIL_NAME} PRIVATE
	${SHARED_UTIL_INCLUDES}
	${CMAKE_CURRENT_SOURCE_DIR}
	${EXTERNAL_DIR}/SDL2/include
	${OPENGL_INCLUDE_DIR}
)

target_compile_definitions( ${UTIL_NAME} PRIVATE
	${SHARED_DEFS}
)

if( ENABLE_3D_DRAWING )
	target_link_libraries( ${UTIL_NAME} PRIVATE
		${SDL2}
		${OPENGL_gl_LIBRARY}
		${OPENGL_glu_LIBRARY}
	)
	
	if( MSVC )
		foreach( CONFIG in LISTS ${CMAKE_CONFIGURATION_TYPES} )
			file( COPY ${SDL2_LIBNAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG} )
		endforeach()
	else()
		file( COPY ${SDL2_LIBNAME} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
	endif()
endif()
