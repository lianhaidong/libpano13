# Makefile for pano12.dll
# This should compile on Windows systems using mingw 2.0 (www.mingw.org).
# You'll need to adjust the path to libraries, include files, etc. if your 
# system isn't configured the same as mine.  (See below).
# Modified by Max Lyons (July 2003)

sources = panorama.h filter.h fftn.h f2c.h pteditor.h  \
          ptutils.h sys_win.h version.h \
          filter.c parser.c sys_win.c  correct.c perspect.c \
          adjust.c  remap.c lmdif.c  file.c \
	  math.c pan.c PTDialogs.c fftn.c fourier.c resample.c \
          optimize.c morpher.c Triangulate.c \
	  seamer.c ptpicker.c pteditor.c seamer_.c \
          tiff.c bmp.c jpeg.c png.c  multilayer.c \
          Makefile pano12.rc pano12.def sys_ansi.c ppm.c

objects = filter.o parser.o  correct.o perspect.o \
          adjust.o  remap.o lmdif.o  file.o math.o pan.o \
          PTDialogs.o fftn.o fourier.o resample.o optimize.o \
          morpher.o Triangulate.o seamer.o ptpicker.o pteditor.o \
          tiff.o jpeg.o png.o multilayer.o 

winobj =  sys_win.o bmp.o pano12rc.o

ansobj =  sys_ansi.o ppm.o

libDirs =  -LC:/MinGW/lib/ -L./Libs
incDirs =  -I./Libs

CC = gcc -O2 -mms-bitfields -I c:/mingw/include -I c:/jdk1.3.1/include -I c:/jdk1.3.1/include/win32 -I ./Libs

pano12rc.o  : pano12.rc
	windres  -i pano12.rc -o pano12rc.o

pano12.dll  : $(objects) $(winobj)
	dllwrap --enable-stdcall-fixup -o pano12.dll --def pano12.def $(objects) $(winobj) $(libDirs) $(incDirs) -lcomdlg32 -ljpeg -ltiff -lpng -lz -mwindows
	strip pano12.dll
	dlltool --dllname pano12.dll --def pano12.def --output-lib libpano12.a

install  : pano12.dll
	mv pano12.dll G:/WINDOWS/SYSTEM32/pano12.dll	
	
.PHONY : clean
clean :
	-rm *.o pano12.dll libpano12.a panosrc.zip

panosrc.zip : $(sources)
	zip -R panosrc.zip $(sources)
