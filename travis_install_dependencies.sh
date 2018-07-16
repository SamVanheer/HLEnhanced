#!/bin/bash
#
# Installs dependencies
# Arguments:
# $1: Dependencies directory path
# $2: Compiler settings
#

# Environment
readonly DEPENDENCIES_DIR_PATH=$1
readonly COMPILER_SETTINGS=$2
readonly ROOT_DIR=${PWD}
readonly CONFIGURATION=Release

# Dependency configuration
readonly XERCESC_VERSION=3.2.0

# Generates, builds and installs a CMake project
# Creates the directories build and install, generates the project files from the location $src_path
# 
# Expects the current directory to be where the build and install directories will be created
# 
# generate_args
# Arguments to append to the build generation step. An array of strings
# 
# src_path
# Location of the root CMakeLists.txt. Defaults to a directory "src" in the current directory
# 
CMake_GenerateBuildAndInstall()
{
	declare -r -a generate_args=("${!1}")
	readonly src_path=${2:-"src"}
	
	readonly currentDir=${PWD}
	readonly full_src_path="$currentDir/$src_path"
	
	# Directory name is suitable for project name
	readonly projectName=$( basename "${currentDir}" )
	
	mkdir build
	mkdir install
	
	cd build
	
	echo "[Dependency] [${projectName}] Generating"
	# Disable warnings, we don't need to know about those here
	cmake ${COMPILER_SETTINGS} -DCMAKE_INSTALL_PREFIX="$currentDir/install" -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -w -m32" -DCMAKE_C_FLAGS="${CMAKE_C_FLAGS} -w -m32" ${generate_args[@]} $full_src_path
	
	echo "[Dependency] [${projectName}] Building"
	cmake --build . --clean-first --config ${CONFIGURATION}
	
	echo "[Dependency] [${projectName}] Installing"
	cmake --build . --target install --config ${CONFIGURATION}
	
	echo "[Dependency] [${projectName}] Done"
	
	cd ..
}

echo "=== Downloading and installing dependencies ==="

readonly dependencies_dir_path=$( dirname  "$DEPENDENCIES_DIR_PATH" )
readonly dependencies_dir_name=$( basename "$DEPENDENCIES_DIR_PATH" )

cd ${dependencies_dir_path}

# Install only if it wasn't cached
if [ ! -d "$dependencies_dir_name" ]; then

	mkdir ${dependencies_dir_name}
	cd ${dependencies_dir_name}
	
	readonly dependencies_root_dir=${PWD}
	
	# Use a common directory structure:
	# dependencies/dependency_name/src
	# dependencies/dependency_name/build
	# dependencies/dependency_name/install

	# XercesC begin
	mkdir xercesc
	cd xercesc
	
	echo "[Dependency] Building XercesC"
	
	# Acquire files
	XERCESC_URL="https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-$XERCESC_VERSION.tar.gz"
	mkdir src
	wget --no-check-certificate --quiet -O - ${XERCESC_URL} | tar --strip-components=1 -xz -C src
  
	readonly xercesc_args=( "-DBUILD_SHARED_LIBS=OFF" "-Dtranscoder=iconv" )
	CMake_GenerateBuildAndInstall xercesc_args[@]
	
	cd install
	
	# We don't need these files, and it reduces cache size
	rm -rf bin
	rm -rf share
	
	cd ${dependencies_root_dir}
	# XercesC End
	
	# Restore to old path
	cd ${ROOT_DIR}
else
	echo "Using Cached dependencies"
fi
