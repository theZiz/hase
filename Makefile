DYNAMIC = -lSDL_ttf -lSDL_mixer -lSDL_image -lSDL -lm
CFLAGS = -O3 -fsingle-precision-constant -Wimplicit-function-declaration -Wunused
GENERAL_TWEAKS = -ffast-math
#==PC defaults==
FLAGS = -g -DDESKTOP $(GENERAL_TWEAKS)
SDL = `sdl-config --cflags`

SPARROW_FOLDER = ../sparrow3d
SPARROW3D_STATIC_LIB = libsparrow3d.a
SPARROWNET_STATIC_LIB = libsparrowNet.a
SPARROWSOUND_STATIC_LIB = libsparrowSound.a

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

DYNAMIC += -lSDL_net
STATIC += $(SPARROW_LIB)/$(SPARROW3D_STATIC_LIB) $(SPARROW_LIB)/$(SPARROWSOUND_STATIC_LIB) $(SPARROW_LIB)/$(SPARROWNET_STATIC_LIB)

ifneq ($(TARGET),win32)
DYNAMIC += -lz
endif

CFLAGS += $(PARAMETER) $(FLAGS)  -DBUILDING_DLL

all: hase
	@echo "=== Built for Target "$(TARGET)" ==="

targets:
	@echo "The targets are the same like for sparrow3d. :P"

hase: lobby.c client.o lobbyList.o lobbyGame.o level.o window.o hase.o about.o options.o mapping.o makeBuildDir
	$(CC) $(CFLAGS) $(LINK_FLAGS) $< client.o lobbyList.o lobbyGame.o window.o level.o hase.o about.o options.o mapping.o $(SDL) $(INCLUDE) $(LIB) $(STATIC) $(DYNAMIC) -o $(BUILD)/$@$(SUFFIX)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< $(SDL) $(INCLUDE)

makeBuildDir:
	 @if [ ! -d $(BUILD:/hase=/) ]; then mkdir $(BUILD:/hase=/);fi
	 @if [ ! -d $(BUILD) ]; then mkdir $(BUILD);fi

clean:
	rm -f *.o
	rm -f $(BUILD)/hase$(SUFFIX)
	rm -f $(BUILD)/testclient$(SUFFIX)

oclean:
	rm -f *.o
