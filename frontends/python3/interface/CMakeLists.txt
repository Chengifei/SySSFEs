cmake_minimum_required(VERSION 3.8)
project(python)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
SET(BUILD_WITH_INSTALL_RPATH  TRUE)

include_directories(../../../core)
include_directories(../../../common)
include_directories(SYSTEM /usr/include/python3.6m)
link_directories(../../../core/codegen)

set(SOURCE_FILES
        cSrcLocation.cpp
        cSrcLocation.hpp
        support_main.cpp
        cTypes.cpp
        cTypes.hpp
        cVariable.cpp
        cVariable.hpp
        )

link_libraries(python3 codegen)

add_library(support SHARED ${SOURCE_FILES})
set_target_properties(support PROPERTIES PREFIX "")
set_target_properties(support PROPERTIES SUFFIX .so)
add_custom_command(TARGET support POST_BUILD COMMAND cp support.so ../)

set(SOURCE_FILES
        cNVar.cpp
        cNVar.hpp
        python_interface.cpp
        resolver_main.cpp
        cRule.cpp
        cRule.hpp
        )

link_libraries(python3)
add_library(resolver SHARED ${SOURCE_FILES})
target_link_libraries(resolver support)
set_target_properties(resolver PROPERTIES PREFIX "")
set_target_properties(resolver PROPERTIES SUFFIX .so)
add_custom_command(TARGET resolver POST_BUILD COMMAND mv resolver.so ../)
