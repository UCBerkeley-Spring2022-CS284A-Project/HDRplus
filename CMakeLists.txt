cmake_minimum_required(VERSION 3.0)

# project name
project(hdrplus)

# set c++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -march=native -O3")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=c++11 -Wall -march=native -O3")

# Default build with release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# LibRaw-cmake
find_library(LIBRAW_LIBRARY NAMES raw raw_r)
include_directories( BEFORE "/usr/local/include/")
message(STATUS "Found LIBRAW_LIBRARY to be ${LIBRAW_LIBRARY}" )

# dependency opencv
find_package( OpenCV 4 REQUIRED )
include_directories( BEFORE ${OpenCV_INCLUDE_DIRS})
message(STATUS "Found OpenCV ${OpenCV_INCLUDE_DIRS} ${OpenCV_LIBS}")

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

target_link_libraries(${PROJECT_NAME}
  ${OpenCV_LIBS}
  ${LIBRAW_LIBRARY} )

# example
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin )

add_executable( demo 
  bin/demo.cpp )
target_link_libraries( demo 
  ${PROJECT_NAME} )

# unit test
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/tests )

add_executable( test_bayer_image 
  tests/test_bayer_image.cpp )
target_link_libraries( test_bayer_image 
  ${PROJECT_NAME} )

add_executable( test_utility 
  tests/test_utility.cpp )
target_link_libraries( test_utility 
  ${PROJECT_NAME} )

add_executable( test_burst 
  tests/test_burst.cpp )
target_link_libraries( test_burst 
  ${PROJECT_NAME} )
