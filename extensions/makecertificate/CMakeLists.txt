# iocom/extensions/makecertificate/CmakeLists.txt - Cmake build for makecertificate library.
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_PROJECT "makecertificate")
project(${E_PROJECT})

# Select which TLS library, mbedtls or openssl to use for the OS.
set(E_TLSLIB "mbedtls")

# include build environment setting common to all projects.
include(../../../eosal/osbuild/cmakedefs/eosal-defs.txt)

# Set path to where to keep libraries.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_LIBOUT})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIBOUT})

# Set path to source files.
set(BACKBONE "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}")

# Add include paths.
include_directories("${BACKBONE}")
# include_directories("$ENV{E_ROOT}/iocom")
include_directories("$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}")

# Add main headers.
set(MAIN_HEADERS "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/${E_PROJECT}.h")

# Add source files common to all operating systems.
file(GLOB BB_CM_SOURCES "${BACKBONE}/code/common/*.c")
file(GLOB BB_CM_HEADERS "${BACKBONE}/code/common/*.h")
file(GLOB BB_OS_SOURCES "${BACKBONE}/code/${E_TLSLIB}/*.c" "${BACKBONE}/code/$ENV{E_OS}/*.cpp")
file(GLOB BB_OS_HEADERS "${BACKBONE}/code/${E_TLSLIB}/*.h")

# Generate the static library.
add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_CM_HEADERS} ${BB_OS_HEADERS} ${BB_CM_SOURCES} ${BB_OS_SOURCES})

