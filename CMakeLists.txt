# iocom/CmakeLists.txt - Cmake build for iocom library, 8.1.2020/pekka.
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_PROJECT "iocom")
project(${E_PROJECT})

# include build environment setting common to all projects.
include(../eosal/build/cmakedefs/eosal-defs.txt)

# Set path to where to keep libraries.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_LIBOUT})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIBOUT})

# Set path to source files.
set(BACKBONE "$ENV{E_ROOT}/${E_PROJECT}/code")
set(EXTENSIONS "$ENV{E_ROOT}/${E_PROJECT}/extensions")

# Add iocom to include path.
include_directories("$ENV{E_ROOT}/iocom")

# Add main headers.
set(MAIN_HEADERS "$ENV{E_ROOT}/${E_PROJECT}/${E_PROJECT}.h" "$ENV{E_ROOT}/${E_PROJECT}/${E_PROJECT}x.h")

# Add source files common to all operating systems.
file(GLOB BB_SOURCES "${BACKBONE}/*.c")
file(GLOB BB_HEADERS "${BACKBONE}/*.h")
file(GLOB EX_SOURCES "${EXTENSIONS}/dynamicio/*.c")
file(GLOB EX_HEADERS "${EXTENSIONS}/dynamicio/*.h")

# Generate the static library.
add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_HEADERS} ${EX_HEADERS} ${BB_SOURCES} ${EX_SOURCES})
