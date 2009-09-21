# This file is part of libpano13, the hugin build environment.
# Licence details can be found in the file gpl.txt.
#
# Copyright (c) 2008, 2009 Kornel Benko, <Kornel.Benko@berlin.de>
#

project(tools)

set(commands PTtiff2psd PTtiffdump PTcrop PTblender
    PTroller PTuncrop PToptimizer PTmender PTinfo PTmasker)
if (HAVE_JAVA)
  # this one depends on java (uses writeProject() from ptpicker)
  list(APPEND commands PTAInterpolate)
endif()

# panoinfo needs windows.h

if(WIN32)
  list(APPEND commands panoinfo)
endif()

include_directories(${TOP_SRC_DIR})

foreach(_cmd ${commands})
  add_executable(${_cmd} ${_cmd}.c)
  target_link_libraries(${_cmd} pano13 ${_common_libs})
  install(TARGETS ${_cmd} DESTINATION bin CONFIGURATIONS Release RelWithDebInfo MinSizeRel)
endforeach(_cmd)

add_custom_target(tools DEPENDS ${commands})
