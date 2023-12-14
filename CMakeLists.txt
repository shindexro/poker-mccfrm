cmake_minimum_required(VERSION 3.22.1)

project(poker-mccfrm)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize-coverage=inline-8bit-counters -fsanitize-coverage=trace-cmp")

add_subdirectory(src)
add_subdirectory(test)

set(Boost_INCLUDE_DIR /usr/include/boost)
set(Boost_NO_WARN_NEW_VERSIONS 1)

set(TBB_DIR "/opt/intel/oneapi/tbb/latest/lib/cmake/tbb")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/7f036c5563af7d0329f20e8bb42effb04629f0c0.zip
)
FetchContent_MakeAvailable(googletest)

