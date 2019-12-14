cmake_minimum_required(VERSION 2.8)
# specify project name
project(ClothSimulation)
set(PROJECT_NAME ClothSimulation)

# include the project setting file
include($ENV{DEVKIT_LOCATION}/cmake/pluginEntry.cmake)
include_directories(${PROJECT_SOURCE_DIR})

# set SOURCE_FILES
set(SOURCE_FILES

   clothCmd.cpp
)

# set linking libraries
set(LIBRARIES
     OpenMaya
     OpenMayaAnim
     OpenMayaRender
     Foundation
     clew

)


# Build plugin
build_plugin()

