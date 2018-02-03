#
#	Toolchain file used for cross-compiling Linux 32 bit applications on Linux 64 bit
#

set( CMAKE_SYSTEM_NAME Linux )
set( CMAKE_SYSTEM_VERSION 1 )
set( CMAKE_SYSTEM_PROCESSOR "i386" )

set( CMAKE_FIND_ROOT_PATH  /usr/lib/i386-linux-gnu )
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH )

# Needed for FindThreads to work
set( THREADS_PTHREAD_ARG "2" CACHE STRING "Forcibly set by CMakeLists.txt." FORCE )
