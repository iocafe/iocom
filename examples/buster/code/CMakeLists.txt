# iocom/examples/buster/code/CmakeLists.txt, 14.3.2026/pekka
#
# ESP-IDF build only: The second CMakeLists.txt in project source directory "code" 
# controls the the build process of the main component. 
# This component CMakeLists.txt file lists source files and registers the component
# to the ESP32 build system using idf_component_register.
#
cmake_minimum_required(VERSION 3.12)

# Project name.
set(E_PROJECT "buster")

# Select hardware setup to use (IO pins, etc)
set(E_HW "esphw")

# Set root folder E_ROOT for the build OS (/coderoot or c:/coderoot).
include("${COMPONENT_PATH}/../../../../eosal/osbuild/cmakedefs/eosal-root-path.txt")

# Include build variable settings common to ESP32 projects.
include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-defs-espidf.txt")

# List source files for the project. We do not need header files for now.
file(GLOB srcs "${COMPONENT_PATH}/*.c" "${COMPONENT_PATH}/*.cpp")

# This project depends on: 
set(requires ${E_REQUIRES})
list(APPEND requires "ioserver")
list(APPEND requires "iodevice")

set(includes ${E_INCLUDE_DIRS})
list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/ioserver")
list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/iodevice")
# list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/buster")

# Set include path for the project and for IO pin and communication signal configuration (for this hardware).
list(APPEND includes "$ENV{E_ROOT}/iocom/examples/${E_PROJECT}")
list(APPEND includes "$ENV{E_ROOT}/iocom/examples/${E_PROJECT}/config/include/${E_HW}")

# Register eosal as ESP-IDF component.
idf_component_register(SRCS "${srcs}" INCLUDE_DIRS "${includes}" REQUIRES "${requires}" )
