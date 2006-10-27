# Over-ride default includes so that we have finer control in the makefiles
DEFAULT_INCLUDES="-I."
INCLUDES    = -I${top_srcdir}

PANOLIB=pano13

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

bin_PROGRAMS = panoinfo PToptimizer PTmender PTblender PTtiff2psd PTuncrop PTtiffdump PTroller PTremap

PTblender_SOURCES = PTblender.c 
PTblender_LDADD = -l${PANOLIB} -ltiff

PTremap_SOURCES = PTremap.c 
PTremap_LDADD = -l${PANOLIB} -ltiff


PTroller_SOURCES = PTroller.c 
PTroller_LDADD = -l${PANOLIB} -ltiff

PTmender_SOURCES = PTmender.c PTmender.h 
PTmender_LDADD   = -l${PANOLIB} -ljpeg -ltiff

PTtiff2psd_SOURCES = PTtiff2psd.c 
PTtiff2psd_LDADD   = -l${PANOLIB} -ljpeg -ltiff

PTuncrop_SOURCES = PTuncrop.c
PTuncrop_LDADD   = -l${PANOLIB} -ljpeg -ltiff

PTtiffdump_SOURCES = PTtiffdump.c
PTtiffdump_LDADD   = -l${PANOLIB} -ljpeg -ltiff


PToptimizer_SOURCES = PToptimizer.c
PToptimizer_LDADD   = -l${PANOLIB}

panoinfo_SOURCES    = $(WIN_SRC) $(LIN_SRC) $(DAR_SRC)
panoinfo_LDADD      = -l${PANOLIB}

EXTRA_DIST = makefile.panoinfo.win32 makefile.ptoptimizer.win32 \
             README.PTmender PTStitcher.cpp PTStitcher.vcproj \
             PTpano12.vcproj PTpano12_readme.txt PTpano12.dsp \
             ptpano12.cpp ptpano12.h ptpano12.def

MAINTAINERCLEANFILES = \
	Makefile.in
