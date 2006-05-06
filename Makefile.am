ACLOCAL_AMFLAGS = -I m4

STD_SRC = filter.c parser.c queryfeature.c correct.c perspect.c \
	  adjust.c  remap.c lmdif.c  file.c \
	  math.c pan.c PTDialogs.c fftn.c fourier.c resample.c \
	  optimize.c morpher.c Triangulate.c \
	  seamer.c hdrfile.c rgbe.c rgbe.h \
	  tiff.c ZComb.c jpeg.c png.c  multilayer.c sys_common.c PTcommon.c ColourBrightness.c

if HasJava
JAVA_SRC = ptpicker.c pteditor.c
else
JAVA_SRC = javastub.c
endif

STD_HDR = panorama.h filter.h version.h panotypes.h

INTERNAL_HDR = fftn.h f2c.h ZComb.h adjust.h ColourBrightness.h PTcommon.h

TOOL_HDR = queryfeature.h pteditor.h ptutils.h 

SYS_HDR = sys_X11.h sys_ansi.h sys_mac.h sys_win.h 

if HAVE_X11
X11_SRC =  sys_X11.c 
else
X11_SRC =
endif

if HAVE_CYGWIN
WIN_SRC =  sys_win.c bmp.c
WIN_DEFS = -D__Win__=1
WIN_LDFS = -mwindows -avoid-version -no-undefined -export-symbols $(srcdir)/libpano12.def 

$(PANO_WIN32_RESOURCE): pano12.rc
	        $(top_srcdir)/build/win32/lt-compile-resource pano12.rc $@

install-exec-hook:
	mv $(DESTDIR)$(bindir)/cygpano12.dll $(DESTDIR)$(bindir)/pano12.dll
endif

if HAVE_MINGW
WIN_SRC =  sys_win.c bmp.c
WIN_DEFS = -D__Win__=1
WIN_LDFS = -mwindows -avoid-version -no-undefined -export-symbols $(srcdir)/libpano12.def 

$(PANO_WIN32_RESOURCE): pano12.rc
	        $(top_srcdir)/build/win32/lt-compile-resource pano12.rc $@

install-exec-hook:
	mv $(DESTDIR)$(bindir)/libpano12.dll $(DESTDIR)$(bindir)/pano12.dll
endif

if HAVE_MAC
MAC_SRC =  PixMap.c pict.c shell_mac.c sys_mac.c ppm.c
MAC_HDR =  PixMap.h

install-exec-hook:
	@true

endif

if HAVE_LINUX
LIN_SRC =  sys_ansi.c ppm.c
LIN_DEFS = -D__Ansi__=1
LIN_LDFS = -version-info 0:0:0

install-exec-hook:
	@true

endif

if HAVE_DARWIN
DAR_SRC =  sys_ansi.c ppm.c
DAR_DEFS = -D__Ansi__=1
DAR_LDFS = -version-info 0:0:0

install-exec-hook:
	@true

endif

AM_CPPFLAGS = $(JAVA_FLAGS) $(JPEG_FLAGS) $(PNG_FLAGS) $(TIFF_FLAGS) $(ZLIB_FLAGS) $(WIN_DEFS) $(LIN_DEFS) $(DAR_DEFS) $(ENDIAN_FLAG)

lib_LTLIBRARIES = libpano12.la

libpano12_la_SOURCES = $(STD_SRC) $(JAVA_SRC) $(X11_SRC) $(WIN_SRC) $(MAC_SRC) $(LIN_SRC) $(DAR_SRC)
libpano12_la_LIBADD = $(LIB_JPEG) $(LIB_PNG) $(LIB_TIFF) $(LIB_ZLIB) @PANO_WIN32_RESOURCE@
libpano12_la_LDFLAGS = $(LIN_LDFS) $(WIN_LDFS) $(DAR_LDFS)
libpano12_la_DEPENDENCIES = @PANO_WIN32_RESOURCE@

pkginclude_HEADERS = $(STD_HDR) $(TOOL_HDR)

noinst_HEADERS = $(INTERNAL_HDR) $(SYS_HDR) $(MAC_HDR)

SUBDIRS=m4 . tools doc build

distdir=lib$(PACKAGE)-$(VERSION)

dist-hook:
	rm -rf `find $(distdir) $(distdir)/tools -name CVS`
	rm -rf `find $(distdir) $(distdir)/tools -name .cvsignore`

EXTRA_DIST =  gpl.txt README.linux README.windows README.mac \
	      makefile.linux makefile.mac makefile.osx makefile.win32 \
	      bootstrap filter.r pano12.def libpano12.def pano12.rc \
	      seamer_.c sys_X11.c sys_win.c bmp.c PixMap.c pict.c shell_mac.c \
	      sys_mac.c ppm.c PixMap.h sys_ansi.c ppm.c ZComb.h adjust.h \
	      pano12vc.def pano12vcd.def libpano.vcproj PanoTools.pbproj.tgz

MAINTAINERCLEANFILES = \
	COPYING				\
	INSTALL				\
	aclocal.m4			\
	config.h.in			\
	config.h.in~		\
	Makefile.in			\
	configure

maintainer-clean-local:
	rm -rf config

# I know this is wrong!!!
darwin: libpano12.la filter.r
	echo "FIXME: What should this be on osx??"
	/Developer/Tools/Rez -t APPL -o libpano12.dynlib filter.r
