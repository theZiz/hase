#ifndef LEVEL_H
#define LEVEL_H

#include <sparrow3d.h>
#include <string.h>

#define TEXTURE_SIZE 256
#define TEXTURE_MASK 255
#define TEXTURE_COUNT 9
#define LEVEL_BORDER 256

#define LEVEL_WIDTH 1536
#define LEVEL_HEIGHT 1536

Uint16 get_border_color();
Uint16 get_level_color();
void add_to_string(char* buffer,char* add);
char* ltostr(unsigned int l,char* buffer,int base);
char* create_level_string(char* buffer,int width,int height,int circles,int triangles,int quads);
SDL_Surface* create_level(char* level_string,int alt_width,int alt_height,int color);
void texturize_level(SDL_Surface* level,char* level_string);

#endif
