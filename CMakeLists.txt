cmake_minimum_required(VERSION 3.8)
project(object_pool LANGUAGES CXX)

if(MSVC)
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/W4$<SEMICOLON>/Za>)
else()
  add_compile_options(
    $<$<CXX_COMPILER_ID:GNU,Clang>-Wextra$<SEMICOLON>-Wpedantic>)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING ON)

add_executable(demo src/demo.cpp include/object_pool.hpp)
target_include_directories(demo PUBLIC include)

if(WIN32)
  target_link_libraries(demo WS2_32 Iphlpapi shlwapi)
else(NOT ANDROID OR IOS)
  target_link_libraries(demo pthread)
endif()

if(BUILD_TESTING)
  add_subdirectory(test)
endif()
