#ifndef MAPPING_H
#define MAPPING_H

#include <sparrow3d.h>

void mapping_window(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ),int mapSet);

unsigned char sdlkey_to_char(SDLKey key);

#endif
