#ifndef ABOUT_H
#define ABOUT_H

#include <sparrow3d.h>
#include "client.h"
#include "lobbyList.h"

#define VERSION "1.6.8"

void start_about(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ));
void start_help(spFontPointer font, void ( *resize )( Uint16 w, Uint16 h ));

#endif
