LIBJPEG=$(wildcard ../*jpeg*)
LIBTIFF=$(wildcard ../*tiff*)
LIBZ=$(wildcard ../z*)
LIBPNG=$(wildcard ../*png*)
# note: if you have installed the JAVA SDK somewhere else you need to change 
# JAVASDK to point to it.
JAVASDK=$(wildcard ../*sdk*)

LIBS=$(LIBJPEG) $(LIBTIFF) $(LIBZ) $(LIBPNG)

.PHONY: $(LIBS)

all: $(LIBS) pano12.dll

ifeq "$(LIBJPEG)"  ""
$(warning "No jpeg library found - assuming it is already installed")
else
$(LIBJPEG):
	@echo "configuring $@" && cd $@ && ./configure
	@echo "building $@" && cd $@ && make
	@echo "installing $@" && cd $@ && make install-lib
endif

ifeq "$(LIBTIFF)"  ""
$(warning "No tiff library found - assuming it is already installed")
else
$(LIBTIFF):
	@echo "configuring $@" && cd $@ && yes yes | ./configure
	@echo "building $@" && cd $@ && make
	@echo "installing $@" && cd $@ && cp libtiff/*.h /usr/local/include && cp libtiff/*.a /usr/local/lib
endif

ifeq "$(LIBZ)"  ""
$(warning "No zlib library found - assuming it is already installed")
else
$(LIBZ):
	@echo "configuring $@" && cd $@ && ./configure
	@echo "building $@" && cd $@ && make
	@echo "installing $@" && cd $@ && make install
endif

ifeq "$(LIBPNG)"  ""
$(warning "No png library found - assuming it is already installed")
else
$(LIBPNG):
	@echo "configuring $@" && cd $@ && cp scripts/makefile.gcc makefile
	@echo "building $@" && cd $@ && make ZLIBINC=/usr/local/include ZLIBLIB=/usr/local/lib
	@echo "installing $@" && cd $@ && cp *.h /usr/local/include && cp *.a /usr/local/lib
endif

ifeq "$(JAVASDK)" ""
$(warning "No java sdk found - assuming it is already in your include and library paths")
endif

sources = panorama.h filter.h fftn.h f2c.h pteditor.h \
          ptutils.h sys_win.h version.h \
          filter.c parser.c queryfeature.c sys_win.c  correct.c perspect.c \
          adjust.c  remap.c lmdif.c  file.c \
          math.c pan.c PTDialogs.c fftn.c fourier.c resample.c \
          optimize.c morpher.c Triangulate.c \
          seamer.c ptpicker.c pteditor.c seamer_.c \
          tiff.c bmp.c jpeg.c png.c  multilayer.c \
          Makefile pano12.rc pano12.def sys_ansi.c ppm.c

objects = filter.o parser.o queryfeature.o correct.o perspect.o \
          adjust.o  remap.o lmdif.o  file.o math.o pan.o \
          PTDialogs.o fftn.o fourier.o resample.o optimize.o \
          morpher.o Triangulate.o seamer.o ptpicker.o pteditor.o \
          tiff.o jpeg.o png.o multilayer.o

winobj =  sys_win.o bmp.o pano12rc.o

ansobj =  sys_ansi.o ppm.o

libDirs =  -L/usr/local/lib
incDirs =  -I/usr/local/include -I$(JAVASDK)/include -I$(JAVASDK)/include/win32
CC = gcc -O2 -mms-bitfields $(incDirs) 

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
	-rm -f *.o pano12.dll libpano12.a panosrc.zip

panosrc.zip : $(sources)
	zip -R panosrc.zip $(sources)
