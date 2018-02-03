#
# Finds vstdlib (GoldSource specific)
#

# Prefer game install directory (for shared libs)
find_library( VSTDLIB_LIB vstdlib PATHS ${tier0_DIR} ${CMAKE_SOURCE_DIR}/lib/public NO_DEFAULT_PATH )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( vstdlib DEFAULT_MSG VSTDLIB_LIB )

if( VSTDLIB_LIB )
	add_library( vstdlib SHARED IMPORTED )
	
	if( MSVC )
		set_property( TARGET vstdlib PROPERTY IMPORTED_IMPLIB ${VSTDLIB_LIB} )
	else()
		set_property( TARGET vstdlib PROPERTY IMPORTED_LOCATION ${VSTDLIB_LIB} )
	endif()
endif()

unset( VSTDLIB_LIB CACHE )
