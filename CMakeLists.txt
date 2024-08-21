cmake_minimum_required(VERSION 3.10)

project(cpp_project)

set(CMAKE_CXX_STANDARD 17)

set(root ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${root}/bin)
set(LIBRARY_OUTPUT_PATH ${root}/lib)

include_directories(${root}/include)
include_directories(${root}/include/components)
link_directories(${root}/lib)
include_directories(${root}/thirdparty/include)
link_directories(${root}/thirdparty/lib)


add_subdirectory(${root}/src)