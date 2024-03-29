# iocom/examples/gina/CmakeLists.txt - Cmake build for Gina IO board example.
cmake_minimum_required(VERSION 3.5)

# Set project name (= project root folder name).
set(E_PROJECT "gina")
set(E_HW "carol")
set(E_UP "../../../eosal/osbuild/cmakedefs")

# Set build root environment variable E_ROOT
include("${E_UP}/eosal-root-path.txt")

if("${IDF_TARGET}" STREQUAL "esp32")
  # ESP-IFD only: Include IDF project setup and declare the project.
  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  project(${E_PROJECT})

  # include build information common to all projects.
  include("${E_UP}/eosal-defs-espidf.txt")

else()
  project(${E_PROJECT})

  # include build information common to all projects.
  include("${E_UP}/eosal-defs.txt")

  # Build individual library projects.
  add_subdirectory($ENV{E_ROOT}/eosal "${CMAKE_CURRENT_BINARY_DIR}/eosal")
  add_subdirectory($ENV{E_ROOT}/iocom "${CMAKE_CURRENT_BINARY_DIR}/iocom")
  add_subdirectory($ENV{E_ROOT}/iocom/extensions/nodeconf "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/nodeconf")
  add_subdirectory($ENV{E_ROOT}/iocom/extensions/lighthouse "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/lighthouse")
  add_subdirectory($ENV{E_ROOT}/iocom/extensions/selectwifi "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/deviceinfo")
  add_subdirectory($ENV{E_ROOT}/iocom/extensions/gazerbeam "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/gazerbeam")
  add_subdirectory($ENV{E_ROOT}/pins "${CMAKE_CURRENT_BINARY_DIR}/pins")

  # The devicedir library is here for testing only, take away.
  add_subdirectory($ENV{E_ROOT}/iocom/extensions/devicedir "${CMAKE_CURRENT_BINARY_DIR}/iocom/extensions/devicedir")

  # Set path to where to keep libraries.
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{E_BIN})

  # Set path to source files.
  set(E_SOURCE_PATH "$ENV{E_ROOT}/iocom/examples/${E_PROJECT}/code")

  # Build C code from JSON configuration
  execute_process(COMMAND "$ENV{E_ROOT}/iocom/examples/${E_PROJECT}/scripts/config_to_c_code.py")

  # Include path for the project and for IO pin and communication signal configuration (for this hardware).
  include_directories("$ENV{E_ROOT}/iocom/examples/${E_PROJECT}")
  include_directories("$ENV{E_ROOT}/iocom/examples/${E_PROJECT}/config/include/${E_HW}")

  # Add iocom and pins to include path.
  include_directories("$ENV{E_ROOT}/iocom")
  include_directories("$ENV{E_ROOT}/iocom/extensions/nodeconf")
  include_directories("$ENV{E_ROOT}/iocom/extensions/lighthouse")
  include_directories("$ENV{E_ROOT}/iocom/extensions/deviceinfo")
  include_directories("$ENV{E_ROOT}/iocom/extensions/gazerbeam")
  include_directories("$ENV{E_ROOT}/pins")

  # The devicedir library is here for testing only, take away.
  include_directories("$ENV{E_ROOT}/iocom/extensions/devicedir")

  # Add header files, the file(GLOB_RECURSE...) allows for wildcards and recurses subdirs.
  file(GLOB_RECURSE HEADERS "${E_SOURCE_PATH}/*.h")

  # Add source files.
  file(GLOB_RECURSE SOURCES "${E_SOURCE_PATH}/*.c")

  # Build executable. Set library folder and libraries to link with.
  link_directories($ENV{E_LIB})
  add_executable(${E_PROJECT}${E_POSTFIX} ${HEADERS} ${SOURCES})
  target_link_libraries(${E_PROJECT}${E_POSTFIX} devicedir${E_POSTFIX};deviceinfo${E_POSTFIX};gazerbeam${E_POSTFIX};nodeconf${E_POSTFIX};lighthouse${E_POSTFIX};pins${E_POSTFIX};iocom${E_POSTFIX};$ENV{OSAL_TLS_APP_LIBS})

endif()
