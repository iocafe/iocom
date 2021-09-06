# iocom/code/CmakeLists.txt, 4.9.2021/pekka
#
# ESP-IDF build only: The second CMakeLists.txt in src_dir (code) sis responsible for controlling 
# the build process of the component and its integration into the overall project. 
# The component CMakeLists.txt file lists source files and registers the component to the build 
# system using idf_component_register.
#
cmake_minimum_required(VERSION 3.16.0)

# Project name, development root folder and iocom name for operatig system.
set(E_PROJECT "iocom")
set(ENV{E_OS} "esp32")
include("${COMPONENT_PATH}/../../eosal/osbuild/cmakedefs/eosal-root-path.txt")

# Set path to source files.
set(BACKBONE "$ENV{E_ROOT}/${E_PROJECT}/code")
set(EXTENSIONS "$ENV{E_ROOT}/${E_PROJECT}/extensions")

# Add main headers.
# set(MAIN_HEADERS "$ENV{E_ROOT}/${E_PROJECT}/${E_PROJECT}.h" "$ENV{E_ROOT}/${E_PROJECT}/${E_PROJECT}x.h")

# Add source files common to all operating systems.
file(GLOB BB_SOURCES "${BACKBONE}/*.c")
file(GLOB EX_SOURCES "${EXTENSIONS}/dynamicio/*.c")
# file(GLOB BB_HEADERS "${BACKBONE}/*.h")
# file(GLOB EX_HEADERS "${EXTENSIONS}/dynamicio/*.h")

# Merge the .c files as one list.
set(srcs ${BB_SOURCES})
list(APPEND srcs ${EX_SOURCES})

# The iocom library depends on: 
set(requires, "idf::eosal")

# Register iocom as ESP-IDEF component.
idf_component_register(SRCS "${srcs}" INCLUDE_DIRS "." REQUIRES "${requires}" )
