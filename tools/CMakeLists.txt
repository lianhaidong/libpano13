# This file is part of libpano13, the hugin build environment.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(tools)

set(commands PTtiff2psd PTtiffdump PTcrop PTblender
    PTroller PTuncrop PToptimizer PTmender PTinfo PTmasker)
if (SUPPORT_JAVA_PROGRAMS)
  # this one depends on java (uses writeProject() from ptpicker)
  list(APPEND commands PTAInterpolate)
endif()

# panoinfo needs windows.h

set(extra_commands)
macro(make_command exe_name source_name)
  list(APPEND extra_commands ${exe_name})
  add_executable(${exe_name} ${source_name})
  target_link_libraries(${exe_name} ${_pano_target} ${_common_libs})
  install(TARGETS ${exe_name} DESTINATION bin COMPONENT tools CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
endmacro()

if(WIN32)
  list(APPEND commands panoinfo)
  IF(BUILD_DLL)
    add_library(getopt STATIC compat_win32/getopt.c compat_win32/getopt.h)
    list(APPEND _common_libs getopt)
  ENDIF(BUILD_DLL)
else()
  make_command(panoinfo panoinfo_unix.c)
endif()

include_directories(${TOP_SRC_DIR})

foreach(_cmd ${commands})
  make_command(${_cmd} ${_cmd}.c)
endforeach(_cmd)

add_custom_target(tools DEPENDS ${extra_commands})
