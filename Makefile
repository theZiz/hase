SRC = client.c lobbyList.c lobbyGame.c level.c window.c hase.c about.c options.c mapping.c
OBJ = $(SRC:.c=.o)
DEP = $(SRC:.c=.d)

DYNAMIC = -lSDL_ttf -lSDL_mixer -lSDL_image -lSDL -lm
CFLAGS = -O3 -fsingle-precision-constant -Wimplicit-function-declaration -Wunused
GENERAL_TWEAKS = -ffast-math
#==PC defaults==
FLAGS = -DDESKTOP $(GENERAL_TWEAKS) -mtune=native -march=native

ifdef NO_DEBUG
	FLAGS += -O3
else
	FLAGS += -g
endif

SDL = `sdl-config --cflags`

SPARROW_FOLDER = ../sparrow3d
SPARROW3D_STATIC_LIB = libsparrow3d.a
SPARROWNET_STATIC_LIB = libsparrowNet.a
SPARROWSOUND_STATIC_LIB = libsparrowSound.a

DYNAMIC += -lSDL_net

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

HASE_STATIC = $(SPARROW_LIB)/$(SPARROW3D_STATIC_LIB) $(SPARROW_LIB)/$(SPARROWSOUND_STATIC_LIB) $(SPARROW_LIB)/$(SPARROWNET_STATIC_LIB) $(STATIC)

ifneq ($(TARGET),win32)
DYNAMIC += -lz
endif

CFLAGS += $(PARAMETER) $(FLAGS)  -DBUILDING_DLL

all: hase
	@echo "=== Built for Target "$(TARGET)" ==="

targets:
	@echo "The targets are the same like for sparrow3d. :P"

hase: lobby.c $(OBJ) makeBuildDir
	$(CC) $(CFLAGS) $(LINK_FLAGS) $< $(OBJ) $(SDL) $(INCLUDE) $(LIB) $(HASE_STATIC) $(DYNAMIC) -o $(BUILD)/$@$(SUFFIX)

%.d: %.c
	$(CC) -MM $(CFLAGS) $(INCLUDE) $(SDL) $< -MF $@

%.o:
	$(CC) $(CFLAGS) -c $< $(SDL) $(INCLUDE)

makeBuildDir:
	 @if [ ! -d $(BUILD:/hase=/) ]; then mkdir $(BUILD:/hase=/);fi
	 @if [ ! -d $(BUILD) ]; then mkdir $(BUILD);fi

clean:
	rm -f *.o
	rm -f *.d
	rm -f $(BUILD)/hase$(SUFFIX)
	rm -f $(BUILD)/testclient$(SUFFIX)

oclean:
	rm -f *.o
	rm -f *.d

-include $(DEP)
