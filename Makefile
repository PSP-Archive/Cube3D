TARGET = Cube3D

OBJS = cube.o logo.o callbacks.o vram.o intraFont.o libccc.o
LIBS = -lpng -lz -lpspgum -lpspgu -lpsprtc -lm

CFLAGS = -O3 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

EXTRA_TARGETS    = EBOOT.PBP
PSP_EBOOT_TITLE  = Cube3D by Geecko
PSP_FW_VERSION   = 500
PSP_EBOOT_ICON   = icon0.png
BUILD_PRX        = 0
PSP_LARGE_MEMORY = 0

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak 

logo.o: logo.raw
	bin2o -i logo.raw logo.o logo