# iocom/extensions/iocompython/CmakeLists.txt - Cmake build for python (Python wrapper for IOCOM).
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_PROJECT "iocompython")
project(${E_PROJECT})

# include build environment setting common to all projects.
include(../../../eosal/osbuild/cmakedefs/eosal-defs.txt)

find_package(PythonInterp 3.5 REQUIRED)
find_package(PythonLibs 3.5 REQUIRED)

# Build individual library projects.
add_subdirectory($ENV{E_ROOT}/eosal "${CMAKE_CURRENT_BINARY_DIR}/eosal")
add_subdirectory($ENV{E_ROOT}/iocom "${CMAKE_CURRENT_BINARY_DIR}/iocom")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/devicedir "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/devicedir")

# Set path to where to keep libraries.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_BIN})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIBOUT})

set (E_TARGET ${E_PROJECT}${E_POSTFIX})

# iocompython is exception how debug build is handled. There is no 'd' postfix and
# resuting library is placed in debug folder. This allows .py files to import iocompython
# without modification while debugging.
if (${E_POSTFIX} MATCHES "d")
    set(E_POSTFIX "")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_BIN}/debug)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIBOUT}/debug)
endif()

# Set path to source files.
set(BACKBONE "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/code")

# Add include paths.
include_directories("$ENV{E_ROOT}/iocom")
include_directories("$ENV{E_ROOT}/iocom/extensions/devicedir")
include_directories("$ENV{E_ROOT}/iocom/extensions/iocompython")

# Guess where Python header could be? Is there better way to do this?
# if(EXISTS "/coderoot/python/include/python3.9d" AND CMAKE_BUILD_TYPE MATCHES Debug)
#   include_directories("/coderoot/python/include/python3.9d")
# elseif(EXISTS "/usr/include/python3.8")
if(EXISTS "/usr/include/python3.8")
  include_directories("/usr/include/python3.8")
elseif(EXISTS "/usr/include/python3.7m")
  include_directories("/usr/include/python3.7m")
endif()

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
