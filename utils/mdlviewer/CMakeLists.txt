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
	${CMAKE_SOURCE_DIR}/common
	${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions( ${UTIL_NAME} PRIVATE
	${SHARED_DEFS}
)

if( MSVC )
	target_link_libraries( ${UTIL_NAME} PRIVATE
		opengl32.lib
		glu32.lib
	)
endif()