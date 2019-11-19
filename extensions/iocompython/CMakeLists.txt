# iocom/extensions/iocompython/CmakeLists.txt - Cmake build for python (Python wrapper for IOCOM).
cmake_minimum_required(VERSION 2.8.11)

# Set project name (= project root folder name).
set(E_PROJECT "iocompython")
project(${E_PROJECT})

# include build environment setting common to all projects.
include(../../../eosal/build/cmakedefs/eosal-defs.txt)

find_package(PythonInterp 3.5 REQUIRED)
find_package(PythonLibs 3.5 REQUIRED)

# Build individual library projects.
add_subdirectory($ENV{E_ROOT}/eosal "${CMAKE_CURRENT_BINARY_DIR}/eosal")
add_subdirectory($ENV{E_ROOT}/iocom "${CMAKE_CURRENT_BINARY_DIR}/iocom")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/devicedir "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/devicedir")

# Set path to where to keep libraries.
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_LIB})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_BIN})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIB})

# Set path to source files.
set(BACKBONE "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/code")

# Add iocom to include path.
include_directories("$ENV{E_ROOT}/iocom")
include_directories("$ENV{E_ROOT}/iocom/extensions/devicedir")

# Add main headers.
set(MAIN_HEADERS "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/${E_PROJECT}.h")

# Add source files common to all operating systems.
file(GLOB BB_CM_SOURCES "${BACKBONE}/*.c")
file(GLOB BB_CM_HEADERS "${BACKBONE}/*.h")

# Generate the static library.
link_directories($ENV{E_LIB})
add_library(${E_PROJECT}${E_POSTFIX} SHARED ${MAIN_HEADERS} ${BB_CM_HEADERS} ${BB_CM_SOURCES})
target_link_libraries(${E_PROJECT}${E_POSTFIX} devicedir${E_POSTFIX};iocom${E_POSTFIX};$ENV{OSAL_TLS_APP_LIBS})

set_target_properties(
    ${E_PROJECT}${E_POSTFIX}
    PROPERTIES
        PREFIX ""
        OUTPUT_NAME ${E_PROJECT}${E_POSTFIX}
        LINKER_LANGUAGE C
    )
