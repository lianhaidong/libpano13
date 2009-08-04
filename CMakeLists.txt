## cmakelists.txt for libpano13 in a Hugin build environment.
##     03-04 Aug 2009 TKSharpless
## 
## Builds libpano13 only, by default without support for the
## Java programs ptpicker and pteditor.  To include that, you
## can define HAVE_JAVA on the CMake command line.  The Java 
## header jni.h and its dependencies must then be in your 
## include search path (this script does not find them).
## 
## This script uses the Hugin CMake modules to find required
## image file format libraries, so it needs to know the name 
## of the Hign source tree root.  By default that must be 
## "../hugin" relative to the libpano root (that contains
## this file).  But you can set HUGIN_BASE_DIR to the path
## name of the Hugin root on the CMake command line.
##
## The parent directory of the Hugin root is one place where
## dependencies will be looked for.  On Windows that should
## be the root directory of the Hugin SDK, and the wxWidgets
## package will likely supply the dependencies.  Note:  you
## may need to edit the wxWidgets version number below.
##


# require at least cmake 2.4
cmake_minimum_required(VERSION 2.4)

## Locate the hugin source root and its parent directory
IF ( HUGIN_BASE_DIR )
  GET_FILENAME_COMPONENT( SOURCE_BASE_DIR ${HUGIN_SOURCE_DIR} PATH CACHE )
  set(CMAKE_MODULE_PATH ${HUGIN_BASE_DIR}/CMakeModules )
ELSE ( HUGIN_BASE_DIR )
  GET_FILENAME_COMPONENT( SOURCE_BASE_DIR ${CMAKE_SOURCE_DIR} PATH CACHE )
  set(CMAKE_MODULE_PATH ${SOURCE_BASE_DIR}/hugin/CMakeModules )
ENDIF ( HUGIN_BASE_DIR )

## load the Hugin cmake modules
include(HuginMacros)
include(CheckIncludeFiles)

##  global setup
project(libpano13)

##
## wxWidgets
##
#  This must come before FINDs for tiff, jpeg, png, zlib to enable
#  finding the wxWidgets distributions of those packages on Windows.
IF (WIN32)
  SET(wxWidgets_ROOT_DIR ${SOURCE_BASE_DIR}/wxWidgets-2.8.10)
  ADD_DEFINITIONS(-D__Win__)
ENDIF(WIN32)

FIND_PACKAGE(wxWidgets REQUIRED)

##
## Graphics libraries
##
# wxWidgets sub-packages will be used if native ones aren't found

FIND_PACKAGE(TIFF REQUIRED)
INCLUDE_DIRECTORIES(${TIFF_INCLUDE_DIR})
FIND_PACKAGE(JPEG REQUIRED)
INCLUDE_DIRECTORIES(${JPEG_INCLUDE_DIR})
FIND_PACKAGE(PNG REQUIRED)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${PNG_INCLUDE_DIR})

#
# Check build type and set options accordingly
# 
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
IF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  ADD_DEFINITIONS(-DDEBUG)
  IF(UNIX)
    ADD_DEFINITIONS(-Wall)
  ENDIF(UNIX)
ENDIF(${CMAKE_BUILD_TYPE} STREQUAL "Debug")

IF (UNIX)
  ADD_DEFINITIONS(-D__Ansi__)
ELSE (UNIX)
  IF (MSVC)
    # Stop MSVC8 from bitching about the C library
    ADD_DEFINITIONS(/D_CRT_SECURE_NO_DEPRECATE)
    # Stop MSVC9 from bitching about possibly invalid STL usage
    ADD_DEFINITIONS(/D_SCL_SECURE_NO_WARNINGS)
    # Stop MSVC9 from bitching about POSIX names without underscores
    ADD_DEFINITIONS(/D_CRT_NONSTDC_NO_DEPRECATE)
    # compile everything for the static C runtime
	FOREACH(TYPE C CXX)
		# makefiles use blank configuration
		FOREACH(CFG "_DEBUG" "_MINSIZEREL" "_RELEASE" "_RELWITHDEBINFO")
			#MESSAGE("Replacing CMAKE_${TYPE}_FLAGS${CFG}: ${CMAKE_${TYPE}_FLAGS${CFG}}")
			SET(NEW_FLAGS "${CMAKE_${TYPE}_FLAGS${CFG}}")
			# fix up static libc flags
			STRING(REPLACE "/MD" "/MT" NEW_FLAGS "${NEW_FLAGS}")
			# *FORCE* to override whats already placed into the cache
			SET(CMAKE_${TYPE}_FLAGS${CFG} "${NEW_FLAGS}" CACHE STRING
				"CMAKE_${TYPE}_FLAGS${CFG} (overwritten to ensure static build)" FORCE)
			#MESSAGE("New       CMAKE_${TYPE}_FLAGS${CFG}: ${CMAKE_${TYPE}_FLAGS${CFG}}")
		ENDFOREACH(CFG)
	ENDFOREACH(TYPE)
    # link only with the static C runtime !! THIS IS A MUST !!
	SET(NEW_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	# ensure that the flags are not duplicaated on subsequent runs
	STRING(REPLACE " /NODEFAULTLIB:\"MSVCRT.lib\" /NODEFAULTLIB:\"MSVCRTd.lib\"" "" NEW_FLAGS "${NEW_FLAGS}")
	# note that flag names (NODEFAULTLIB) etc.) are case sensitive
    SET( CMAKE_EXE_LINKER_FLAGS
        "${NEW_FLAGS} /NODEFAULTLIB:\"MSVCRT.lib\" /NODEFAULTLIB:\"MSVCRTd.lib\""
       CACHE STRING "for MSVC" FORCE
    )
  ENDIF(MSVC)
ENDIF (UNIX)

##
## Here is the part that builds libpano
##
if ( HAVE_JAVA )
  set( java_h pteditor.h ptutils.h )
  set( java_c ptpicker.c pteditor.c )
endif ( HAVE_JAVA )

set( pano13_hdr 
	${java_h}
	ColourBrightness.h
	f2c.h
	fftn.h
	filter.h 
	panorama.h 
	pt_stdint.h
	PTcommon.h
	rgbe.h
	sys_ansi.h 
	sys_win.h 
	version.h 
	ZComb.h
)

set( pano13_res 
	pano13.rc
	pano13vc.def
)
          
set( pano13_src 
	${java_c}
	${win_c}
	adjust.c 
	bmp.c
	ColourBrightness.c
	correct.c
	fftn.c
	file.c
	filter.c
	fourier.c
	hdrfile.c
	jpeg.c
	jpegicc.c
	lmdif.c
	math.c
	metadata.c
	morpher.c
	multilayer.c
	optimize.c
	pan.c
	parser.c
	perspect.c
	png.c
	ppm.c
	PTcommon.c
	PTDialogs.c
	ptfeather.c
	ptstitch.c
	queryfeature.c
	remap.c
	resample.c
	rgbe.c
	seamer.c
	sys_ansi.c
	sys_common.c
	tiff.c
	Triangulate.c
	ZComb.c
)
	
add_library ( libpano13 ${pano13_hdr} ${pano13_res} ${pano13_src} )