# Over-ride default includes so that we have finer control in the makefiles
DEFAULT_INCLUDES="-I."
INCLUDES    = -I${top_srcdir}

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

AM_CPPFLAGS = $(JAVA_FLAGS) $(LIN_DEFS) $(DAR_DEFS)
AM_LDFLAGS  = -L${top_builddir}

bin_PROGRAMS = panoinfo PTOptimizer

PTOptimizer_SOURCES = PTOptimizer.c
PTOptimizer_LDADD   = -lpano12

panoinfo_SOURCES    = $(WIN_SRC) $(LIN_SRC) $(DAR_SRC)
panoinfo_LDADD      = -lpano12

EXTRA_DIST = makefile.panoinfo.win32 makefile.ptoptimizer.win32

