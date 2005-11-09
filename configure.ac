#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.5)
AC_INIT([pano12], [2.7.0.12], BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([PTDialogs.c])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([no-define dist-bzip2 dist-zip])
AC_DISABLE_STATIC

AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_MAKE_SET

dnl
dnl  Check the operating system
dnl
case "${target_os}" in
  "")
    PANO_WIN32_RESOURCE=
    HSYS=unknown
    HCPU="${target_cpu}"
	;;
  linux*)
    PANO_WIN32_RESOURCE=
    HSYS=linux
    dnl
	dnl Check the cpu to allow lib/lib64 decisions
	dnl
	case "${target_cpu}" in
	  "")
		HCPU=unknown
		;;
	  x86_64*)
		HCPU=amd64
		;;
	  *)
		HCPU="${target_cpu}"
		;;
	esac
	;;
  *darwin*)
    PANO_WIN32_RESOURCE=
    HSYS=darwin
    HCPU="${target_cpu}"
    ;;
  *mingw32*)
    PANO_WIN32_RESOURCE=pano12.lo
    HSYS=mingw32
    HCPU="${target_cpu}"
    ;;
  *cygwin*)
    PANO_WIN32_RESOURCE=pano12.lo
    CFLAGS="-mno-cygwin $CFLAGS"
    CPPFLAGS="-mno-cygwin $CPPFLAGS"
    CXXFLAGS="-mno-cygwin $CXXFLAGS"
    LDFLAGS="-mno-cygwin $LDFLAGS"
    HSYS=cygwin
    HCPU="${target_cpu}"
    ;;
  *)
    PANO_WIN32_RESOURCE=
    HSYS="${target_os}"
    HCPU="${target_cpu}"
    ;;
esac

AM_CONDITIONAL(HAVE_DARWIN, test "${HSYS}" = "darwin")
AM_CONDITIONAL(HAVE_LINUX, test "${HSYS}" = "linux")
AM_CONDITIONAL(HAVE_MAC, test "${HSYS}" = "classic")
AM_CONDITIONAL(HAVE_MINGW, test "${HSYS}" = "mingw32")
AM_CONDITIONAL(HAVE_CYGWIN, test "${HSYS}" = "cygwin")
AM_CONDITIONAL(HAVE_X11, test "${HSYS}" = "x11")
AM_CONDITIONAL(HAVE_AMD64, test "${HCPU}" = "amd64")

AC_SUBST(PANO_WIN32_RESOURCE)

# Checks for libraries.

AX_CHECK_JAVA
if test "x$have_java" != xyes; then
    AC_MSG_WARN([java will not be used! PTEditor and PTPicker support disabled])
fi

AX_CHECK_ZLIB
if test "x$have_zlib" != xyes; then
    AC_MSG_ERROR([
        the zlib library must be installed on your system
        but configure could not find it.
    ])
fi

AX_CHECK_PNG
if test "x$have_png" != xyes; then
    AC_MSG_ERROR([
        the png library must be installed on your system
        but configure could not find it.
    ])
fi

AX_CHECK_JPEG
if test "x$have_jpeg" != xyes; then
    AC_MSG_ERROR([
        the jpeg library must be installed on your system
        but configure could not find it.
    ])
fi

AX_CHECK_TIFF
if test "x$have_tiff" != xyes; then
    AC_MSG_ERROR([
        the tiff library must be installed on your system
        but configure could not find it.
    ])
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h limits.h locale.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor memset pow setlocale sqrt strrchr])

AC_CONFIG_FILES([Makefile build/Makefile build/win32/Makefile doc/Makefile m4/Makefile tools/Makefile])

AC_SUBST(ac_aux_dir)

AC_OUTPUT

echo ""
echo "libpano is configured as follows. Please verify that this configuration"
echo "matches your expectations."
echo ""
echo "Host system type : $host"
echo "Target os : $target_os"
echo ""
echo "                  Option                        Value"
echo "-------------------------------------------------------------------------"
echo "Shared libraries  --enable-shared=$enable_shared          $libtool_build_shared_libs"
echo "Static libraries  --enable-static=$enable_static          $libtool_build_static_libs"
echo "JPEG              --with-jpeg=$with_jpeg          have_jpeg = $have_jpeg"
echo "PNG               --with-png=$with_png            have_png  = $have_png"
echo "TIFF              --with-tiff=$with_tiff          have_tiff = $have_tiff"
echo "ZLIB              --with-zlib=$with_zlib          have_zlib = $have_zlib"
echo "JAVA              --with-java=$with_java          have_java = $have_java"
echo ""
echo "Options used to compile and link:"
echo "  PREFIX         = $prefix"
echo "  VERSION        = $PACKAGE_VERSION"
echo "  CC             = $CC"
echo "  CFLAGS         = $CFLAGS"
echo "  CPPFLAGS       = $CPPFLAGS"
echo "  DEFS           = $DEFS"
echo "  LDFLAGS        = $LDFLAGS"
echo "  LIBS           = $LIBS"
echo "  CXX            = $CXX"
echo "  CXXFLAGS       = $CXXFLAGS"
echo "  JPEG_FLAGS     = $JPEG_FLAGS"
echo "  LIB_JPEG       = $LIB_JPEG"
echo "  PNG_FLAGS      = $PNG_FLAGS"
echo "  LIB_PNG        = $LIB_PNG"
echo "  TIFF_FLAGS     = $TIFF_FLAGS"
echo "  LIB_TIFF       = $LIB_TIFF"
echo "  ZLIB_FLAGS     = $ZLIB_FLAGS"
echo "  LIB_ZLIB       = $LIB_ZLIB"
echo "  JAVA_FLAGS     = $JAVA_FLAGS"

