ACLOCAL_AMFLAGS = -I m4

SUFFIXES    = .rc

RC          = @WINDRES@
RCFLAGS     = --use-temp-file --verbose -O COFF

RCCOMPILE   = $(RC) $(RCFLAGS) \
              `echo $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) |\
	      sed -e 's/-I/--include-dir /g;s/-D/--define /g;s/-m.*//g'`

STD_SRC = filter.c parser.c queryfeature.c correct.c perspect.c \
	  adjust.c  remap.c lmdif.c  file.c \
	  math.c pan.c PTDialogs.c fftn.c fourier.c resample.c \
	  optimize.c morpher.c Triangulate.c \
	  seamer.c ptpicker.c pteditor.c \
	  tiff.c jpeg.c png.c  multilayer.c 

STD_HDR = panorama.h filter.h version.h 

INTERNAL_HDR = fftn.h f2c.h 

TOOL_HDR = queryfeature.h pteditor.h ptutils.h 

SYS_HDR = sys_X11.h sys_ansi.h sys_mac.h sys_win.h 

if HAVE_X11
X11_SRC =  sys_X11.c 
else
X11_SRC =
endif

if HAVE_MINGW
WIN_SRC =  sys_win.c bmp.c pano12.rc
else
WIN_SRC =
endif

if HAVE_MAC
MAC_SRC =  PixMap.c pict.c shell_mac.c sys_mac.c ppm.c
MAC_HDR =  PixMap.h
else
MAC_SRC =
MAC_HDR =  PixMap.h
endif

if HAVE_LINUX
LIN_SRC =  sys_ansi.c ppm.c
LIN_DEFS = -D__Ansi__=1
else
LIN_SRC =
LIN_DEFS = 
endif

if HAVE_DARWIN
DAR_SRC =  sys_ansi.c ppm.c
DAR_DEFS = -I/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers -I/sw/include -D__Ansi__=1
else
DAR_SRC =
DAR_DEFS = 
endif

AM_CPPFLAGS = $(JAVA_FLAGS) $(JPEG_FLAGS) $(PNG_FLAGS) $(TIFF_FLAGS) $(ZLIB_FLAGS) $(LIN_DEFS) $(DAR_DEFS)
AM_LDFLAGS  = $(LIB_JAVA) $(LIB_JPEG) $(LIB_PNG) $(LIB_TIFF) $(LIB_ZLIB)

lib_LTLIBRARIES = libpano12.la

libpano12_la_SOURCES = $(STD_SRC) $(X11_SRC) $(WIN_SRC) $(MAC_SRC) $(LIN_SRC) $(DAR_SRC)
libpano12_la_LDFLAGS = -version-info 0:0:0

pkginclude_HEADERS = $(STD_HDR) $(TOOL_HDR)

noinst_HEADERS = $(INTERNAL_HDR) $(SYS_HDR) $(MAC_HDR)

SUBDIRS=m4 . tools 

distdir=lib$(PACKAGE)-$(VERSION)

dist-hook:
	rm -rf `find $(distdir) $(distdir)/tools -name CVS`
	rm -rf `find $(distdir) $(distdir)/tools -name .cvsignore`

EXTRA_DIST =  gpl.txt README.linux README.windows README.mac \
	      makefile.linux makefile.mac makefile.osx makefile.win32 \
	      bootstrap filter.r pano12.def \
	      seamer_.c

.rc.o:
	$(RCCOMPILE) -i $< -o $@

# I know this is wrong!!!
darwin: libpano12.la filter.r
	echo "FIXME: What should this be on osx??"
	/Developer/Tools/Rez -t APPL -o libpano12.dynlib filter.r
