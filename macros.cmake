macro(use_pkg target)

  find_package(PkgConfig REQUIRED)

  foreach (pkgname ${ARGN})
    set(cachevar ${pkgname}_pkgconfig)
    pkg_check_modules(${cachevar} ${pkgname})

    if (NOT ${cachevar}_FOUND)
      message(SEND_ERROR "required package ${pkgname} not found. PKG_CONFIG_PATH=$ENV{PKG_CONFIG_PATH}")
    endif()

    string(REPLACE ";" " " _cflags_str "${${cachevar}_CFLAGS}")
    string(REPLACE ";" " " _ldflags_str "${${cachevar}_LDFLAGS}")
    set_property(TARGET ${target} APPEND_STRING PROPERTY COMPILE_FLAGS "${_cflags_str} ")
    set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS "${_ldflags_str} ")
    link_directories(${${cachevar}_LIBRARY_DIRS})
    target_link_libraries(${target} ${${cachevar}_LIBRARIES})

  endforeach()

endmacro()

macro(setup_pkg_config_path)
  set(ENV{PKG_CONFIG_PATH} "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
  foreach(_prefix_path ${CMAKE_PREFIX_PATH})
    set(ENV{PKG_CONFIG_PATH} $ENV{PKG_CONFIG_PATH}:${_prefix_path}/lib/pkgconfig)
  endforeach()
endmacro()
