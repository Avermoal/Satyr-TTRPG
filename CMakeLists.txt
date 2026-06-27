cmake_minimum_required(VERSION 3.10)

project(Main-Satyr-TTRPG)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED NO)

add_subdirectory(app)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Satyr-TTRPG)
