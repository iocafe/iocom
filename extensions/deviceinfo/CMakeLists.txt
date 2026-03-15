# deviceinfo/CmakeLists.txt - Cmake build for DEVICEINFO library, 14.3.2026/pekka.
cmake_minimum_required(VERSION 3.12)

# Set project name (= project root folder name).
set(E_PROJECT "deviceinfo")

# Set build root environment variable E_ROOT
include("${CMAKE_CURRENT_SOURCE_DIR}/../../../eosal/osbuild/cmakedefs/eosal-root-path.txt")

if (DEFINED ESP_PLATFORM)
  # Include ESP-IDF build specific eosal settings for the top level cmake. 
  include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-top-level-espidf-setup.txt")

  # Add project's code component directory.
  list(APPEND EXTRA_COMPONENT_DIRS $ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/deviceinfo_esp32)

  # Enable dummy app_main for ESP-IDF build, so we can build eosal as executable ESP32 component. 
  add_definitions(-DOSAL_DUMMY_ESP32_APP_MAIN)

  # Include ESP-IDFbuild stuff. This will call the project() command and start the build process.
  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
 
  # Name the project.
  project(${E_PROJECT})

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
  include_directories("$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}")

  # Add main headers.
  set(MAIN_HEADERS "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/${E_PROJECT}.h")

  # Add source files common to all operating systems.
  file(GLOB BB_CM_SOURCES "${BACKBONE}/*.c")
  file(GLOB BB_CM_HEADERS "${BACKBONE}/*.h")

  # Generate the static library.
  add_library(${E_PROJECT}${E_POSTFIX} STATIC ${MAIN_HEADERS} ${BB_CM_HEADERS} ${BB_CM_SOURCES})

endif()
