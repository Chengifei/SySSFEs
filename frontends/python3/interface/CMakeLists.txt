cmake_minimum_required(VERSION 3.8)
project(llvm)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
get_filename_component(INST_DIR .. REALPATH)
get_filename_component(DRIVER_PTH ../../../core/LLVM REALPATH)

include_directories(../../../core ../../../common ../../cxx/ /usr/include/python3.7m)
link_directories(../../../core/LLVM /usr/local/Cellar/python/3.7.0/Frameworks/Python.framework/Versions/3.7/lib /usr/local/opt/llvm/lib)
SET(CMAKE_INSTALL_RPATH ${DRIVER_PTH})

set(SOURCE_FILES
    Module.cpp
    Module.hpp
    cTypes.hpp
    cTypes.cpp
    llvm_main.cpp
    )

add_library(llvm SHARED ${SOURCE_FILES})
target_include_directories(llvm SYSTEM PUBLIC /usr/local/Cellar/python/3.7.0/Frameworks/Python.framework/Versions/3.7/include/python3.7m /usr/local/opt/llvm/include)
target_link_libraries(llvm driver python3 LLVM)
set_target_properties(llvm PROPERTIES PREFIX "")
set_target_properties(llvm PROPERTIES SUFFIX .so)
install(TARGETS llvm LIBRARY DESTINATION ${INST_DIR} NAMELINK_SKIP)
