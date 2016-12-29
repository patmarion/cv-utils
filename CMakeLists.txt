cmake_minimum_required(VERSION 2.8.7)
project(cv_utils)


option(BUILD_SHARED_LIBS "Build shared libraries" ON)
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type options: Debug, Release, MinSizeRel" FORCE)
endif()

include(macros.cmake)
find_package(JPEG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PCL REQUIRED)

setup_pkg_config_path()

include_directories(${PCL_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(image_utils_src
  image_utils/pixels.c
  image_utils/jpeg.c)

set(multisense_image_utils_src
  multisense_image_utils/multisense_image_utils.cpp)

set(multisense_utils_src
  multisense_utils/multisense_utils.cpp)

add_library(cv-utils SHARED
  ${image_utils_src}
  ${multisense_image_utils_src}
  ${multisense_utils_src})

set_property(SOURCE
  ${image_utils_src}
  PROPERTY COMPILE_FLAGS "-std=c99 -fPIC -msse -msse2 -msse3")

set_property(SOURCE
  ${image_utils_src}
  PROPERTY INCLUDE_DIRS ${JPEG_INCLUDE_DIR})

set_property(SOURCE
  ${multisense_image_utils_src}
  ${multisense_utils_src}
  PROPERTY COMPILE_FLAGS "-std=c++11")

#set_property(SOURCE
#  ${multisense_utils_src}
#  PROPERTY INCLUDE_DIRECTORIES /foo/bar)

target_link_libraries(cv-utils
  ${JPEG_LIBRARIES}
  ${OpenCV_LIBS}
  ${PCL_LIBRARIES})

use_pkg(cv-utils bot2-core)

install(FILES
  image_utils/jpeg.h
  image_utils/pixels.h
  DESTINATION include/cv-utils/image_utils)

install(FILES
  multisense_image_utils/multisense_image_utils.hpp
  DESTINATION include/cv-utils/multisense_image_utils)

install(FILES
  multisense_utils/conversions_lcm.hpp
  multisense_utils/multisense_utils.hpp
  DESTINATION include/cv-utils/multisense_utils)

install(TARGETS cv-utils
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
