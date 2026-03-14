# iocom/CmakeLists.txt - Cmake build for IOCOM library, 14.3.2026/pekka.
cmake_minimum_required(VERSION 3.12)

# Set project name (= project root folder name).
set(E_PROJECT "iocom")

# Set build root environment variable E_ROOT
include("../eosal/osbuild/cmakedefs/eosal-root-path.txt")

if (DEFINED ESP_PLATFORM)
  # Include ESP-IDF build specific eosal settings for the top level cmake. 
  include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-top-level-espidf-setup.txt")
  list(APPEND EXTRA_COMPONENT_DIRS $ENV{E_ROOT}/iocom/iocom_esp32)

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

endif()
