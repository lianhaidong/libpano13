# Makefile for pano12.dll

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

libs = ../tiff/libtiff.a ../jpeg/libjpeg.a \
       ../png/libpng.a ../zlib/libz.a c:/mingw/lib/libcomdlg32.a

CC = gcc



CFLAGS = -O2 -fnative-struct \
         -I c:/mingw/include/w32api \
	 -I c:/mingw/usr/include \
	 -I c:/jdk1.3/include \
	 -I c:/jdk1.3/include/win32


pano12rc.o  : pano12.rc
	windres  -i pano12.rc -o pano12rc.o



pano12.dll  : $(objects) $(winobj)
	dllwrap --enable-stdcall-fixup -o pano12.dll --def pano12.def $(objects) $(winobj) -lcomdlg32 -L c:/mingw/usr/lib -ljpeg -ltiff -lpng -lz -mwindows 
	strip pano12.dll
	dlltool --dllname pano12.dll --def pano12.def --output-lib libpano12.a

install  : pano12.dll
	mv pano12.dll c:/WINDOWS/SYSTEM/
	mv libpano12.a c:/mingw/usr/lib


	
.PHONY : clean
clean :
	-rm *.o pano12.dll libpano12.a panosrc.zip


panosrc.zip : $(sources)
	zip -r panosrc.zip $(sources)





