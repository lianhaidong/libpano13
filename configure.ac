#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([pano12], [2.7.0.7], BUG-REPORT-ADDRESS)
AC_CONFIG_SRCDIR([PTDialogs.c])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADER([config.h])
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([no-define dist-bzip2 dist-zip])

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
    HSYS=unknown
    ;;
  linux*)
    HSYS=linux
    ;;
  darwin*)
    HSYS=darwin
    ;;
  *mingw32*)
    AC_ARG_VAR(WINDRES, [Windows Resource compiler tool path])
    AC_PATH_TOOL(WINDRES,windres,
                     [AC_MSG_ERROR(Could not find a windres tool in your PATH. Will not be able to compile hugin or nona_gui:)]
                     )
    AC_SUBST(WINDRES)

    HSYS=mingw32

    ;;
  *)
    HSYS="${target_os}"
    ;;
esac

AM_CONDITIONAL(HAVE_DARWIN, test "${HSYS}" = "darwin")
AM_CONDITIONAL(HAVE_LINUX, test "${HSYS}" = "linux")
AM_CONDITIONAL(HAVE_MAC, test "${HSYS}" = "classic")
AM_CONDITIONAL(HAVE_MINGW, test "${HSYS}" = "mingw32")
AM_CONDITIONAL(HAVE_X11, test "${HSYS}" = "x11")

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lcomdlg32':
# AC_CHECK_LIB([comdlg32], [main])

AX_CHECK_JAVA
if test "x$have_java" != xyes; then
    AC_MSG_WARN([
        java must be installed on your system
        but configure could not find it.
    ])
fi

AX_CHECK_ZLIB
if test "x$have_zlib" != xyes; then
    AC_MSG_WARN([
        the zlib library must be installed on your system
        but configure could not find it.
    ])
fi

AX_CHECK_PNG
if test "x$have_png" != xyes; then
    AC_MSG_WARN([
        the png library must be installed on your system
        but configure could not find it.
    ])
fi

AX_CHECK_JPEG
if test "x$have_jpeg" != xyes; then
    AC_MSG_WARN([
        the jpeg library must be installed on your system
        but configure could not find it.
    ])
fi

AX_CHECK_TIFF
if test "x$have_tiff" != xyes; then
    AC_MSG_WARN([
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

AC_CONFIG_FILES([Makefile
		 tools/Makefile
		 m4/Makefile])

AC_SUBST(ac_aux_dir)

AC_OUTPUT

echo ""
echo "libpano is configured as follows. Please verify that this configuration"
echo "matches your expectations."
echo ""
echo "Host system type : $host"
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

