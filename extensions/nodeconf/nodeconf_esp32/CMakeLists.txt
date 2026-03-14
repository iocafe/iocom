# nodeconf/nodeconf_esp32/CmakeLists.txt, 13.3.2026/pekka
#
# ESP-IDF build only: The second CMakeLists.txt in "nodeconf_esp32" is responsible for controlling 
# the build process of the component and its integration into the overall project. 
# The component CMakeLists.txt file lists source files and registers the component to the build 
# system using idf_component_register.
#
# In ESP_IDF the directory name is the component name: The ESP-IDF build system identifies components 
# based on the name of the directory containing the CMakeLists.txt file. Here, a directory named 
# nodeconf_esp32 is treated as the component nodeconf_esp32.
#
# Since nodeconf code is placed in "code" directory for the other build system, the new directory 
# holds the CmakeList.txt, which compiles and registers the ESP-IDF component.
#
cmake_minimum_required(VERSION 3.12)

# Project name, is development root folder name for the project.
set(E_PROJECT "nodeconf")

# Set E_ROOT environment variable to point to the project root folder. 
# This is used in the CMakeLists.txt files to locate source files and include paths.
include("${COMPONENT_PATH}/../../osbuild/cmakedefs/eosal-root-path.txt")

# Include ESP32 specific setting for build.
include("$ENV{E_ROOT}/eosal/osbuild/cmakedefs/eosal-defs-espidf.txt")

# List source files common to all operating systems. We do not need header files for now.
file(GLOB srcs "${BACKBONE}/*.c")

# The iocom library depends on: 
set(requires, "idf::eosal")

# Set include paths.
set(includes $ENV{E_ROOT}/eosal)
list(APPEND includes $ENV{E_ROOT}/iocom)

# Register eosal as ESP-IDF component.
idf_component_register(SRCS "${srcs}" 
    INCLUDE_DIRS "${includes}" 
    REQUIRES "${requires}" )
