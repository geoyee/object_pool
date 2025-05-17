set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
if(MSVC)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
add_subdirectory(googletest)
include_directories(googletest/googletest/include)

include_directories(../include ${GTEST_INCLUDE_DIRS})

add_executable(object_pool_test object_pool_test.cpp)
target_link_libraries(object_pool_test ${GTEST_BOTH_LIBRARIES} gtest gtest_main)
