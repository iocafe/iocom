# iocom/CmakeLists.txt - Cmake build for iodomain library.
cmake_minimum_required(VERSION 2.8.11)

# Set project name (= project root folder name).
set(E_PROJECT "iodomain")
project(${E_PROJECT})

# include build environment setting common to all projects.
include(../../../eosal/build/cmakedefs/eosal-defs.txt)

# Set path to where to keep libraries.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_LIB})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIB})

# Set path to source files.
set(BACKBONE "$ENV{E_ROOT}/iocom/applications/${E_PROJECT}/code")

# Add iocom to include path.
include_directories("$ENV{E_ROOT}/iocom")

# Add main headers.
set(MAIN_HEADERS "$ENV{E_ROOT}/iocom/${E_PROJECT}.h")

# Add source files common to all operating systems.
file(GLOB BB_CM_SOURCES "${BACKBONE}/*.c")
file(GLOB BB_CM_HEADERS "${BACKBONE}/*.h")

# Generate the static library.
add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_CM_HEADERS} ${BB_CM_SOURCES})
