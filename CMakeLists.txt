cmake_minimum_required(VERSION 3.0)

# project name
project(hdrplus)

# set c++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -march=native -O3")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -march=native -O3")

# Default build with release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Arm / X86 optimize flag
if(${CMAKE_HOST_SYSTEM_PROCESSOR} MATCHES "arm*")
  message(STATUS "ARM processor detected, will attempt to use NEON.")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon")
else()
  message(STATUS "Assuming AVX2 instructions available.")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
endif()

# LibRaw-cmake
find_library(LIBRAW_LIBRARY NAMES raw raw_r)
include_directories( BEFORE "/usr/local/include/")
message(STATUS "Found LIBRAW_LIBRARY to be ${LIBRAW_LIBRARY}" )

# dependency opencv
find_package( OpenCV REQUIRED )
include_directories( BEFORE ${OpenCV_INCLUDE_DIRS})

# library
include_directories( 
  ${PROJECT_SOURCE_DIR}/include )

add_library(${PROJECT_NAME} SHARED 
  src/align.cpp
  src/bayer_image.cpp
  src/burst.cpp
  src/finish.cpp
  src/hdrplus_pipeline.cpp
  src/merge.cpp )

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${OpenCV_LIBS}
  ${LIBRAW_LIBRARY} )

# example
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )

add_executable( demo 
  bin/demo.cpp )
target_link_libraries( demo ${PROJECT_NAME} )

# unit test
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/tests )

add_executable( test_bayer_image 
  tests/test_bayer_image.cpp )
target_link_libraries( test_bayer_image ${PROJECT_NAME} )
