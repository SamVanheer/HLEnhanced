#
#	Model Viewer
#

set( UTIL_NAME mdlviewer )

add_sources(
	${SHARED_UTIL_SRCS}
	${BSPLIB_SRCS}
	mdlviewer.cpp
	mdlviewer.h
	studio_render.cpp
	studio_utils.cpp
)

preprocess_sources()

add_executable( ${UTIL_NAME} ${PREP_SRCS} )

create_source_groups( "${ROOT_FILTER_DIR}" )

clear_sources()

target_include_directories( ${UTIL_NAME} PRIVATE
	${SHARED_UTIL_INCLUDES}
	${CMAKE_SOURCE_DIR}/public/math
	${CMAKE_CURRENT_SOURCE_DIR}
	${EXTERNAL_DIR}/SDL2/include
	${OPENGL_INCLUDE_DIR}
)

target_compile_definitions( ${UTIL_NAME} PRIVATE
	${SHARED_DEFS}
)

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
