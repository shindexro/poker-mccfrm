cmake_minimum_required(VERSION 3.22.1)

project(poker-mccfrm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

set(Boost_INCLUDE_DIR /usr/local/boost_1_83_0)
set(Boost_NO_WARN_NEW_VERSIONS 1)

set(TBB_DIR "/usr/local/onetbb/lib/cmake/TBB")

add_subdirectory(src)
