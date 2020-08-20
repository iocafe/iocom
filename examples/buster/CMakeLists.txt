# iocom/examples/buster/CmakeLists.txt - Cmake build for Buster IO controller example.
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_PROJECT "buster")
project(${E_PROJECT})

# include build information common to all projects.
include(../../../eosal/osbuild/cmakedefs/eosal-defs.txt)

# Select hardware setup to use (IO pins, etc)
set(E_HW "grumpy")

# Select libraries to link with application.
set(E_APPLIBS "iodevice${E_POSTFIX};ioserver${E_POSTFIX};devicedir${E_POSTFIX};gazerbeam${E_POSTFIX};deviceinfo${E_POSTFIX};nodeconf${E_POSTFIX};lighthouse${E_POSTFIX};pins${E_POSTFIX};iocom${E_POSTFIX};$ENV{OSAL_TLS_APP_LIBS}")
if ($ENV{E_OSVER} MATCHES "pi")
  set(E_APPLIBS "${E_APPLIBS};pigpio;raspicam;rt")
endif()
set(E_APPLIBS "${E_APPLIBS};$ENV{OSAL_TLS_APP_LIBS}")

# Build individual library projects.
add_subdirectory($ENV{E_ROOT}/eosal "${CMAKE_CURRENT_BINARY_DIR}/eosal")
add_subdirectory($ENV{E_ROOT}/iocom "${CMAKE_CURRENT_BINARY_DIR}/iocom")
add_subdirectory($ENV{E_ROOT}/pins "${CMAKE_CURRENT_BINARY_DIR}/pins")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/nodeconf "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/nodeconf")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/deviceinfo "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/deviceinfo")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/ioserver "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/ioserver")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/iodevice "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/iodevice")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/lighthouse "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/lighthouse")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/devicedir "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/devicedir")
add_subdirectory($ENV{E_ROOT}/iocom/extensions/gazerbeam "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/gazerbeam")

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

# Add iocom, nodeconf, deviceinfo, ioserver, iodevice, lighthouse and devicedir libraries to include path.
include_directories("$ENV{E_ROOT}/iocom")
include_directories("$ENV{E_ROOT}/pins")
include_directories("$ENV{E_ROOT}/iocom/extensions/nodeconf")
include_directories("$ENV{E_ROOT}/iocom/extensions/deviceinfo")
include_directories("$ENV{E_ROOT}/iocom/extensions/ioserver")
include_directories("$ENV{E_ROOT}/iocom/extensions/iodevice")
include_directories("$ENV{E_ROOT}/iocom/extensions/lighthouse")
include_directories("$ENV{E_ROOT}/iocom/extensions/devicedir")
include_directories("$ENV{E_ROOT}/iocom/extensions/gazerbeam")

# Add header files, the file(GLOB_RECURSE...) allows for wildcards and recurses subdirs.
file(GLOB_RECURSE HEADERS "${E_SOURCE_PATH}/*.h")

# Add source files.
file(GLOB_RECURSE SOURCES "${E_SOURCE_PATH}/*.c" "${E_SOURCE_PATH}/*.cpp")

# Build executable. Set library folder and libraries to link with.
link_directories($ENV{E_LIB})
add_executable(${E_PROJECT}${E_POSTFIX} ${HEADERS} ${SOURCES})
target_link_libraries(${E_PROJECT}${E_POSTFIX} ${E_APPLIBS})
