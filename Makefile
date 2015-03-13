#==stuff linked to
DYNAMIC = -lSDL_ttf -lSDL_mixer -lSDL_image -lSDL -lm
#==global Flags. Even on the gp2x with 16 kb Cache, -O3 is much better then -Os
CFLAGS = -O3 -fsingle-precision-constant -fPIC
# Testtweaks: -fgcse-lm -fgcse-sm -fsched-spec-load -fmodulo-sched -funsafe-loop-optimizations -Wunsafe-loop-optimizations -fgcse-las -fgcse-after-reload -fvariable-expansion-in-unroller -ftracer -fbranch-target-load-optimize
GENERAL_TWEAKS = -ffast-math
#==PC==
FLAGS = -g -DDESKTOP $(GENERAL_TWEAKS)
SDL = `sdl-config --cflags`

SPARROW_FOLDER = ../sparrow3d

SPARROW3D_LIB = libsparrow3d.so
SPARROWNET_LIB = libsparrowNet.so
SPARROWSOUND_LIB = libsparrowSound.so

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
DYNAMIC += -lsparrow3d -lsparrowNet -lsparrowSound

CFLAGS += $(PARAMETER) $(FLAGS)

all: hase
	@echo "=== Built for Target "$(TARGET)" ==="

targets:
	@echo "The targets are the same like for sparrow3d. :P"

testclient: testclient.c client.o level.o
	cp -u $(SPARROW_LIB)/$(SPARROW3D_LIB) $(BUILD)
	cp -u $(SPARROW_LIB)/$(SPARROWNET_LIB) $(BUILD)
	cp -u $(SPARROW_LIB)/$(SPARROWSOUND_LIB) $(BUILD)
	$(CC) $(CFLAGS) testclient.c client.o level.o $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC) -o $(BUILD)/testclient$(SUFFIX)

hase: lobby.c client.o lobbyList.o lobbyGame.o level.o window.o hase.o about.o options.o makeBuildDir
	cp -u $(SPARROW_LIB)/$(SPARROW3D_LIB) $(BUILD)
	cp -u $(SPARROW_LIB)/$(SPARROWNET_LIB) $(BUILD)
	cp -u $(SPARROW_LIB)/$(SPARROWSOUND_LIB) $(BUILD)
	$(CC) $(CFLAGS) lobby.c client.o lobbyList.o lobbyGame.o window.o level.o hase.o about.o options.o $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC) -o $(BUILD)/hase$(SUFFIX)
	
client.o: client.c client.h
	$(CC) $(CFLAGS) -c client.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC)

lobbyList.o: lobbyList.c lobbyList.h lobbyGame.h
	$(CC) $(CFLAGS) -c lobbyList.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC)

lobbyGame.o: lobbyGame.c lobbyGame.h lobbyList.h
	$(CC) $(CFLAGS) -c lobbyGame.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC)

window.o: window.c window.h lobbyList.h
	$(CC) $(CFLAGS) -c window.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC)

about.o: about.c about.h lobbyList.h
	$(CC) $(CFLAGS) -c about.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC)

hase.o: hase.c hase.h gravity.c player.c logic.c help.c bullet.c trace.c level.h items.c
	$(CC) $(CFLAGS) -c hase.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC)

level.o: level.c level.h
	$(CC) $(CFLAGS) -c level.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC)

options.o: options.c options.h
	$(CC) $(CFLAGS) -c options.c $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC)

makeBuildDir:
	 @if [ ! -d $(BUILD:/hase=/) ]; then mkdir $(BUILD:/hase=/);fi
	 @if [ ! -d $(BUILD) ]; then mkdir $(BUILD);fi

clean:
	rm -f *.o
	rm -f $(BUILD)/lobby
	rm -f $(BUILD)/hase$(SUFFIX)
	rm -f $(BUILD)/testclient$(SUFFIX)

oclean:
	rm -f *.o
