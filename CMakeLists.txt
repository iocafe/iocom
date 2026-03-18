# iocom/CmakeLists.txt - Cmake build for IOCOM library, 14.3.2026/pekka.
cmake_minimum_required(VERSION 3.12)

# Set project name (= project root folder name).
set(E_PROJECT "iocom")

# Set build root environment variable E_ROOT
include("${CMAKE_CURRENT_LIST_DIR}/../eosal/osbuild/cmakedefs/eosal-root-path.txt")

if (DEFINED ESP_PLATFORM)
  # Include ESP32 specific setting for build.
  include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-defs-espidf.txt")

  # Set path to source files.
  set(BACKBONE "$ENV{E_ROOT}/${E_PROJECT}/code")
  set(EXTENSIONS "$ENV{E_ROOT}/${E_PROJECT}/extensions")

  # List source files common to all operating systems. We do not need header files for now.
  file(GLOB BB_SOURCES "${BACKBONE}/*.c")
  file(GLOB EX_SOURCES "${EXTENSIONS}/dynamicio/*.c")

  # Merge the .c files as one list.
  set(srcs ${BB_SOURCES})
  list(APPEND srcs ${EX_SOURCES})

  # The iocom library depends on: 
  set(requires, "eosal")

  # Set include paths
  set(includes $ENV{E_ROOT}/eosal)
  list(APPEND includes $ENV{E_ROOT}/iocom)

  # Register eosal as ESP-IDF component.
  idf_component_register(SRCS "${srcs}" 
      INCLUDE_DIRS "${includes}" 
      REQUIRES "${requires}" )

  # This results correct link order. 
  idf_component_optional_requires(PRIVATE eosal)

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
