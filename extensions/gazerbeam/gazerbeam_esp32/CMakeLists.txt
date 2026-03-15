# gazerbeam/gazerbeam_esp32/CmakeLists.txt, 13.3.2026/pekka
#
# ESP-IDF build only: The second CMakeLists.txt in "gazerbeam_esp32" is responsible for controlling 
# the build process of the component and its integration into the overall project. 
# The component CMakeLists.txt file lists source files and registers the component to the build 
# system using idf_component_register.
#
# In ESP_IDF the directory name is the component name: The ESP-IDF build system identifies components 
# based on the name of the directory containing the CMakeLists.txt file. Here, a directory named 
# gazerbeam_esp32 is treated as the component gazerbeam_esp32.
#
# Since gazerbeam code is placed in "code" directory for the other build system, the new directory 
# holds the CmakeList.txt, which compiles and registers the ESP-IDF component.
#
cmake_minimum_required(VERSION 3.12)

# Project name, is development root folder name for the project.
set(E_PROJECT "gazerbeam")

# Set E_ROOT environment variable to point to the project root folder. 
# This is used in the CMakeLists.txt files to locate source files and include paths.
include("${COMPONENT_PATH}/../../../../eosal/osbuild/cmakedefs/eosal-root-path.txt")

# Include ESP32 specific setting for build.
include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-defs-espidf.txt")

# List source files common to all operating systems. We do not need header files for now.
set(BACKBONE "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}/code")
file(GLOB srcs "${BACKBONE}/*.c")

# The component depends on: 
set(requires "eosal_esp32")
list(APPEND requires "iocom_esp32")

# Set include paths.
set(includes "$ENV{E_ROOT}/eosal")
list(APPEND includes "$ENV{E_ROOT}/iocom")
list(APPEND includes "$ENV{E_ROOT}/pins")
list(APPEND includes "$ENV{E_ROOT}/iocom/extensions/${E_PROJECT}")

# Register eosal as ESP-IDF component.
idf_component_register(SRCS "${srcs}" 
    INCLUDE_DIRS "${includes}" 
    REQUIRES "${requires}" )
