# iocom/examples/tito/CmakeLists.txt - Cmake build for Tito IO controller example.
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_PROJECT "tito")
set(E_HW "generic")
project(${E_PROJECT})

# include build information common to all projects.
include(../../../eosal/osbuild/cmakedefs/eosal-defs.txt)

# Build individual library projects.
add_subdirectory($ENV{E_ROOT}/eosal "${CMAKE_CURRENT_BINARY_DIR}/eosal")
add_subdirectory($ENV{E_ROOT}/iocom "${CMAKE_CURRENT_BINARY_DIR}/iocom")
add_subdirectory($ENV{E_ROOT}/pins "${CMAKE_CURRENT_BINARY_DIR}/pins")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/nodeconf "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/nodeconf")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/ioserver "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/ioserver")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/lighthouse "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/lighthouse")

# The devicedir library is here for testing only, take away.
add_subdirectory($ENV{E_ROOT}/iocom/extensions/devicedir "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/devicedir")

# Set path to where to keep libraries.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{E_BIN})

# Set path to source files.
set(E_SOURCE_PATH "$ENV{E_ROOT}/iocom/examples/${E_PROJECT}/code")

# Build C code from JSON configuration
if (WIN32)
  execute_process(COMMAND "$ENV{E_ROOT}/iocom/examples/${E_PROJECT}/scripts/config_to_c_code.py")
else()
  execute_process(COMMAND "$ENV{E_ROOT}/iocom/examples/${E_PROJECT}/scripts/config_to_c_code.py")
endif()

# Set include path for the project and for IO pin and communication signal configuration (for this hardware).
include_directories("$ENV{E_ROOT}/iocom/examples/${E_PROJECT}")
include_directories("$ENV{E_ROOT}/iocom/examples/${E_PROJECT}/config/include/${E_HW}")

# Add iocom, nodeconf, ioserver, lighthouse and devicedir to include path.
include_directories("$ENV{E_ROOT}/iocom")
include_directories("$ENV{E_ROOT}/pins")
include_directories("$ENV{E_ROOT}/iocom/extensions/nodeconf")
include_directories("$ENV{E_ROOT}/iocom/extensions/ioserver")
include_directories("$ENV{E_ROOT}/iocom/extensions/lighthouse")

# The devicedir library is here for testing only, take away.
include_directories("$ENV{E_ROOT}/iocom/extensions/devicedir")

# Add header files, the file(GLOB_RECURSE...) allows for wildcards and recurses subdirs.
file(GLOB_RECURSE HEADERS "${E_SOURCE_PATH}/*.h")

# Add source files.
file(GLOB_RECURSE SOURCES "${E_SOURCE_PATH}/*.c" "${E_SOURCE_PATH}/*.cpp")

# Build executable. Set library folder and libraries to link with.
link_directories($ENV{E_LIB})
add_executable(${E_PROJECT}${E_POSTFIX} ${HEADERS} ${SOURCES})
target_link_libraries(${E_PROJECT}${E_POSTFIX} devicedir${E_POSTFIX};ioserver${E_POSTFIX};lighthouse${E_POSTFIX};nodeconf${E_POSTFIX};pins${E_POSTFIX};iocom${E_POSTFIX};$ENV{OSAL_TLS_APP_LIBS})
