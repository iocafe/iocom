# iodevice/CmakeLists.txt - Cmake build for iodevice library, 14.3.2026/pekka.
cmake_minimum_required(VERSION 3.12)

# Set project name (= project root folder name).
set(E_PROJECT "iodevice")

# Set build root environment variable E_ROOT
include("${CMAKE_CURRENT_SOURCE_DIR}/../../../eosal/osbuild/cmakedefs/eosal-root-path.txt")

if (DEFINED ESP_PLATFORM)
  # Include ESP-IDF build specific eosal settings for the top level cmake. 
  include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-top-level-espidf-setup.txt")

  # Add project's code component directory.
  set(EXTRA_COMPONENT_DIRS ${E_EXTRA_COMPONENT_DIRS})
  list(APPEND EXTRA_COMPONENT_DIRS "$ENV{E_ROOT}/iocom/extensions/ioserver/ioserver_esp32")
  list(APPEND EXTRA_COMPONENT_DIRS "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/${E_PROJECT}_esp32")
  
  # Enable dummy app_main for ESP-IDF build, so we can build eosal as executable ESP32 component. 
  add_definitions(-DOSAL_DUMMY_ESP32_APP_MAIN)

  # iodevice acts also as server, so it needs full athentication IOC_FULL_AUTHENTICATION (2). 
  add_definitions(-DIOC_AUTHENTICATION_CODE=2)
  add_definitions(-DIOC_DYNAMIC_MBLK_CODE=1) 

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
