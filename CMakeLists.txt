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
find_package(ZLIB REQUIRED)

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
  ${ZLIB_LIBRARIES}
  ${JPEG_LIBRARIES}
  ${OpenCV_LIBS}
  ${PCL_LIBRARIES})

find_package(bot2-core-lcmtypes QUIET)
if(bot2-core-lcmtypes_FOUND)
  # Find glib-2.0 and gthread-2.0 which are not automatically found by CMake.
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(PC_GLIB2 glib-2.0)
  find_path(GLIB2_INCLUDE_DIR NAMES glib.h PATHS "${PC_GLIB2_INCLUDE_DIRS}"
    PATH_SUFFIXES glib-2.0 glib-2.0/include)
  find_library(GLIB2_LIBRARY NAMES glib-2.0 PATHS "${PC_GLIB2_LIBRARY_DIRS}")
  pkg_check_modules(PC_GTHREAD2 gthread-2.0)
  find_library(GTHREAD2_LIBRARY NAMES gthread-2.0
    PATHS "${PC_GTHREAD2_LIBRARY_DIRS}")
  if(NOT GLIB2_INCLUDE_DIR OR NOT GLIB2_LIBRARY OR NOT GTHREAD2_LIBRARY)
    message(FATAL_ERROR "bot2-core-lcmtypes requires gthread-2.0 and it was not found.")
  endif()
  include_directories("${GLIB2_INCLUDE_DIR}")
  target_link_libraries(cv-utils bot2-core-lcmtypes::lcmtypes_bot2-core-cpp
    "${GLIB2_LIBRARY}" "${GTHREAD2_LIBRARY}")
else()
  use_pkg(cv-utils lcmtypes_bot2-core)
endif()



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
