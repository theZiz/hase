#==stuff linked to
DYNAMIC = -lSDL_ttf -lSDL_mixer -lSDL_image -lSDL -lm
#==global Flags. Even on the gp2x with 16 kb Cache, -O3 is much better then -Os
CFLAGS = -O3 -fsingle-precision-constant -fPIC
# Testtweaks: -fgcse-lm -fgcse-sm -fsched-spec-load -fmodulo-sched -funsafe-loop-optimizations -Wunsafe-loop-optimizations -fgcse-las -fgcse-after-reload -fvariable-expansion-in-unroller -ftracer -fbranch-target-load-optimize
GENERAL_TWEAKS = -ffast-math
#==PC==
CPP = gcc -g -DX86CPU $(GENERAL_TWEAKS)
SDL = `sdl-config --cflags`

SPARROW_FOLDER = ../sparrow3d

ifdef TARGET
include $(SPARROW_FOLDER)/target-files/$(TARGET).mk
BUILD = ./build/$(TARGET)/hase
SPARROW_LIB = $(SPARROW_FOLDER)/build/$(TARGET)/sparrow3d
else
TARGET = "Default (change with make TARGET=otherTarget. See All targets with make targets)"
BUILD = .
SPARROW_LIB = $(SPARROW_FOLDER)
endif
LIB += -L$(SPARROW_LIB)
INCLUDE += -I$(SPARROW_FOLDER)
DYNAMIC += -lsparrow3d -lsparrowNet

all: hase
	@echo "=== Built for Target "$(TARGET)" ==="

targets:
	@echo "The targets are the same like for sparrow3d. :P"

client: client.c
	cp $(SPARROW_LIB)/libsparrowNet.so $(BUILD)
	$(CPP) $(CFLAGS) client.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC) -o $(BUILD)/client

hase: hase.c gravity.c player.c logic.c help.c bullet.c trace.c makeBuildDir
	cp $(SPARROW_LIB)/libsparrow3d.so $(BUILD)
	$(CPP) $(CFLAGS) hase.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC) -o $(BUILD)/hase

makeBuildDir:
	 @if [ ! -d $(BUILD:/hase=/) ]; then mkdir $(BUILD:/hase=/);fi
	 @if [ ! -d $(BUILD) ]; then mkdir $(BUILD);fi

clean:
	rm -f *.o
	rm -f hase
