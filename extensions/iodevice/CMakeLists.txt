# iodevice/CmakeLists.txt - Cmake build for iodevice library, 14.3.2026/pekka.
cmake_minimum_required(VERSION 3.12)

# Set project name (= project root folder name).
set(E_PROJECT "iodevice")

# Set build root environment variable E_ROOT
include("${CMAKE_CURRENT_LIST_DIR}/../../../eosal/osbuild/cmakedefs/eosal-root-path.txt")

if (DEFINED ESP_PLATFORM)
  # Include ESP32 specific setting for build.
  include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-defs-espidf.txt")

  # List source files common to all operating systems. We do not need header files for now.
  set(BACKBONE "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/code")
  file(GLOB srcs "${BACKBONE}/*.c" "${BACKBONE}/*.cpp")

  # The component depends on: 
  set(requires "eosal")
  list(APPEND requires "iocom")
  list(APPEND requires "pins")
  list(APPEND requires "nodeconf")
  list(APPEND requires "ioserver")
  list(APPEND requires "lighthouse")
  list(APPEND requires "deviceinfo")
  list(APPEND requires "iocom")

  # Set include paths.
  set(includes "$ENV{E_ROOT}/eosal")
  list(APPEND includes "$ENV{E_ROOT}/iocom")
  list(APPEND includes "$ENV{E_ROOT}/pins")
  list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/nodeconf")
  list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/ioserver")
  list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/lighthouse")
  list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/deviceinfo")
  list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/devicedir")
  list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}")

  # Register eosal as ESP-IDF component.
  idf_component_register(SRCS "${srcs}" 
      INCLUDE_DIRS "${includes}" 
      REQUIRES "${requires}" )

  idf_component_optional_requires(PRIVATE devicedir)

else()
  project(${E_PROJECT})

  # Include common setting for most of builds.
  include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-defs.txt")

  # Set path to where to keep libraries.
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY $ENV{E_LIBOUT})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY $ENV{E_LIBOUT})

  # Set path to source files.
  set(BACKBONE "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/code")

  # Add include paths.
  include_directories("$ENV{E_ROOT}/iocom")
  include_directories("$ENV{E_ROOT}/iocom/extensions/nodeconf")
  include_directories("$ENV{E_ROOT}/iocom/extensions/deviceinfo")
  include_directories("$ENV{E_ROOT}/iocom/extensions/devicedir")
  include_directories("$ENV{E_ROOT}/iocom/extensions/ioserver")
  include_directories("$ENV{E_ROOT}/iocom/extensions/lighthouse")
  include_directories("$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}")
  include_directories("$ENV{E_ROOT}/pins")

  # Add main headers.
  set(MAIN_HEADERS "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/${E_PROJECT}.h")

  # Add source files common to all operating systems.
  file(GLOB BB_CM_SOURCES "${BACKBONE}/*.c")
  file(GLOB BB_CM_CPP_SOURCES "${BACKBONE}/*.cpp")
  file(GLOB BB_CM_HEADERS "${BACKBONE}/*.h")

  # Generate the static library.
  add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_CM_HEADERS} ${BB_CM_SOURCES} ${BB_CM_CPP_SOURCES})

endif()
