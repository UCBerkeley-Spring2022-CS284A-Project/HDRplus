cmake_minimum_required(VERSION 3.0)

# project name
project(hdrplus)

# set c++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -march=native -O3")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -std=c++11 -Wall -march=native -O3")

# make sure we use Release and warn otherwise
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  message(WARNING "CMAKE_BUILD_TYPE not set to 'Release'. Performance may be terrible.")
else()
  message(STATUS "Building with build type '${CMAKE_BUILD_TYPE}'")
endif()

# require OpenCV
find_package( OpenCV 3 REQUIRED )
include_directories( BEFORE ${OpenCV_INCLUDE_DIRS} )
message(STATUS "Found OpenCV ${OpenCV_INCLUDE_DIRS} ${OpenCV_LIBS}")

# LibRaw-cmake
find_library(LIBRAW_LIBRARY NAMES raw raw_r)
include_directories( BEFORE "/usr/local/include/")
message(STATUS "Found LIBRAW_LIBRARY to be ${LIBRAW_LIBRARY}" )

# Exiv2
find_package(exiv2 REQUIRED CONFIG NAMES exiv2)
link_libraries(exiv2lib)
message(STATUS "Found Exiv2 and linked")

if(NOT APPLE)
  # The clang compiler (on osx) is somehow much more strict
  # than the compilers on ubuntu and so this does not seem
  # possible on OSX just yet.
  add_definitions( -Werror )
endif()

# library
include_directories( include )

# all source files
set( src_files 
  src/align.cpp
  src/bayer_image.cpp
  src/burst.cpp
  src/finish.cpp
  src/hdrplus_pipeline.cpp
  src/merge.cpp 
  src/params.cpp)

# Build runtime load dynamic shared library
# https://cmake.org/cmake/help/latest/command/add_library.html
add_library(${PROJECT_NAME} SHARED ${src_files} )

# and link it
# https://cmake.org/cmake/help/v3.0/command/target_link_libraries.html
# use public interace because our .h / .hpp file include opencv .h / .hpp file
target_link_libraries(${PROJECT_NAME} PUBLIC
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

add_executable( test_bayer_image tests/test_bayer_image.cpp)
target_link_libraries( test_bayer_image 
  ${PROJECT_NAME} )

add_executable( test_utility tests/test_utility.cpp )
target_link_libraries( test_utility 
  ${PROJECT_NAME} )

add_executable( test_burst tests/test_burst.cpp )
target_link_libraries( test_burst 
  ${PROJECT_NAME} )
