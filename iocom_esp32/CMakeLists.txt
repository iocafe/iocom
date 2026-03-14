# iocom/iocom_esp32/CmakeLists.txt, 13.3.2026/pekka
#
# ESP-IDF build only: The second CMakeLists.txt in "iocom_esp32" is responsible for controlling 
# the build process of the component and its integration into the overall project. 
# The component CMakeLists.txt file lists source files and registers the component to the build 
# system using idf_component_register.
#
# In ESP_IDF the directory name is the component name: The ESP-IDF build system identifies components 
# based on the name of the directory containing the CMakeLists.txt file. Here, a directory named 
# iocom_esp32 is treated as the component iocom_esp32.
#
# Since icoom code is placed in "code" and "extensions/dynamicio" folders for other build system, new directory 
# iocom_esp32 holds the CmakeList.txt which compiles files in code and extensions directories.
#
cmake_minimum_required(VERSION 3.12)

# Project name, is development root folder name for the project.
set(E_PROJECT "iocom")

# Set E_ROOT environment variable to point to the project root folder. 
# This is used in the CMakeLists.txt files to locate source files and include paths.
include("${COMPONENT_PATH}/../osbuild/cmakedefs/eosal-root-path.txt")

# Include ESP32 specific setting for build.
include("${COMPONENT_PATH}/../osbuild/cmakedefs/eosal-defs-espidf.txt")

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
set(requires, "idf::eosal")

# Set include paths
set(includes $ENV{E_ROOT}/eosal)
# list(APPEND includes "$ENV{E_ROOT}/${E_PROJECT}/build/config")

# Register eosal as ESP-IDEF component.
idf_component_register(SRCS "${srcs}" 
    INCLUDE_DIRS "${includes}" 
    REQUIRES "${requires}" )
