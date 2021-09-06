# devicedir/code/CmakeLists.txt, 4.9.2021/pekka
#
# ESP-IDF build only: The second CMakeLists.txt in src_dir (code) sis responsible for controlling 
# the build process of the component and its integration into the overall project. 
# The component CMakeLists.txt file lists source files and registers the component to the build 
# system using idf_component_register.
#
cmake_minimum_required(VERSION 3.5)

# Project name, development root folder, and name for operatig system.
set(E_PROJECT "devicedir")
set(ENV{E_ROOT} "/coderoot")
set(ENV{E_OS} "esp32")

# Set path to source files.
set(BACKBONE "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/code")

# Add main headers.
# set(MAIN_HEADERS "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/${E_PROJECT}.h")

# Add source files common to all operating systems.
file(GLOB BB_CM_SOURCES "${BACKBONE}/*.c")
# file(GLOB BB_CM_HEADERS "${BACKBONE}/*.h")

# Merge the .c files as one list.
set(srcs ${BB_CM_SOURCES})

# The devicedir library depends on: 
set(requires, "idf::iocom")

# Register devicedir as ESP-IDEF component.
idf_component_register(SRCS "${srcs}" INCLUDE_DIRS "." REQUIRES "${requires}")
