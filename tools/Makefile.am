#SUBDIRS=tools

SUFFIXES    = .rc

RC          = @WINDRES@
RCFLAGS     = --use-temp-file --verbose -O COFF

RCCOMPILE   = $(RC) $(RCFLAGS) \
              `echo $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) |\
	      sed -e 's/-I/--include-dir /g;s/-D/--define /g;s/-m.*//g'`

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
DAR_DEFS = "-I/System/Library/Frameworks/JavaVM.framework/Versions/Current/Headers -I/sw/include -D__Ansi__=1"
DAR_LDFLAGS = "-L/sw/lib -ltiff -ljpeg"
else
DAR_SRC =
DAR_DEFS = 
DAR_LDFLAGS =
endif

AM_CPPFLAGS = $(JAVA_FLAGS) $(LIN_DEFS) $(DAR_DEFS)
AM_LDFLAGS  = -L.. $(DAR_LDFLAGS)

bin_PROGRAMS = panoinfo PTOptimizer

PTOptimizer_SOURCES = PTOptimizer.c
PTOptimizer_LDADD   = -lpano12

panoinfo_SOURCES    = $(WIN_SRC) $(LIN_SRC) $(DAR_SRC)
panoinfo_LDADD      = -lpano12

EXTRA_DIST = makefile.panoinfo.win32 makefile.ptoptimizer.win32

.rc.o:
	$(RCCOMPILE) -i $< -o $@
