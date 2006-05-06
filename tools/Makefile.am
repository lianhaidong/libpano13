# Over-ride default includes so that we have finer control in the makefiles
DEFAULT_INCLUDES="-I."
INCLUDES    = -I${top_srcdir}

COPTIONS="-Wall"

if HAVE_CYGWIN
WIN_SRC =  panoinfo.c
else
WIN_SRC =
endif

if HAVE_MINGW
WIN_SRC =  panoinfo.c
else
WIN_SRC =
endif

if HAVE_LINUX
LIN_SRC =  panoinfo_unix.c
LIN_DEFS = -D__Ansi__=1
else
LIN_SRC =
LIN_DEFS = 
endif

if HAVE_DARWIN
DAR_SRC =  panoinfo_unix.c
DAR_DEFS = -D__Ansi__=1
else
DAR_SRC =
DAR_DEFS = 
endif

AM_CPPFLAGS = $(JAVA_FLAGS) $(LIN_DEFS) $(DAR_DEFS) $(TIFF_FLAGS) $(JPEG_FLAGS)
AM_LDFLAGS  = -L${top_builddir}

bin_PROGRAMS = panoinfo PTOptimizer PTmender PTblender PTtiff2psd

PTblender_SOURCES = PTblender.c 
PTblender_LDADD = -lpano12 -ltiff

PTmender_SOURCES = PTmender.c PTmender.h 
PTmender_LDADD   = -lpano12 -ljpeg -ltiff

PTtiff2psd_SOURCES = PTtiff2psd.c 
PTtiff2psd_LDADD   = -lpano12 -ljpeg -ltiff


PTOptimizer_SOURCES = PTOptimizer.c
PTOptimizer_LDADD   = -lpano12

panoinfo_SOURCES    = $(WIN_SRC) $(LIN_SRC) $(DAR_SRC)
panoinfo_LDADD      = -lpano12

EXTRA_DIST = makefile.panoinfo.win32 makefile.ptoptimizer.win32

MAINTAINERCLEANFILES = \
	Makefile.in
